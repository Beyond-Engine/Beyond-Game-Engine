#include <beyond/graphics/backend.hpp>
#include <beyond/platform/platform.hpp>

constexpr int initial_width = 1024;
constexpr int initial_height = 800;

int main()
{
  // Setting up
  beyond::Window window(initial_width, initial_height, "Triangle");
  const auto device = beyond::graphics::create_gpu_device(window);
  // const auto swapchain = device->create_swapchain();

  while (!window.should_close()) {
    window.poll_events();
    window.swap_buffers();
  }

  return 0;
}
