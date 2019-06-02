#ifndef BEYOND_CORE_UTILS_TYPE_TRAITS_HPP
#define BEYOND_CORE_UTILS_TYPE_TRAITS_HPP

/**
 * @file type_traits.hpp
 * @brief A complementary set of type traits to the standard \<type_traits\>
 * @ingroup util
 */

#include <type_traits>

namespace beyond {

/**
 * @addtogroup core
 * @{
 * @addtogroup util
 * @{
 */

namespace detail {

template <class T, std::size_t = sizeof(T)> std::true_type is_complete_impl(T*);
std::false_type is_complete_impl(...);

} // namespace detail

/**
 * @brief Test if the type is a complete type
 * @tparam T The type to test
 * @return True if the type T is complete, false if it is incomplete type
 */
template <class T>
using is_complete = decltype(detail::is_complete_impl(std::declval<T*>()));

/** @} @} */

} // namespace beyond

#endif // BEYOND_CORE_UTILS_TYPE_TRAITS_HPP
