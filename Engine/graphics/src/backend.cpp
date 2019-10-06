#include <beyond/core/utils/assert.hpp>
#include <beyond/graphics/backend.hpp>

#include <beyond/vulkan/vulkan_fwd.hpp>

namespace beyond::graphics {

struct MockContext : Context {
  MockContext(const Window&) {}
};

[[nodiscard]] auto create_context(Backend backend,
                                  const Window& window) noexcept
    -> std::unique_ptr<Context>
{
  switch (backend) {
  case Backend::mock:
    return std::make_unique<MockContext>(window);

#ifdef BEYOND_BUILD_VULKAN_BACKEND
  case Backend::vulkan:
    return beyond::graphics::vulkan::create_vulkan_context(window);
#endif
  }

  BEYOND_UNREACHABLE();
}

} // namespace beyond::graphics
