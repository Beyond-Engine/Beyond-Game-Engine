#ifndef BEYOND_CORE_CONSTANTS_HPP
#define BEYOND_CORE_CONSTANTS_HPP

/**
 * @file constants.hpp
 * @brief This file contains various mathematics constants
 * @ingroup math
 */

namespace beyond {

/**
 * @addtogroup core
 * @{
 * @addtogroup math
 * @{
 */

/// @brief Value Templates for mathematics constants
namespace constant {
template <typename T>
constexpr T pi = static_cast<T>(
    3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798214808651);
}; // namespace constant

/// @brief `float` version of mathematics constants
/// @related constant
namespace float_constants {
constexpr float pi = beyond::constant::pi<float>;
}; // namespace float_constants

/// @brief `double` version of mathematics constants
namespace double_constants {
constexpr double pi = beyond::constant::pi<double>;
}; // namespace double_constants

/** @}
 *  @} */

} // namespace beyond

#endif // BEYOND_CORE_CONSTANTS_HPP
