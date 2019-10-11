#pragma once

#ifndef BEYOND_GRAPHICS_VULKAN_CONTEXT_HPP
#define BEYOND_GRAPHICS_VULKAN_CONTEXT_HPP

#include <vulkan_fwd.hpp>

#include <volk.h>

#include <vk_mem_alloc.h>

#include <beyond/core/utils/panic.hpp>
#include <beyond/platform/platform.hpp>

#include <beyond/graphics/backend.hpp>

#include "vulkan_swapchain.hpp"

#include <algorithm>
#include <array>
#include <optional>
#include <vector>

namespace beyond::graphics::vulkan {

class VulkanContext : public Context {
public:
  explicit VulkanContext(const Window& window);
  ~VulkanContext() override;

  [[nodiscard]] auto create_swapchain() -> Swapchain override;

private:
  VkInstance instance_ = nullptr;

  VkSurfaceKHR surface_ = nullptr;

#ifdef BEYOND_VULKAN_ENABLE_VALIDATION_LAYER
  VkDebugUtilsMessengerEXT debug_messager_ = nullptr;
#endif

  VkPhysicalDevice physical_device_ = nullptr;
  vulkan::QueueFamilyIndices queue_family_indices_;
  VkDevice device_ = nullptr;

  VkQueue graphics_queue_ = nullptr;
  VkQueue present_queue_ = nullptr;
  VkQueue compute_queue_ = nullptr;

  VmaAllocator allocator_ = nullptr;

  std::vector<VulkanSwapchain> swapchains_;
};

} // namespace beyond::graphics::vulkan

#endif // BEYOND_GRAPHICS_VULKAN_CONTEXT_HPP
