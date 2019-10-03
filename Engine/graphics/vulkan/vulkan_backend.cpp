#include <vulkan/vulkan.h>

#include "beyond/graphics/backend.hpp"

#include "beyond/core/utils/panic.hpp"

#include <cstdio>

namespace beyond::graphics {

struct Context {
  VkInstance instance_;
};

[[nodiscard]] auto create_instance(std::string_view title) noexcept
{
  VkInstance instance;

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = title.data();
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "Beyond Game Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  //  uint32_t glfwExtensionCount = 0;
  //  const char** glfwExtensions;
  //  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  //  createInfo.enabledExtensionCount = glfwExtensionCount;
  //  createInfo.ppEnabledExtensionNames = glfwExtensions;

  createInfo.enabledLayerCount = 0;

  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
    panic("Cannot create vulkan instance!");
  }

  std::puts("Created vulkan instance!");
  std::fflush(stdout);

  return instance;
}

/// @brief Create a graphics context
[[nodiscard]] auto create_context(const Window& window) noexcept -> Context*
{
  auto* context = new Context;
  context->instance_ = create_instance(window.title());
  return context;
}

/// @brief destory a graphics context
auto destory_context(Context* context) noexcept -> void
{
  vkDestroyInstance(context->instance_, nullptr);
  delete context;
}

} // namespace beyond::graphics
