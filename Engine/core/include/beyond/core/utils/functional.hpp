#ifndef BEYOND_CORE_UTILS_FUNCTIONAL_HPP
#define BEYOND_CORE_UTILS_FUNCTIONAL_HPP

#include <utility>

/**
 * @file functional.hpp
 * @brief This header provides various helper function objects
 * @ingroup util
 */

namespace beyond {

/**
 * @addtogroup core
 * @{
 * @addtogroup util
 * @{
 * @defgroup functional Function Object
 * Provides various helpful function objects
 * @{
 */

/// @brief Function object for `lhs = rhs`
template <typename T1, typename T2 = T1> struct Assign {
  static_assert(std::is_assignable_v<T1&, T2>);

  auto operator()(T1& lhs, T2&& rhs) const noexcept -> T1&
  {
    lhs = std::move(rhs);
    return lhs;
  }

  auto operator()(T1& lhs, const T2& rhs) const noexcept -> T1&
  {
    lhs = rhs;
    return lhs;
  }
};

/// @brief Function object for lhs += rhs
template <typename T1, typename T2 = T1> struct PlusEqual {
  static_assert(std::is_assignable_v<T1&, T2>);

  auto operator()(T1& lhs, T2&& rhs) const noexcept -> T1&
  {
    lhs += std::move(rhs);
    return lhs;
  }

  auto operator()(T1& lhs, const T2& rhs) const noexcept -> T1&
  {
    lhs += rhs;
    return lhs;
  }
};

/// @brief Function object for lhs -= rhs
template <typename T1, typename T2 = T1> struct MinusEqual {
  static_assert(std::is_assignable_v<T1&, T2>);

  auto operator()(T1& lhs, T2&& rhs) const noexcept -> T1&
  {
    lhs -= std::move(rhs);
    return lhs;
  }

  auto operator()(T1& lhs, const T2& rhs) const noexcept -> T1&
  {
    lhs -= rhs;
    return lhs;
  }
};

/** @}@}@} */

} // namespace beyond

#endif // BEYOND_CORE_UTILS_FUNCTIONAL_HPP
