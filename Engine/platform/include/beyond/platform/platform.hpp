#pragma once

#ifndef BEYOND_PLATFORM_PLATFORM_HPP
#define BEYOND_PLATFORM_PLATFORM_HPP

#include <memory>

#include <string_view>
#include <vector>

namespace beyond {

class Window;

enum class GraphicsBackend {
  mock = 0,
#ifdef BEYOND_GRAPHICS_BACKEND_VULKAN
  vulkan
#endif
};

class Platform {
public:
  Platform() noexcept;
  ~Platform() noexcept;

  Platform(const Platform&) = delete;
  auto operator=(const Platform&) -> Platform& = delete;

  Platform(Platform&&) noexcept = default;
  auto operator=(Platform&&) noexcept -> Platform& = default;

  auto poll_events() noexcept -> void;

  /**
   * @brief Creates a window with default graphics backend
   */
  [[nodiscard]] auto create_window(int width, int height,
                                   std::string_view title) noexcept -> Window;

  /**
   * @brief Creates a window and specify what graphics backend to use
   */
  [[nodiscard]] auto create_window(int width, int height,
                                   std::string_view title,
                                   GraphicsBackend backend) noexcept -> Window;

  auto make_context_current(const Window& window) noexcept -> void;

private:
  std::unique_ptr<struct PlatformImpl> pimpl_;
};

class Window {
public:
  ~Window() noexcept;

  Window(const Window& window) = delete;
  auto operator=(const Window& window) -> Window& = delete;

  Window(Window&& other) noexcept = default;
  auto operator=(Window&& other) noexcept -> Window& = default;

  [[nodiscard]] auto should_close() const noexcept -> bool;

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
#ifdef BEYOND_GRAPHICS_BACKEND_VULKAN
  /// @brief Get the extensions needed for the vulkan instance
  [[nodiscard]] auto get_required_instance_extensions() const noexcept
      -> std::vector<const char*>;
#endif

private:
  std::string title_;
  GraphicsBackend backend_;
  std::unique_ptr<struct WindowImpl> pimpl_;
  Window(std::string title, GraphicsBackend backend,
         std::unique_ptr<WindowImpl>&& impl) noexcept;

  friend Platform;
};

} // namespace beyond

#endif // BEYOND_PLATFORM_PLATFORM_HPP
