#pragma once

#ifndef BEYOND_GRAPHICS_BACKEND_HPP
#define BEYOND_GRAPHICS_BACKEND_HPP

/**
 * @file backend.hpp
 * @brief Interface of the graphics backend
 */

#include <memory>

#include "beyond/platform/platform.hpp"

namespace beyond::graphics {

/**
 * @addtogroup graphics
 * @{
 */

/**
 * @defgroup backend Backend
 * @brief Interface between the underlying graphics API and high level graphics
 * codes
 *
 * @{
 */

enum class Backend {
  mock = 0,
#ifdef BEYOND_BUILD_VULKAN_BACKEND
  vulkan
#endif
};

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

protected:
  Context() = default;
};

/// @brief Create a graphics context
[[nodiscard]] auto create_context(Backend backend,
                                  const Window& window) noexcept
    -> std::unique_ptr<Context>;

/** @}@} */

} // namespace beyond::graphics

#endif // BEYOND_GRAPHICS_BACKEND_HPP
