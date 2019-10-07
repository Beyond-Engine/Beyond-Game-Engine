// Supress windows Macro redefinition of GLFW
#ifdef WIN32
#include <windows.h>
#endif

#include <algorithm>

#include <beyond/core/utils/panic.hpp>
#include <beyond/platform/platform.hpp>

#ifdef BEYOND_GRAPHICS_BACKEND_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

namespace beyond {

struct PlatformImpl {
  PlatformImpl() noexcept
  {
    if (glfwInit() == 0) {
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

  explicit WindowImpl(GLFWwindow* data) : data_{data} {}
};

[[nodiscard]] auto Platform::create_window(int width, int height,
                                           std::string_view title) noexcept
    -> Window
{
#ifdef BEYOND_GRAPHICS_BACKEND_VULKAN
  return create_window(width, height, title, GraphicsBackend::vulkan);
#else
  return create_window(width, height, title, GraphicsBackend::mock);
#endif
}

[[nodiscard]] auto Platform::create_window(int width, int height,
                                           std::string_view title,
                                           GraphicsBackend backend) noexcept
    -> Window
{
#ifdef BEYOND_GRAPHICS_BACKEND_VULKAN
  if (backend == GraphicsBackend::vulkan) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  }
#endif

  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  auto glfw_window =
      glfwCreateWindow(width, height, title.data(), nullptr, nullptr);

  if (!glfw_window) {
    beyond::panic("Cannot Create a GLFW Window");
  }

  return Window{std::string{title}, backend,
                std::make_unique<WindowImpl>(glfw_window)};
}

void Platform::make_context_current(const Window& window) noexcept
{
  glfwMakeContextCurrent(window.pimpl_->data_);
}

Window::Window(std::string title, GraphicsBackend backend,
               std::unique_ptr<WindowImpl>&& impl) noexcept
    : title_{std::move(title)}, backend_{backend}, pimpl_{std::move(impl)}
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
    -> std::vector<const char*>
{
  uint32_t glfw_extension_count = 0;
  const char** glfw_extensions;
  glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

  std::vector<const char*> extensions;
  std::copy_n(glfw_extensions, glfw_extension_count,
              std::back_inserter(extensions));
  return extensions;
}

auto Window::create_vulkan_surface(VkInstance instance,
                                   const VkAllocationCallbacks* allocator,
                                   VkSurfaceKHR& surface) const noexcept -> void
{
  if (glfwCreateWindowSurface(instance, pimpl_->data_, allocator, &surface) !=
      VK_SUCCESS) {
    beyond::panic("Failed to create Vulkan window surface!");
  }
}
#endif

} // namespace beyond
