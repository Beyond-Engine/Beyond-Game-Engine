#include <beyond/utils/assert.hpp>
#include <beyond/utils/bit_cast.hpp>

#include "vulkan_context.hpp"
#include "vulkan_shader_module.hpp"
#include "vulkan_utils.hpp"

#include <fmt/format.h>

#define BAIL_ON_BAD_RESULT(result)                                             \
  if (VK_SUCCESS != (result)) {                                                \
    fprintf(stderr, "Failure at %u %s\n", __LINE__, __FILE__);                 \
    exit(-1);                                                                  \
  }

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

  const auto get_device_queue = [this](std::uint32_t family_index,
                                       std::uint32_t index) {
    VkQueue queue;
    vkGetDeviceQueue(this->device_, family_index, index, &queue);
    return queue;
  };
  graphics_queue_ = get_device_queue(queue_family_indices_.graphics_family, 0);
  present_queue_ = get_device_queue(queue_family_indices_.present_family, 0);
  compute_queue_ = get_device_queue(queue_family_indices_.compute_family, 0);

  VmaAllocatorCreateInfo allocator_info{};
  allocator_info.physicalDevice = physical_device_;
  allocator_info.device = device_;
  if (vmaCreateAllocator(&allocator_info, &allocator_) != VK_SUCCESS) {
    beyond::panic("Cannot create an allocator for vulkan");
  }
} // namespace beyond::graphics::vulkan

VulkanContext::~VulkanContext() noexcept
{
  swapchains_pool_.clear();
  buffers_pool_.clear();
  compute_pipelines_pool_.clear();

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
  const auto index = swapchains_pool_.size();

  if (index >= 1) {
    beyond::panic("Currently, more than 2 swapchains are not supported");
  }

  swapchains_pool_.emplace_back(physical_device_, device_, surface_,
                                queue_family_indices_);
  return Swapchain{static_cast<Swapchain::UnderlyingType>(index)};
}

[[nodiscard]] auto
VulkanContext::create_buffer(const BufferCreateInfo& create_info) -> Buffer
{
  // TODO(lesley): error handling

  const auto index = static_cast<Buffer::Index>(buffers_pool_.size());

  if (Buffer::is_overflow(index)) {
    beyond::panic("Created too many buffers");
  }

  const VkBufferCreateInfo buffer_info{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .size = create_info.size,
      .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      .sharingMode = {},
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
  };

  VmaAllocationCreateInfo alloc_info{};
  switch (create_info.memory_usage) {
  case MemoryUsage::device:
    alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    break;
  case MemoryUsage::host:
    alloc_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    break;
  case MemoryUsage::host_to_device:
    alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    break;
  case MemoryUsage::device_to_host:
    alloc_info.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
    break;
  }

  VkBuffer buffer;
  VmaAllocation allocation;
  if (vmaCreateBuffer(allocator_, &buffer_info, &alloc_info, &buffer,
                      &allocation, nullptr) != VK_SUCCESS) {
    beyond::panic("Vulkan backend failed to allocate a buffer");
  }

  buffers_pool_.emplace_back(allocator_, buffer, allocation, create_info.size);
  return Buffer{index};
}

auto VulkanContext::destory_buffer(Buffer& buffer_handle) -> void
{
  const auto index = buffer_handle.index();

  if (index >= buffers_pool_.size() || !buffers_pool_[index]) {
    return;
  }

  buffers_pool_[index] = VulkanBuffer{};
}

[[nodiscard]] auto VulkanContext::map_memory_impl(Buffer buffer_handle) noexcept
    -> MappingInfo
{
  if (buffers_pool_.size() < buffer_handle.index()) {
    return {nullptr, 0};
  }

  auto& buffer = buffers_pool_[buffer_handle.index()];
  return {buffer.map(), buffer.size()};
}

auto VulkanContext::unmap_memory_impl(Buffer buffer_handle) noexcept -> void
{
  if (buffers_pool_.size() < buffer_handle.index()) {
    // TODO(llai): error handling in unmap_memory?
    beyond::panic("unmap an invalid buffer handle");
  }

  return buffers_pool_[buffer_handle.index()].unmap();
}

[[nodiscard]] auto VulkanContext::create_compute_pipeline(
    const ComputePipelineCreateInfo& create_info) -> ComputePipeline
{
  const auto index = compute_pipelines_pool_.size();
  compute_pipelines_pool_.emplace_back(
      VulkanPipeline::create_compute(create_info, device_));

  return ComputePipeline{static_cast<ComputePipeline::UnderlyingType>(index)};
}

