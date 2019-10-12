#include "vulkan_context.hpp"
#include "vulkan_utils.hpp"

#include <fmt/format.h>

namespace vulkan = beyond::graphics::vulkan;
using vulkan::QueueFamilyIndices;

namespace {

constexpr std::array validation_layers = {"VK_LAYER_KHRONOS_validation"};
constexpr std::array device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef BEYOND_VULKAN_ENABLE_VALIDATION_LAYER
constexpr bool enable_validation_layers = true;
#else
constexpr bool enable_validation_layers = false;
#endif

[[nodiscard]] auto
check_device_extension_support(VkPhysicalDevice device) noexcept -> bool
{
  const auto available = vulkan::get_vector_with<VkExtensionProperties>(
      [device](uint32_t* count, VkExtensionProperties* data) {
        vkEnumerateDeviceExtensionProperties(device, nullptr, count, data);
      });

  std::set<std::string> required(device_extensions.begin(),
                                 device_extensions.end());

  for (const auto& extension : available) {
    required.erase(static_cast<const char*>(extension.extensionName));
  }

  return required.empty();
}

// Higher is better, negative means not suitable
[[nodiscard]] auto rate_physical_device(VkPhysicalDevice device,
                                        VkSurfaceKHR surface) noexcept -> int
{
  static constexpr int failing_score = -1000;

  // If cannot find indices for all the queues, return -1000
  const auto maybe_indices = vulkan::find_queue_families(device, surface);
  if (!maybe_indices) {
    return failing_score;
  }

  // If not support extension, return -1000
  if (!check_device_extension_support(device)) {
    return failing_score;
  }

  // If swapchain not adequate, return -1000
  const auto swapchain_support =
      vulkan::query_swapchain_support(device, surface);
  if (swapchain_support.formats.empty() ||
      swapchain_support.present_modes.empty()) {
    return failing_score;
  }

  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(device, &properties);

  VkPhysicalDeviceFeatures features;
  vkGetPhysicalDeviceFeatures(device, &features);

  // Biased toward discrete GPU
  int score = 0;
  if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    score += 100;
  }

  return score;
}

#ifdef BEYOND_VULKAN_ENABLE_VALIDATION_LAYER
VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
               VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
               const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
               void* /*pUserData*/)
{
  fmt::print("validation layer: {}\n", p_callback_data->pMessage);
  return VK_FALSE;
}

constexpr auto populate_debug_messenger_create_info() noexcept
    -> VkDebugUtilsMessengerCreateInfoEXT
{
  return {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .pNext = nullptr,
      .flags = 0,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = debug_callback,
      .pUserData = nullptr,
  };
}
#endif

[[nodiscard]] auto create_instance(const beyond::Window& window) noexcept
    -> VkInstance;

#ifdef BEYOND_VULKAN_ENABLE_VALIDATION_LAYER
[[nodiscard]] auto create_debug_messager(VkInstance instance) noexcept
    -> VkDebugUtilsMessengerEXT;
#endif

[[nodiscard]] auto pick_physical_device(VkInstance instance,
                                        VkSurfaceKHR surface) noexcept
    -> VkPhysicalDevice;

[[nodiscard]] auto
create_logical_device(VkPhysicalDevice pd,
                      const QueueFamilyIndices& indices) noexcept -> VkDevice;

} // anonymous namespace

namespace beyond::graphics::vulkan {

[[nodiscard]] auto create_vulkan_context(Window& window) noexcept
    -> std::unique_ptr<Context>
{
  return std::make_unique<VulkanContext>(window);
}

VulkanContext::VulkanContext(Window& window)
{
  std::puts("Vulkan Graphics backend");

  if (volkInitialize() != VK_SUCCESS) {
    panic("Cannot find a Vulkan Loader in the system!");
  }

  instance_ = create_instance(window);
  volkLoadInstance(instance_);

  window.create_vulkan_surface(instance_, nullptr, surface_);

#ifdef BEYOND_VULKAN_ENABLE_VALIDATION_LAYER
  debug_messager_ = create_debug_messager(instance_);
#endif

  physical_device_ = pick_physical_device(instance_, surface_);
  queue_family_indices_ = *find_queue_families(physical_device_, surface_);
  device_ = create_logical_device(physical_device_, queue_family_indices_);
  volkLoadDevice(device_);

  vkGetDeviceQueue(device_, queue_family_indices_.graphics_family, 0,
                   &graphics_queue_);
  vkGetDeviceQueue(device_, queue_family_indices_.present_family, 0,
                   &present_queue_);
  vkGetDeviceQueue(device_, queue_family_indices_.compute_family, 0,
                   &compute_queue_);

  VmaAllocatorCreateInfo allocator_info{};
  allocator_info.physicalDevice = physical_device_;
  allocator_info.device = device_;
  vmaCreateAllocator(&allocator_info, &allocator_);
}

VulkanContext::~VulkanContext()
{
  swapchains_.clear();

  vmaDestroyAllocator(allocator_);

  vkDestroyDevice(device_, nullptr);

#ifdef BEYOND_VULKAN_ENABLE_VALIDATION_LAYER
  vkDestroyDebugUtilsMessengerEXT(instance_, debug_messager_, nullptr);
#endif

  vkDestroySurfaceKHR(instance_, surface_, nullptr);
  vkDestroyInstance(instance_, nullptr);
}

[[nodiscard]] auto VulkanContext::create_swapchain() -> Swapchain
{
  const auto index = swapchains_.size();

  if (index != 0) {
    beyond::panic("Currently, more than one swapchain is not supported");
  }

  swapchains_.emplace_back(physical_device_, device_, surface_,
                           queue_family_indices_);
  return Swapchain{static_cast<Swapchain::Index>(index)};
}

} // namespace beyond::graphics::vulkan

