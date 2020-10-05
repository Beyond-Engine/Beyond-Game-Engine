#pragma once

#ifndef BEYOND_GRAPHICS_VULKAN_PIPELINE_HPP
#define BEYOND_GRAPHICS_VULKAN_PIPELINE_HPP

#include <utility>
#include <volk.h>

#include <beyond/graphics/backend.hpp>

namespace beyond::graphics::vulkan {

class VulkanPipeline {
public:
  static auto create_compute(ComputePipelineCreateInfo info, VkDevice device)
      -> VulkanPipeline;

  ~VulkanPipeline() noexcept;
  VulkanPipeline(const VulkanPipeline&) = delete;
  auto operator=(const VulkanPipeline&) & = delete;

  VulkanPipeline(VulkanPipeline&& other) noexcept
      : device_{std::exchange(other.device_, {})},
        descriptor_set_layout_{
            std::exchange(other.descriptor_set_layout_, {})},
        pipeline_layout_{std::exchange(other.pipeline_layout_, {})},
        pipeline_{std::exchange(other.pipeline_, {})}
  {
  }

  auto operator=(VulkanPipeline&& other) & noexcept
  {
    device_ = std::exchange(other.device_, {});
    descriptor_set_layout_ =
        std::exchange(other.descriptor_set_layout_, {});
    pipeline_layout_ = std::exchange(other.pipeline_layout_, {});
    pipeline_ = std::exchange(other.pipeline_, {});
  }

  [[nodiscard]] auto descriptor_set_layout() const noexcept
  {
    return descriptor_set_layout_;
  }

  [[nodiscard]] auto pipeline_layout() const noexcept
  {
    return pipeline_layout_;
  }

  [[nodiscard]] auto pipeline() const noexcept
  {
    return pipeline_;
  }

private:
  explicit VulkanPipeline(VkDevice device,
                          VkDescriptorSetLayout descriptor_set_layout,
                          VkPipelineLayout pipeline_layout, VkPipeline pipeline)
      : device_{device}, descriptor_set_layout_{descriptor_set_layout},
        pipeline_layout_{pipeline_layout}, pipeline_{pipeline}
  {
  }

  VkDevice device_{};
  VkDescriptorSetLayout descriptor_set_layout_{};
  VkPipelineLayout pipeline_layout_{};
  VkPipeline pipeline_{};
};

} // namespace beyond::graphics::vulkan

#endif // BEYOND_GRAPHICS_VULKAN_PIPELINE_HPP
