#include <beyond/core/utils/panic.hpp>

#include "d3d12_interface.hpp"

#include <D3d12.h>
#include <wrl.h>

namespace WRL = Microsoft::WRL;

namespace beyond::graphics::d3d12 {

class D3D12Context final : public Context {
public:
  D3D12Context()
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

private:
  [[nodiscard]] auto map_memory_impl(Buffer) noexcept -> MappingInfo override
  {
    beyond::panic("Unimplemented\n");
  }

  auto unmap_memory_impl(Buffer) noexcept -> void override
  {
    beyond::panic("Unimplemented\n");
  }
};

[[nodiscard]] auto create_d3d12_context(Window& /*window*/) noexcept
    -> std::unique_ptr<Context>
{
  return std::make_unique<D3D12Context>();
}

} // namespace beyond::graphics::d3d12
