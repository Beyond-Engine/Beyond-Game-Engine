#pragma once

#ifndef BEYOND_GRAPHICS_VULKAN_CONTEXT_HPP
#define BEYOND_GRAPHICS_VULKAN_CONTEXT_HPP

#include <volk.h>

#include <beyond/core/utils/panic.hpp>
#include <beyond/platform/platform.hpp>

#include "vulkan_utils.hpp"

#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <optional>
#include <set>
#include <vector>

namespace beyond::graphics {
namespace vulkan {

struct QueueFamilyIndices {
  std::uint32_t graphics_family;

  [[nodiscard]] auto to_set() const noexcept -> std::set<std::uint32_t>
  {
    return std::set{graphics_family};
  }
};

[[nodiscard]] auto find_queue_families(VkPhysicalDevice device) noexcept
    -> std::optional<QueueFamilyIndices>;
} // namespace vulkan

class Context {
public:
  Context(const Window& window);
  ~Context();

  Context(const Context&) = delete;
  auto operator=(const Context&) -> Context& = delete;
  Context(Context&&) = delete;
  auto operator=(Context &&) -> Context& = delete;

private:
  VkInstance instance_;

#ifdef BEYOND_VULKAN_ENABLE_VALIDATION_LAYER
  VkDebugUtilsMessengerEXT debug_messager_;
#endif

  VkPhysicalDevice physical_device_;
  vulkan::QueueFamilyIndices queue_family_indices_;
  VkDevice device_;
};

} // namespace beyond::graphics

#endif // BEYOND_GRAPHICS_VULKAN_CONTEXT_HPP
