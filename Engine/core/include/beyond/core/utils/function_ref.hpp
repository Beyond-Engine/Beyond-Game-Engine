#ifndef BEYOND_CORE_UTILS_FUNCTION_REF_HPP
#define BEYOND_CORE_UTILS_FUNCTION_REF_HPP

/**
 * @file function_ref.hpp
 * @brief Provides a non-owning reference to a function
 * @ingroup util
 */

#include <functional>
#include <type_traits>
#include <utility>

#include "bit_cast.hpp"

namespace beyond {

/**
 * @addtogroup core
 * @{
 * @addtogroup util
 * @{
 */

/// @brief The main template of `function_ref` is intentionaly undefined
template <class F> class function_ref;

/// @brief Provides a non-owning reference to a function
template <class R, class... Args>
class function_ref<auto(Args...)->R> { // NOLINT
public:
  constexpr function_ref() noexcept = delete;

  /// @brief Creates a `function_ref` which refers to the same callable as
  /// `rhs`.
  constexpr function_ref(const function_ref<auto(Args...)->R>& rhs) noexcept =
      default;

  /// @brief Constructs a `function_ref` referring to `f`
  template <
      typename F,
      std::enable_if_t<!std::is_same_v<std::decay_t<F>, function_ref> &&
                       std::is_invocable_r<R, F&&, Args...>::value>* = nullptr>
  constexpr function_ref(F&& f) noexcept // NOLINT
      : obj_(bit_cast<void*>(std::addressof(f)))
  {
    callback_ = [](void* obj, Args... args) -> R {
      return std::invoke(*bit_cast<typename std::add_pointer<F>::type>(obj),
                         std::forward<Args>(args)...);
    };
  }

  /// Makes `*this` refer to the same callable as `rhs`.
  constexpr auto operator=(const function_ref<R(Args...)>& rhs) noexcept
      -> function_ref<R(Args...)>& = default;

  /// @brief Makes `*this` refer to `f`.
  template <
      typename F,
      std::enable_if_t<std::is_invocable_r<R, F&&, Args...>::value>* = nullptr>
  constexpr auto operator=(F&& f) noexcept -> function_ref<R(Args...)>&
  {
    obj_ = bit_cast<void*>(std::addressof(f));
    callback_ = [](void* obj, Args... args) {
      return std::invoke(*bit_cast<typename std::add_pointer<F>::type>(obj),
                         std::forward<Args>(args)...);
    };

    return *this;
  }

  /// @brief Swaps the referred callables of `*this` and `rhs`.
  constexpr auto swap(function_ref<R(Args...)>& rhs) noexcept -> void
  {
    std::swap(obj_, rhs.obj_);
    std::swap(callback_, rhs.callback_);
  }

  /// @brief Invokes the stored callable with the given arguments.
  auto operator()(Args... args) const -> R
  {
    return callback_(obj_, std::forward<Args>(args)...);
  }

private:
  void* obj_ = nullptr;
  R (*callback_)(void*, Args...) = nullptr;
};

/// @brief Swaps the referred callables of `lhs` and `rhs`.
template <typename R, typename... Args>
constexpr auto swap(function_ref<R(Args...)>& lhs,
                    function_ref<R(Args...)>& rhs) noexcept -> void
{
  lhs.swap(rhs);
}

template <typename R, typename... Args>
function_ref(R (*)(Args...))->function_ref<R(Args...)>;

/** @}@} */

} // namespace beyond

#endif // BEYOND_CORE_UTILS_FUNCTION_REF_HPP
