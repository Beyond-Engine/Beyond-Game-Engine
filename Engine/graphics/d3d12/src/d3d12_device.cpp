#include "d3d12_device.hpp"

namespace beyond::graphics::d3d12 {

void panic_if_failed(HRESULT hr)
{
  if (FAILED(hr)) {
    beyond::panic("Dx12 failed!");
  }
}

D3D12GPUDevice::D3D12GPUDevice(Window* window) : window_{window}
{
  std::puts("Pick Direct12 Graphics backend");

  // Create Factory
  {
    UINT dxgi_factory_flags = 0;
    //#if defined(_DEBUG)
    // Create a Debug Controller to track errors
    {
      ID3D12Debug* dc = nullptr;
      panic_if_failed(D3D12GetDebugInterface(IID_PPV_ARGS(&dc)));
      panic_if_failed(dc->QueryInterface(IID_PPV_ARGS(&debug_controller_)));
      debug_controller_->EnableDebugLayer();
      debug_controller_->SetEnableGPUBasedValidation(1);

      dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;

      dc->Release();
      dc = nullptr;
    }
    //#endif

    panic_if_failed(
        CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&factory_)));
  }

  // Create Adapter
  for (UINT adapterIndex = 0;
       DXGI_ERROR_NOT_FOUND != factory_->EnumAdapters1(adapterIndex, &adapter_);
       ++adapterIndex) {
    DXGI_ADAPTER_DESC1 desc;
    adapter_->GetDesc1(&desc);

    if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0u) {
      // Don't select the Basic Render Driver adapter.
      continue;
    }

    // Check to see if the adapter supports Direct3D 12, but don't create
    // the actual device yet.
    if (SUCCEEDED(D3D12CreateDevice(adapter_, D3D_FEATURE_LEVEL_12_0,
                                    _uuidof(ID3D12Device), nullptr))) {
      break;
    }

    // We won't use this adapter, so release it
    adapter_->Release();
  }

  // Create Device
  panic_if_failed(D3D12CreateDevice(adapter_, D3D_FEATURE_LEVEL_12_0,
                                    IID_PPV_ARGS(&device_)));

  // Get debug device
  //#if defined(_DEBUG)
  panic_if_failed(device_->QueryInterface(&debug_device_));
  //#endif

  // Create comment queues
  const D3D12_COMMAND_QUEUE_DESC queue_desc{
      .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
      .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
  };
  panic_if_failed(
      device_->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue_)));

  // Create Command Allocator
  panic_if_failed(device_->CreateCommandAllocator(
      D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator_)));

  // Create fence
  panic_if_failed(
      device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)));

  init_swapchain();
  init_root_signature();
  init_vertex_buffer();
  init_index_buffer();
}

D3D12GPUDevice::~D3D12GPUDevice() noexcept
{
  if (vertex_buffer_) {
    vertex_buffer_->Release();
  }

  if (root_signature_) {
    root_signature_->Release();
  }

  if (swapchain_) {
    swapchain_->Release();
  }

  if (fence_) {
    fence_->Release();
  }

  if (command_allocator_) {
    command_allocator_->Release();
  }

  if (command_queue_) {
    command_queue_->Release();
  }

  if (debug_device_) {
    debug_device_->Release();
  }

  if (device_) {
    device_->Release();
  }

  if (adapter_) {
    adapter_->Release();
  }

  if (factory_) {
    factory_->Release();
  }

  if (debug_controller_) {
    debug_controller_->Release();
  }
}

