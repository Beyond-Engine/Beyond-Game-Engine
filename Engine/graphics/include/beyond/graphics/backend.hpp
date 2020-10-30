#pragma once

#ifndef BEYOND_GRAPHICS_BACKEND_HPP
#define BEYOND_GRAPHICS_BACKEND_HPP

/**
 * @file backend.hpp
 * @brief Interface of the graphics backend
 */

#include <memory>

#include <gsl/span>

#include "beyond/core/utils/named_type.hpp"
#include "beyond/platform/platform.hpp"

#define DEFINE_HANDLE(type)                                                    \
  struct type {                                                                \
    std::uint64_t id = 0;                                                      \
                                                                               \
    [[nodiscard]] constexpr friend auto operator==(type lhs, type rhs)         \
        -> bool = default;                                                     \
  };

namespace beyond::graphics {

/**
 * @addtogroup graphics
 * @{
 */

/**
 * @defgroup backend Backend
 * @brief Interface between the underlying graphics API and high level
 * graphics codes
 *
 * @{
 */

/// @brief A handle to a GPU Swapchain
DEFINE_HANDLE(GPUSwapchain)

/// @brief The place of buffer memory it reside in
enum struct MemoryUsage {
  /**
   * Memory will be used on device only. There are no garentee this
   memory will be mappable on the host.
   *
   * Usage:
   * - Resources written and read by device, e.g. images used as attachments.
   * - Resources transferred from host once (immutable) or infrequently and read
   by device multiple times, e.g. textures to be sampled, vertex buffers,
   and uniform buffers.
   */
  device,
  /**
   * Memory will be mappable on host. It usually means CPU memory. Resources
   * created in this pool may still be accessible to the device, but access to
   * them can be slow.
   * Usage: Staging copy of resources used as transfer source.
   */
  host,
  /**
   * Memory that is both mappable on host and preferably fast to access by GPU.
   * CPU access is typically uncached. Writes may be write-combined.
   *
   * Usage: Resources written frequently by host (dynamic), read by device. E.g.
   * textures, vertex buffers, uniform buffers updated every frame or every draw
   * call.
   */
  host_to_device,
  /**
   *Memory mappable on host and cached.
   *
   *Usage:
   * - Resources written by device, read by host - results of some computations,
   *e.g. screen capture, average scene luminance for HDR tone mapping. Any
   *resources read or accessed randomly on host, e.g. CPU-side copy of vertex
   *buffer used as source of transfer, but also used for collision detection.
   */
  device_to_host,
};

/**
 * @brief The information used to create a GPU buffer
 */
struct BufferCreateInfo {
  std::uint32_t size = 0;
  MemoryUsage memory_usage = MemoryUsage::device;
};

/// @brief A handle to a GPU buffer
DEFINE_HANDLE(Buffer)

struct ComputePipelineCreateInfo {
};

/// @brief A handle to a GPU compute pipeline
DEFINE_HANDLE(ComputePipeline)

/// @brief Structure specifying a commands submit operation
struct SubmitInfo {
  // Temporaraily hack around
  Buffer input{};
  Buffer output{};
  std::uint32_t buffer_size{};
  ComputePipeline pipeline;
};

/**
 * @brief Interface of the graphics context
 */
class GPUDevice {
public:
  virtual ~GPUDevice();

  GPUDevice(const GPUDevice&) = delete;
  auto operator=(const GPUDevice&) -> GPUDevice& = delete;
  GPUDevice(GPUDevice&&) = delete;
  auto operator=(GPUDevice&&) -> GPUDevice& = delete;

  [[nodiscard]] virtual auto create_swapchain(std::uint32_t width,
                                              std::uint32_t height)
      -> GPUSwapchain = 0;

  virtual void destroy_swapchain(GPUSwapchain swapchain) = 0;
  [[nodiscard]] virtual auto
  get_swapchain_back_buffer_index(GPUSwapchain swapchain) -> std::uint32_t = 0;
  virtual void resize_swapchain(GPUSwapchain& swapchain, std::uint32_t width,
                                std::uint32_t height) = 0;

  [[nodiscard]] virtual auto create_buffer(const BufferCreateInfo& create_info)
      -> Buffer = 0;

  /**
   * @brief Creates a compute pipeline
   * @note There are not way to destory a pipeline manually. All pipelines will
   * be destoryed when the context get destoryed.
   */
  [[nodiscard]] virtual auto
  create_compute_pipeline(const ComputePipelineCreateInfo& create_info)
      -> ComputePipeline = 0;

  /**
   * @brief Destories the device buffer.
   *
   * This function destories the device buffer refered to by `buffer_handle` and
   * release its underlying memory. If the `buffer_handle` does not refer to a
   * valid device buffer, this function will do nothing.
   */
  virtual auto destory_buffer(Buffer& buffer_handle) -> void = 0;

  /// @brief Submits a sequence of command buffers to execute
  virtual auto submit(gsl::span<SubmitInfo> infos) -> void = 0;

  /**
   * @brief Maps the underlying memory of buffer to a pointer
   *
   * After a successful call to `map` buffer memory is
   * considered to be currently host mapped. If the `buffer` handle does not
   * refer to an real buffer, or if its underlying memory is not host visible,
   * this function will return `nullptr`
   */
  [[nodiscard]] virtual auto map(Buffer buffer) noexcept -> void* = 0;

  /**
   * @brief Unmaps the underlying memory of buffer
   */
  virtual auto unmap(Buffer buffer) noexcept -> void = 0;

  // Temporary hacks
  virtual auto render(GPUSwapchain) -> void {}
  virtual void resize(GPUSwapchain& swapchain, unsigned width, unsigned height)
  {
  }
  virtual void initialize_resources(GPUSwapchain swapchain) {}
  virtual void setup_commands() {}

protected:
  GPUDevice() = default;

  template <typename T> friend class Mapping;

  struct MappingInfo {
    void* data = nullptr;
    std::size_t size = 0; // In bytes
  };
};

/// @brief Create a graphics context
[[nodiscard]] auto create_gpu_device(Window& window) noexcept
    -> std::unique_ptr<GPUDevice>;

/** @}@} */

} // namespace beyond::graphics

#endif // BEYOND_GRAPHICS_BACKEND_HPP
