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

/**
 * @brief The information used to create a GPU buffer
 */
struct BufferCreateInfo {
  std::uint32_t size = 0;
};

/// @brief A handle to buffer
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

class Context;

template <typename T> class MappingPtr {
public:
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;

  MappingPtr() = default;
  explicit MappingPtr(Context& context, Buffer buffer);
  ~MappingPtr() noexcept;

  MappingPtr(const MappingPtr&) = delete;
  auto operator=(const MappingPtr&) & -> MappingPtr& = delete;
  MappingPtr(MappingPtr&& other) noexcept
      : context_{std::exchange(other.context_, nullptr)},
        buffer_{std::move(other.buffer_)}, data_{std::exchange(other.data_,
                                                               nullptr)}
  {
  }
  auto operator=(MappingPtr&& other) & noexcept -> MappingPtr&
  {
    context_ = std::exchange(other.context_, nullptr);
    buffer_ = std::move(other.buffer_);
    data_ = std::exchange(other.data_, nullptr);
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

  /// @brief returns a pointer to the managed object
  [[nodiscard]] auto get() const noexcept -> const_pointer
  {
    return data_;
  }

  /// @overload
  [[nodiscard]] auto get() noexcept -> pointer
  {
    return data_;
  }

  /// @brief Releases the managed mapping
  auto release() noexcept -> void;

private:
  Context* context_ = nullptr;
  Buffer buffer_{};
  T* data_ = nullptr;
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

  /// @brief Submits a sequence of command buffers to execute
  virtual auto submit(gsl::span<SubmitInfo> infos) -> void = 0;

  template <typename T> auto map_memory(Buffer buffer) noexcept -> MappingPtr<T>
  {
    return MappingPtr<T>{*this, buffer};
  }

protected:
  Context() = default;

  template <typename T> friend class MappingPtr;

  /**
   * @brief Maps the underlying memory of buffer to a pointer
   *
   * After a successful call to `map_memory` the buffer memory is
   * considered to be currently host mapped. If the `buffer` handle does not
   * refer to an real buffer, or if its underlying memory is not host visible,
   * this function will return `nullptr`
   */
  [[nodiscard]] virtual auto map_memory_impl(Buffer buffer) noexcept
      -> void* = 0;

  /**
   * @brief Unmaps the underlying memory  of buffer
   */
  virtual auto unmap_memory_impl(Buffer buffer) noexcept -> void = 0;
};

/// @brief Create a graphics context
[[nodiscard]] auto create_context(Window& window) noexcept
    -> std::unique_ptr<Context>;

template <typename T>
MappingPtr<T>::MappingPtr(Context& context, Buffer buffer)
    : context_{&context}, buffer_{buffer}, data_{static_cast<pointer>(
                                               context.map_memory_impl(buffer))}
{
}

template <typename T> MappingPtr<T>::~MappingPtr() noexcept
{
  if (*this) {
    context_->unmap_memory_impl(buffer_);
  }
}

template <typename T> auto MappingPtr<T>::release() noexcept -> void
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
