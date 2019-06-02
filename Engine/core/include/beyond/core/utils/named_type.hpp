#ifndef BEYOND_CORE_UTILS_NAMED_TYPE_HPP
#define BEYOND_CORE_UTILS_NAMED_TYPE_HPP

#include <functional>
#include <utility>

#include "beyond/core/utils/crtp.hpp"
#include "beyond/core/utils/type_traits.hpp"

// Enable empty base class optimization with multiple inheritance on Visual
// Studio.
#if defined(_MSC_VER)
#define BEYOND_EBCO __declspec(empty_bases)
#else
#define BEYOND_EBCO
#endif

/**
 * @file named_type.hpp
 * @brief Provides named type used in place of another type to carry specific
 * meaning through its name.
 * @ingroup util
 */

namespace beyond {

/**
 * @addtogroup core
 * @{
 * @addtogroup util
 * @{
 */

/// @brief Support ++T and T++
/// @note The Derived type T should be a value wrapper that supports `.get()`
/// function
/// @note The wrapped type should support both prefix and postfix operator++
template <typename T> struct IncrementableBase : CRTP<T, IncrementableBase> {
  constexpr auto operator++() noexcept(noexcept(++this->underlying().get()))
  {
    ++this->underlying().get();
    return this->underlying();
  }

  constexpr auto operator++(int) noexcept(noexcept(this->underlying().get()++))
  {
    const auto ret = this->underlying().get()++;
    return T{ret};
  }
};

/// @brief Support ++T and T++
/// @note The Derived type T should be a value wrapper that supports `.get()`
/// function
/// @note The wrapped type should support both prefix and postfix operator++
template <typename T> struct DecrementableBase : CRTP<T, DecrementableBase> {
  constexpr auto operator--() noexcept(noexcept(++this->underlying().get()))
  {
    --this->underlying().get();
    return this->underlying();
  }

  constexpr auto operator--(int) noexcept(noexcept(this->underlying().get()++))
  {
    const auto ret = this->underlying().get()--;
    return T{ret};
  }
};

/// @brief Support T + T
/// @note The Derived type T should be a value wrapper that supports `.get()`
/// function
/// @note The wrapped type should support binary operator +
template <typename T> struct AddableBase : CRTP<T, AddableBase> {
  constexpr auto operator+(const T& other) const
      noexcept(noexcept(this->underlying().get() + other.get())) -> T
  {
    return T{this->underlying().get() + other.get()};
  }
};

/// @brief Support T - T
/// @note The Derived type T should be a value wrapper that supports `.get()`
/// function
/// @note The wrapped type should support binary operator -
template <typename T> struct SubtractableBase : CRTP<T, SubtractableBase> {
  constexpr auto operator-(const T& other) const
      noexcept(noexcept(this->underlying().get() - other.get())) -> T
  {
    return T{this->underlying().get() - other.get()};
  }
};

/// @brief Support -T
/// @note The Derived type T should be a value wrapper that supports `.get()`
/// function
/// @note The wrapped type should support unary operator -
template <typename T> struct NegatabeBase : CRTP<T, NegatabeBase> {
  constexpr auto operator-() const noexcept(noexcept(-this->underlying().get()))
      -> T
  {
    return T{-this->underlying().get()};
  }
};

//

/// @brief Support T == T, T != T
/// @note The Derived type T should be a value wrapper that supports `.get()`
/// function
/// @note The wrapped type should support operator == and !=
template <typename T> struct EquableBase : CRTP<T, EquableBase> {
  constexpr auto operator==(const T& other) const
      noexcept(noexcept(this->underlying().get() == other.get())) -> bool
  {
    return this->underlying().get() == other.get();
  }

  constexpr auto operator!=(const T& other) const
      noexcept(noexcept(this->underlying().get() == other.get())) -> bool
  {
    return this->underlying().get() != other.get();
  }
};

/// @brief ComparableBase is EquableBase | support T < T, T <= T, T > T, T >= T
/// @note The Derived type T should be a value wrapper that supports `.get()`
/// function
/// @note The wrapped type should support all comparison operations
template <typename T> struct ComparableBase : EquableBase<T> {
  constexpr auto operator<(const T& other) const
      noexcept(noexcept(this->underlying().get() < other.get())) -> bool
  {
    return this->underlying().get() < other.get();
  }

  constexpr auto operator<=(const T& other) const
      noexcept(noexcept(this->underlying().get() <= other.get())) -> bool
  {
    return this->underlying().get() <= other.get();
  }

  constexpr auto operator>(const T& other) const
      noexcept(noexcept(this->underlying().get() > other.get())) -> bool
  {
    return this->underlying().get() > other.get();
  }

  constexpr auto operator>=(const T& other) const
      noexcept(noexcept(this->underlying().get() >= other.get())) -> bool
  {
    return this->underlying().get() >= other.get();
  }
};

template <typename T, typename Tag, template <typename> typename... Mixins>
class BEYOND_EBCO NamedType : public Mixins<NamedType<T, Tag, Mixins...>>... {
public:
  using UnderlyingType = T;
  using Ref = NamedType<T&, Tag, Mixins...>;
  using ConstRef = NamedType<const T&, Tag, Mixins...>;

  /// @brief Create a NamedType from a lvalue of the underlying type
  constexpr explicit NamedType(const T& value) : value_{value} {}

  /// @brief Create a NamedType from a rvalue of the underlying type
  template <typename U = T,
            typename = std::enable_if_t<!std::is_reference_v<U>>>
  constexpr explicit NamedType(T&& value) : value_{std::move(value)}
  {
  }

  /// @brief Gets a reference with the type of the underlying type
  [[nodiscard]] constexpr auto get() noexcept -> T&
  {
    return value_;
  }

  /// @overload
  [[nodiscard]] constexpr auto get() const noexcept -> const T&
  {
    return value_;
  }

  /// @brief Converts to the NamedType of the reference of the underlying type
  operator Ref()
  {
    return Ref{value_};
  }

  /// @brief Converts to the NamedType of the const reference of the underlying
  /// type
  operator ConstRef() const
  {
    return ConstRef{value_};
  }

private:
  T value_;
};

/** @}@} */

} // namespace beyond

#endif // BEYOND_CORE_UTILS_NAMED_TYPE_HPP
