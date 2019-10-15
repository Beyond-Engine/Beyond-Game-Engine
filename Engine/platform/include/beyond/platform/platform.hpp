#pragma once

#ifndef BEYOND_PLATFORM_PLATFORM_HPP
#define BEYOND_PLATFORM_PLATFORM_HPP

#include <memory>
#include <string_view>
#include <vector>

struct VkInstance_T;
struct VkSurfaceKHR_T;
struct VkAllocationCallbacks;

using VkInstance = struct VkInstance_T*;
using VkSurfaceKHR = struct VkSurfaceKHR_T*;

namespace beyond {

class Window;

enum class GraphicsBackend {
  mock = 0,
#ifdef BEYOND_BUILD_GRAPHICS_BACKEND_VULKAN
  vulkan,
#endif
#ifdef BEYOND_BUILD_GRAPHICS_BACKEND_D3D12
  d3d12,
#endif
};

constexpr auto preferred_graphics_backend() noexcept -> GraphicsBackend
{
#ifdef WIN32
#ifdef BEYOND_BUILD_GRAPHICS_BACKEND_D3D12
  return GraphicsBackend::d3d12;
#elif BEYOND_BUILD_GRAPHICS_BACKEND_VULKAN
  return GraphicsBackend::vulkan;
#else
  return GraphicsBackend::mock;
#endif

#else

#ifdef BEYOND_BUILD_GRAPHICS_BACKEND_VULKAN
  return GraphicsBackend::vulkan;
#else
  return GraphicsBackend::mock;
#endif

#endif
}

class Window {
public:
  /**
   * @brief Creates a window
   * @param width Initial width of the window
   * @param height Initial height of the window
   * @param title Title of the window
   *
   * The window will decide its prefered graphics backend depend on the platform
   * and build settings.
   */
  Window(int width, int height, std::string title) noexcept;

  /**
   * @brief Creates a window, and explicit specify what graphics backend it will
   * use
   * @param width Initial width of the window
   * @param height Initial height of the window
   * @param title Title of the window
   * @param backend An enum of graphcis backends
   */
  Window(int width, int height, std::string title,
         GraphicsBackend backend) noexcept;

  ~Window() noexcept;

  Window(const Window& window) = delete;
  auto operator=(const Window& window) -> Window& = delete;

  Window(Window&& other) noexcept;
  auto operator=(Window&& other) noexcept -> Window&;

  [[nodiscard]] auto should_close() const noexcept -> bool;

  auto poll_events() noexcept -> void;

  auto swap_buffers() -> void;

  /// @brief Gets the title of the window
  [[nodiscard]] auto title() const -> std::string
  {
    return title_;
  }

  /// @brief Gets the Graphics Backend of the window
  [[nodiscard]] auto backend() const -> GraphicsBackend
  {
    return backend_;
  }

// TODO(llai): An extension mechanism for Window
#ifdef BEYOND_BUILD_GRAPHICS_BACKEND_VULKAN
  /// @brief Get the extensions needed for the vulkan instance
  [[nodiscard]] auto get_required_instance_extensions() const noexcept
      -> std::vector<const char*>;

  /**
   * @brief Create a VkSurfaceKHR from Window
   * @param[in] instance The Vulkan Instance
   * @param[in] allocator The allocator to use, or `nullptr` to use the default
   * allocator.
   * @param[out] surface The Vulkan surface to create
   */
  auto create_vulkan_surface(VkInstance instance,
                             const VkAllocationCallbacks* allocator,
                             VkSurfaceKHR& surface) noexcept -> void;
#endif

private:
  std::string title_;
  std::unique_ptr<struct WindowImpl> pimpl_;

  GraphicsBackend backend_{};
};

} // namespace beyond

#endif // BEYOND_PLATFORM_PLATFORM_HPP
