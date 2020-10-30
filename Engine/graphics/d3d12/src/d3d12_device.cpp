#include "d3d12_device.hpp"

#include <beyond/core/utils/bit_cast.hpp>
#include <beyond/core/utils/panic.hpp>

#include <d3dcompiler.h>

using namespace glm;

namespace {

#define PANIC_IF_FAILED(expr)                                                  \
  {                                                                            \
    if (expr) {                                                                \
      beyond::panic("Fail to execute DX12 Function " #expr);                   \
    }                                                                          \
  }

} // anonymous namespace

namespace beyond::graphics::d3d12 {

D3D12Device::D3D12Device(Window& window)
{
  // Resources
  mRootSignature = nullptr;
  pipeline_state_ = nullptr;

  // Current Frame
  mRtvHeap = nullptr;
  // Sync
  fence_ = nullptr;

  initializeAPI(window);
  t_start = std::chrono::high_resolution_clock::now();
}

D3D12Device::~D3D12Device()
{
  destroyCommands();
  destroy_frame_buffer();
  destroyResources();
  destroyAPI();
}

[[nodiscard]] auto D3D12Device::create_swapchain(std::uint32_t width,
                                                 std::uint32_t height)
    -> GPUSwapchain
{
  IDXGISwapChain3* swapchain = nullptr;

  DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {
      .Width = width,
      .Height = height,
      .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
      .SampleDesc{
          .Count = 1,
      },
      .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
      .BufferCount = backbuffer_count_,
      .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
  };

  IDXGISwapChain1* swapchain1 = nullptr;
  PANIC_IF_FAILED(factory_->CreateSwapChainForHwnd(
      command_queue_, window_, &swapchain_desc, nullptr, nullptr, &swapchain1));

  HRESULT swapchain_support = swapchain1->QueryInterface(&swapchain);
  if (!SUCCEEDED(swapchain_support)) {
    beyond::panic("Cannot create swapchain");
  }
  return GPUSwapchain{.id = beyond::bit_cast<std::uint64_t>(swapchain)};
}

void D3D12Device::destroy_swapchain(GPUSwapchain swapchain)
{
  auto* dx_swapchain = beyond::bit_cast<IDXGISwapChain3*>(swapchain.id);
  if (dx_swapchain != nullptr) {
    dx_swapchain->SetFullscreenState(0, nullptr);
    dx_swapchain->Release();
  }
}

void D3D12Device::resize_swapchain(GPUSwapchain& swapchain, std::uint32_t width,
                                   std::uint32_t height)
{
  auto* dx_swapchain = beyond::bit_cast<IDXGISwapChain3*>(swapchain.id);
  dx_swapchain->ResizeBuffers(backbuffer_count_, width, height,
                              DXGI_FORMAT_R8G8B8A8_UNORM, 0);
}

[[nodiscard]] auto
D3D12Device::get_swapchain_back_buffer_index(GPUSwapchain swapchain)
    -> std::uint32_t
{
  auto* dx_swapchain = beyond::bit_cast<IDXGISwapChain3*>(swapchain.id);
  return dx_swapchain->GetCurrentBackBufferIndex();
}

void D3D12Device::initializeAPI(Window& window)
{
  // The renderer needs the window when resizing the swapchain
  window_ = window.get_win32_window();

  // Create Factory

  UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
  {
    ID3D12Debug* debug_controller = nullptr;
    PANIC_IF_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)));
    PANIC_IF_FAILED(
        debug_controller->QueryInterface(IID_PPV_ARGS(&debug_controller_)));
    debug_controller_->EnableDebugLayer();
    debug_controller_->SetEnableGPUBasedValidation(true);

    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    debug_controller->Release();
  }

#endif
  PANIC_IF_FAILED(
      CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory_)));

  // Create Adapter

  for (UINT adapterIndex = 0;
       DXGI_ERROR_NOT_FOUND != factory_->EnumAdapters1(adapterIndex, &adapter_);
       ++adapterIndex) {
    DXGI_ADAPTER_DESC1 desc;
    adapter_->GetDesc1(&desc);

    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
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
  ID3D12Device* pDev = nullptr;
  PANIC_IF_FAILED(D3D12CreateDevice(adapter_, D3D_FEATURE_LEVEL_12_0,
                                    IID_PPV_ARGS(&device_)));

  device_->SetName(L"Hello Triangle Device");

#if defined(_DEBUG)
  // Get debug device
  PANIC_IF_FAILED(device_->QueryInterface(&mDebugDevice));
#endif

  // Create Command Queue
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

  PANIC_IF_FAILED(
      device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&command_queue_)));

  // Create Command Allocator
  PANIC_IF_FAILED(device_->CreateCommandAllocator(
      D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator_)));

  // Sync
  PANIC_IF_FAILED(
      device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)));

  // Create Swapchain
  const Resolution get_resolution = window.get_resolution();
}

