#pragma once

#ifndef BEYOND_GRAPHICS_VULKAN_CONTEXT_HPP
#define BEYOND_GRAPHICS_VULKAN_CONTEXT_HPP

#include <vulkan_fwd.hpp>

#include <volk.h>

#include <vk_mem_alloc.h>

#include <beyond/core/container/static_vector.hpp>
#include <beyond/core/utils/panic.hpp>
#include <beyond/platform/platform.hpp>

#include <beyond/graphics/backend.hpp>

#include "vulkan_pipeline.hpp"
#include "vulkan_swapchain.hpp"

#include <algorithm>
#include <array>
#include <optional>
#include <vector>

namespace beyond::graphics::vulkan {

class VulkanGPUDevice final : public GPUDevice {
public:
  explicit VulkanGPUDevice(Window& window);
  ~VulkanGPUDevice() noexcept override;

  [[nodiscard]] auto create_swapchain() -> Swapchain override;
  [[nodiscard]] auto create_buffer(const BufferCreateInfo& create_info)
      -> Buffer override;
  auto destory_buffer(Buffer& buffer) -> void override;

  [[nodiscard]] auto
  create_compute_pipeline(const ComputePipelineCreateInfo& create_info)
      -> ComputePipeline override;

  auto submit(gsl::span<SubmitInfo> infos) -> void override;

private:
  VkInstance instance_{};

  VkSurfaceKHR surface_{};

#ifdef BEYOND_VULKAN_ENABLE_VALIDATION_LAYER
  VkDebugUtilsMessengerEXT debug_messager_{};
#endif

  VkPhysicalDevice physical_device_{};
  vulkan::QueueFamilyIndices queue_family_indices_{};
  VkDevice device_{};

  VkQueue graphics_queue_{};
  VkQueue present_queue_{};
  VkQueue compute_queue_{};

  VmaAllocator allocator_{};

  beyond::static_vector<VulkanSwapchain, 2> swapchains_pool_;
  std::unordered_map<VkBuffer, VmaAllocation> buffer_allocations_;
  std::vector<VulkanPipeline> compute_pipelines_pool_;

  [[nodiscard]] auto map(Buffer buffer_handle) noexcept -> void* override;
  auto unmap(Buffer buffer_handle) noexcept -> void override;
};

} // namespace beyond::graphics::vulkan

#endif // BEYOND_GRAPHICS_VULKAN_CONTEXT_HPP
