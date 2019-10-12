#include "d3d12_interface.hpp"

#include <D3d12.h>
#include <wrl.h>

namespace WRL = Microsoft::WRL;

namespace beyond::graphics::d3d12 {

class D3D12Context : public Context {
public:
  D3D12Context() {}

  [[nodiscard]] auto create_swapchain() -> Swapchain override
  {
    return Swapchain{0};
  }

private:
};

[[nodiscard]] auto create_d3d12_context(Window& /*window*/) noexcept
    -> std::unique_ptr<Context>
{
  return std::make_unique<D3D12Context>();
}

} // namespace beyond::graphics::d3d12
