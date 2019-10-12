#include <fmt/format.h>

#include <beyond/graphics/backend.hpp>

#include <beyond/platform/platform.hpp>

#include <cassert>

constexpr int initial_width = 1024;
constexpr int initial_height = 800;

int main()
{
  beyond::Window window(initial_width, initial_height, "Test");
  const auto graphics_context = beyond::graphics::create_context(window);
  assert(graphics_context);

  [[maybe_unused]] const auto swapchain = graphics_context->create_swapchain();

  while (!window.should_close()) {
    // render
    window.swap_buffers();
    window.poll_events();
  }

  return 0;
}
