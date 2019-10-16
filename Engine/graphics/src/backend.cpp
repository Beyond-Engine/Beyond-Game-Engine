#include <beyond/core/utils/assert.hpp>
#include <beyond/graphics/backend.hpp>

#include <fmt/format.h>

#ifdef BEYOND_BUILD_GRAPHICS_BACKEND_VULKAN
#include <beyond/vulkan/vulkan_fwd.hpp>
#endif

#ifdef BEYOND_BUILD_GRAPHICS_BACKEND_D3D12
#include <beyond/d3d12/d3d12_interface.hpp>
#endif

namespace beyond::graphics {

Context::~Context() = default;

[[nodiscard]] auto create_context(Window& window) noexcept
    -> std::unique_ptr<Context>
{
  switch (window.backend()) {
  case GraphicsBackend::no:
    return nullptr;
#ifdef BEYOND_BUILD_GRAPHICS_BACKEND_VULKAN
  case GraphicsBackend::vulkan:
    return graphics::vulkan::create_vulkan_context(window);
#endif
#ifdef BEYOND_BUILD_GRAPHICS_BACKEND_D3D12
  case GraphicsBackend::d3d12:
    return graphics::d3d12::create_d3d12_context(window);
#endif
  }

  BEYOND_UNREACHABLE();
}

} // namespace beyond::graphics
