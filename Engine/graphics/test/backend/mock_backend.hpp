#pragma once

#ifndef BEYOND_GRAPHICS_TEST_MOCK_BACKEND_HPP
#define BEYOND_GRAPHICS_TEST_MOCK_BACKEND_HPP

#include <cstdio>
#include <memory>
#include <memory_resource>

#include <beyond/graphics/backend.hpp>

#include <gsl/span>

namespace beyond::graphics {

class MockContext : public Context {
  using MockBuffer = std::pmr::vector<std::byte>;

public:
  MockContext()
  {
    std::puts("Mock Graphics backend");
    std::fflush(stdout);
  }

  [[nodiscard]] auto create_swapchain() noexcept -> Swapchain override
  {
    return Swapchain{0};
  }

  [[nodiscard]] auto create_buffer(const BufferCreateInfo& info) noexcept
      -> Buffer override
  {
    const auto index = static_cast<Buffer::Index>(buffers_.size());
    buffers_.emplace_back(info.size);
    return Buffer{index};
  }

  auto destory_buffer(Buffer& buffer_handle) -> void override
  {
    const auto index = static_cast<Buffer::Index>(buffers_.size());
    if (index >= buffers_.size()) {
      return;
    }

    buffers_[index].clear();
  }

  auto submit(gsl::span<SubmitInfo>) -> void override {}

  [[nodiscard]] auto map_memory_impl(Buffer buffer) noexcept
      -> MappingInfo override
  {
    const auto index = buffer.index();
    if (index >= buffers_.size()) {
      return {nullptr, 0};
    }

    if (buffers_[index].empty()) {
      return {nullptr, 0};
    }

    return {buffers_[index].data(), buffers_[index].size()};
  }

  auto unmap_memory_impl(Buffer) noexcept -> void override {}

private:
  std::pmr::memory_resource& memory_resource_ =
      *std::pmr::get_default_resource();
  std::pmr::vector<MockBuffer> buffers_;
};

} // namespace beyond::graphics

#endif // BEYOND_GRAPHICS_TEST_MOCK_BACKEND_HPP
