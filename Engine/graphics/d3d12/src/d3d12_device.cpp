#include <beyond/core/utils/panic.hpp>
#include <fmt/format.h>

#include "d3d12_interface.hpp"

#include <D3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>

namespace WRL = Microsoft::WRL;

inline void panic_if_failed(HRESULT hr)
{
  if (FAILED(hr)) {
    beyond::panic("Dx12 failed!");
  }
}

namespace beyond::graphics::d3d12 {

class D3D12GPUDevice final : public GPUDevice {
public:
  D3D12GPUDevice()
  {
    std::puts("Pick Direct12 Graphics backend");

    // Declare DirectX 12 Handles
    IDXGIFactory4* factory = nullptr;
    ID3D12Debug1* debug_controller = nullptr;

    // 🏭 Create Factory
    UINT dxgi_factory_flags = 0;

#if defined(_DEBUG)
    // 🐛 Create a Debug Controller to track errors
    ID3D12Debug* dc = nullptr;
    panic_if_failed(D3D12GetDebugInterface(IID_PPV_ARGS(&dc)));
    panic_if_failed(dc->QueryInterface(IID_PPV_ARGS(&debug_controller)));
    debug_controller->EnableDebugLayer();
    debug_controller->SetEnableGPUBasedValidation(true);

    dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;

    dc->Release();
    dc = nullptr;
#endif

    HRESULT result =
        CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&factory));

    // Create Adapter

    IDXGIAdapter1* adapter = nullptr;

    for (UINT adapterIndex = 0;
         DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter);
         ++adapterIndex) {
      DXGI_ADAPTER_DESC1 desc;
      adapter->GetDesc1(&desc);

      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        // Don't select the Basic Render Driver adapter.
        continue;
      }

      // Check to see if the adapter supports Direct3D 12, but don't create
      // the actual device yet.
      if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0,
                                      _uuidof(ID3D12Device), nullptr))) {
        break;
      }

      // We won't use this adapter, so release it
      adapter->Release();
    }

    ID3D12Device* device = nullptr;

    // Create Device
    panic_if_failed(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0,
                                      IID_PPV_ARGS(&device)));

    // Get debug device
    ID3D12DebugDevice* debug_device = nullptr;
#if defined(_DEBUG)
    panic_if_failed(device->QueryInterface(&debug_device));
#endif

    // Create comment queues
    ID3D12CommandQueue* command_queue = nullptr;
    const D3D12_COMMAND_QUEUE_DESC queue_desc{
        .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
    };

    panic_if_failed(
        device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue)));

    // Create Command Allocator
    ID3D12CommandAllocator* command_allocator = nullptr;
    panic_if_failed(device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator)));

    // Create fence
    UINT frameIndex = 0;
    HANDLE fenceEvent = {};
    ID3D12Fence* fence = nullptr;
    UINT64 fenceValue = 0;
    panic_if_failed(
        device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
  }

  [[nodiscard]] auto create_swapchain() -> Swapchain override
  {
    beyond::panic("Unimplemented\n");
  }

  [[nodiscard]] auto create_buffer(const BufferCreateInfo&) -> Buffer override
  {
    beyond::panic("Unimplemented\n");
  }

  auto destory_buffer(Buffer& /*buffer_handle*/) -> void override
  {
    beyond::panic("Unimplemented\n");
  }

  [[nodiscard]] auto create_compute_pipeline(const ComputePipelineCreateInfo&
                                             /*create_info*/)
      -> ComputePipeline override
  {
    beyond::panic("Unimplemented\n");
  }

  auto submit(gsl::span<SubmitInfo>) -> void override
  {
    beyond::panic("Unimplemented\n");
  }

  [[nodiscard]] auto map(Buffer) noexcept -> void* override
  {
    beyond::panic("Unimplemented\n");
  }

  auto unmap(Buffer) noexcept -> void override
  {
    beyond::panic("Unimplemented\n");
  }
}; // namespace beyond::graphics::d3d12

[[nodiscard]] auto create_d3d12_gpu_device(Window& /*window*/) noexcept
    -> std::unique_ptr<GPUDevice>
{
  return std::make_unique<D3D12GPUDevice>();
}

} // namespace beyond::graphics::d3d12
