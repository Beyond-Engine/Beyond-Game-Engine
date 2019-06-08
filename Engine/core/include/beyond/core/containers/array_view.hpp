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

/**
 * @brief ArrayView is an immutable view of an array
 * @warning Modification of the underlying array can invalidates the view
 */
template <typename T> class ArrayView {
public:
  using Iterator = const T*;

  /// @brief Default constructor
  constexpr ArrayView() noexcept = default;

  /// @brief Creates an ArrayView with a raw pointer and a size
  /// @warning If the `size` is greater than the actual number of the elements
  /// inside `data`, accessing this ArrayView will cause undefined behavior
  constexpr ArrayView(const T* data, std::size_t size) noexcept
      : size_{size}, data_{data}
  {
  }

  //// @brief Constructs an ArrayView from a raw array
  template <typename U, std::size_t size>
  explicit constexpr ArrayView(U (&data)[size]) noexcept
      : size_{size}, data_{data}
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
      : size_{array.size()}, data_{array.data()}
  {
  }

  template <typename ArrayLike>
  ArrayView(ArrayLike&& x,
            typename std::enable_if_t<!std::is_lvalue_reference_v<ArrayLike>,
                                      std::nullptr_t> = nullptr) = delete;

  /// @brief Returns true if the ArrayView is empty
  [[nodiscard]] constexpr auto empty() const noexcept -> bool
  {
    return size_ == 0;
  }

  /// @brief Gets the size of the ArrayView
  [[nodiscard]] constexpr auto size() const noexcept -> std::size_t
  {
    return size_;
  }

  /// @brief Gets the pointer to the raw buffer of the ArrayView
  [[nodiscard]] constexpr auto data() const noexcept -> const T*
  {
    return data_;
  }

  /// @brief Accesses an element
  /// @warning The result is undefined if `idx` is out of inde
  /// @param idx the index of the element to access
  [[nodiscard]] constexpr auto operator[](std::size_t idx) const noexcept
      -> const T&
  {
    BEYOND_ASSERT(idx < size_);
    return data_[idx];
  }

  /// @brief Accesses an element
  /// @warning The result is undefined if `idx` is out of inde
  /// @param idx the index of the element to access
  [[nodiscard]] constexpr auto operator()(std::size_t idx) const noexcept
      -> const T&
  {
    BEYOND_ASSERT(idx < size_);
    return data_[idx];
  }

  /// @brief Returns an iterator points to the beginning of the `ArrayView`
  [[nodiscard]] constexpr auto begin() const noexcept -> Iterator
  {
    return data_;
  }

  /// @brief Returns an iterator points to the end of the ArrayView
  [[nodiscard]] constexpr auto end() const noexcept -> Iterator
  {
    return data_ + size_;
  }

  /// @brief Returns the first element of the ArrayView
  /// @warning If the ArrayView is empty, the result is undefined
  [[nodiscard]] constexpr auto front() const noexcept -> const T&
  {
    return *data_;
  }

  /// @brief Returns the last element of the ArrayView
  /// @warning If the ArrayView is empty, the result is undefined
  [[nodiscard]] constexpr auto back() const noexcept -> const T&
  {
    return *(data_ + size_ - 1);
  }

private:
  std::size_t size_ = 0;
  const T* data_ = nullptr;
};

/** @}@} */

} // namespace beyond

#endif // BEYOND_CORE_CONTAINERS_ARRAY_VIEW_HPP