namespace {

auto check_validation_layer_support() noexcept -> bool
{
  const auto available = vulkan::get_vector_with<VkLayerProperties>(
      vkEnumerateInstanceLayerProperties);

  return std::all_of(
      std::begin(validation_layers), std::end(validation_layers),
      [&](const char* layer_name) {
        return std::find_if(std::begin(available), std::end(available),
                            [&](const auto& layer_properties) {
                              return strcmp(layer_name,
                                            static_cast<const char*>(
                                                layer_properties.layerName));
                            }) != std::end(available);
      });
}

[[nodiscard]] auto create_instance(const beyond::Window& window) noexcept
    -> VkInstance
{
  if (enable_validation_layers && !check_validation_layer_support()) {
    beyond::panic("validation layers requested, but not available!");
  }

  VkInstance instance;

  const VkApplicationInfo app_info = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = window.title().c_str(),
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "Beyond Game Engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_API_VERSION_1_0,
  };

  auto extensions = window.get_required_instance_extensions();
#ifdef BEYOND_VULKAN_ENABLE_VALIDATION_LAYER
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount = vulkan::to_u32(extensions.size());
  create_info.ppEnabledExtensionNames = extensions.data();

#ifdef BEYOND_VULKAN_ENABLE_VALIDATION_LAYER
  create_info.enabledLayerCount =
      static_cast<uint32_t>(validation_layers.size());
  create_info.ppEnabledLayerNames = validation_layers.data();

  VkDebugUtilsMessengerCreateInfoEXT debug_create_info =
      populate_debug_messenger_create_info();
  create_info.pNext = &debug_create_info;
#else
  create_info.enabledLayerCount = 0;
  create_info.pNext = nullptr;
#endif

  if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
    beyond::panic("Cannot create vulkan instance!");
  }

  return instance;
} // namespace

[[nodiscard]] auto pick_physical_device(VkInstance instance,
                                        VkSurfaceKHR surface) noexcept
    -> VkPhysicalDevice
{
  const auto avaliable_devices = vulkan::get_vector_with<VkPhysicalDevice>(
      [instance](uint32_t* count, VkPhysicalDevice* data) {
        return vkEnumeratePhysicalDevices(instance, count, data);
      });
  if (avaliable_devices.empty()) {
    beyond::panic("failed to find GPUs with Vulkan support!");
  }

  using ScoredPair = std::pair<int, VkPhysicalDevice>;
  std::vector<ScoredPair> scored_pairs;
  scored_pairs.reserve(avaliable_devices.size());
  for (const auto& device : avaliable_devices) {
    const auto score = rate_physical_device(device, surface);
    if (score > 0) {
      scored_pairs.emplace_back(score, device);
    }
  }

  if (scored_pairs.empty()) {
    beyond::panic(
        "Vulkan failed to find GPUs with enough nessesory graphics support!");
  }

  std::sort(std::begin(scored_pairs), std::end(scored_pairs),
            [](const ScoredPair& lhs, const ScoredPair& rhs) {
              return lhs.first > rhs.first;
            });

  const auto physical_device = scored_pairs.front().second;

  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(physical_device, &properties);
  fmt::print("GPU: {}\n", properties.deviceName);
  std::fflush(stdout);

  // Returns the pair with highest score
  return physical_device;
}

[[nodiscard]] auto
create_logical_device(VkPhysicalDevice pd,
                      const QueueFamilyIndices& indices) noexcept -> VkDevice
{
  const auto unique_indices = indices.to_set();

  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  queue_create_infos.resize(unique_indices.size());

  float queue_priority = 1.0f;
  std::transform(std::begin(unique_indices), std::end(unique_indices),
                 std::begin(queue_create_infos), [&](uint32_t index) {
                   return VkDeviceQueueCreateInfo{
                       .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                       .pNext = nullptr,
                       .flags = 0,
                       .queueFamilyIndex = index,
                       .queueCount = 1,
                       .pQueuePriorities = &queue_priority,
                   };
                 });

  const VkPhysicalDeviceFeatures features = {};

  const VkDeviceCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueCreateInfoCount = vulkan::to_u32(queue_create_infos.size()),
      .pQueueCreateInfos = queue_create_infos.data(),
#ifdef BEYOND_VULKAN_ENABLE_VALIDATION_LAYER
      .enabledLayerCount = vulkan::to_u32(validation_layers.size()),
      .ppEnabledLayerNames = validation_layers.data(),
#else
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
#endif
      .enabledExtensionCount = vulkan::to_u32(device_extensions.size()),
      .ppEnabledExtensionNames = device_extensions.data(),
      .pEnabledFeatures = &features,
  };

  VkDevice device = nullptr;
  if (vkCreateDevice(pd, &create_info, nullptr, &device) != VK_SUCCESS) {
    beyond::panic("Vulkan: failed to create logical device!");
  }

  return device;
}

#ifdef BEYOND_VULKAN_ENABLE_VALIDATION_LAYER
[[nodiscard]] auto create_debug_messager(VkInstance instance) noexcept
    -> VkDebugUtilsMessengerEXT
{

  const VkDebugUtilsMessengerCreateInfoEXT create_info =
      populate_debug_messenger_create_info();

  VkDebugUtilsMessengerEXT debug_mesenger;
  auto result = vkCreateDebugUtilsMessengerEXT(instance, &create_info, nullptr,
                                               &debug_mesenger);
  if (result != VK_SUCCESS) {
    beyond::panic("failed to set up debug messenger!");
  }

  return debug_mesenger;
}
#endif

} // anonymous namespace