void D3D12Device::destroyAPI()
{
  if (fence_) {
    fence_->Release();
    fence_ = nullptr;
  }

  if (command_allocator_) {
    PANIC_IF_FAILED(command_allocator_->Reset());
    command_allocator_->Release();
    command_allocator_ = nullptr;
  }

  if (command_queue_) {
    command_queue_->Release();
    command_queue_ = nullptr;
  }

  if (device_) {
    device_->Release();
    device_ = nullptr;
  }

  if (adapter_) {
    adapter_->Release();
    adapter_ = nullptr;
  }

  if (factory_) {
    factory_->Release();
    factory_ = nullptr;
  }

#if defined(_DEBUG)
  if (debug_controller_) {
    debug_controller_->Release();
    debug_controller_ = nullptr;
  }

  D3D12_RLDO_FLAGS flags =
      D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL;

  mDebugDevice->ReportLiveDeviceObjects(flags);

  if (mDebugDevice) {
    mDebugDevice->Release();
    mDebugDevice = nullptr;
  }
#endif
}

void D3D12Device::init_frame_buffer(GPUSwapchain swapchain)
{
  auto* dx_swapchain = beyond::bit_cast<IDXGISwapChain3*>(swapchain.id);

  mCurrentBuffer = dx_swapchain->GetCurrentBackBufferIndex();

  // Create descriptor heaps.
  {
    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = backbuffer_count_;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    PANIC_IF_FAILED(
        device_->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap)));

    mRtvDescriptorSize = device_->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  }

  // Create frame resources.
  {
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        mRtvHeap->GetCPUDescriptorHandleForHeapStart());

    // Create a RTV for each frame.
    for (UINT n = 0; n < backbuffer_count_; n++) {
      PANIC_IF_FAILED(
          dx_swapchain->GetBuffer(n, IID_PPV_ARGS(&render_target_[n])));
      device_->CreateRenderTargetView(render_target_[n], nullptr, rtvHandle);
      rtvHandle.ptr += (1 * mRtvDescriptorSize);
    }
  }
}

void D3D12Device::destroy_frame_buffer()
{
  for (size_t i = 0; i < backbuffer_count_; ++i) {
    if (render_target_[i]) {
      render_target_[i]->Release();
      render_target_[i] = 0;
    }
  }
  if (mRtvHeap) {
    mRtvHeap->Release();
    mRtvHeap = nullptr;
  }
}

