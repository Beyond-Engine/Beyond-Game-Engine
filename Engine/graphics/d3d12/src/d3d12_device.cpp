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
}

D3D12GPUDevice::~D3D12GPUDevice()
{
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

  // 🎞️ Create frame resources

  D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle_(
      render_target_view_heap_->GetCPUDescriptorHandleForHeapStart());

  // Create a RTV for each frame.
  for (UINT n = 0; n < backbuffer_count; n++) {
    panic_if_failed(
        swapchain_->GetBuffer(n, IID_PPV_ARGS(&render_targets_[n])));
    device_->CreateRenderTargetView(render_targets_[n], nullptr, rtv_handle_);
    rtv_handle_.ptr += (1 * rtv_descriptor_size);
  }
}

[[nodiscard]] auto create_d3d12_gpu_device(Window& window) noexcept
    -> std::unique_ptr<GPUDevice>
{
  return std::make_unique<D3D12GPUDevice>(&window);
}

} // namespace beyond::graphics::d3d12