void D3D12GPUDevice::init_swapchain()
{
  if (!window_) {
    beyond::panic(
        "Cannot create a swapchain if no window is attached to the GPU Device");
  }
  const Resolution resolution = window_->get_resolution();
  const auto width = static_cast<unsigned>(resolution.width);
  const auto height = static_cast<unsigned>(resolution.height);

  viewport_ = {
      .TopLeftX = 0.0f,
      .TopLeftY = 0.0f,
      .Width = static_cast<float>(width),
      .Height = static_cast<float>(height),
      .MinDepth = .1f,
      .MaxDepth = 1000.f,
  };
  surface_size_ = {
      .left = 0,
      .top = 0,
      .right = static_cast<LONG>(width),
      .bottom = static_cast<LONG>(height),
  };

  if (swapchain_ != nullptr) {
    // Create Render Target Attachments from swapchain
    swapchain_->ResizeBuffers(backbuffer_count, width, height,
                              DXGI_FORMAT_R8G8B8A8_UNORM, 0);
  } else {
    // Create swapchain
    const DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {
        .Width = width,
        .Height = height,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleDesc =
            {
                .Count = 1,
            },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = backbuffer_count,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
    };

    IDXGISwapChain1* new_swapchain = nullptr;
    auto* win32_window = window_->get_win32_window();
    const auto res = factory_->CreateSwapChainForHwnd(
        command_queue_, win32_window, &swapchain_desc, nullptr, nullptr,
        &new_swapchain);
    if (SUCCEEDED(res)) {
      new_swapchain->QueryInterface(&swapchain_);
    } else {
      beyond::panic("Cannot create a swapchain");
    }
  }

  frame_index_ = swapchain_->GetCurrentBackBufferIndex();

  // Describe and create a render target view (RTV) descriptor heap.
  static constexpr D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {
      .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
      .NumDescriptors = backbuffer_count,
      .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
  };

  panic_if_failed(device_->CreateDescriptorHeap(
      &rtv_heap_desc, IID_PPV_ARGS(&render_target_view_heap_)));

  const unsigned rtv_descriptor_size =
      device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  // Create frame resources

  D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle(
      render_target_view_heap_->GetCPUDescriptorHandleForHeapStart());

  // Create a RTV for each frame.
  for (UINT n = 0; n < backbuffer_count; n++) {
    panic_if_failed(
        swapchain_->GetBuffer(n, IID_PPV_ARGS(&render_targets_[n])));
    device_->CreateRenderTargetView(render_targets_[n], nullptr, rtv_handle);
    rtv_handle.ptr += (1 * rtv_descriptor_size);
  }
}

void D3D12GPUDevice::init_root_signature()
{

  // This is the highest version the sample supports. If CheckFeatureSupport
  // succeeds, the HighestVersion returned will not be greater than this.
  D3D12_FEATURE_DATA_ROOT_SIGNATURE feature_data = {
      .HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1};

  if (FAILED(device_->CheckFeatureSupport(
          D3D12_FEATURE_ROOT_SIGNATURE, &feature_data, sizeof(feature_data)))) {
    feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
  }

  // Individual uniforms
  static constexpr D3D12_DESCRIPTOR_RANGE1 ranges[1] = {{
      .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
      .NumDescriptors = 1,
      .BaseShaderRegister = 0,
      .RegisterSpace = 0,
      .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
      .OffsetInDescriptorsFromTableStart = 0,
  }};

  // Groups of Uniforms
  D3D12_ROOT_PARAMETER1 root_parameters[1] = {};
  root_parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  root_parameters[0].DescriptorTable = {
      .NumDescriptorRanges = 1,
      .pDescriptorRanges = static_cast<const D3D12_DESCRIPTOR_RANGE1*>(ranges),
  };
  root_parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

  //  Overall layout
  const D3D12_VERSIONED_ROOT_SIGNATURE_DESC root_signature_desc = {
      .Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
      .Desc_1_1 =
          {
              .NumParameters = 1,
              .pParameters =
                  static_cast<const D3D12_ROOT_PARAMETER1*>(root_parameters),
              .NumStaticSamplers = 0,
              .pStaticSamplers = nullptr,
              .Flags =
                  D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
          }

  };

  ID3DBlob* signature = nullptr;
  ID3DBlob* error = nullptr;
  // Create the root signature
  HRESULT result = D3D12SerializeVersionedRootSignature(&root_signature_desc,
                                                        &signature, &error);
  if (SUCCEEDED(result)) {
    result = device_->CreateRootSignature(0, signature->GetBufferPointer(),
                                          signature->GetBufferSize(),
                                          IID_PPV_ARGS(&root_signature_));

    if (SUCCEEDED(result)) {
      root_signature_->SetName(L"Hello Triangle Root Signature");
    }
  }

  if (FAILED(result)) {
    const auto* err_str = static_cast<const char*>(error->GetBufferPointer());
    std::fputs(err_str, stderr);
    error->Release();
  }

  if (signature) {
    signature->AddRef();
  }
}

struct Vertex {
  float position[3];
  float color[3];
};

