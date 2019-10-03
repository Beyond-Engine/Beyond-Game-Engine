#include <GLFW/glfw3.h>

#include "beyond/platform/platform.hpp"
#include <beyond/core/utils/panic.hpp>

namespace beyond {

struct PlatformImpl {
  PlatformImpl() noexcept
  {
    if (!glfwInit()) {
      beyond::panic("Cannot initialize GLFW platform");
    }
  }

  ~PlatformImpl() noexcept
  {
    glfwTerminate();
  }
};

Platform::Platform() noexcept : pimpl_{std::make_unique<PlatformImpl>()} {}

Platform::~Platform() noexcept = default;

auto Platform::poll_events() noexcept -> void
{
  glfwPollEvents();
}

Window::~Window() noexcept = default;

struct WindowImpl {
  GLFWwindow* data_ = nullptr;

  WindowImpl(GLFWwindow* data) : data_{data} {}

  auto swap_buffers() -> void
  {
    glfwSwapBuffers(data_);
  }

  [[nodiscard]] auto should_close() const noexcept -> bool
  {
    return static_cast<bool>(glfwWindowShouldClose(data_));
  }
};

[[nodiscard]] auto Platform::create_window(int width, int height,
                                           std::string_view title) noexcept
    -> tl::expected<Window, PlatformError>
{
  auto glfw_window =
      glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
  if (glfw_window != nullptr) {
    return Window{std::string{title},
                  std::make_unique<WindowImpl>(glfw_window)};
  } else {
    return tl::unexpected{PlatformError::cannot_create_window};
  }
}

void Platform::make_context_current(const Window& window) noexcept
{
  glfwMakeContextCurrent(window.pimpl_->data_);
}

Window::Window(std::string title, std::unique_ptr<WindowImpl>&& impl) noexcept
    : title_{std::move(title)}, pimpl_{std::move(impl)}
{
}

auto Window::swap_buffers() -> void
{
  pimpl_->swap_buffers();
}

[[nodiscard]] auto Window::should_close() const noexcept -> bool
{
  return pimpl_->should_close();
}

} // namespace beyond
