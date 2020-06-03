#include "vulkan_swapchain.hpp"
#include "vulkan_utils.hpp"

#include <beyond/utils/panic.hpp>

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

  VkSwapchainCreateInfoKHR create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = surface;
  create_info.minImageCount = image_count;
  create_info.imageFormat = surface_format.format;
  create_info.imageColorSpace = surface_format.colorSpace;
  create_info.imageExtent = extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  std::array queue_family_indices = {indices.graphics_family,
                                     indices.present_family};

  if (indices.graphics_family != indices.present_family) {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount =
        vulkan::to_u32(queue_family_indices.size());
    create_info.pQueueFamilyIndices = queue_family_indices.data();
  } else {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
  }

  create_info.preTransform = swapchain_support.capabilities.currentTransform;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode = present_mode;
  create_info.clipped = VK_TRUE;
  create_info.oldSwapchain = nullptr;

  if (vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain_) !=
      VK_SUCCESS) {
    beyond::panic("Cannot create swapchain!");
  }

  swapchain_images_ = vulkan::get_vector_with<VkImage>(
      [device, this](uint32_t* count, VkImage* data) {
        vkGetSwapchainImagesKHR(device, swapchain_, count, data);
      });

  swapchain_images_format_ = surface_format.format;
  swapchain_extent_ = extent;

  swapchain_image_views_.resize(swapchain_images_.size());
  for (size_t i = 0; i < swapchain_images_.size(); i++) {
    const VkImageViewCreateInfo view_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = swapchain_images_[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = swapchain_images_format_,
        .components =
            {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }};

    if (vkCreateImageView(device, &view_create_info, nullptr,
                          &swapchain_image_views_[i]) != VK_SUCCESS) {
      beyond::panic("Failed to create swapchain image views!");
    }
  }
} // namespace beyond::graphics::vulkan

VulkanSwapchain::~VulkanSwapchain()
{
  for (auto view : swapchain_image_views_) {
    vkDestroyImageView(device_, view, nullptr);
  }
  vkDestroySwapchainKHR(device_, swapchain_, nullptr);
}

} // namespace beyond::graphics::vulkan
