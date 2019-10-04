#include <volk.h>

#include <fmt/format.h>

#include <algorithm>
#include <numeric>
#include <optional>
#include <vector>

#include <beyond/core/utils/panic.hpp>
#include <beyond/graphics/backend.hpp>

namespace {

// Transform the two stage query vulkan function into directly return vector
template <typename T, typename F> auto get_vector_with(F func) -> std::vector<T>
{
  std::uint32_t count;
  func(&count, nullptr);

  std::vector<T> vec(count);
  func(&count, vec.data());

  return vec;
}

} // namespace

namespace beyond::vulkan {

struct QueueFamilyIndices {
  std::uint32_t graphics_family;
};

[[nodiscard]] auto find_queue_families(VkPhysicalDevice device) noexcept
    -> std::optional<QueueFamilyIndices>
{
  std::optional<std::uint32_t> graphics_family;

  const auto queue_families = get_vector_with<VkQueueFamilyProperties>(
      [device](uint32_t* count, VkQueueFamilyProperties* data) {
        vkGetPhysicalDeviceQueueFamilyProperties(device, count, data);
      });

  for (std::uint32_t i = 0; i < queue_families.size(); ++i) {
    const auto& queue_family = queue_families[i];

    if (queue_family.queueCount > 0 &&
        queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphics_family = i;
    }

    if (graphics_family) {
      return QueueFamilyIndices{*graphics_family};
    }
  }

  return {};
}

} // namespace beyond::vulkan

namespace {

constexpr std::array validation_layers = {"VK_LAYER_KHRONOS_validation"};

#ifdef BEYOND_VULKAN_ENABLE_VALIDATION_LAYER
constexpr bool enable_validation_layers = true;
#else
constexpr bool enable_validation_layers = false;
#endif

template <typename T> constexpr auto to_u32(T value) noexcept -> std::uint32_t
{
  return static_cast<std::uint32_t>(value);
}

// Higher is better, negative means not suitable
[[nodiscard]] auto rate_physical_device(VkPhysicalDevice device) noexcept -> int
{

  const auto maybe_indices = beyond::vulkan::find_queue_families(device);
  if (!maybe_indices) {
    return -1000;
  }

  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(device, &properties);

  VkPhysicalDeviceFeatures features;
  vkGetPhysicalDeviceFeatures(device, &features);

  int score = 0;
  if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    score += 100;
  }

  return score;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
               VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
               const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
               void* /*pUserData*/)
{
  fmt::print("validation layer: {}\n", p_callback_data->pMessage);
  return VK_FALSE;
}

} // anonymous namespace

namespace beyond::graphics {

[[nodiscard]] auto create_instance(const Window& window) noexcept -> VkInstance;

#ifdef BEYOND_VULKAN_ENABLE_VALIDATION_LAYER
[[nodiscard]] auto create_debug_messager(VkInstance instance) noexcept
    -> VkDebugUtilsMessengerEXT;
#endif

[[nodiscard]] auto pick_physical_device(VkInstance instance) noexcept
    -> VkPhysicalDevice;

struct Context {
  VkInstance instance;
  VkPhysicalDevice physical_device;
#ifdef BEYOND_VULKAN_ENABLE_VALIDATION_LAYER
  VkDebugUtilsMessengerEXT debug_messager;
#endif

  Context(const Window& window)
  {
    if (volkInitialize() != VK_SUCCESS) {
      panic("Cannot find a Vulkan Loader in the system!");
    }

    instance = create_instance(window);
    volkLoadInstance(instance);
    physical_device = pick_physical_device(instance);

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_device, &properties);
    fmt::print("GPU: {}\n", properties.deviceName);
    std::fflush(stdout);

#ifdef BEYOND_VULKAN_ENABLE_VALIDATION_LAYER
    debug_messager = create_debug_messager(instance);
#endif
  }

  ~Context()
  {
#ifdef BEYOND_VULKAN_ENABLE_VALIDATION_LAYER
    vkDestroyDebugUtilsMessengerEXT(instance, debug_messager, nullptr);
#endif

    vkDestroyInstance(instance, nullptr);
  }
};

constexpr auto populate_debug_messenger_create_info() noexcept
    -> VkDebugUtilsMessengerCreateInfoEXT
{
  VkDebugUtilsMessengerCreateInfoEXT create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pfnUserCallback = debug_callback;
  return create_info;
}

auto check_validation_layer_support() noexcept -> bool
{
  const auto available =
      get_vector_with<VkLayerProperties>(vkEnumerateInstanceLayerProperties);

  return std::all_of(std::begin(validation_layers), std::end(validation_layers),
                     [&](const char* layer_name) {
                       return std::find_if(
                                  std::begin(available), std::end(available),
                                  [&](const auto& layer_properties) {
                                    return strcmp(layer_name,
                                                  layer_properties.layerName);
                                  }) != std::end(available);
                     });
}

[[nodiscard]] auto create_instance(const Window& window) noexcept -> VkInstance
{
  if (enable_validation_layers && !check_validation_layer_support()) {
    panic("validation layers requested, but not available!");
  }

  VkInstance instance;

  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = window.title().c_str();
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "Beyond Game Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;

  const auto extensions = window.get_required_instance_extensions();
  create_info.enabledExtensionCount = to_u32(extensions.size());
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
    panic("Cannot create vulkan instance!");
  }

  return instance;
}

[[nodiscard]] auto pick_physical_device(VkInstance instance) noexcept
    -> VkPhysicalDevice
{
  const auto avaliable_devices = get_vector_with<VkPhysicalDevice>(
      [instance](uint32_t* count, VkPhysicalDevice* data) {
        return vkEnumeratePhysicalDevices(instance, count, data);
      });
  if (avaliable_devices.empty()) {
    panic("failed to find GPUs with Vulkan support!");
  }

  using ScoredPair = std::pair<int, VkPhysicalDevice>;
  std::vector<ScoredPair> scored_pairs;
  scored_pairs.reserve(avaliable_devices.size());
  for (const auto& device : avaliable_devices) {
    const auto score = rate_physical_device(device);
    if (score > 0) {
      scored_pairs.emplace_back(score, device);
    }
  }

  if (scored_pairs.empty()) {
    panic("Vulkan failed to find GPUs with enough nessesory graphics support!");
  }

  std::sort(std::begin(scored_pairs), std::end(scored_pairs),
            [](const ScoredPair& lhs, const ScoredPair& rhs) {
              return lhs.first > rhs.first;
            });

  // Returns the pair with highest score
  return scored_pairs.front().second;
}

#ifdef BEYOND_VULKAN_ENABLE_VALIDATION_LAYER
[[nodiscard]] auto create_debug_messager(VkInstance instance) noexcept
    -> VkDebugUtilsMessengerEXT
{

  const VkDebugUtilsMessengerCreateInfoEXT create_info =
      populate_debug_messenger_create_info();

  VkDebugUtilsMessengerEXT debug_mesenger;
  if (vkCreateDebugUtilsMessengerEXT(instance, &create_info, nullptr,
                                     &debug_mesenger) != VK_SUCCESS) {
    panic("failed to set up debug messenger!");
  }

  return debug_mesenger;
}
#endif

[[nodiscard]] auto create_context(const Window& window) noexcept -> Context*
{
  return new Context(window);
}

auto destory_context(Context* context) noexcept -> void
{
  delete context;
}

} // namespace beyond::graphics
