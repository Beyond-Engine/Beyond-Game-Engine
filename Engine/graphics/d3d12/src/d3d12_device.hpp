#ifndef BEYOND_GRAPHICS_D3D12_GPU_DEVICE_HPP
#define BEYOND_GRAPHICS_D3D12_GPU_DEVICE_HPP

#include <beyond/core/utils/panic.hpp>
#include <fmt/format.h>

#include "../include/beyond/d3d12/d3d12_interface.hpp"

#include <D3d12.h>
#include <dxgi1_4.h>

namespace beyond::graphics::d3d12 {

class D3D12GPUDevice final : public GPUDevice {
  Window* window_ = nullptr;

  //#if defined(_DEBUG)
  ID3D12Debug1* debug_controller_ = nullptr;
  //#endif
  IDXGIFactory4* factory_ = nullptr;
  IDXGIAdapter1* adapter_ = nullptr;
  ID3D12Device* device_ = nullptr;
  //#if defined(_DEBUG)
  ID3D12DebugDevice* debug_device_ = nullptr;
  //#endif

  ID3D12CommandQueue* command_queue_ = nullptr;
  ID3D12CommandAllocator* command_allocator_ = nullptr;

  // Sync
  ID3D12Fence* fence_ = nullptr;
  UINT frame_index_ = 0;
  HANDLE fence_event = {};
  UINT64 fence_value = 0;

  // Current Frame
  static constexpr UINT backbuffer_count = 2;
  UINT current_buffer_ = 0;
  ID3D12DescriptorHeap* render_target_view_heap_ = nullptr;
  ID3D12Resource* render_targets_[backbuffer_count];
  IDXGISwapChain3* swapchain_ = nullptr;

  // Resources
  D3D12_VIEWPORT viewport_;
  D3D12_RECT surface_size_;

  ID3D12RootSignature* root_signature_ = nullptr;

  ID3D12Resource* vertex_buffer_ = nullptr;
  D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_ = {};

  ID3D12Resource* index_buffer_ = nullptr;
  D3D12_INDEX_BUFFER_VIEW index_buffer_view_ = {};

public:
  explicit D3D12GPUDevice(Window* window = nullptr);
  ~D3D12GPUDevice() noexcept override;

  D3D12GPUDevice(const D3D12GPUDevice&) = delete;
  auto operator=(const D3D12GPUDevice&) -> D3D12GPUDevice& = delete;
  D3D12GPUDevice(D3D12GPUDevice&&) noexcept = delete;
  auto operator=(D3D12GPUDevice&&) noexcept -> D3D12GPUDevice& = delete;

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

private:
  void init_swapchain();
  void init_root_signature();
  void init_vertex_buffer();
  void init_index_buffer();
};

} // namespace beyond::graphics::d3d12

#endif // BEYOND_GRAPHICS_D3D12_GPU_DEVICE_HPP
