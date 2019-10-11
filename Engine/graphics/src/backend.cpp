#include <beyond/core/utils/assert.hpp>
#include <beyond/graphics/backend.hpp>

#ifdef BEYOND_GRAPHICS_BACKEND_VULKAN
#include <beyond/vulkan/vulkan_fwd.hpp>
#endif

namespace beyond::graphics {

struct MockContext : Context {
  explicit MockContext(const Window&) {}

  auto create_swapchain() -> Swapchain override
  {
    return Swapchain{0};
  }
};

[[nodiscard]] auto create_context(const Window& window) noexcept
    -> std::unique_ptr<Context>
{
  switch (window.backend()) {
  case GraphicsBackend::mock:
    return std::make_unique<MockContext>(window);

#ifdef BEYOND_GRAPHICS_BACKEND_VULKAN
  case GraphicsBackend::vulkan:
    return beyond::graphics::vulkan::create_vulkan_context(window);
#endif
  }

  BEYOND_UNREACHABLE();
}

} // namespace beyond::graphics
