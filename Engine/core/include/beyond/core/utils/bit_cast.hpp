#ifndef BEYOND_CORE_UTILS_BIT_CAST_HPP
#define BEYOND_CORE_UTILS_BIT_CAST_HPP

/**
 * @file bit_cast.hpp
 * @brief C++20 like `bit_cast`
 * @ingroup util
 */

#include <cstring>
#include <type_traits>

namespace beyond {

/**
 * @addtogroup core
 * @{
 */

/**
 * @defgroup util Utilities
 * @brief Utility functionalities
 * @ingroup core
 *
 * @{
 */

/**
 * @brief Implements safer equivalent of `*reinterpret_cast<Dest*>(&source)`
 */
template <typename Dest, typename Source>
[[nodiscard]] constexpr auto bit_cast(const Source& source) noexcept -> Dest
{
  static_assert(sizeof(Dest) == sizeof(Source),
                "size of destination and source objects must be equal");
  static_assert(std::is_trivially_copyable<Dest>::value,
                "destination type must be trivially copyable.");
  static_assert(std::is_trivially_copyable<Source>::value,
                "source type must be trivially copyable");

  Dest dest;
  std::memcpy(&dest, &source, sizeof(dest));
  return dest;
}

/** @}@} */

} // namespace beyond

#endif // BEYOND_CORE_UTILS_BIT_CAST_HPP
