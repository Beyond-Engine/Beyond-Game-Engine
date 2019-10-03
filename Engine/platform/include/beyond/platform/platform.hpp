#pragma once

#ifndef BEYOND_PLATFORM_PLATFORM_HPP
#define BEYOND_PLATFORM_PLATFORM_HPP

#include <memory>

#include <gsl/span>
#include <string_view>

namespace beyond {

class Window;

enum class PlatformError : char { cannot_create_window };

class Platform {
public:
  Platform() noexcept;
  ~Platform() noexcept;

  Platform(const Platform&) = delete;
  auto operator=(const Platform&) -> Platform& = delete;

  Platform(Platform&&) noexcept = default;
  auto operator=(Platform&&) noexcept -> Platform& = default;

  auto poll_events() noexcept -> void;

  [[nodiscard]] auto create_window(int width, int height,
                                   std::string_view title) noexcept -> Window;

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

#ifdef BEYOND_GRAPHICS_BACKEND_VULKAN
  /// @brief Get the extensions needed for the vulkan instance
  [[nodiscard]] auto get_required_instance_extensions() const noexcept
      -> gsl::span<const char*>;
#endif

private:
  std::string title_;
  std::unique_ptr<struct WindowImpl> pimpl_;
  Window(std::string title, std::unique_ptr<WindowImpl>&& impl) noexcept;

  friend Platform;
};

} // namespace beyond

#endif // BEYOND_PLATFORM_PLATFORM_HPP
