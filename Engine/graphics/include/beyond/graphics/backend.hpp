#pragma once

#ifndef BEYOND_GRAPHICS_BACKEND_HPP
#define BEYOND_GRAPHICS_BACKEND_HPP

/**
 * @file backend.hpp
 * @brief Interface of the graphics backend
 */

#include <memory>

#include <gsl/span>

#include "beyond/core/utils/handle.hpp"
#include "beyond/core/utils/named_type.hpp"
#include "beyond/platform/platform.hpp"

namespace beyond::graphics {

/**
 * @addtogroup graphics
 * @{
 */

/**
 * @defgroup backend Backend
 * @brief Interface between the underlying graphics API and high level
 * graphics codes
 *
 * @{
 */

struct Swapchain : beyond::NamedType<std::uint32_t, struct SwapchainTag,
                                     beyond::EquableBase> {
  using NamedType::NamedType;
};

/// @brief The place of buffer memory it reside in
enum struct MemoryUsage {
  /**
   * Memory will be used on device only. There are no garentee this
   memory will be mappable on the host.
   *
   * Usage:
   * - Resources written and read by device, e.g. images used as attachments.
   * - Resources transferred from host once (immutable) or infrequently and read
   by device multiple times, e.g. textures to be sampled, vertex buffers,
   and uniform buffers.
   */
  device,
  /**
   * Memory will be mappable on host. It usually means CPU memory. Resources
   * created in this pool may still be accessible to the device, but access to
   * them can be slow.
   * Usage: Staging copy of resources used as transfer source.
   */
  host,
  /**
   * Memory that is both mappable on host and preferably fast to access by GPU.
   * CPU access is typically uncached. Writes may be write-combined.
   *
   * Usage: Resources written frequently by host (dynamic), read by device. E.g.
   * textures, vertex buffers, uniform buffers updated every frame or every draw
   * call.
   */
  host_to_device,
  /**
   *Memory mappable on host and cached.
   *
   *Usage:
   * - Resources written by device, read by host - results of some computations,
   *e.g. screen capture, average scene luminance for HDR tone mapping. Any
   *resources read or accessed randomly on host, e.g. CPU-side copy of vertex
   *buffer used as source of transfer, but also used for collision detection.
   */
  device_to_host,
};

/**
 * @brief The information used to create a GPU buffer
 */
struct BufferCreateInfo {
  std::uint32_t size = 0;
  MemoryUsage memory_usage = MemoryUsage::device;
};

/// @brief A handle to a GPU buffer
struct Buffer : Handle<Buffer, std::uint32_t, 20, 12> {
  using Handle::Handle;
};

/// @brief Structure specifying a commands submit operation
struct SubmitInfo {
  // Temporaraily hack around
  Buffer input{};
  Buffer output{};
  std::uint32_t buffer_size{};
};

struct ComputePipelineCreateInfo {
};

/// @brief A handle to a GPU pipeline
struct Pipeline : beyond::NamedType<std::uint32_t, struct PipelineTag,
                                    beyond::EquableBase> {
  using NamedType::NamedType;
};

class Context;

/**
 * @brief A mapping is a view of host visible device memory
 * @warning Destorying a buffer while a `Mapping` to it is still alive is
 * undefined behavior
 */
template <typename T> class Mapping {
public:
  using size_type = std::size_t;
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;

  using iterator = T*;
  using const_iterator = const T*;

  Mapping() = default;
  explicit Mapping(Context& context, Buffer buffer);
  ~Mapping() noexcept;

  Mapping(const Mapping&) = delete;
  auto operator=(const Mapping&) & -> Mapping& = delete;
  Mapping(Mapping&& other) noexcept
      : context_{std::exchange(other.context_, nullptr)}, buffer_{std::move(
                                                              other.buffer_)},
        data_{std::exchange(other.data_, nullptr)}, size_{std::exchange(
                                                        other.size_, 0)}
  {
  }
  auto operator=(Mapping&& other) & noexcept -> Mapping&
  {
    context_ = std::exchange(other.context_, nullptr);
    buffer_ = std::move(other.buffer_);
    data_ = std::exchange(other.data_, nullptr);
    size_ = std::exchange(other.size_, 0);
    return *this;
  }

  /// @brief  dereferences pointer to the managed mapping
  /// @return the mapped object owned by `*this`, equivalent to `*get()`
  /// @warning The behavior is undefined if `get() == nullptr`
  [[nodiscard]] auto operator*() noexcept -> reference
  {
    return *data_;
  }

  /// @brief overload
  [[nodiscard]] auto operator*() const noexcept -> const_reference
  {
    return *data_;
  }

  /// @brief Return `true` if the `MappingPtr` is null
  [[nodiscard]] explicit operator bool() const noexcept
  {
    return data_ != nullptr;
  }

  /// @brief returns a pointer to the managed memory
  [[nodiscard]] auto data() noexcept -> pointer
  {
    return data_;
  }

  /// @overload
  [[nodiscard]] auto data() const noexcept -> const_pointer
  {
    return data_;
  }

  /// @brief Releases the managed mapping
  auto release() noexcept -> void;

  [[nodiscard]] auto begin() noexcept -> iterator
  {
    return data_;
  }

  [[nodiscard]] auto end() noexcept -> iterator
  {
    return data_ + size_;
  }

  [[nodiscard]] auto begin() const noexcept -> const_iterator
  {
    return data_;
  }

  [[nodiscard]] auto end() const noexcept -> const_iterator
  {
    return data_ + size_;
  }

  [[nodiscard]] auto cbegin() const noexcept -> const_iterator
  {
    return data_;
  }

  [[nodiscard]] auto cend() const noexcept -> const_iterator
  {
    return data_ + size_;
  }

private:
  Context* context_ = nullptr;
  Buffer buffer_{};

  T* data_ = nullptr;
  size_type size_{};
};

/**
 * @brief Interface of the graphics context
 */
class Context {
public:
  virtual ~Context();

