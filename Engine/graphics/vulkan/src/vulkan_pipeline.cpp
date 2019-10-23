#include <beyond/core/utils/panic.hpp>

#include <array>

#include "vulkan_pipeline.hpp"
#include "vulkan_shader_module.hpp"
#include "vulkan_utils.hpp"

namespace beyond::graphics::vulkan {

auto VulkanPipeline::create_compute(ComputePipelineCreateInfo /*info*/,
                                    VkDevice device) -> VulkanPipeline
{
  const auto shader_module =
      create_shader_module("shaders/copy.comp.spv", device);

  std::array descriptor_set_layout_bindings{
      VkDescriptorSetLayoutBinding{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                   VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
      VkDescriptorSetLayoutBinding{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                   VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};

  const VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .bindingCount = vulkan::to_u32(descriptor_set_layout_bindings.size()),
      .pBindings = descriptor_set_layout_bindings.data()};

  VkDescriptorSetLayout descriptor_set_layout;
  if (vkCreateDescriptorSetLayout(device, &descriptor_set_layout_create_info,
                                  nullptr,
                                  &descriptor_set_layout) != VK_SUCCESS) {
    beyond::panic("Vulkan backend failed to create descriptor set layout");
  }

  const VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .setLayoutCount = 1,
      .pSetLayouts = &descriptor_set_layout,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr};

  VkPipelineLayout pipeline_layout;
  if (vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr,
                             &pipeline_layout) != VK_SUCCESS) {
    beyond::panic("Vulkan backend failed to create pipeline layout");
  }

  const VkComputePipelineCreateInfo compute_pipeline_create_info{
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .stage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
                VK_SHADER_STAGE_COMPUTE_BIT, shader_module, "main", nullptr},
      .layout = pipeline_layout,
      .basePipelineHandle = nullptr,
      .basePipelineIndex = 0,
  };

  VkPipeline pipeline;
  if (vkCreateComputePipelines(device, nullptr, 1,
                               &compute_pipeline_create_info, nullptr,
                               &pipeline) != VK_SUCCESS) {
    beyond::panic("Vulkan backend failed to create compute pipeline");
  }

  return VulkanPipeline{device, descriptor_set_layout, pipeline_layout,
                        pipeline};
}

VulkanPipeline::~VulkanPipeline() noexcept
{
  if (device_) {
    vkDestroyPipeline(device_, pipeline_, nullptr);
    vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
    vkDestroyDescriptorSetLayout(device_, descriptor_set_layout_, nullptr);
  }

  // vkDestroyShaderModule(device_, shader_module, nullptr);
}

} // namespace beyond::graphics::vulkan
