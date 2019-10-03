#include <vulkan/vulkan.h>

#include "beyond/graphics/graphics_backend.hpp"

#include <cstdio>

namespace beyond::graphics {

Context::Context(std::unique_ptr<ContextImpl>&& impl) : impl_{std::move(impl)}
{
}
Context::~Context() = default;

class ContextImpl {
public:
  ContextImpl(VkInstance instance) : instance_{instance} {}

  ~ContextImpl()
  {
    vkDestroyInstance(instance_, nullptr);
  }

  ContextImpl(const ContextImpl&) = delete;
  auto operator=(const ContextImpl&) -> ContextImpl& = delete;

  ContextImpl(ContextImpl&&) = delete;
  auto operator=(ContextImpl &&) -> ContextImpl& = delete;

  friend auto create_context(Window& window) noexcept -> Context;

private:
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

  vkCreateInstance(&createInfo, nullptr, &instance);

  std::puts("Created vulkan instance!");
  std::fflush(stdout);

  return instance;
  //  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
  //    throw std::runtime_error("failed to create instance!");
  //  }
}

[[nodiscard]] auto create_context(Window& window) noexcept -> Context
{
  const auto instance = create_instance(window.title());
  return Context{std::make_unique<ContextImpl>(instance)};
}

} // namespace beyond::graphics
