#pragma once

#ifndef BEYOND_GRAPHICS_D3D12_HPP
#define BEYOND_GRAPHICS_D3D12_HPP

#include <beyond/graphics/backend.hpp>
#include <memory>

namespace beyond::graphics::d3d12 {

/// @brief Create a D3D12 context
[[nodiscard]] auto create_d3d12_context(Window& window) noexcept
    -> std::unique_ptr<Context>;

} // namespace beyond::graphics::d3d12

#endif // BEYOND_GRAPHICS_D3D12_HPP
