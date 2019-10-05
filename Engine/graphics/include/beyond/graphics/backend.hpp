#pragma once

#ifndef BEYOND_GRAPHICS_BACKEND_HPP
#define BEYOND_GRAPHICS_BACKEND_HPP

/**
 * @file backend.hpp
 * @brief Interface of the graphics backend
 */

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

/// @brief Graphics context
///
/// The actual definition is depend on the rendering backend.
class Context;

/// @brief Create a graphics context
[[nodiscard]] auto create_context(const Window& window) noexcept -> Context*;

/// @brief destory a graphics context
auto destory_context(Context* context) noexcept -> void;

/** @}@} */

} // namespace beyond::graphics

#endif // BEYOND_GRAPHICS_BACKEND_HPP
