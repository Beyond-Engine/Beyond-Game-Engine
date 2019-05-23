#ifndef BEYOND_CORE_ASSERT_HPP
#define BEYOND_CORE_ASSERT_HPP

#ifdef BEYOND_GAME_ENGINE_CORE_DEBUG
#include <iostream>
#include <string_view>
#endif

#ifdef BEYOND_GAME_ENGINE_CORE_DEBUG
#define BEYOND_ASSERT(condition, message)                                      \
  do {                                                                         \
    if (!(condition)) {                                                        \
      []() {                                                                   \
        std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] "                \
                  << "Assert failed in "                                       \
                  << std::string_view{static_cast<const char*>(__func__)}      \
                  << ": "                                                      \
                  << std::string_view{static_cast<const char*>(message)}       \
                  << '\n'                                                      \
                  << "This is probabaly an internal bug of the Beyond Game "   \
                     "Engine Implementation, please fill a bug report.\n";     \
        std::abort();                                                          \
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
    std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] "                    \
              << "This code should not be reached in "                         \
              << std::string_view{static_cast<const char*>(__func__)}          \
              << "\nThis is probabaly an internal bug of the Beyond Game "     \
                 "Engine Implementation, please fill a bug report.\n";         \
    std::abort();                                                              \
  } while (0)
#else
#define BEYOND_ASSERT(condition, message)                                      \
  do {                                                                         \
  } while (0)

// Tell the compiler that this part of the code will never be reached.
#if defined(_MSC_VER)
#define BEYOND_UNREACHABLE() __assume(0)
#elif defined(__GNUC__) || defined(__GNUG__) || defined(__clang__)
#define BEYOND_UNREACHABLE() __builtin_unreachable()
#else
#define BEYOND_UNREACHABLE()
#endif
#endif

#endif // BEYOND_CORE_ASSERT_HPP
