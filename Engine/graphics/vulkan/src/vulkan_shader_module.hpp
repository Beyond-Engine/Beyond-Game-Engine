#pragma once

#ifndef BEYOND_GRAPHICS_VULKAN_SHADER_MODULE_HPP
#define BEYOND_GRAPHICS_VULKAN_SHADER_MODULE_HPP

#include <volk.h>

#include <string_view>
#include <vector>

namespace beyond::graphics::vulkan {

[[nodiscard]] auto create_shader_module(const std::string_view& filename,
                                        VkDevice device) -> VkShaderModule;

[[nodiscard]] auto create_shader_module(std::size_t size, const uint32_t*,
                                        VkDevice device) -> VkShaderModule;

} // namespace beyond::graphics::vulkan

#endif // BEYOND_GRAPHICS_VULKAN_SHADER_MODULE_HPP
