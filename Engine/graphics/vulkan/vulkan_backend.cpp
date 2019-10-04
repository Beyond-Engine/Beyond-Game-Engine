#include <volk.h>

#include <fmt/format.h>

#include <vector>

#include <beyond/core/utils/panic.hpp>
#include <beyond/graphics/backend.hpp>

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

struct Context {
  VkInstance instance;
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
  std::uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  for (const char* layer_name : validation_layers) {
    bool layer_found = false;

    for (const auto& layerProperties : available_layers) {
      if (strcmp(layer_name, layerProperties.layerName) == 0) {
        layer_found = true;
        break;
      }
    }

    if (!layer_found) {
      return false;
    }
  }

  return true;
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
} // namespace beyond::graphics

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
