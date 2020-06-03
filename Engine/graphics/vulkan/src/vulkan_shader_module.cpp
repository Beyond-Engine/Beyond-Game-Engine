#include "vulkan_shader_module.hpp"

#include <cstddef>
#include <fstream>

#include <fmt/format.h>

#include <beyond/utils/bit_cast.hpp>
#include <beyond/utils/panic.hpp>

namespace {

[[nodiscard]] auto read_file(const std::string_view filename)
    -> std::vector<char>
{

  std::ifstream file(filename.data(), std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    // TODO(lesley): error handling
    beyond::panic(fmt::format("failed to open file: {}\n", filename));
  }

  size_t file_size = static_cast<size_t>(file.tellg());
  std::vector<char> buffer;
  buffer.resize(file_size);

  file.seekg(0);
  file.read(buffer.data(), static_cast<std::streamsize>(file_size));

  return buffer;
}

} // anonymous namespace

namespace beyond::graphics::vulkan {

[[nodiscard]] auto create_shader_module(const std::string_view& filename,
                                        VkDevice device) -> VkShaderModule
{
  const auto buffer = read_file(filename);
  return create_shader_module(buffer.size(),
                              bit_cast<const uint32_t*>(buffer.data()), device);
}

[[nodiscard]] auto create_shader_module(std::size_t size, const uint32_t* data,
                                        VkDevice device) -> VkShaderModule
{
  const VkShaderModuleCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .codeSize = size,
      .pCode = data,
  };

  VkShaderModule module;

  // TODO(lesley): error handling
  if (vkCreateShaderModule(device, &create_info, nullptr, &module) !=
      VK_SUCCESS) {
    beyond::panic("Cannot load shader\n");
  }

  return module;
}

} // namespace beyond::graphics::vulkan
