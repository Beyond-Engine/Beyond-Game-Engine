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

  fmt::print("Done compute!\n");
  std::fflush(stdout);

  return 0;
}
