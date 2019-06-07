#ifndef BEYOND_CORE_MATH_ANGLE_HPP
#define BEYOND_CORE_MATH_ANGLE_HPP

#include <iostream>
#include <type_traits>

#include "constants.hpp"

/**
 * @file angle.hpp
 * @brief float point Degrees and Radians classes, along with literal operators.
 * @ingroup math
 */

namespace beyond {

/**
 * @addtogroup core
 * @{
 * @addtogroup math
 * @{
 */

template <typename Value> class Degree;

/**
 * @brief Radian angle wrapper
 * @see Degree
 */
template <typename T> class Radian {
public:
  static_assert(std::is_floating_point_v<T>);
  using ValueType = T;

  constexpr Radian() = default;
  explicit constexpr Radian(T v) noexcept : value_{v} {}

  /// @brief Converts a Degree to Radian
  constexpr Radian(Degree<T> r) noexcept
      : value_{r.value() * constant::pi<T> / 180}
  {
  }

  /**
   * @brief Convert from radian of another underlying type
   */
  template <typename U>
  explicit constexpr Radian(Radian<U> r) noexcept
      : value_{static_cast<T>(r.value())}
  {
  }

  /// @brief Gets the underlying numerical value of the Radian
  [[nodiscard]] constexpr auto value() const noexcept -> T
  {
    return value_;
  }

  /// @brief Adds another Radian to this radian
  constexpr auto operator+=(Radian<T> rhs) noexcept -> Radian&
  {
    value_ += rhs.value_;
    return *this;
  }

  /// @brief Subtracts another Radian to this radian
  constexpr auto operator-=(Radian<T> rhs) noexcept -> Radian&
  {
    value_ -= rhs.value_;
    return *this;
  }

  /// @brief Multiples a scalar to this radian
  constexpr auto operator*=(T rhs) noexcept -> Radian&
  {
    value_ *= rhs;
    return *this;
  }

  /// @brief Divides this radian by a scalar
  constexpr auto operator/=(T rhs) noexcept -> Radian&
  {
    value_ /= rhs;
    return *this;
  }

private:
  T value_ = 0;
};

/**
 * @brief Degree angle wrapper
 * @see Radian
 */
template <typename T> class Degree {
public:
  static_assert(std::is_floating_point_v<T>);
  using ValueType = T;

  constexpr Degree() = default;
  explicit constexpr Degree(T v) noexcept : value_{v} {}

  constexpr Degree(Radian<T> r) noexcept
      : value_{r.value() / constant::pi<T> * 180}
  {
  }

  /**
   * @brief Convert from Degree of another underlying type
   */
  template <typename U>
  explicit constexpr Degree(Degree<U> r) noexcept
      : value_{static_cast<T>(r.value())}
  {
  }

  /// @brief Gets the underlying numerical value of the Degree
  [[nodiscard]] constexpr auto value() const noexcept -> T
  {
    return value_;
  }

  /// @brief Adds another Degree to this Degree
  constexpr auto operator+=(Degree<T> rhs) noexcept -> Degree&
  {
    value_ += rhs.value_;
    return *this;
  }

  /// @brief Subtracts another Degree to this Degree
  constexpr auto operator-=(Degree<T> rhs) noexcept -> Degree&
  {
    value_ -= rhs.value_;
    return *this;
  }

  /// @brief Multiples a scalar to this Degree
  constexpr auto operator*=(T rhs) noexcept -> Degree&
  {
    value_ *= rhs;
    return *this;
  }

  /// @brief Divides this Degree by a scalar
  constexpr auto operator/=(T rhs) noexcept -> Degree&
  {
    value_ /= rhs;
    return *this;
  }

private:
  T value_ = 0;
};

/// @brief Negates the Radian
/// @related Radian
template <typename T>
[[nodiscard]] constexpr auto operator-(Radian<T> r) noexcept -> Radian<T>
{
  return Radian<T>{-r.value()};
}

/// @brief Adds two Radians
/// @related Radian
template <typename T>
[[nodiscard]] constexpr auto operator+(Radian<T> lhs, Radian<T> rhs) noexcept
    -> Radian<T>
{
  return Radian<T>{lhs.value() + rhs.value()};
}

/// @brief Subtracts two Radians
/// @related Radian
template <typename T>
[[nodiscard]] constexpr auto operator-(Radian<T> lhs, Radian<T> rhs) noexcept
    -> Radian<T>
{
  return Radian<T>{lhs.value() - rhs.value()};
}

/// @brief Multiplies Radian with a scalar
/// @related Radian
template <typename T>
[[nodiscard]] constexpr auto operator*(Radian<T> lhs, T rhs) noexcept
    -> Radian<T>
{
  return Radian<T>{lhs.value() * rhs};
}

/// @overload
/// @related Radian
template <typename T>
[[nodiscard]] constexpr auto operator*(T lhs, Radian<T> rhs) noexcept
    -> Radian<T>
{
  return Radian<T>{lhs * rhs.value()};
}

/// @brief Divides this Radian by a scalar
/// @related Radian
template <typename T>
[[nodiscard]] constexpr auto operator/(Radian<T> lhs, T rhs) noexcept
    -> Radian<T>
{
  return Radian<T>{lhs.value() / rhs};
}

/**
 * @brief Divides a Radian value by another Radian.
 * @related Radian
 */
template <typename T>
[[nodiscard]] constexpr auto operator/(Radian<T> lhs, Radian<T> rhs) noexcept
    -> T
{
  return lhs.value() / rhs.value();
}

/**
 * @brief Equality comparison between two `Radian`s
 * @related Radian
 */
template <typename T>
[[nodiscard]] constexpr auto operator==(Radian<T> lhs, Radian<T> rhs) noexcept
    -> bool
{
  return lhs.value() == rhs.value();
}

/**
 * @brief Inequality comparison between two `Radian`s
 * @related Radian
 */
template <typename T>
[[nodiscard]] constexpr auto operator!=(Radian<T> lhs, Radian<T> rhs) noexcept
    -> bool
{
  return lhs.value() != rhs.value();
}

/**
 * @brief Less than operator between two `Radian`s
 * @related Radian
 */
template <typename T>
[[nodiscard]] constexpr auto operator<(Radian<T> lhs, Radian<T> rhs) noexcept
    -> bool
{
  return lhs.value() < rhs.value();
}

/**
 * @brief Greater than operator between two `Radian`s
 * @related Radian
 */
template <typename T>
[[nodiscard]] constexpr auto operator>(Radian<T> lhs, Radian<T> rhs) noexcept
    -> bool
{
  return lhs.value() > rhs.value();
}

/**
 * @brief Less than or equal operator between two `Radian`s
 * @related Radian
 */
template <typename T>
[[nodiscard]] constexpr auto operator<=(Radian<T> lhs, Radian<T> rhs) noexcept
    -> bool
{
  return lhs.value() <= rhs.value();
}

/**
 * @brief Greater than or equal operator between two `Radian`s
 * @related Radian
 */
template <typename T>
[[nodiscard]] constexpr auto operator>=(Radian<T> lhs, Radian<T> rhs) noexcept
    -> bool
{
  return lhs.value() >= rhs.value();
}

/// @brief Construct a Radian<float> by a literial
/// @related Radian
[[nodiscard]] constexpr auto operator""_rad(long double v) noexcept
    -> Radian<float>
{
  return Radian<float>(static_cast<float>(v));
}

/// @brief Prints the Radian
/// @related Degree
template <typename T>
auto operator<<(std::ostream& os, Radian<T> r) -> std::ostream&
{
  os << r.value() << "_radian";
  return os;
}

/// @brief Negates the Degree
/// @related Degree
template <typename T>
[[nodiscard]] constexpr auto operator-(Degree<T> r) noexcept -> Degree<T>
{
  return Degree<T>{-r.value()};
}

/// @brief Adds two Degrees
/// @related Degree
template <typename T>
[[nodiscard]] constexpr auto operator+(Degree<T> lhs, Degree<T> rhs) noexcept
    -> Degree<T>
{
  return Degree<T>{lhs.value() + rhs.value()};
}

/// @brief Subtracts two Degrees
/// @related Degree
template <typename T>
[[nodiscard]] constexpr auto operator-(Degree<T> lhs, Degree<T> rhs) noexcept
    -> Degree<T>
{
  return Degree<T>{lhs.value() - rhs.value()};
}

/// @brief Multiplies Degree with a scalar
/// @related Degree
template <typename T>
[[nodiscard]] constexpr auto operator*(Degree<T> lhs, T rhs) noexcept
    -> Degree<T>
{
  return Degree<T>{lhs.value() * rhs};
}

/// @overload
/// @related Degree
template <typename T>
[[nodiscard]] constexpr auto operator*(T lhs, Degree<T> rhs) noexcept
    -> Degree<T>
{
  return Degree<T>{lhs * rhs.value()};
}

/// @brief Divides this Degree by a scalar
/// @related Degree
template <typename T>
[[nodiscard]] constexpr auto operator/(Degree<T> lhs, T rhs) noexcept
    -> Degree<T>
{
  return Degree<T>{lhs.value() / rhs};
}

/**
 * @brief Divides a Degree value by another Degree.
 * @related Degree
 */
template <typename T>
[[nodiscard]] constexpr auto operator/(Degree<T> lhs, Degree<T> rhs) noexcept
    -> T
{
  return lhs.value() / rhs.value();
}

/**
 * @brief Equality comparison between two `Degree`s
 * @related Degree
 */
template <typename T>
[[nodiscard]] constexpr auto operator==(Degree<T> lhs, Degree<T> rhs) noexcept
    -> bool
{
  return lhs.value() == rhs.value();
}

/**
 * @brief Inequality comparison between two `Degree`s
 * @related Degree
 */
template <typename T>
[[nodiscard]] constexpr auto operator!=(Degree<T> lhs, Degree<T> rhs) noexcept
    -> bool
{
  return lhs.value() != rhs.value();
}

/**
 * @brief Less than operator between two `Degree`s
 * @related Degree
 */
template <typename T>
[[nodiscard]] constexpr auto operator<(Degree<T> lhs, Degree<T> rhs) noexcept
    -> bool
{
  return lhs.value() < rhs.value();
}

/**
 * @brief Greater than operator between two `Degree`s
 * @related Degree
 */
template <typename T>
[[nodiscard]] constexpr auto operator>(Degree<T> lhs, Degree<T> rhs) noexcept
    -> bool
{
  return lhs.value() > rhs.value();
}

/**
 * @brief Less than or equal operator between two `Degree`s
 * @related Degree
 */
template <typename T>
[[nodiscard]] constexpr auto operator<=(Degree<T> lhs, Degree<T> rhs) noexcept
    -> bool
{
  return lhs.value() <= rhs.value();
}

/**
 * @brief Greater than or equal operator between two `Degree`s
 * @related Degree
 */
template <typename T>
[[nodiscard]] constexpr auto operator>=(Degree<T> lhs, Degree<T> rhs) noexcept
    -> bool
{
  return lhs.value() >= rhs.value();
}

/// @brief Construct a Degree<float> by a literial
/// @related Degree
[[nodiscard]] constexpr auto operator""_deg(long double v) noexcept
    -> Degree<float>
{
  return Degree<float>(static_cast<float>(v));
}

/// @brief Prints the Degree
/// @related Degree
template <typename T>
auto operator<<(std::ostream& os, Degree<T> r) -> std::ostream&
{
  os << r.value() << "_degree";
  return os;
}

/** @}
 *  @} */

} // namespace beyond

#endif // BEYOND_CORE_MATH_ANGLE_HPP