auto VulkanContext::submit(gsl::span<SubmitInfo> info) -> void
{
  const auto& pipeline = compute_pipelines_pool_[info[0].pipeline.get()];

  const VkDescriptorPoolSize descriptor_pool_size{
      .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 2};

  const VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .maxSets = 1,
      .poolSizeCount = 1,
      .pPoolSizes = &descriptor_pool_size};

  VkDescriptorPool descriptor_pool;
  if (vkCreateDescriptorPool(device_, &descriptor_pool_create_info, nullptr,
                             &descriptor_pool) != VK_SUCCESS) {
    beyond::panic("Vulkan backend failed to create descriptor pool");
  }

  const auto descriptor_set_layout = pipeline.descriptor_set_layout();

  const VkDescriptorSetAllocateInfo descriptor_set_allocate_info{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext = nullptr,
      .descriptorPool = descriptor_pool,
      .descriptorSetCount = 1,
      .pSetLayouts = &descriptor_set_layout};

  VkDescriptorSet descriptor_set;
  if (vkAllocateDescriptorSets(device_, &descriptor_set_allocate_info,
                               &descriptor_set) != VK_SUCCESS) {
    beyond::panic("Vulkan backend failed to allocate descriptor set");
  }

  const auto in_handle = info[0].input;
  const auto out_handle = info[0].output;
  const auto buffer_size = info[0].buffer_size;

  auto& in_buffer = buffers_pool_[in_handle.index()];
  const auto in_buffer_raw = in_buffer.vkbuffer();

  auto& out_buffer = buffers_pool_[out_handle.index()];
  const auto out_buffer_raw = out_buffer.vkbuffer();

  const VkDescriptorBufferInfo in_descriptor_buffer_info{
      .buffer = in_buffer_raw,
      .offset = 0,
      .range = VK_WHOLE_SIZE,
  };

  const VkDescriptorBufferInfo out_descriptor_buffer_info{
      .buffer = out_buffer_raw,
      .offset = 0,
      .range = VK_WHOLE_SIZE,
  };

  const std::array write_descriptor_set = {
      VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
                           descriptor_set, 0, 0, 1,
                           VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr,
                           &in_descriptor_buffer_info, nullptr},
      VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
                           descriptor_set, 1, 0, 1,
                           VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr,
                           &out_descriptor_buffer_info, nullptr}};

  vkUpdateDescriptorSets(device_, vulkan::to_u32(write_descriptor_set.size()),
                         write_descriptor_set.data(), 0, nullptr);

  const VkCommandPoolCreateInfo command_pool_create_info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueFamilyIndex = queue_family_indices_.compute_family};
  VkCommandPool command_pool;
  if (vkCreateCommandPool(device_, &command_pool_create_info, nullptr,
                          &command_pool) != VK_SUCCESS) {
    beyond::panic("Vulkan backend failed to create command pool");
  }

  const VkCommandBufferAllocateInfo command_buffer_allocate_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = command_pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1};

  VkCommandBuffer command_buffer;
  if (vkAllocateCommandBuffers(device_, &command_buffer_allocate_info,
                               &command_buffer) != VK_SUCCESS) {
    beyond::panic("Vulkan backend failed to allocate command buffer");
  }

  const VkCommandBufferBeginInfo command_buffer_begin_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo = nullptr};

  if (vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info) !=
      VK_SUCCESS) {
    beyond::panic("Vulkan backend failed to begin command buffer");
  }
  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                    pipeline.pipeline());
  vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                          pipeline.pipeline_layout(), 0, 1, &descriptor_set, 0,
                          nullptr);
  vkCmdDispatch(command_buffer,
                static_cast<uint32_t>(buffer_size / sizeof(int32_t)), 1, 1);
  if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
    beyond::panic("Vulkan backend failed to end command buffer");
  }

  VkSubmitInfo submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                              .pNext = nullptr,
                              .waitSemaphoreCount = 0,
                              .pWaitSemaphores = nullptr,
                              .pWaitDstStageMask = nullptr,
                              .commandBufferCount = 1,
                              .pCommandBuffers = &command_buffer,
                              .signalSemaphoreCount = 0,
                              .pSignalSemaphores = nullptr};

  const VkFenceCreateInfo fence_create_info{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
  };
  VkFence fence;
  vkCreateFence(device_, &fence_create_info, nullptr, &fence);
  if (vkQueueSubmit(compute_queue_, 1, &submit_info, fence) != VK_SUCCESS) {
    beyond::panic("Vulkan backend failed to submit to queue");
  }

  static constexpr auto compute_timeout = static_cast<std::uint64_t>(1e6);
  while (vkWaitForFences(device_, 1, &fence, VK_TRUE, compute_timeout) ==
         VK_TIMEOUT) {
    fmt::print("busy waiting\n");
  }
  if (vkWaitForFences(device_, 1, &fence, VK_TRUE, 0) != VK_SUCCESS) {
    beyond::panic("Vulkan backend failed to wait for fence");
  }

  vkDestroyFence(device_, fence, nullptr);

  vkDestroyCommandPool(device_, command_pool, nullptr);
  vkDestroyDescriptorPool(device_, descriptor_pool, nullptr);
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
} // anonymous namespace

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
