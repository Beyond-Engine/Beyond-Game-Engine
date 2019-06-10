#ifndef BEYOND_CORE_CONTAINERS_ARRAY_VIEW_HPP
#define BEYOND_CORE_CONTAINERS_ARRAY_VIEW_HPP

/**
 * @file array_view.hpp
 * @brief Provides an immutable view to array-like structures
 */

#include <cstddef>
#include <limits>
#include <type_traits>

#include "beyond/core/utils/assert.hpp"

namespace beyond {

/**
 * @addtogroup core
 * @{
 * @addtogroup containers
 * @{
 */

static constexpr std::size_t dynamic_extent =
    std::numeric_limits<std::size_t>::max();

namespace detail {
template <typename T, std::size_t extent> struct ArrayViewStorage {
  const T* data_ = nullptr;

  ArrayViewStorage() noexcept = default;
  ArrayViewStorage(const T* data, std::size_t) noexcept : data_{data} {}
};

template <typename T> struct ArrayViewStorage<T, dynamic_extent> {
  std::size_t size_ = 0;
  const T* data_ = nullptr;

  ArrayViewStorage() noexcept = default;
  ArrayViewStorage(const T* data, std::size_t size) noexcept
      : size_{size}, data_{data}
  {
  }
};

} // namespace detail

/**
 * @brief ArrayView is an immutable view of an array
 * @warning Modification of the underlying array can invalidates the view
 */
template <typename T, std::size_t extent = dynamic_extent>
class ArrayView : public detail::ArrayViewStorage<T, extent> {
public:
  using Iterator = const T*;

  /// @brief Default constructor
  constexpr ArrayView() noexcept = default;

  /// @brief Creates an ArrayView with a raw pointer and a size
  /// @warning If the `size` is greater than the actual number of the elements
  /// inside `data`, accessing this ArrayView will cause undefined behavior
  constexpr ArrayView(const T* data, std::size_t size) noexcept
      : detail::ArrayViewStorage<T, extent>{data, size}
  {
  }

  //// @brief Constructs an ArrayView from a raw array
  template <typename U, std::size_t size>
  explicit constexpr ArrayView(U (&data)[size]) noexcept
      : detail::ArrayViewStorage<T, extent>{data, size}
  {
  }

  /**
   * @brief Constructs from another ArrayView
   * @note Will only participant in overload resolution if constructing a
   * dynamic extent ArrayView
   */
  template <class U, std::size_t other_extent, std::size_t E = extent,
            typename std::enable_if_t<(E == dynamic_extent)> = 0>
  constexpr ArrayView(const beyond::ArrayView<U, other_extent>& other) noexcept
      : detail::ArrayViewStorage<T, extent>{other.size(), other.data()}
  {
  }

  /**
   * @brief Constructs an ArrayView with an external type
   * @tparam `ArrayLike` An lvalue reference to an array or vector like type
   * that support `size()` and `data()`. The underlying storage of `ArrayLike`
   * should be contiguous.
   */
  template <typename ArrayLike>
  explicit constexpr ArrayView(const ArrayLike& array) noexcept
      : detail::ArrayViewStorage<T, extent>{array.data(), array.size()}
  {
  }

  template <typename ArrayLike>
  ArrayView(ArrayLike&& x,
            typename std::enable_if_t<!std::is_lvalue_reference_v<ArrayLike>,
                                      std::nullptr_t> = nullptr) = delete;

  /// @brief Returns true if the ArrayView is empty
  [[nodiscard]] constexpr auto empty() const noexcept -> bool
  {
    return size() == 0;
  }

  /// @brief Gets the size of the ArrayView
  [[nodiscard]] constexpr auto size() const noexcept -> std::size_t
  {
    if constexpr (extent == dynamic_extent) {
      return this->size_;
    } else {
      return extent;
    }
  }

  /// @brief Gets the pointer to the raw buffer of the ArrayView
  [[nodiscard]] constexpr auto data() const noexcept -> const T*
  {
    return this->data_;
  }

  /// @brief Accesses an element
  /// @warning The result is undefined if `idx` is out of inde
  /// @param idx the index of the element to access
  [[nodiscard]] constexpr auto operator[](std::size_t idx) const noexcept
      -> const T&
  {
    BEYOND_ASSERT(idx < size());
    return data()[idx];
  }

  /// @brief Accesses an element
  /// @warning The result is undefined if `idx` is out of inde
  /// @param idx the index of the element to access
  [[nodiscard]] constexpr auto operator()(std::size_t idx) const noexcept
      -> const T&
  {
    BEYOND_ASSERT(idx < size());
    return data()[idx];
  }

  /// @brief Returns an iterator points to the beginning of the `ArrayView`
  [[nodiscard]] constexpr auto begin() const noexcept -> Iterator
  {
    return data();
  }

  /// @brief Returns an iterator points to the end of the ArrayView
  [[nodiscard]] constexpr auto end() const noexcept -> Iterator
  {
    return data() + size();
  }

  /// @brief Returns the first element of the ArrayView
  /// @warning If the ArrayView is empty, the result is undefined
  [[nodiscard]] constexpr auto front() const noexcept -> const T&
  {
    return *data();
  }

  /// @brief Returns the last element of the ArrayView
  /// @warning If the ArrayView is empty, the result is undefined
  [[nodiscard]] constexpr auto back() const noexcept -> const T&
  {
    return *(data() + size() - 1);
  }
};

/// @brief ArrayView deduction guide for pointer size
template <typename T>
ArrayView(const T* data, std::size_t size)->ArrayView<T, dynamic_extent>;

/// @brief ArrayView deduction guide for raw array
template <typename T, std::size_t size>
ArrayView(T (&data)[size])->ArrayView<T, size>;

/// @brief ArrayView deduction guide for std::array
template <typename T, std::size_t size>
ArrayView(const std::array<T, size>&)->ArrayView<T, size>;

/// @brief ArrayView deduction guide for dynamic sized containers
template <typename Container>
ArrayView(const Container&)
    ->ArrayView<typename Container::value_type, dynamic_extent>;

/** @}@} */

} // namespace beyond

#endif // BEYOND_CORE_CONTAINERS_ARRAY_VIEW_HPP
