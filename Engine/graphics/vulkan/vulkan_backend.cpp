#include <beyond/graphics/backend.hpp>

#include "vulkan_context.hpp"

namespace beyond::graphics {

[[nodiscard]] auto create_context(const Window& window) noexcept -> Context*
{
  return new Context(window);
}

auto destory_context(Context* context) noexcept -> void
{
  delete context;
}

} // namespace beyond::graphics