constexpr Vertex vertex_buffer_data[3] = {
    {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

void D3D12GPUDevice::init_vertex_buffer()
{
  constexpr UINT vertex_buffer_size = sizeof(vertex_buffer_data);

  constexpr D3D12_HEAP_PROPERTIES heap_props{
      .Type = D3D12_HEAP_TYPE_UPLOAD,
      .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
      .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
      .CreationNodeMask = 1,
      .VisibleNodeMask = 1,
  };

  constexpr D3D12_RESOURCE_DESC vertex_buffer_resource_desc = {
      .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
      .Alignment = 0,
      .Width = vertex_buffer_size,
      .Height = 1,
      .DepthOrArraySize = 1,
      .MipLevels = 1,
      .Format = DXGI_FORMAT_UNKNOWN,
      .SampleDesc =
          {
              .Count = 1,
              .Quality = 0,
          },
      .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
      .Flags = D3D12_RESOURCE_FLAG_NONE,
  };

  panic_if_failed(device_->CreateCommittedResource(
      &heap_props, D3D12_HEAP_FLAG_NONE, &vertex_buffer_resource_desc,
      D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
      IID_PPV_ARGS(&vertex_buffer_)));

  // Copy the triangle data to the vertex buffer.
  UINT8* p_vertex_data_begin = nullptr;

  // We do not intend to read from this resource on the CPU.
  constexpr D3D12_RANGE read_range = {
      .Begin = 0,
      .End = 0,
  };

  panic_if_failed(vertex_buffer_->Map(
      0, &read_range, reinterpret_cast<void**>(&p_vertex_data_begin)));
  memcpy(p_vertex_data_begin, static_cast<const Vertex*>(vertex_buffer_data),
         sizeof(vertex_buffer_data));
  vertex_buffer_->Unmap(0, nullptr);

  // Initialize the vertex buffer view.
  vertex_buffer_view_ = {
      .BufferLocation = vertex_buffer_->GetGPUVirtualAddress(),
      .SizeInBytes = vertex_buffer_size,
      .StrideInBytes = sizeof(Vertex),
  };
}

void D3D12GPUDevice::init_index_buffer()
{
  // Declare Data
  constexpr uint32_t index_buffer_data[3] = {0, 1, 2};
  constexpr UINT index_buffer_size = sizeof(index_buffer_data);

  constexpr D3D12_HEAP_PROPERTIES heap_props{
      .Type = D3D12_HEAP_TYPE_UPLOAD,
      .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
      .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
      .CreationNodeMask = 1,
      .VisibleNodeMask = 1,
  };

  constexpr D3D12_RESOURCE_DESC index_buffer_resource_desc = {
      .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
      .Alignment = 0,
      .Width = index_buffer_size,
      .Height = 1,
      .DepthOrArraySize = 1,
      .MipLevels = 1,
      .Format = DXGI_FORMAT_UNKNOWN,
      .SampleDesc =
          {
              .Count = 1,
              .Quality = 0,
          },
      .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
      .Flags = D3D12_RESOURCE_FLAG_NONE,
  };

  panic_if_failed(device_->CreateCommittedResource(
      &heap_props, D3D12_HEAP_FLAG_NONE, &index_buffer_resource_desc,
      D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
      IID_PPV_ARGS(&index_buffer_)));

  // 📄 Copy data to DirectX 12 driver memory:
  UINT8* index_data_begin = nullptr;

  constexpr D3D12_RANGE read_range = {
      .Begin = 0,
      .End = 0,
  };

  panic_if_failed(index_buffer_->Map(
      0, &read_range, reinterpret_cast<void**>(&index_data_begin)));
  memcpy(index_data_begin, static_cast<const uint32_t*>(index_buffer_data),
         sizeof(index_buffer_data));
  index_buffer_->Unmap(0, nullptr);

  // Initialize the index buffer view.
  index_buffer_view_ = {
      .BufferLocation = index_buffer_->GetGPUVirtualAddress(),
      .SizeInBytes = index_buffer_size,
      .Format = DXGI_FORMAT_R32_UINT,
  };
}

[[nodiscard]] auto create_d3d12_gpu_device(Window& window) noexcept
    -> std::unique_ptr<GPUDevice>
{
  return std::make_unique<D3D12GPUDevice>(&window);
}

} // namespace beyond::graphics::d3d12