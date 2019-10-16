#pragma once

#ifndef BEYOND_GRAPHICS_BACKEND_HPP
#define BEYOND_GRAPHICS_BACKEND_HPP

/**
 * @file backend.hpp
 * @brief Interface of the graphics backend
 */

#include <memory>

#include <gsl/span>

#include "beyond/core/utils/handle.hpp"
#include "beyond/core/utils/named_type.hpp"
#include "beyond/platform/platform.hpp"

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

struct Swapchain : beyond::NamedType<std::uint32_t, struct SwapchainTag,
                                     beyond::EquableBase> {
  using NamedType::NamedType;
};

/**
 * @brief The information used to create a GPU buffer
 */
struct BufferCreateInfo {
  std::uint32_t size = 0;
};

/// @brief A handle to buffer
struct Buffer : Handle<Buffer, std::uint32_t, 20, 12> {
  using Handle::Handle;
};

/// @brief Structure specifying a commands submit operation
struct SubmitInfo {
  // Temporaraily hack around
  Buffer input{};
  Buffer output{};
  std::uint32_t buffer_size{};
};

/**
 * @brief Interface of the graphics context
 */
class Context {
public:
  virtual ~Context();

  Context(const Context&) = delete;
  auto operator=(const Context&) -> Context& = delete;
  Context(Context&&) = delete;
  auto operator=(Context &&) -> Context& = delete;

  [[nodiscard]] virtual auto create_swapchain() -> Swapchain = 0;

  [[nodiscard]] virtual auto create_buffer(const BufferCreateInfo& create_info)
      -> Buffer = 0;

  /**
   * @brief Maps the underlying memory of buffer to a pointer
   *
   * After a successful call to `map_memory` the buffer memory is
   * considered to be currently host mapped. If the `buffer` handle does not
   * refer to an real buffer, or if its underlying memory is not host visible,
   * this function will return `nullptr`
   */
  [[nodiscard]] virtual auto map_memory(Buffer buffer) noexcept -> void* = 0;

  /**
   * @brief Unmaps the underlying memory  of buffer
   */
  virtual auto unmap_memory(Buffer buffer) noexcept -> void = 0;

  /// @brief Submits a sequence of command buffers to execute
  virtual auto submit(gsl::span<SubmitInfo> infos) -> void = 0;

protected:
  Context() = default;
};

/// @brief Create a graphics context
[[nodiscard]] auto create_context(Window& window) noexcept
    -> std::unique_ptr<Context>;

/** @}@} */

} // namespace beyond::graphics

#endif // BEYOND_GRAPHICS_BACKEND_HPP
