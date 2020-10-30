#include <beyond/graphics/backend.hpp>
#include <beyond/platform/platform.hpp>

constexpr int initial_width = 1024;
constexpr int initial_height = 800;

int main()
{
  // Setting up
  beyond::Window window(initial_width, initial_height, "Triangle");
  auto device = beyond::graphics::create_gpu_device(window);
  const auto res = window.get_resolution();
  const auto width = static_cast<std::uint32_t>(res.width);
  const auto height = static_cast<std::uint32_t>(res.height);
  auto swapchain = device->create_swapchain(width, height);
  device->resize(swapchain, width, height);
  device->initialize_resources(swapchain);
  device->setup_commands();

  while (!window.should_close()) {
    window.poll_events();
    window.swap_buffers();

    device->render(swapchain);
  }

  device->destroy_swapchain(swapchain);

  return 0;
}
