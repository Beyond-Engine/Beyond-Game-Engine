#pragma once

#ifndef BEYOND_GRAPHICS_UNIQUE_HANDLES_HPP
#define BEYOND_GRAPHICS_UNIQUE_HANDLES_HPP

/**
 * @file unique_handles.hpp
 * @brief Unique handle wrappers for the graphics backend objects
 */

#include <memory>

#include "backend.hpp"

namespace beyond::graphics {

/**
 * @addtogroup graphics
 * @{
 * @addtogroup backend
 * @{
 */

[[nodiscard]] inline auto make_unique_context(Window& window) noexcept
{
  return std::unique_ptr<Context, decltype(&destory_context)>(
      create_context(window), &destory_context);
}

/** @}@} */

} // namespace beyond::graphics

#endif // BEYOND_GRAPHICS_UNIQUE_HANDLES_HPP
