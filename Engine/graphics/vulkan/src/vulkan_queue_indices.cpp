#include "vulkan_queue_indices.hpp"
#include "vulkan_utils.hpp"

namespace beyond::graphics::vulkan {

auto find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) noexcept
    -> std::optional<QueueFamilyIndices>
{
  std::optional<std::uint32_t> graphics_family;
  std::optional<std::uint32_t> present_family;

  const auto queue_families = get_vector_with<VkQueueFamilyProperties>(
      [device](uint32_t* count, VkQueueFamilyProperties* data) {
        vkGetPhysicalDeviceQueueFamilyProperties(device, count, data);
      });

  for (std::uint32_t i = 0; i < queue_families.size(); ++i) {
    const auto& queue_family = queue_families[i];

    if (queue_family.queueCount > 0 &&
        ((queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u)) {
      graphics_family = i;
    }

    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
    if (queue_family.queueCount > 0 && present_support) {
      present_family = i;
    }

    if (graphics_family && graphics_family) {
      return QueueFamilyIndices{*graphics_family, *present_family};
    }
  }

  return {};
}

} // namespace beyond::graphics::vulkan
