#ifndef BEYOND_CORE_MATH_HPP
#define BEYOND_CORE_MATH_HPP

/**
 * @file math.hpp
 * @brief This file contains various mathmatic utility functions
 * @ingroup math
 */

/**
 * @defgroup math Math
 * @brief Mathematics and geometry codes of the beyond game engine
 * @ingroup core
 */

#include "beyond/core/math/angle.hpp"

#include <cmath>
#include <type_traits>

namespace beyond {

/**
 * @addtogroup core
 * @{
 * @addtogroup math
 * @{
 */

/**
 * @brief Computes the sine of arg
 * @tparam Angle The type of the angle, can be either degree or radian
 * @note Unlike the standard library conterpart, this function only accept
 * either Degree or Radian
 * @see Degree
 * @see Radian
 */
template <typename Angle>
[[nodiscard]] inline auto sin(Angle arg) noexcept -> typename Angle::ValueType
{
  beyond::Radian r{arg};
  return std::sin(r.value());
}

/**
 * @brief Computes the cosine of arg
 * @tparam Angle The type of the angle, can be either degree or radian
 * @note Unlike the standard library conterpart, this function only accept
 * either Degree or Radian
 * @see Degree
 * @see Radian
 */
template <typename Angle>
[[nodiscard]] inline auto cos(Angle arg) noexcept -> typename Angle::ValueType
{
  beyond::Radian r{arg};
  return std::cos(r.value());
}

/**
 * @brief Computes the tangent of arg
 * @tparam Angle The type of the angle, can be either degree or radian
 * @note Unlike the standard library conterpart, this function only accept
 * either Degree or Radian
 * @see Degree
 * @see Radian
 */
template <typename Angle>
[[nodiscard]] inline auto tan(Angle arg) noexcept -> typename Angle::ValueType
{
  beyond::Radian r{arg};
  return std::tan(r.value());
}

/**
 * @brief Computes the principal value of the arc sine of arg
 * @tparam T A floating-point type
 * @see Radian
 */
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
[[nodiscard]] inline auto asin(T arg) noexcept -> Radian<T>
{
  return beyond::Radian<T>{std::asin(arg)};
}

/**
 * @overload
 * @tparam T An integral type
 * @see Radian
 */
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
[[nodiscard]] inline auto asin(T arg) noexcept -> Radian<double>
{
  return beyond::Radian<double>{std::asin(arg)};
}

/**
 * @brief Computes the principal value of the arc cosine of arg
 * @tparam T A floating-point type
 * @see Radian
 */
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
[[nodiscard]] inline auto acos(T arg) noexcept -> Radian<T>
{
  return beyond::Radian<T>{std::acos(arg)};
}

/**
 * @overload
 * @tparam T An integral type
 * @see Radian
 */
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
[[nodiscard]] inline auto acos(T arg) noexcept -> Radian<double>
{
  return beyond::Radian<double>{std::acos(arg)};
}

/**
 * @brief Computes the principal value of the arc tangent of arg
 * @tparam T A floating-point type
 * @see Radian
 */
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
[[nodiscard]] inline auto atan(T arg) noexcept -> Radian<T>
{
  return beyond::Radian<T>{std::atan(arg)};
}

/**
 * @overload
 * @tparam T An integral type
 * @see Radian
 */
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
[[nodiscard]] inline auto atan(T arg) noexcept -> Radian<double>
{
  return beyond::Radian<double>{std::atan(arg)};
}

/**
 * @brief Computes the arc tangent of y/x using the signs of arguments to
 * determine the correct quadrant.
 * @tparam T1 A floating-point type for y
 * @tparam T2 A floating-point type for x
 * @see Radian
 */
template <typename T1, typename T2,
          typename = std::enable_if_t<std::conjunction_v<
              std::is_arithmetic<T1>, std::is_arithmetic<T2>>>>
[[nodiscard]] inline auto atan2(T1 y, T2 x) noexcept
{
  using PromotedType = std::common_type_t<T1, T2>;
  if constexpr (std::is_integral_v<PromotedType>) {
    return Radian<double>{std::atan2(y, x)};
  } else {
    return Radian<PromotedType>{std::atan2(y, x)};
  }
}

/**
 * @brief Linear interpolation of two values.
 * @param a First value
 * @param b Second value
 * @param t Interpolation phase (from range \f$[0, 1]\f$)
 */
template <typename T1, typename T2, typename T3,
          typename = std::enable_if_t<std::is_arithmetic_v<T3>>>
[[nodiscard]] constexpr auto lerp(const T1& a, const T2& b,
                                  const T3& t) noexcept
    -> std::common_type_t<T1, T2, T3>
{
  using ResultType = std::common_type_t<T1, T2, T3>;
  return b * (static_cast<ResultType>(1) - t) + (a * t);
}

using std::abs;
using std::fabs;
using std::fdim;
using std::fma;
using std::fmod;
using std::remainder;
using std::remquo;

using std::exp;
using std::exp2;
using std::expm1;
using std::log;
using std::log10;
using std::log1p;
using std::log2;

using std::cbrt;
using std::hypot;
using std::pow;
using std::sqrt;

using std::acosh;
using std::asinh;
using std::atanh;
using std::cosh;
using std::sinh;
using std::tanh;

using std::ceil;
using std::floor;
using std::nearbyint;
using std::round;
using std::trunc;

/** @}
 *  @} */

} // namespace beyond

#endif // BEYOND_CORE_MATH_HPP
