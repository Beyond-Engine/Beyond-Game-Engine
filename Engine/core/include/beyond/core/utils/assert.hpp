#ifndef BEYOND_CORE_UTILS_ASSERT_HPP
#define BEYOND_CORE_UTILS_ASSERT_HPP

/**
 * @file assert.hpp
 * @brief Assertions for the Beyond Game Engine Core
 * @ingroup util
 */

/**
 * @addtogroup core
 * @{
 * @addtogroup util
 * @{
 */

#ifdef BEYOND_GAME_ENGINE_CORE_ENABLE_ASSERT
#include "beyond/core/utils/panic.hpp"
#include <fmt/format.h>
#include <string_view>
#endif

#ifdef BEYOND_GAME_ENGINE_CORE_ENABLE_ASSERT
#define BEYOND_ASSERT(condition)                                               \
  do {                                                                         \
    if (!(condition)) {                                                        \
      []() {                                                                   \
        ::beyond::panic(fmt::format("[{}:{}] Assert failed in {}\n", __FILE__, \
                                    __LINE__, __func__));                      \
      }();                                                                     \
    }                                                                          \
  } while (0)

#define BEYOND_ASSERT_MSG(condition, message)                                  \
  do {                                                                         \
    if (!(condition)) {                                                        \
      []() {                                                                   \
        ::beyond::panic(fmt::format("[{}:{}] Assert failed in {}: {}\n",       \
                                    __FILE__, __LINE__, __func__, message));   \
      }();                                                                     \
    }                                                                          \
  } while (0)

// Indicates that we know execution should never reach this point in the
// program. In debug mode, we assert this fact because it's a bug to get here.
//
// In release mode, we use compiler-specific built in functions to tell the
// compiler the code can't be reached. This avoids "missing return" warnings
// in some cases and also lets it perform some optimizations by assuming the
// code is never reached.
#define BEYOND_UNREACHABLE()                                                   \
  do {                                                                         \
    panic(fmt::format("[{}:{}] Reach unreachable code {}\n", __FILE__,         \
                      __LINE__, __func__));                                    \
  } while (0)
#else
#define BEYOND_ASSERT(condition)                                               \
  do {                                                                         \
  } while (0)

#define BEYOND_ASSERT_MSG(condition, message)                                  \
  do {                                                                         \
  } while (0)

// Tell the compiler that this part of the code will never be reached.
#if defined(_MSC_VER)
#define BEYOND_UNREACHABLE() __assume(0)
#elif defined(__GNUC__) || defined(__GNUG__) || defined(__clang__)
#define BEYOND_UNREACHABLE() __builtin_unreachable()
#else
#define BEYOND_UNREACHABLE() std::terminate()
#endif
#endif

/** @}
 *  @} */

#endif // BEYOND_CORE_UTILS_ASSERT_HPP
