#pragma once

#ifndef BEYOND_GRAPHICS_VULKAN_BUFFER_HPP
#define BEYOND_GRAPHICS_VULKAN_BUFFER_HPP

#include <vk_mem_alloc.h>
#include <volk.h>

#include <utility>

#include <beyond/utils/panic.hpp>

namespace beyond::graphics::vulkan {

/// @brief Half RAII wrapper of a vulkan buffer
///
/// The buffer is allocated in the outside code but destoryed in the destructor.
/// The reason is that we can create multiple buffers at once.
class VulkanBuffer {
public:
  VulkanBuffer() = default;

  VulkanBuffer(VmaAllocator allocator, VkBuffer buffer,
               VmaAllocation allocation, std::uint32_t size)
      : allocator_{allocator}, buffer_{buffer},
        allocation_{allocation}, size_{size}
  {
  }

  ~VulkanBuffer()
  {
    if (allocator_) {
      vmaDestroyBuffer(allocator_, buffer_, allocation_);
    }
  }

  VulkanBuffer(VulkanBuffer& other) = delete;
  auto operator=(const VulkanBuffer& other) & -> VulkanBuffer& = delete;

  VulkanBuffer(VulkanBuffer&& other) noexcept
      : allocator_{std::exchange(other.allocator_, nullptr)},
        buffer_{std::exchange(other.buffer_, nullptr)},
        allocation_{std::exchange(other.allocation_, nullptr)},
        size_{std::exchange(other.size_, 0)}
  {
  }

  auto operator=(VulkanBuffer&& other) & noexcept -> VulkanBuffer&
  {
    allocator_ = std::exchange(other.allocator_, nullptr);
    buffer_ = std::exchange(other.buffer_, nullptr);
    allocation_ = std::exchange(other.allocation_, nullptr);
    size_ = std::exchange(other.size_, 0);
    return *this;
  }

  /// @brief Returns `false` if the buffer object does not refer to a valid
  /// buffer
  [[nodiscard]] operator bool() noexcept
  {
    return !buffer_;
  }

  /// @brief Gets a direct handle to the underlying VkBuffer
  [[nodiscard]] auto vkbuffer() noexcept -> VkBuffer
  {
    return buffer_;
  }

  /**
   * @brief Map a buffer with host visible memory
   * @note: If cannot map to the buffer, returns nullptr
   */
  [[nodiscard]] auto map() noexcept -> void*
  {
    void* payload;
    if (vmaMapMemory(allocator_, allocation_, &payload) != VK_SUCCESS) {
      return nullptr;
    }
    return payload;
  }

  /// @brief Unmap a buffer
  auto unmap() noexcept -> void
  {
    vmaUnmapMemory(allocator_, allocation_);
  }

  [[nodiscard]] auto size() noexcept -> std::uint32_t
  {
    return size_;
  }

private:
  VmaAllocator allocator_ = nullptr;
  VkBuffer buffer_ = nullptr;
  VmaAllocation allocation_ = nullptr;
  std::uint32_t size_ = 0;
};

} // namespace beyond::graphics::vulkan

#endif
