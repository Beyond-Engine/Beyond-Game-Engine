#include "d3d12_interface.hpp"

#include <D3d12.h>
#include <wrl.h>

namespace WRL = Microsoft::WRL;

namespace beyond::graphics::d3d12 {

class D3D12Context final : public Context {
public:
  D3D12Context()
  {
    std::fputs("Direct3D 12 Backend is currently a stub, please use Vulkan "
               "backend instead\n",
               stderr);
    std::exit(1);
  }

  [[nodiscard]] auto create_swapchain() -> Swapchain override
  {
    return Swapchain{0};
  }

  [[nodiscard]] auto create_buffer(const BufferCreateInfo&) -> Buffer override
  {
    return Buffer{0};
  }

  auto submit(gsl::span<SubmitInfo>) -> void override {}

private:
  [[nodiscard]] auto map_memory_impl(Buffer) noexcept -> MappingInfo override
  {
    return {nullptr, 0};
  }

  auto unmap_memory_impl(Buffer) noexcept -> void override {}
};

[[nodiscard]] auto create_d3d12_context(Window& /*window*/) noexcept
    -> std::unique_ptr<Context>
{
  return std::make_unique<D3D12Context>();
}

} // namespace beyond::graphics::d3d12
