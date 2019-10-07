#pragma once

#ifndef BEYOND_GRAPHICS_VULKAN_CONTEXT_HPP
#define BEYOND_GRAPHICS_VULKAN_CONTEXT_HPP

#include <vulkan_fwd.hpp>

#include <volk.h>

#include <beyond/core/utils/panic.hpp>
#include <beyond/platform/platform.hpp>

#include <beyond/graphics/backend.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <optional>
#include <set>
#include <vector>

namespace beyond::graphics::vulkan {

struct QueueFamilyIndices {
  std::uint32_t graphics_family;

  [[nodiscard]] auto to_set() const noexcept -> std::set<std::uint32_t>
  {
    return std::set{graphics_family};
  }
};

[[nodiscard]] auto find_queue_families(VkPhysicalDevice device) noexcept
    -> std::optional<QueueFamilyIndices>;

class VulkanContext : public Context {
public:
  VulkanContext(const Window& window);
  ~VulkanContext();

private:
  VkInstance instance_;

  VkSurfaceKHR surface_;

#ifdef BEYOND_VULKAN_ENABLE_VALIDATION_LAYER
  VkDebugUtilsMessengerEXT debug_messager_;
#endif

  VkPhysicalDevice physical_device_;
  vulkan::QueueFamilyIndices queue_family_indices_;
  VkDevice device_;
};

} // namespace beyond::graphics::vulkan

#endif // BEYOND_GRAPHICS_VULKAN_CONTEXT_HPP
