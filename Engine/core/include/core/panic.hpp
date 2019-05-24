#ifndef BEYOND_CORE_PANIC_HPP
#define BEYOND_CORE_PANIC_HPP

/**
 * @file panic.hpp
 * @ingroup util
 */

#include <string_view>

namespace beyond {

/**
 * @addtogroup core
 * @{
 * @addtogroup util
 * @{
 */

/**
 * @brief Dumps some error messages and terminates the program
 * @param msg The error message to output before abort
 */
[[noreturn]] auto panic(std::string_view msg) -> void;

} // namespace beyond

/** @}@} */

#endif // BEYOND_CORE_PANIC_HPP
