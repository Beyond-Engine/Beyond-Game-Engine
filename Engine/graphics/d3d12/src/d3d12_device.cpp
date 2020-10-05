#include <beyond/core/utils/panic.hpp>

#include "d3d12_interface.hpp"

#include <D3d12.h>
#include <wrl.h>

namespace WRL = Microsoft::WRL;

namespace beyond::graphics::d3d12 {

class D3D12GPUDevice final : public GPUDevice {
public:
  D3D12GPUDevice()
  {
    beyond::panic("Direct3D 12 Backend is currently a stub, please use Vulkan "
                  "backend instead\n");
  }

  [[nodiscard]] auto create_swapchain() -> Swapchain override
  {
    beyond::panic("Unimplemented\n");
  }

  [[nodiscard]] auto create_buffer(const BufferCreateInfo&) -> Buffer override
  {
    beyond::panic("Unimplemented\n");
  }

  auto destory_buffer(Buffer & /*buffer_handle*/) -> void override
  {
    beyond::panic("Unimplemented\n");
  }

  [[nodiscard]] auto create_compute_pipeline(const ComputePipelineCreateInfo &
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
};

[[nodiscard]] auto create_d3d12_gpu_device(Window& /*window*/) noexcept
    -> std::unique_ptr<GPUDevice>
{
  return std::make_unique<D3D12GPUDevice>();
}

} // namespace beyond::graphics::d3d12
