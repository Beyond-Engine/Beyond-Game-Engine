#include "vulkan_swapchain.hpp"
#include "vulkan_utils.hpp"

#include <beyond/core/utils/panic.hpp>

#include <algorithm>
#include <gsl/span>
#include <limits>

static constexpr std::uint32_t width = 1024;
static constexpr std::uint32_t height = 768;

namespace {

[[nodiscard]] auto choose_surface_format(
    const gsl::span<const VkSurfaceFormatKHR> available_formats)
{
  const auto prefered = std::find_if(
      std::begin(available_formats), std::end(available_formats),
      [](const VkSurfaceFormatKHR& format) {
        return format.format == VK_FORMAT_B8G8R8A8_UNORM &&
               format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      });

  return prefered != std::end(available_formats) ? *prefered
                                                 : available_formats[0];
}

[[nodiscard]] auto
choose_present_mode(const gsl::span<const VkPresentModeKHR> available_modes)
{
  const auto preferred =
      std::find(std::begin(available_modes), std::end(available_modes),
                VK_PRESENT_MODE_MAILBOX_KHR);
  return preferred != std::end(available_modes) ? *preferred
                                                : VK_PRESENT_MODE_FIFO_KHR;
}

[[nodiscard]] auto choose_extent(const VkSurfaceCapabilitiesKHR& capabilities)
    -> VkExtent2D
{
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    return {std::clamp(width, capabilities.minImageExtent.width,
                       capabilities.maxImageExtent.width),
            std::clamp(height, capabilities.minImageExtent.height,
                       capabilities.maxImageExtent.height)};
  }
}

} // namespace

namespace beyond::graphics::vulkan {

[[nodiscard]] auto query_swapchain_support(VkPhysicalDevice device,
                                           VkSurfaceKHR surface) noexcept
    -> SwapchainSupportDetails
{
  SwapchainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilities);

  details.formats = vulkan::get_vector_with<VkSurfaceFormatKHR>(
      [device, surface](uint32_t* count, VkSurfaceFormatKHR* data) {
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, count, data);
      });

  details.present_modes = vulkan::get_vector_with<VkPresentModeKHR>(
      [device, surface](uint32_t* count, VkPresentModeKHR* data) {
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, count, data);
      });

  return details;
}

VulkanSwapchain::VulkanSwapchain(VkPhysicalDevice pd, VkDevice device,
                                 VkSurfaceKHR surface,
                                 const QueueFamilyIndices& indices)
    : device_{device}
{
  const auto swapchain_support = query_swapchain_support(pd, surface);

  const auto surface_format = choose_surface_format(swapchain_support.formats);
  const auto present_mode =
      choose_present_mode(swapchain_support.present_modes);
  const auto extent = choose_extent(swapchain_support.capabilities);

  std::uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
  if (swapchain_support.capabilities.maxImageCount > 0) {
    image_count =
        std::max(image_count, swapchain_support.capabilities.maxImageCount);
  }

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;

  createInfo.minImageCount = image_count;
  createInfo.imageFormat = surface_format.format;
  createInfo.imageColorSpace = surface_format.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  std::array queue_family_indices = {indices.graphics_family,
                                     indices.present_family};

  if (indices.graphics_family != indices.present_family) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount =
        vulkan::to_u32(queue_family_indices.size());
    createInfo.pQueueFamilyIndices = queue_family_indices.data();
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  createInfo.preTransform = swapchain_support.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = present_mode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = nullptr;

  if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain_) !=
      VK_SUCCESS) {
    beyond::panic("Cannot create swapchain!");
  }

  swapchain_images_ = vulkan::get_vector_with<VkImage>(
      [device, this](uint32_t* count, VkImage* data) {
        vkGetSwapchainImagesKHR(device, swapchain_, count, data);
      });

  swapchain_images_format_ = surface_format.format;
  swapchain_extent_ = extent;
}

VulkanSwapchain::~VulkanSwapchain()
{
  vkDestroySwapchainKHR(device_, swapchain_, nullptr);
}

} // namespace beyond::graphics::vulkan
