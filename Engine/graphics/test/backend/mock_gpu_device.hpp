#pragma once

#ifndef BEYOND_GRAPHICS_TEST_MOCK_BACKEND_HPP
#define BEYOND_GRAPHICS_TEST_MOCK_BACKEND_HPP

#include <beyond/core/utils/bit_cast.hpp>

#include <cstdio>
#include <memory>

#include <beyond/graphics/backend.hpp>

#include <gsl/span>

namespace beyond::graphics {

class MockGPUDevice : public GPUDevice {
  using MockBuffer = std::vector<std::byte>;

public:
  MockGPUDevice()
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
    const auto index = buffers_.size();
    buffers_.emplace_back(static_cast<std::size_t>(info.size));
    return Buffer{index};
  }

  auto destory_buffer(Buffer& buffer_handle) -> void override
  {
    const auto index = buffer_handle.id;
    if (index >= buffers_.size()) {
      return;
    }

    buffers_[index].clear();
  }

  [[nodiscard]] auto create_compute_pipeline(const ComputePipelineCreateInfo&
                                             /*create_info*/)
      -> ComputePipeline override
  {
    return ComputePipeline{0};
  }

  auto submit(gsl::span<SubmitInfo>) -> void override {}

  [[nodiscard]] auto map(Buffer buffer) noexcept -> void* override
  {
    const auto index = buffer.id;
    if (index >= buffers_.size()) {
      return nullptr;
    }

    if (buffers_[index].empty()) {
      return nullptr;
    }

    return buffers_[index].data();
  }

  auto unmap(Buffer) noexcept -> void override {}

private:
  std::vector<MockBuffer> buffers_{};
};

} // namespace beyond::graphics

#endif // BEYOND_GRAPHICS_TEST_MOCK_BACKEND_HPP
