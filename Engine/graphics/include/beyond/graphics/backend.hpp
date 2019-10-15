#pragma once

#ifndef BEYOND_GRAPHICS_BACKEND_HPP
#define BEYOND_GRAPHICS_BACKEND_HPP

/**
 * @file backend.hpp
 * @brief Interface of the graphics backend
 */

#include <memory>

#include "beyond/core/utils/handle.hpp"
#include "beyond/core/utils/named_type.hpp"
#include "beyond/platform/platform.hpp"

namespace beyond::graphics {

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

/**
 * @brief Interface of the graphics context
 */
class Context {
public:
  virtual ~Context() = default;

  Context(const Context&) = delete;
  auto operator=(const Context&) -> Context& = delete;
  Context(Context&&) = delete;
  auto operator=(Context &&) -> Context& = delete;

  [[nodiscard]] virtual auto create_swapchain() -> Swapchain = 0;

  [[nodiscard]] virtual auto create_buffer(const BufferCreateInfo& create_info)
      -> Buffer = 0;

protected:
  Context() = default;
};

/// @brief Create a graphics context
[[nodiscard]] auto create_context(Window& window) noexcept
    -> std::unique_ptr<Context>;

/** @}@} */

} // namespace beyond::graphics

#endif // BEYOND_GRAPHICS_BACKEND_HPP
