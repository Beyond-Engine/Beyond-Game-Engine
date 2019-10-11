#include <fmt/format.h>

#include <beyond/graphics/backend.hpp>

#include <beyond/platform/platform.hpp>

constexpr int initial_width = 1024;
constexpr int initial_height = 800;

int main()
{
  beyond::Platform platform;
  auto window = platform.create_window(initial_width, initial_height, "Test");
  platform.make_context_current(window);
  const auto graphics_context = beyond::graphics::create_context(window);
  [[maybe_unused]] const auto swapchain = graphics_context->create_swapchain();

  while (!window.should_close()) {
    // render
    window.swap_buffers();
    platform.poll_events();
  }

  return 0;
}
