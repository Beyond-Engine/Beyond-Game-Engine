#include <fmt/format.h>

#include <beyond/core/utils/panic.hpp>
#include <beyond/graphics/backend.hpp>

#include <beyond/platform/platform.hpp>

#include <algorithm>
#include <cassert>
#include <random>

constexpr int initial_width = 1024;
constexpr int initial_height = 800;

int main()
{
  using namespace beyond;

  // Setting up
  Window window(initial_width, initial_height, "Test");
  const auto context = beyond::graphics::create_context(window);
  if (!context) {
    std::fputs("Error: Cannot create Graphics context\n", stderr);
    std::exit(1);
  }

  static constexpr std::uint32_t buffer_size = 1024;
  static constexpr auto payload_size = buffer_size / sizeof(int32_t);

  // Create buffers
  auto in_handle = context->create_buffer(
      {.size = buffer_size,
       .memory_usage = graphics::MemoryUsage::host_to_device});

  auto out_handle = context->create_buffer(
      {.size = buffer_size,
       .memory_usage = graphics::MemoryUsage::device_to_host});

  // Create pipeline
  const auto pipeline_handle =
      context->create_compute_pipeline(graphics::ComputePipelineCreateInfo{});

  {
    // Filling input buffer
    auto in_payload = context->map_memory<std::int32_t>(in_handle);
    std::random_device rd;
    std::uniform_int_distribution<std::int32_t> dist;
    std::generate_n(in_payload.begin(), payload_size,
                    [&]() { return dist(rd); });

    // Compute
    std::vector<graphics::SubmitInfo> infos;
    infos.push_back({in_handle, out_handle, buffer_size, pipeline_handle});
    context->submit(infos);

    // Done
    std::puts("Done compute");
    auto out_payload = context->map_memory<std::int32_t>(out_handle);
    if (!std::equal(in_payload.begin(), in_payload.end(),
                    out_payload.begin())) {
      std::fputs("Error: incorrect compute result", stderr);
    }
  }

  // It is okay let the context destorying buffers itself on destruction, but we
  // can manually destory buffers
  context->destory_buffer(in_handle);
  context->destory_buffer(out_handle);

  return 0;
}