  Context(const Context&) = delete;
  auto operator=(const Context&) -> Context& = delete;
  Context(Context&&) = delete;
  auto operator=(Context &&) -> Context& = delete;

  [[nodiscard]] virtual auto create_swapchain() -> Swapchain = 0;

  [[nodiscard]] virtual auto create_buffer(const BufferCreateInfo& create_info)
      -> Buffer = 0;

  /**
   * @brief Creates a compute pipeline
   * @note There are not way to destory a pipeline manually. All pipelines will
   * be destoryed when the context get destoryed.
   */
  [[nodiscard]] virtual auto
  create_compute_pipeline(const ComputePipelineCreateInfo& create_info)
      -> Pipeline = 0;

  /**
   * @brief Destories the device buffer.
   *
   * This function destories the device buffer refered to by `buffer_handle` and
   * release its underlying memory. If the `buffer_handle` does not refer to a
   * valid device buffer, this function will do nothing.
   */
  virtual auto destory_buffer(Buffer& buffer_handle) -> void = 0;

  /// @brief Submits a sequence of command buffers to execute
  virtual auto submit(gsl::span<SubmitInfo> infos) -> void = 0;

  template <typename T> auto map_memory(Buffer buffer) noexcept -> Mapping<T>
  {
    return Mapping<T>{*this, buffer};
  }

protected:
  Context() = default;

  template <typename T> friend class Mapping;

  struct MappingInfo {
    void* data = nullptr;
    std::size_t size = 0; // In bytes
  };

  /**
   * @brief Maps the underlying memory of buffer to a pointer
   *
   * After a successful call to `map_Mapping buffer memory is
   * considered to be currently host mapped. If the `buffer` handle does not
   * refer to an real buffer, or if its underlying memory is not host visible,
   * this function will return `nullptr`
   */
  [[nodiscard]] virtual auto map_memory_impl(Buffer buffer) noexcept
      -> MappingInfo = 0;

  /**
   * @brief Unmaps the underlying memory  of buffer
   */
  virtual auto unmap_memory_impl(Buffer buffer) noexcept -> void = 0;
};

/// @brief Create a graphics context
[[nodiscard]] auto create_context(Window& window) noexcept
    -> std::unique_ptr<Context>;

template <typename T>
Mapping<T>::Mapping(Context& context, Buffer buffer)
    : context_{&context}, buffer_{buffer}
{
  const auto info = context.map_memory_impl(buffer);
  data_ = static_cast<pointer>(info.data);
  size_ = info.size / sizeof(value_type);
}

template <typename T> Mapping<T>::~Mapping() noexcept
{
  if (*this) {
    context_->unmap_memory_impl(buffer_);
  }
}

template <typename T> auto Mapping<T>::release() noexcept -> void
{
  if (*this) {
    context_->unmap_memory_impl(buffer_);
  }
  context_ = nullptr;
  data_ = nullptr;
}

/** @}@} */

} // namespace beyond::graphics

#endif // BEYOND_GRAPHICS_BACKEND_HPP
