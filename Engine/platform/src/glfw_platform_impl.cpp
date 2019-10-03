#include <GLFW/glfw3.h>

#include <beyond/core/utils/panic.hpp>
#include <beyond/platform/platform.hpp>

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
};

[[nodiscard]] auto Platform::create_window(int width, int height,
                                           std::string_view title) noexcept
    -> Window
{
  auto glfw_window =
      glfwCreateWindow(width, height, title.data(), nullptr, nullptr);

  if (!glfw_window) {
    beyond::panic("Cannot Create a GLFW Window");
  }

  return Window{std::string{title}, std::make_unique<WindowImpl>(glfw_window)};
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
  glfwSwapBuffers(pimpl_->data_);
}

[[nodiscard]] auto Window::should_close() const noexcept -> bool
{
  return static_cast<bool>(glfwWindowShouldClose(pimpl_->data_));
}

#ifdef BEYOND_GRAPHICS_BACKEND_VULKAN
/// @brief Get the extensions needed for the vulkan instance
[[nodiscard]] auto Window::get_required_instance_extensions() const noexcept
    -> gsl::span<const char*>
{
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  return gsl::span(glfwExtensions, glfwExtensionCount);
}
#endif

} // namespace beyond
