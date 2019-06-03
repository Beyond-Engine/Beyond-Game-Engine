#ifndef BEYOND_CORE_UTILS_TYPE_EXPECTED_HPP
#define BEYOND_CORE_UTILS_TYPE_EXPECTED_HPP

/**
 * @file expected.hpp
 * @brief Provides expected class for local error handling
 * @ingroup util
 */

#include <type_traits>
#include <utility>

#include "beyond/core/utils/assert.hpp"

namespace beyond {

/**
 * @addtogroup core
 * @{
 * @addtogroup util
 * @{
 */

/// @brief A tag type to tell expected to construct the unexpected value
struct unexpect_t {
  unexpect_t() = default;
};
/// @brief A tag to tell expected to construct the unexpected value
static constexpr unexpect_t unexpect{};

/// @brief Used as a wrapper to store the unexpected value
template <typename E> class Unexpected {
  static_assert(std::is_nothrow_move_constructible_v<E>);
  static_assert(std::is_nothrow_move_assignable_v<E>);

  static_assert(!std::is_same<E, void>::value, "E must not be void");

public:
  Unexpected() = delete;
  constexpr explicit Unexpected(const E& e) : val_(e) {}
  constexpr explicit Unexpected(E&& e) noexcept : val_(std::move(e)) {}

  /// @brief Returns the contained value
  [[nodiscard]] constexpr auto value() const & noexcept -> const E&
  {
    return val_;
  }
  /// @overload
  [[nodiscard]] constexpr auto value() & noexcept -> E&
  {
    return val_;
  }
  /// @overload
  [[nodiscard]] constexpr auto value() && noexcept -> E&&
  {
    return std::move(val_);
  }
  /// @overload
  [[nodiscard]] constexpr auto value() const && noexcept -> const E&&
  {
    return std::move(val_);
  }

private:
  E val_;
};

/// @brief Constructs an object of type E and wraps it in a Unexpected
template <typename E>
[[nodiscard]] auto make_unexpected(E error) -> Unexpected<E>
{
  return Unexpected<E>{std::move(error)};
}

/**
 * @brief Stores either a value or an error
 * @tparam T Type of the value to store
 * @tparam E Type of the error
 * @note Unlike in the `std::expected` proposal, T and E must both be nothrow
 * movable to be used in this type
 */
template <typename T, typename E> class Expected {
  static_assert(std::is_nothrow_move_constructible_v<T>);
  static_assert(std::is_nothrow_move_constructible_v<E>);
  static_assert(std::is_nothrow_move_assignable_v<T>);
  static_assert(std::is_nothrow_move_assignable_v<E>);

  static_assert(!std::is_reference<T>::value, "T must not be a reference");
  static_assert(!std::is_same<T, std::remove_cv<std::in_place_t>>::value,
                "T must not be std::in_place_t");
  //  static_assert(!std::is_same<T, std::remove_cv<unexpect_t>>::value,
  //                "T must not be unexpect_t");
  static_assert(!std::is_same<T, std::remove_cv<Unexpected<E>>>::value,
                "T must not be Unexpected<E>");
  static_assert(!std::is_reference<E>::value, "E must not be a reference");

public:
  using value_type = T;
  using unexpected_type = E;

  /// @brief Default constructor of Expected
  constexpr Expected() = default;

  /// @cond
  constexpr Expected(const Expected& rhs) = default;
  constexpr Expected(Expected&& rhs) noexcept = default; // NOLINT
  Expected& operator=(const Expected& rhs) = default;
  Expected& operator=(Expected&& rhs) noexcept = default; // NOLINT
  ~Expected() noexcept
  {
    if (has_value()) {
      val_.~T();
    } else {
      unexpected_.~E();
    }
  }
  /// @endcond

  /// @brief Constructs the Expected with a value
  constexpr Expected(T value) : val_{std::move(value)} {} // NOLINT

  /// @brief In place construction of the value
  template <class... Args,
            std::enable_if_t<std::is_constructible_v<T, Args&&...>>* = nullptr>
  explicit constexpr Expected(std::in_place_t, Args&&... args)
      : val_{std::forward<Args>(args)...}
  {
  }

  /// @overload
  template <class U, class... Args>
  explicit constexpr Expected(std::in_place_t, std::initializer_list<U> list,
                              Args&&... args)
      : val_{list, std::forward<Args>(args)...}
  {
  }

  /// @brief Constructs Expected from an Unexpected directly
  constexpr Expected(Unexpected<E> e) noexcept // NOLINT
      : unexpected_{std::move(e.value())}, has_value_{false}
  {
  }

  /// @brief Constructs Expected from an Unexpected directly
  template <class... Args,
            std::enable_if_t<std::is_constructible_v<E, Args&&...>>* = nullptr>
  constexpr Expected(unexpect_t, Args... args) noexcept // NOLINT
      : unexpected_{std::forward<Args>(args)...}, has_value_{false}
  {
  }

  /// @brief Return true if this Expected holds a value, false if it holds
  /// an error
  [[nodiscard]] constexpr auto has_value() const noexcept -> bool
  {
    return has_value_;
  }

  /// @brief Convert this Expected to a boolean
  /// @return true if this Expected holds a value, false if it holds an error
  [[nodiscard]] constexpr explicit operator bool() const noexcept
  {
    return has_value_;
  }

  /// @brief Returns the holded value of the Expected
  /// @pre a value is stored
  /// @warning If the Expected holds an error, the result is undefined
  [[nodiscard]] constexpr auto operator*() const & noexcept -> const T&
  {
    BEYOND_ASSERT(has_value());
    return val_;
  }

  /// @overload
  [[nodiscard]] constexpr auto operator*() & noexcept -> T&
  {
    BEYOND_ASSERT(has_value());
    return val_;
  }

  /// @overload
  [[nodiscard]] constexpr auto operator*() const && noexcept -> const T&&
  {
    BEYOND_ASSERT(has_value());
    return std::move(val_);
  }

  /// @overload
  [[nodiscard]] constexpr auto operator*() && noexcept -> T&&
  {
    BEYOND_ASSERT(has_value());
    return std::move(val_);
  }

  /// @brief Returns the holded error of the Expected
  /// @pre an error is stored
  /// @warning If the Expected does not hold an error, the result is undefined
  [[nodiscard]] constexpr auto error() const & noexcept -> const T&
  {
    BEYOND_ASSERT(!has_value());
    return unexpected_;
  }

  /// @overload
  [[nodiscard]] constexpr auto error() & noexcept -> T&
  {
    BEYOND_ASSERT(!has_value());
    return unexpected_;
  }

  /// @overload
  [[nodiscard]] constexpr auto error() const && noexcept -> const T&&
  {
    BEYOND_ASSERT(!has_value());
    return std::move(unexpected_);
  }

  /// @overload
  [[nodiscard]] constexpr auto error() && noexcept -> T&&
  {
    BEYOND_ASSERT(!has_value());
    return std::move(unexpected_);
  }

  auto swap(Expected& rhs) noexcept
      -> std::enable_if_t<std::is_nothrow_swappable_v<T&> &&
                          std::is_nothrow_swappable_v<E&>>
  {
    if (has_value()) {
      if (rhs.has_value()) {
        std::swap(val_, rhs.val_);
      } else {
        auto temp = std::move(rhs.error());
        ::new (&rhs.val_) T(std::move(val_));
        ::new (&this->unexpected_) E(std::move(temp));
        std::swap(rhs.has_value_, this->has_value_);
      }
    } else {
      if (rhs.has_value()) {
        auto temp = std::move(this->error());
        ::new (&this->val_) T(std::move(rhs.val_));
        ::new (&rhs.unexpected_) E(std::move(temp));
        std::swap(this->has_value_, rhs.has_value_);
      } else {
        std::swap(unexpected_, rhs.unexpected_);
      }
    }
  }

private:
  union {
    value_type val_{};
    unexpected_type unexpected_;
  };
  bool has_value_ = true;
}; // namespace beyond

/// @brief Equality test of two expected
template <typename T1, typename E1, typename T2, typename E2>
[[nodiscard]] constexpr auto operator==(const Expected<T1, E1>& e1,
                                        const Expected<T2, E2>& e2) -> bool
{
  if (e1.has_value()) {
    return e2.has_value() ? *e1 == *e2 : false;
  } else {
    return e2.has_value() ? false : e1.error() == e2.error();
  }
}

/// @brief Inequality test of two expected
template <typename T1, typename E1, typename T2, typename E2>
[[nodiscard]] constexpr auto operator!=(const Expected<T1, E1>& e1,
                                        const Expected<T2, E2>& e2) -> bool
{
  return !(e1 == e2);
}

/// @brief Equality test between an Expected and an underlying value
template <typename T, typename E, typename U>
[[nodiscard]] constexpr auto operator==(const Expected<T, E>& e, const U& value)
    -> bool
{
  return e.has_value() ? *e == value : false;
}

/// @overload
template <typename T, typename E, typename U>
[[nodiscard]] constexpr auto operator==(const U& value, const Expected<T, E>& e)
    -> bool
{
  return e.has_value() ? *e == value : false;
}

/// @brief Inequality test between an Expected and an underlying value
template <typename T, typename E, typename U>
[[nodiscard]] constexpr auto operator!=(const Expected<T, E>& e, const U& value)
    -> bool
{
  return !(e == value);
}

/// @overload
template <typename T, typename E, typename U>
[[nodiscard]] constexpr auto operator!=(const U& value, const Expected<T, E>& e)
    -> bool
{
  return !(e == value);
}

/// @brief Equality test between an Expected and an underlying value
template <typename T, typename E1, typename E2>
[[nodiscard]] constexpr auto operator==(const Expected<T, E1>& e,
                                        const Unexpected<E2>& unexpected)
    -> bool
{
  return e.has_value() ? false : e.error() == unexpected.value();
}

/// @brief Equality test between an Expected and an underlying value
template <typename T, typename E1, typename E2>
[[nodiscard]] constexpr auto operator==(const Unexpected<E2>& unexpected,
                                        const Expected<T, E1>& e) -> bool
{
  return e.has_value() ? false : e.error() == unexpected.value();
}

/// @brief Equality test between an Expected and an underlying value
template <typename T, typename E1, typename E2>
[[nodiscard]] constexpr auto operator!=(const Expected<T, E1>& e,
                                        const Unexpected<E2>& unexpected)
    -> bool
{
  return !(e == unexpected);
}

/// @brief Equality test between an Expected and an underlying value
template <typename T, typename E1, typename E2>
[[nodiscard]] constexpr auto operator!=(const Unexpected<E2>& unexpected,
                                        const Expected<T, E1>& e) -> bool
{
  return !(e == unexpected);
}

/// @brief Swap two Expected
template <class T, class E,
          std::enable_if_t<std::is_nothrow_swappable_v<T&> &&
                           std::is_nothrow_swappable_v<E&>>* = nullptr>
auto swap(Expected<T, E>& lhs, Expected<T, E>& rhs) noexcept -> void
{
  lhs.swap(rhs);
}

} // namespace beyond
/** @} @} */

#endif // BEYOND_CORE_UTILS_TYPE_EXPECTED_HPP