void D3D12Device::initialize_resources(GPUSwapchain swapchain)
{
  auto* dx_swapchain = beyond::bit_cast<IDXGISwapChain3*>(swapchain.id);

  // Create the root signature.
  {
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    // This is the highest version the sample supports. If
    // CheckFeatureSupport succeeds, the HighestVersion returned will not be
    // greater than this.
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if (FAILED(device_->CheckFeatureSupport(
            D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) {
      featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    D3D12_DESCRIPTOR_RANGE1 ranges[1];
    ranges[0].BaseShaderRegister = 0;
    ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    ranges[0].NumDescriptors = 1;
    ranges[0].RegisterSpace = 0;
    ranges[0].OffsetInDescriptorsFromTableStart = 0;
    ranges[0].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;

    D3D12_ROOT_PARAMETER1 rootParameters[1];
    rootParameters[0].ParameterType =
        D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[0].DescriptorTable.pDescriptorRanges = ranges;

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    rootSignatureDesc.Desc_1_1.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignatureDesc.Desc_1_1.NumParameters = 1;
    rootSignatureDesc.Desc_1_1.pParameters = rootParameters;
    rootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
    rootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;

    ID3DBlob* signature;
    ID3DBlob* error;
    try {
      PANIC_IF_FAILED(D3D12SerializeVersionedRootSignature(&rootSignatureDesc,
                                                           &signature, &error));
      PANIC_IF_FAILED(device_->CreateRootSignature(
          0, signature->GetBufferPointer(), signature->GetBufferSize(),
          IID_PPV_ARGS(&mRootSignature)));
      mRootSignature->SetName(L"Hello Triangle Root Signature");
    } catch (std::exception e) {
      const char* errStr = (const char*)error->GetBufferPointer();
      std::cout << errStr;
      error->Release();
      error = nullptr;
    }

    if (signature) {
      signature->Release();
      signature = nullptr;
    }
  }

  // Create the pipeline state, which includes compiling and loading shaders.
  {
    ID3DBlob* vertexShader = nullptr;
    ID3DBlob* pixelShader = nullptr;
    ID3DBlob* errors = nullptr;

#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    std::string path = "";
    char pBuf[1024];

    _getcwd(pBuf, 1024);
    path = pBuf;
    path += "\\";
    std::wstring wpath = std::wstring(path.begin(), path.end());

    std::string vertCompiledPath = path, fragCompiledPath = path;
    vertCompiledPath += "assets\\triangle.vert.dxbc";
    fragCompiledPath += "assets\\triangle.frag.dxbc";

#define COMPILESHADERS
#ifdef COMPILESHADERS
    std::wstring vertPath = wpath + L"assets\\triangle.vert.hlsl";
    std::wstring fragPath = wpath + L"assets\\triangle.frag.hlsl";

    try {
      PANIC_IF_FAILED(D3DCompileFromFile(vertPath.c_str(), nullptr, nullptr,
                                         "main", "vs_5_0", compileFlags, 0,
                                         &vertexShader, &errors));
      PANIC_IF_FAILED(D3DCompileFromFile(fragPath.c_str(), nullptr, nullptr,
                                         "main", "ps_5_0", compileFlags, 0,
                                         &pixelShader, &errors));
    } catch (std::exception e) {
      const char* errStr = (const char*)errors->GetBufferPointer();
      std::cout << errStr;
      errors->Release();
      errors = nullptr;
    }

    std::ofstream vsOut(vertCompiledPath, std::ios::out | std::ios::binary),
        fsOut(fragCompiledPath, std::ios::out | std::ios::binary);

    vsOut.write((const char*)vertexShader->GetBufferPointer(),
                vertexShader->GetBufferSize());
    fsOut.write((const char*)pixelShader->GetBufferPointer(),
                pixelShader->GetBufferSize());

#else
    std::vector<char> vsBytecodeData = readFile(vertCompiledPath);
    std::vector<char> fsBytecodeData = readFile(fragCompiledPath);

#endif
    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

    // Create the UBO.
    {
      // Note: using upload heaps to transfer static data like vert
      // buffers is not recommended. Every time the GPU needs it, the
      // upload heap will be marshalled over. Please read up on Default
      // Heap usage. An upload heap is used here for code simplicity and
      // because there are very few verts to actually transfer.
      D3D12_HEAP_PROPERTIES heapProps;
      heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
      heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
      heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
      heapProps.CreationNodeMask = 1;
      heapProps.VisibleNodeMask = 1;

      D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
      heapDesc.NumDescriptors = 1;
      heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
      heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
      PANIC_IF_FAILED(device_->CreateDescriptorHeap(
          &heapDesc, IID_PPV_ARGS(&uniform_buffer_heap_)));

      D3D12_RESOURCE_DESC uboResourceDesc;
      uboResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      uboResourceDesc.Alignment = 0;
      uboResourceDesc.Width = (sizeof(ubo_data_) + 255) & ~255;
      uboResourceDesc.Height = 1;
      uboResourceDesc.DepthOrArraySize = 1;
      uboResourceDesc.MipLevels = 1;
      uboResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
      uboResourceDesc.SampleDesc.Count = 1;
      uboResourceDesc.SampleDesc.Quality = 0;
      uboResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      uboResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

      PANIC_IF_FAILED(device_->CreateCommittedResource(
          &heapProps, D3D12_HEAP_FLAG_NONE, &uboResourceDesc,
          D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
          IID_PPV_ARGS(&uniform_buffer_)));
      uniform_buffer_heap_->SetName(L"Constant Buffer Upload Resource Heap");

      D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
      cbvDesc.BufferLocation = uniform_buffer_->GetGPUVirtualAddress();
      cbvDesc.SizeInBytes = (sizeof(ubo_data_) + 255) &
                            ~255; // CB size is required to be 256-byte aligned.

      D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle(
          uniform_buffer_heap_->GetCPUDescriptorHandleForHeapStart());
      cbvHandle.ptr =
          cbvHandle.ptr + device_->GetDescriptorHandleIncrementSize(
                              D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) *
                              0;

      device_->CreateConstantBufferView(&cbvDesc, cbvHandle);

      // We do not intend to read from this resource on the CPU. (End is
      // less than or equal to begin)
      D3D12_RANGE readRange;
      readRange.Begin = 0;
      readRange.End = 0;

      PANIC_IF_FAILED(uniform_buffer_->Map(
          0, &readRange, reinterpret_cast<void**>(&mapped_uniform_buffer_)));
      memcpy(mapped_uniform_buffer_, &ubo_data_, sizeof(ubo_data_));
      uniform_buffer_->Unmap(0, &readRange);
    }

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = {inputElementDescs, _countof(inputElementDescs)};
    psoDesc.pRootSignature = mRootSignature;

    D3D12_SHADER_BYTECODE vsBytecode;
    D3D12_SHADER_BYTECODE psBytecode;

#ifdef COMPILESHADERS
    vsBytecode.pShaderBytecode = vertexShader->GetBufferPointer();
    vsBytecode.BytecodeLength = vertexShader->GetBufferSize();

    psBytecode.pShaderBytecode = pixelShader->GetBufferPointer();
    psBytecode.BytecodeLength = pixelShader->GetBufferSize();
#else
    vsBytecode.pShaderBytecode = vsBytecodeData.data();
    vsBytecode.BytecodeLength = vsBytecodeData.size();

    psBytecode.pShaderBytecode = fsBytecodeData.data();
    psBytecode.BytecodeLength = fsBytecodeData.size();
#endif

    psoDesc.VS = vsBytecode;
    psoDesc.PS = psBytecode;

    D3D12_RASTERIZER_DESC rasterDesc;
    rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterDesc.CullMode = D3D12_CULL_MODE_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterDesc.DepthClipEnable = TRUE;
    rasterDesc.MultisampleEnable = FALSE;
    rasterDesc.AntialiasedLineEnable = FALSE;
    rasterDesc.ForcedSampleCount = 0;
    rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    psoDesc.RasterizerState = rasterDesc;

    D3D12_BLEND_DESC blendDesc;
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = {
        FALSE,
        FALSE,
        D3D12_BLEND_ONE,
        D3D12_BLEND_ZERO,
        D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE,
        D3D12_BLEND_ZERO,
        D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };
    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
      blendDesc.RenderTarget[i] = defaultRenderTargetBlendDesc;

    psoDesc.BlendState = blendDesc;
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    try {
      PANIC_IF_FAILED(device_->CreateGraphicsPipelineState(
          &psoDesc, IID_PPV_ARGS(&pipeline_state_)));
    } catch (std::exception e) {
      std::cout << "Failed to create Graphics Pipeline!";
    }

    if (vertexShader) {
      vertexShader->Release();
      vertexShader = nullptr;
    }

    if (pixelShader) {
      pixelShader->Release();
      pixelShader = nullptr;
    }
  }

  createCommands();

  // Command lists are created in the recording state, but there is nothing
  // to record yet. The main loop expects it to be closed, so close it now.
  PANIC_IF_FAILED(command_list_->Close());

  // Create the vertex buffer.
  {
    const UINT vertexBufferSize = sizeof(mVertexBufferData);

    // Note: using upload heaps to transfer static data like vert buffers is
    // not recommended. Every time the GPU needs it, the upload heap will be
    // marshalled over. Please read up on Default Heap usage. An upload heap
    // is used here for code simplicity and because there are very few verts
    // to actually transfer.
    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC vertexBufferResourceDesc;
    vertexBufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertexBufferResourceDesc.Alignment = 0;
    vertexBufferResourceDesc.Width = vertexBufferSize;
    vertexBufferResourceDesc.Height = 1;
    vertexBufferResourceDesc.DepthOrArraySize = 1;
    vertexBufferResourceDesc.MipLevels = 1;
    vertexBufferResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    vertexBufferResourceDesc.SampleDesc.Count = 1;
    vertexBufferResourceDesc.SampleDesc.Quality = 0;
    vertexBufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    vertexBufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    PANIC_IF_FAILED(device_->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &vertexBufferResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&vertex_buffer_)));

    // Copy the triangle data to the vertex buffer.
    UINT8* pVertexDataBegin;

    // We do not intend to read from this resource on the CPU.
    D3D12_RANGE readRange;
    readRange.Begin = 0;
    readRange.End = 0;

    PANIC_IF_FAILED(vertex_buffer_->Map(
        0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, mVertexBufferData, sizeof(mVertexBufferData));
    vertex_buffer_->Unmap(0, nullptr);

    // Initialize the vertex buffer view.
    mVertexBufferView.BufferLocation = vertex_buffer_->GetGPUVirtualAddress();
    mVertexBufferView.StrideInBytes = sizeof(Vertex);
    mVertexBufferView.SizeInBytes = vertexBufferSize;
  }

  // Create the index buffer.
  {
    const UINT indexBufferSize = sizeof(mIndexBufferData);

    // Note: using upload heaps to transfer static data like vert buffers is
    // not recommended. Every time the GPU needs it, the upload heap will be
    // marshalled over. Please read up on Default Heap usage. An upload heap
    // is used here for code simplicity and because there are very few verts
    // to actually transfer.
    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC vertexBufferResourceDesc;
    vertexBufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertexBufferResourceDesc.Alignment = 0;
    vertexBufferResourceDesc.Width = indexBufferSize;
    vertexBufferResourceDesc.Height = 1;
    vertexBufferResourceDesc.DepthOrArraySize = 1;
    vertexBufferResourceDesc.MipLevels = 1;
    vertexBufferResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    vertexBufferResourceDesc.SampleDesc.Count = 1;
    vertexBufferResourceDesc.SampleDesc.Quality = 0;
    vertexBufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    vertexBufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    PANIC_IF_FAILED(device_->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &vertexBufferResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&index_buffer_)));

    // Copy the triangle data to the vertex buffer.
    UINT8* pVertexDataBegin;

    // We do not intend to read from this resource on the CPU.
    D3D12_RANGE readRange;
    readRange.Begin = 0;
    readRange.End = 0;

    PANIC_IF_FAILED(index_buffer_->Map(
        0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, mIndexBufferData, sizeof(mIndexBufferData));
    index_buffer_->Unmap(0, nullptr);

    // Initialize the vertex buffer view.
    mIndexBufferView.BufferLocation = index_buffer_->GetGPUVirtualAddress();
    mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    mIndexBufferView.SizeInBytes = indexBufferSize;
  }

  // Create synchronization objects and wait until assets have been uploaded
  // to the GPU.
  {
    fence_value_ = 1;

    // Create an event handle to use for frame synchronization.
    fence_event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (fence_event_ == nullptr) {
      PANIC_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
    }

    // Wait for the command list to execute; we are reusing the same command
    // list in our main loop but for now, we just want to wait for setup to
    // complete before continuing.
    // Signal and increment the fence value.
    const UINT64 fence = fence_value_;
    PANIC_IF_FAILED(command_queue_->Signal(fence_, fence));
    fence_value_++;

    // Wait until the previous frame is finished.
    if (fence_->GetCompletedValue() < fence) {
      PANIC_IF_FAILED(fence_->SetEventOnCompletion(fence, fence_event_));
      WaitForSingleObject(fence_event_, INFINITE);
    }

    frame_index_ = dx_swapchain->GetCurrentBackBufferIndex();
  }
}

void D3D12Device::destroyResources()
{
  // Sync
  CloseHandle(fence_event_);

  if (pipeline_state_) {
    pipeline_state_->Release();
    pipeline_state_ = nullptr;
  }

  if (mRootSignature) {
    mRootSignature->Release();
    mRootSignature = nullptr;
  }

  if (vertex_buffer_) {
    vertex_buffer_->Release();
    vertex_buffer_ = nullptr;
  }

  if (index_buffer_) {
    index_buffer_->Release();
    index_buffer_ = nullptr;
  }

  if (uniform_buffer_) {
    uniform_buffer_->Release();
    uniform_buffer_ = nullptr;
  }

  if (uniform_buffer_heap_) {
    uniform_buffer_heap_->Release();
    uniform_buffer_heap_ = nullptr;
  }
}

void D3D12Device::createCommands()
{
  // Create the command list.
  PANIC_IF_FAILED(device_->CreateCommandList(
      0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator_, pipeline_state_,
      IID_PPV_ARGS(&command_list_)));
  command_list_->SetName(L"Hello Triangle Command List");
}

void D3D12Device::setup_commands()
{
  // Command list allocators can only be reset when the associated
  // command lists have finished execution on the GPU; apps should use
  // fences to determine GPU execution progress.
  PANIC_IF_FAILED(command_allocator_->Reset());

  // However, when ExecuteCommandList() is called on a particular command
  // list, that command list can then be reset at any time and must be before
  // re-recording.
  PANIC_IF_FAILED(command_list_->Reset(command_allocator_, pipeline_state_));

  // Set necessary state.
  command_list_->SetGraphicsRootSignature(mRootSignature);
  command_list_->RSSetViewports(1, &mViewport);
  command_list_->RSSetScissorRects(1, &mSurfaceSize);

  ID3D12DescriptorHeap* pDescriptorHeaps[] = {uniform_buffer_heap_};
  command_list_->SetDescriptorHeaps(_countof(pDescriptorHeaps),
                                    pDescriptorHeaps);

  D3D12_GPU_DESCRIPTOR_HANDLE srvHandle(
      uniform_buffer_heap_->GetGPUDescriptorHandleForHeapStart());
  command_list_->SetGraphicsRootDescriptorTable(0, srvHandle);

  // Indicate that the back buffer will be used as a render target.
  D3D12_RESOURCE_BARRIER renderTargetBarrier;
  renderTargetBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  renderTargetBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  renderTargetBarrier.Transition.pResource = render_target_[frame_index_];
  renderTargetBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
  renderTargetBarrier.Transition.StateAfter =
      D3D12_RESOURCE_STATE_RENDER_TARGET;
  renderTargetBarrier.Transition.Subresource =
      D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

  command_list_->ResourceBarrier(1, &renderTargetBarrier);

  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(
      mRtvHeap->GetCPUDescriptorHandleForHeapStart());
  rtvHandle.ptr = rtvHandle.ptr + (frame_index_ * mRtvDescriptorSize);
  command_list_->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

  // Record commands.
  const float clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
  command_list_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
  command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  command_list_->IASetVertexBuffers(0, 1, &mVertexBufferView);
  command_list_->IASetIndexBuffer(&mIndexBufferView);

  command_list_->DrawIndexedInstanced(3, 1, 0, 0, 0);

  // Indicate that the back buffer will now be used to present.
  const D3D12_RESOURCE_BARRIER present_barrier = {
      .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
      .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
      .Transition = {.pResource = render_target_[frame_index_],
                     .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                     .StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET,
                     .StateAfter = D3D12_RESOURCE_STATE_PRESENT}};

  command_list_->ResourceBarrier(1, &present_barrier);

  PANIC_IF_FAILED(command_list_->Close());
}

void D3D12Device::destroyCommands()
{
  if (command_list_) {
    command_list_->Reset(command_allocator_, pipeline_state_);
    command_list_->ClearState(pipeline_state_);
    PANIC_IF_FAILED(command_list_->Close());
    ID3D12CommandList* ppCommandLists[] = {command_list_};
    command_queue_->ExecuteCommandLists(_countof(ppCommandLists),
                                        ppCommandLists);

    // Wait for GPU to finish work
    const UINT64 fence = fence_value_;
    PANIC_IF_FAILED(command_queue_->Signal(fence_, fence));
    fence_value_++;
    if (fence_->GetCompletedValue() < fence) {
      PANIC_IF_FAILED(fence_->SetEventOnCompletion(fence, fence_event_));
      WaitForSingleObject(fence_event_, INFINITE);
    }

    command_list_->Release();
    command_list_ = nullptr;
  }
}

void D3D12Device::setup_swapchain(GPUSwapchain& swapchain, unsigned width,
                                  unsigned height)
{
  auto* dx_swapchain = beyond::bit_cast<IDXGISwapChain3*>(swapchain.id);

  mSurfaceSize.left = 0;
  mSurfaceSize.top = 0;
  mSurfaceSize.right = static_cast<LONG>(width_);
  mSurfaceSize.bottom = static_cast<LONG>(height_);

  mViewport.TopLeftX = 0.0f;
  mViewport.TopLeftY = 0.0f;
  mViewport.Width = static_cast<float>(width_);
  mViewport.Height = static_cast<float>(height_);
  mViewport.MinDepth = .1f;
  mViewport.MaxDepth = 1000.f;

  // Update Uniforms
  float zoom = 2.5f;

  // Update matrices
  ubo_data_.projectionMatrix =
      glm::perspective(45.0f, (float)width_ / (float)height_, 0.01f, 1024.0f);

  ubo_data_.viewMatrix =
      glm::translate(glm::identity<mat4>(), vec3(0.0f, 0.0f, zoom));

  ubo_data_.modelMatrix = glm::identity<mat4>();

  if (dx_swapchain != nullptr) {
    dx_swapchain->ResizeBuffers(backbuffer_count_, width_, height_,
                                DXGI_FORMAT_R8G8B8A8_UNORM, 0);
  } else {
    swapchain = create_swapchain(width, height);
  }
  frame_index_ = dx_swapchain->GetCurrentBackBufferIndex();
}

void D3D12Device::resize(GPUSwapchain& swapchain, unsigned width,
                         unsigned height)
{

  width_ = clamp(width, 1u, 0xffffu);
  height_ = clamp(height, 1u, 0xffffu);

  // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
  // This is code implemented as such for simplicity. The
  // D3D12HelloFrameBuffering sample illustrates how to use fences for
  // efficient resource usage and to maximize GPU utilization.

  // Signal and increment the fence value.
  const UINT64 fence = fence_value_;
  PANIC_IF_FAILED(command_queue_->Signal(fence_, fence));
  fence_value_++;

  // Wait until the previous frame is finished.
  if (fence_->GetCompletedValue() < fence) {
    PANIC_IF_FAILED(fence_->SetEventOnCompletion(fence, fence_event_));
    WaitForSingleObjectEx(fence_event_, INFINITE, 0);
  }

  destroy_frame_buffer();
  setup_swapchain(swapchain, width, height);
  init_frame_buffer(swapchain);
}

void D3D12Device::render(GPUSwapchain swapchain)
{
  auto* dx_swapchain = beyond::bit_cast<IDXGISwapChain3*>(swapchain.id);

  // Framelimit set to 60 fps
  t_end = std::chrono::high_resolution_clock::now();
  float time =
      std::chrono::duration<float, std::milli>(t_end - t_start).count();
  if (time < (1000.0f / 60.0f)) {
    return;
  }
  t_start = std::chrono::high_resolution_clock::now();

  {
    // Update Uniforms
    elapsed_time_ += 0.001f * time;
    elapsed_time_ = fmodf(elapsed_time_, 6.283185307179586f);

    ubo_data_.modelMatrix = glm::rotate(ubo_data_.modelMatrix, 0.001f * time,
                                        vec3(0.0f, 1.0f, 0.0f));

    static constexpr D3D12_RANGE read_range = {
        .Begin = 0,
        .End = 0,
    };
    PANIC_IF_FAILED(uniform_buffer_->Map(
        0, &read_range, reinterpret_cast<void**>(&mapped_uniform_buffer_)));
    memcpy(mapped_uniform_buffer_, &ubo_data_, sizeof(ubo_data_));
    uniform_buffer_->Unmap(0, &read_range);
  }

  // Record all the commands we need to render the scene into the command
  // list.
  setup_commands();

  // Execute the command list.
  ID3D12CommandList* command_lists[] = {command_list_};
  command_queue_->ExecuteCommandLists(_countof(command_lists), command_lists);
  dx_swapchain->Present(1, 0);

  // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.

  // Signal and increment the fence value.
  const UINT64 fence = fence_value_;
  PANIC_IF_FAILED(command_queue_->Signal(fence_, fence));
  fence_value_++;

  // Wait until the previous frame is finished.
  if (fence_->GetCompletedValue() < fence) {
    PANIC_IF_FAILED(fence_->SetEventOnCompletion(fence, fence_event_));
    WaitForSingleObject(fence_event_, INFINITE);
  }

  frame_index_ = dx_swapchain->GetCurrentBackBufferIndex();
}

/// @brief Create a D3D12 context
[[nodiscard]] auto create_d3d12_gpu_device(Window& window) noexcept
    -> std::unique_ptr<GPUDevice>
{
  return std::make_unique<D3D12Device>(window);
}

} // namespace beyond::graphics::d3d12
