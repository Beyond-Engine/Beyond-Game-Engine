#include <fmt/format.h>

#include <beyond/graphics/graphics_backend.hpp>
#include <beyond/platform/platform.hpp>

constexpr int initial_width = 1024;
constexpr int initial_height = 800;

int main()
{
  beyond::Platform platform;
  platform.create_window(initial_width, initial_height, "Test")
      .map([&platform](beyond::Window&& window) {
        platform.make_context_current(window);
        const auto graphics_context = beyond::graphics::create_context(window);

        while (!window.should_close()) {
          // render
          window.swap_buffers();
          platform.poll_events();
        }
      })
      .map_error([](beyond::PlatformError error) {
        switch (error) {
        case beyond::PlatformError::cannot_create_window:
          fmt::print("Cannot create window\n");
          std::exit(1);
        }
      });

  return 0;
}
