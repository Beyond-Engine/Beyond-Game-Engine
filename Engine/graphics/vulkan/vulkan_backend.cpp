#include <volk.h>

#include <beyond/core/utils/panic.hpp>
#include <beyond/graphics/backend.hpp>

namespace {

constexpr std::array validation_layers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
const bool enable_validation_layers = false;
#else
const bool enable_validation_layers = true;
#endif

template <typename T> std::uint32_t to_u32(T value)
{
  return static_cast<std::uint32_t>(value);
}

} // anonymous namespace

namespace beyond::graphics {

struct Context {
  VkInstance instance_;
};

[[nodiscard]] auto create_instance(const Window& window) noexcept
{
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

  create_info.enabledLayerCount = 0;

  if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
    panic("Cannot create vulkan instance!");
  }

  return instance;
}

[[nodiscard]] auto create_context(const Window& window) noexcept -> Context*
{
  auto* context = new Context;

  if (volkInitialize() != VK_SUCCESS) {
    panic("Cannot find a Vulkan Loader in the system!");
  }

  context->instance_ = create_instance(window);
  volkLoadInstance(context->instance_);
  return context;
}

auto destory_context(Context* context) noexcept -> void
{
  vkDestroyInstance(context->instance_, nullptr);
  delete context;
}

} // namespace beyond::graphics
