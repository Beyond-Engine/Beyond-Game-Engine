#pragma once

#ifndef BEYOND_GRAPHICS_BACKEND_HPP
#define BEYOND_GRAPHICS_BACKEND_HPP

/**
 * @file graphics_backend.hpp
 * @brief Interface of the graphics backend
 */

#include <memory>

#include "beyond/platform/platform.hpp"

namespace beyond::graphics {

/**
 * @addtogroup graphics
 * @{
 * @addtogroup backend
 * @{
 */

class ContextImpl;

/// @brief Graphics context
class Context {
public:
    ~Context();

    Context(const Context&) = delete;
    auto operator=(const Context&) -> Context& = delete;

    Context(Context&&) = delete;
    auto operator=(Context &&) -> Context& = delete;

private:
    Context(std::unique_ptr<ContextImpl>&& impl);

    friend auto create_context(Window& window) noexcept -> Context;

    std::unique_ptr<ContextImpl> impl_;
};

/// @brief Create a graphics context
[[nodiscard]] auto create_context(Window& window) noexcept -> Context;

/** @}@} */

} // namespace beyond

#endif // BEYOND_GRAPHICS_BACKEND_HPP
