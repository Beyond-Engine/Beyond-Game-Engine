#ifndef BEYOND_CORE_CONTAINERS_ARRAY_VIEW_HPP
#define BEYOND_CORE_CONTAINERS_ARRAY_VIEW_HPP

/**
 * @file array_view.hpp
 * @brief Provides an immutable view to array-like structures
 */

#include <cstddef>
#include <type_traits>

namespace beyond {

/**
 * @addtogroup core
 * @{
 * @addtogroup containers
 * @{
 */

/**
 * @brief ArrayView is an immutable view of an array
 */
template <typename T> class ArrayView {
public:
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
   * @tparam ArrayLike An lvalue reference to an array or vector like type that
   * support `size()` and `data()`
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

private:
  std::size_t size_ = 0;
  const T* data_ = nullptr;
};

/** @}@} */

} // namespace beyond

#endif // BEYOND_CORE_CONTAINERS_ARRAY_VIEW_HPP
