/// @file expected.hpp
/// @brief expected - An implementation of std::expected with extensions
/// Written in 2017 by Simon Brand (\@TartanLlama)
/// Adopted by Lesley Lai in 2018 for the Embedded ML project
// To the extent possible under law, the author(s) have dedicated all
// copyright and related and neighboring rights to this software to the
// public domain worldwide. This software is distributed without any warranty.
//
// You should have received a copy of the CC0 Public Domain Dedication
// along with this software. If not, see
// <http://creativecommons.org/publicdomain/zero/1.0/>.

#ifndef BEYOND_CORE_UTILS_EXPECTED_HPP
#define BEYOND_CORE_UTILS_EXPECTED_HPP

#include <exception>
#include <functional>
#include <type_traits>
#include <utility>

#include "beyond/core/utils/assert.hpp"

namespace beyond {

/**
 * @addtogroup core
 * @{
 * @addtogroup util
 * @{
 */

template <class T, class E> class Expected;

#ifndef BEYOND_MONOSTATE_INPLACE_MUTEX
#define BEYOND_MONOSTATE_INPLACE_MUTEX
/// @brief Used to represent an expected with no data
class monostate {
};

#endif

/// Used as a wrapper to store the Unexpected value
template <class E> class Unexpected {
public:
  static_assert(!std::is_same<E, void>::value, "E must not be void");

  Unexpected() = delete;
  constexpr explicit Unexpected(const E& e) : val_(e) {}

  constexpr explicit Unexpected(E&& e) : val_(std::move(e)) {}

  /// @brief Returns the contained value
  constexpr const E& value() const&
  {
    return val_;
  }
  /// @overload
  constexpr E& value() &
  {
    return val_;
  }
  /// @overload
  constexpr E&& value() &&
  {
    return std::move(val_);
  }
  /// @overload
  constexpr const E&& value() const&&
  {
    return std::move(val_);
  }

private:
  E val_;
};

template <class E>
constexpr bool operator==(const Unexpected<E>& lhs, const Unexpected<E>& rhs)
{
  return lhs.value() == rhs.value();
}
template <class E>
constexpr bool operator!=(const Unexpected<E>& lhs, const Unexpected<E>& rhs)
{
  return lhs.value() != rhs.value();
}
template <class E>
constexpr bool operator<(const Unexpected<E>& lhs, const Unexpected<E>& rhs)
{
  return lhs.value() < rhs.value();
}
template <class E>
constexpr bool operator<=(const Unexpected<E>& lhs, const Unexpected<E>& rhs)
{
  return lhs.value() <= rhs.value();
}
template <class E>
constexpr bool operator>(const Unexpected<E>& lhs, const Unexpected<E>& rhs)
{
  return lhs.value() > rhs.value();
}
template <class E>
constexpr bool operator>=(const Unexpected<E>& lhs, const Unexpected<E>& rhs)
{
  return lhs.value() >= rhs.value();
}

/// @brief Creates an `Unexpected` from `e`, deducing the return type
///
/// @example
/// ```cpp
/// auto e1 = beyond::make_unexpected(42);
/// Unexpected<int> e2 (42); //same semantics
/// ```
template <class E>
Unexpected<typename std::decay<E>::type> make_unexpected(E&& e)
{
  return Unexpected<typename std::decay<E>::type>(std::forward<E>(e));
}

/// @brief A tag type to tell expected to construct the Unexpected value
struct unexpect_t {
  unexpect_t() = default;
};
/// @brief A tag to tell expected to construct the Unexpected value
static constexpr unexpect_t unexpect{};

/// @exclude
namespace detail {

// Trait for checking if a type is a beyond::expected
template <class T> struct is_expected_impl : std::false_type {
};
template <class T, class E>
struct is_expected_impl<Expected<T, E>> : std::true_type {
};
template <class T> using is_expected = is_expected_impl<std::decay_t<T>>;
template <class T> constexpr bool is_expected_v = is_expected<T>::value;

template <class T, class E, class U>
using expected_enable_forward_value =
    std::enable_if_t<std::is_constructible<T, U&&>::value &&
                     !std::is_same<std::decay_t<U>, std::in_place_t>::value &&
                     !std::is_same<Expected<T, E>, std::decay_t<U>>::value &&
                     !std::is_same<Unexpected<E>, std::decay_t<U>>::value>;

template <class T, class E, class U, class G, class UR, class GR>
using expected_enable_from_other =
    std::enable_if_t<std::is_constructible<T, UR>::value &&
                     std::is_constructible<E, GR>::value &&
                     !std::is_constructible<T, Expected<U, G>&>::value &&
                     !std::is_constructible<T, Expected<U, G>&&>::value &&
                     !std::is_constructible<T, const Expected<U, G>&>::value &&
                     !std::is_constructible<T, const Expected<U, G>&&>::value &&
                     !std::is_convertible<Expected<U, G>&, T>::value &&
                     !std::is_convertible<Expected<U, G>&&, T>::value &&
                     !std::is_convertible<const Expected<U, G>&, T>::value &&
                     !std::is_convertible<const Expected<U, G>&&, T>::value>;

template <class T, class U>
using is_void_or =
    std::conditional_t<std::is_void<T>::value, std::true_type, U>;

template <class T>
using is_copy_constructible_or_void =
    is_void_or<T, std::is_copy_constructible<T>>;

template <class T>
using is_move_constructible_or_void =
    is_void_or<T, std::is_move_constructible<T>>;

template <class T>
using is_copy_assignable_or_void = is_void_or<T, std::is_copy_assignable<T>>;

template <class T>
using is_move_assignable_or_void = is_void_or<T, std::is_move_assignable<T>>;

} // namespace detail

/// @exclude
namespace detail {
struct no_init_t {
};
static constexpr no_init_t no_init{};

// Implements the storage of the values, and ensures that the destructor is
// trivial if it can be.
//
// This specialization is for where neither `T` or `E` is trivially
// destructible, so the destructors must be called on destruction of the
// `Expected`
template <class T, class E, bool = std::is_trivially_destructible<T>::value,
          bool = std::is_trivially_destructible<E>::value>
struct ExpectedStorageBase {
  constexpr ExpectedStorageBase() : val_(T{}), has_val_(true) {}
  constexpr ExpectedStorageBase(no_init_t) : m_no_init(), has_val_(false) {}

  template <
      class... Args,
      std::enable_if_t<std::is_constructible<T, Args&&...>::value>* = nullptr>
  constexpr ExpectedStorageBase(std::in_place_t, Args&&... args)
      : val_(std::forward<Args>(args)...), has_val_(true)
  {
  }

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible<
                T, std::initializer_list<U>&, Args&&...>::value>* = nullptr>
  constexpr ExpectedStorageBase(std::in_place_t, std::initializer_list<U> il,
                                Args&&... args)
      : val_(il, std::forward<Args>(args)...), has_val_(true)
  {
  }
  template <
      class... Args,
      std::enable_if_t<std::is_constructible<E, Args&&...>::value>* = nullptr>
  constexpr explicit ExpectedStorageBase(unexpect_t, Args&&... args)
      : error_(std::forward<Args>(args)...), has_val_(false)
  {
  }

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible<
                E, std::initializer_list<U>&, Args&&...>::value>* = nullptr>
  constexpr explicit ExpectedStorageBase(unexpect_t,
                                         std::initializer_list<U> il,
                                         Args&&... args)
      : error_(il, std::forward<Args>(args)...), has_val_(false)
  {
  }

  ~ExpectedStorageBase()
  {
    if (has_val_) {
      val_.~T();
    } else {
      error_.~Unexpected<E>();
    }
  }
  union {
    char m_no_init;
    T val_;
    Unexpected<E> error_;
  };
  bool has_val_;
};

// This specialization is for when both `T` and `E` are trivially-destructible,
// so the destructor of the `Expected` can be trivial.
template <class T, class E> struct ExpectedStorageBase<T, E, true, true> {
  constexpr ExpectedStorageBase() : val_(T{}), has_val_(true) {}
  constexpr ExpectedStorageBase(no_init_t) : m_no_init(), has_val_(false) {}

  template <
      class... Args,
      std::enable_if_t<std::is_constructible<T, Args&&...>::value>* = nullptr>
  constexpr ExpectedStorageBase(std::in_place_t, Args&&... args)
      : val_(std::forward<Args>(args)...), has_val_(true)
  {
  }

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible<
                T, std::initializer_list<U>&, Args&&...>::value>* = nullptr>
  constexpr ExpectedStorageBase(std::in_place_t, std::initializer_list<U> il,
                                Args&&... args)
      : val_(il, std::forward<Args>(args)...), has_val_(true)
  {
  }
  template <
      class... Args,
      std::enable_if_t<std::is_constructible<E, Args&&...>::value>* = nullptr>
  constexpr explicit ExpectedStorageBase(unexpect_t, Args&&... args)
      : error_(std::forward<Args>(args)...), has_val_(false)
  {
  }

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible<
                E, std::initializer_list<U>&, Args&&...>::value>* = nullptr>
  constexpr explicit ExpectedStorageBase(unexpect_t,
                                         std::initializer_list<U> il,
                                         Args&&... args)
      : error_(il, std::forward<Args>(args)...), has_val_(false)
  {
  }

  ~ExpectedStorageBase() = default;
  union {
    char m_no_init;
    T val_;
    Unexpected<E> error_;
  };
  bool has_val_;
};

// T is trivial, E is not.
template <class T, class E> struct ExpectedStorageBase<T, E, true, false> {
  constexpr ExpectedStorageBase() : val_(T{}), has_val_(true) {}
  constexpr ExpectedStorageBase(no_init_t) : m_no_init(), has_val_(false) {}

  template <
      class... Args,
      std::enable_if_t<std::is_constructible<T, Args&&...>::value>* = nullptr>
  constexpr ExpectedStorageBase(std::in_place_t, Args&&... args)
      : val_(std::forward<Args>(args)...), has_val_(true)
  {
  }

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible<
                T, std::initializer_list<U>&, Args&&...>::value>* = nullptr>
  constexpr ExpectedStorageBase(std::in_place_t, std::initializer_list<U> il,
                                Args&&... args)
      : val_(il, std::forward<Args>(args)...), has_val_(true)
  {
  }
  template <
      class... Args,
      std::enable_if_t<std::is_constructible<E, Args&&...>::value>* = nullptr>
  constexpr explicit ExpectedStorageBase(unexpect_t, Args&&... args)
      : error_(std::forward<Args>(args)...), has_val_(false)
  {
  }

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible<
                E, std::initializer_list<U>&, Args&&...>::value>* = nullptr>
  constexpr explicit ExpectedStorageBase(unexpect_t,
                                         std::initializer_list<U> il,
                                         Args&&... args)
      : error_(il, std::forward<Args>(args)...), has_val_(false)
  {
  }

  ~ExpectedStorageBase()
  {
    if (!has_val_) {
      error_.~Unexpected<E>();
    }
  }

  union {
    char m_no_init;
    T val_;
    Unexpected<E> error_;
  };
  bool has_val_;
};

// E is trivial, T is not.
template <class T, class E> struct ExpectedStorageBase<T, E, false, true> {
  constexpr ExpectedStorageBase() : val_(T{}), has_val_(true) {}
  constexpr ExpectedStorageBase(no_init_t) : m_no_init(), has_val_(false) {}

  template <
      class... Args,
      std::enable_if_t<std::is_constructible<T, Args&&...>::value>* = nullptr>
  constexpr ExpectedStorageBase(std::in_place_t, Args&&... args)
      : val_(std::forward<Args>(args)...), has_val_(true)
  {
  }

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible<
                T, std::initializer_list<U>&, Args&&...>::value>* = nullptr>
  constexpr ExpectedStorageBase(std::in_place_t, std::initializer_list<U> il,
                                Args&&... args)
      : val_(il, std::forward<Args>(args)...), has_val_(true)
  {
  }
  template <
      class... Args,
      std::enable_if_t<std::is_constructible<E, Args&&...>::value>* = nullptr>
  constexpr explicit ExpectedStorageBase(unexpect_t, Args&&... args)
      : error_(std::forward<Args>(args)...), has_val_(false)
  {
  }

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible<
                E, std::initializer_list<U>&, Args&&...>::value>* = nullptr>
  constexpr explicit ExpectedStorageBase(unexpect_t,
                                         std::initializer_list<U> il,
                                         Args&&... args)
      : error_(il, std::forward<Args>(args)...), has_val_(false)
  {
  }

  ~ExpectedStorageBase()
  {
    if (has_val_) {
      val_.~T();
    }
  }
  union {
    char m_no_init;
    T val_;
    Unexpected<E> error_;
  };
  bool has_val_;
};

// `T` is `void`, `E` is trivially-destructible
template <class E> struct ExpectedStorageBase<void, E, false, true> {
  constexpr ExpectedStorageBase() : has_val_(true) {}
  constexpr ExpectedStorageBase(no_init_t) : val_(), has_val_(false) {}

  constexpr ExpectedStorageBase(std::in_place_t) : has_val_(true) {}

  template <
      class... Args,
      std::enable_if_t<std::is_constructible<E, Args&&...>::value>* = nullptr>
  constexpr explicit ExpectedStorageBase(unexpect_t, Args&&... args)
      : error_(std::forward<Args>(args)...), has_val_(false)
  {
  }

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible<
                E, std::initializer_list<U>&, Args&&...>::value>* = nullptr>
  constexpr explicit ExpectedStorageBase(unexpect_t,
                                         std::initializer_list<U> il,
                                         Args&&... args)
      : error_(il, std::forward<Args>(args)...), has_val_(false)
  {
  }

  ~ExpectedStorageBase() = default;
  struct dummy {
  };
  union {
    dummy val_;
    Unexpected<E> error_;
  };
  bool has_val_;
};

// `T` is `void`, `E` is not trivially-destructible
template <class E> struct ExpectedStorageBase<void, E, false, false> {
  constexpr ExpectedStorageBase() : m_dummy(), has_val_(true) {}
  constexpr ExpectedStorageBase(no_init_t) : m_dummy(), has_val_(false) {}

  constexpr ExpectedStorageBase(std::in_place_t) : m_dummy(), has_val_(true) {}

  template <
      class... Args,
      std::enable_if_t<std::is_constructible<E, Args&&...>::value>* = nullptr>
  constexpr explicit ExpectedStorageBase(unexpect_t, Args&&... args)
      : error_(std::forward<Args>(args)...), has_val_(false)
  {
  }

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible<
                E, std::initializer_list<U>&, Args&&...>::value>* = nullptr>
  constexpr explicit ExpectedStorageBase(unexpect_t,
                                         std::initializer_list<U> il,
                                         Args&&... args)
      : error_(il, std::forward<Args>(args)...), has_val_(false)
  {
  }

  ~ExpectedStorageBase()
  {
    if (!has_val_) {
      error_.~Unexpected<E>();
    }
  }

  union {
    char m_dummy;
    Unexpected<E> error_;
  };
  bool has_val_;
};

// This base class provides some handy member functions which can be used in
// further derived classes
template <class T, class E>
struct ExpectedOperationsBase : ExpectedStorageBase<T, E> {
  using ExpectedStorageBase<T, E>::ExpectedStorageBase;

  template <class... Args> void construct(Args&&... args) noexcept
  {
    new (std::addressof(this->val_)) T(std::forward<Args>(args)...);
    this->has_val_ = true;
  }

  template <class Rhs> void construct_with(Rhs&& rhs) noexcept
  {
    new (std::addressof(this->val_)) T(std::forward<Rhs>(rhs).get());
    this->has_val_ = true;
  }

  template <class... Args> void construct_error(Args&&... args) noexcept
  {
    new (std::addressof(this->error_))
        Unexpected<E>(std::forward<Args>(args)...);
    this->has_val_ = false;
  }

  // These assign overloads ensure that the most efficient assignment
  // implementation is used while maintaining the strong exception guarantee.
  // The problematic case is where rhs has a value, but *this does not.
  //
  // This overload handles the case where we can just copy-construct `T`
  // directly into place without throwing.
  template <
      class U = T,
      std::enable_if_t<std::is_nothrow_copy_constructible<U>::value>* = nullptr>
  void assign(const ExpectedOperationsBase& rhs) noexcept
  {
    if (!this->has_val_ && rhs.has_val_) {
      geterr().~Unexpected<E>();
      construct(rhs.get());
    } else {
      assign_common(rhs);
    }
  }

  // This overload handles the case where we can attempt to create a copy of
  // `T`, then no-throw move it into place if the copy was successful.
  template <
      class U = T,
      std::enable_if_t<!std::is_nothrow_copy_constructible<U>::value &&
                       std::is_nothrow_move_constructible<U>::value>* = nullptr>
  void assign(const ExpectedOperationsBase& rhs) noexcept
  {
    if (!this->has_val_ && rhs.has_val_) {
      T tmp = rhs.get();
      geterr().~Unexpected<E>();
      construct(std::move(tmp));
    } else {
      assign_common(rhs);
    }
  }

  // This overload is the worst-case, where we have to move-construct the
  // Unexpected value into temporary storage, then try to copy the T into place.
  // If the construction succeeds, then everything is fine, but if it throws,
  // then we move the old Unexpected value back into place before rethrowing the
  // exception.
  template <class U = T,
            std::enable_if_t<!std::is_nothrow_copy_constructible<U>::value &&
                             !std::is_nothrow_move_constructible<U>::value>* =
                nullptr>
  void assign(const ExpectedOperationsBase& rhs)
  {
    if (!this->has_val_ && rhs.has_val_) {
      auto tmp = std::move(geterr());
      geterr().~Unexpected<E>();

      try {
        construct(rhs.get());
      } catch (...) {
        geterr() = std::move(tmp);
        throw;
      }
    } else {
      assign_common(rhs);
    }
  }

  // These overloads do the same as above, but for rvalues
  template <
      class U = T,
      std::enable_if_t<std::is_nothrow_move_constructible<U>::value>* = nullptr>
  void assign(ExpectedOperationsBase&& rhs) noexcept
  {
    if (!this->has_val_ && rhs.has_val_) {
      geterr().~Unexpected<E>();
      construct(std::move(rhs).get());
    } else {
      assign_common(std::move(rhs));
    }
  }

  template <class U = T,
            std::enable_if_t<!std::is_nothrow_move_constructible<U>::value>* =
                nullptr>
  void assign(ExpectedOperationsBase&& rhs)
  {
    if (!this->has_val_ && rhs.has_val_) {
      auto tmp = std::move(geterr());
      geterr().~Unexpected<E>();
      try {
        construct(std::move(rhs).get());
      } catch (...) {
        geterr() = std::move(tmp);
        throw;
      }
    } else {
      assign_common(std::move(rhs));
    }
  }

  // The common part of move/copy assigning
  template <class Rhs> void assign_common(Rhs&& rhs)
  {
    if (this->has_val_) {
      if (rhs.has_val_) {
        get() = std::forward<Rhs>(rhs).get();
      } else {
        destroy_val();
        construct_error(std::forward<Rhs>(rhs).geterr());
      }
    } else {
      if (!rhs.has_val_) {
        geterr() = std::forward<Rhs>(rhs).geterr();
      }
    }
  }

  bool has_value() const
  {
    return this->has_val_;
  }

  constexpr T& get() &
  {
    return this->val_;
  }
  constexpr const T& get() const&
  {
    return this->val_;
  }
  constexpr T&& get() &&
  {
    return std::move(this->val_);
  }
  constexpr const T&& get() const&&
  {
    return std::move(this->val_);
  }

  constexpr Unexpected<E>& geterr() &
  {
    return this->error_;
  }
  constexpr const Unexpected<E>& geterr() const&
  {
    return this->error_;
  }
  constexpr Unexpected<E>&& geterr() &&
  {
    return std::move(this->error_);
  }
  constexpr const Unexpected<E>&& geterr() const&&
  {
    return std::move(this->error_);
  }

  constexpr void destroy_val()
  {
    get().~T();
  }
};

// This base class provides some handy member functions which can be used in
// further derived classes
template <class E>
struct ExpectedOperationsBase<void, E> : ExpectedStorageBase<void, E> {
  using ExpectedStorageBase<void, E>::ExpectedStorageBase;

  template <class... Args> void construct() noexcept
  {
    this->has_val_ = true;
  }

  // This function doesn't use its argument, but needs it so that code in
  // levels above this can work independently of whether T is void
  template <class Rhs> void construct_with(Rhs&&) noexcept
  {
    this->has_val_ = true;
  }

  template <class... Args> void construct_error(Args&&... args) noexcept
  {
    new (std::addressof(this->error_))
        Unexpected<E>(std::forward<Args>(args)...);
    this->has_val_ = false;
  }

  template <class Rhs> void assign(Rhs&& rhs) noexcept
  {
    if (!this->has_val_) {
      if (rhs.has_val_) {
        geterr().~Unexpected<E>();
        construct();
      } else {
        geterr() = std::forward<Rhs>(rhs).geterr();
      }
    } else {
      if (!rhs.has_val_) {
        construct_error(std::forward<Rhs>(rhs).geterr());
      }
    }
  }

  bool has_value() const
  {
    return this->has_val_;
  }

  constexpr Unexpected<E>& geterr() &
  {
    return this->error_;
  }
  constexpr const Unexpected<E>& geterr() const&
  {
    return this->error_;
  }
  constexpr Unexpected<E>&& geterr() &&
  {
    return std::move(this->error_);
  }
  constexpr const Unexpected<E>&& geterr() const&&
  {
    return std::move(this->error_);
  }

  constexpr void destroy_val()
  {
    // no-op
  }
};

// This class manages conditionally having a trivial copy constructor
// This specialization is for when T and E are trivially copy constructible
template <class T, class E,
          bool = is_void_or<T, std::is_trivially_copy_constructible<T>>::value&&
              std::is_trivially_copy_constructible<E>::value>
struct ExpectedCopyBase : ExpectedOperationsBase<T, E> {
  using ExpectedOperationsBase<T, E>::ExpectedOperationsBase;
};

// This specialization is for when T or E are not trivially copy constructible
template <class T, class E>
struct ExpectedCopyBase<T, E, false> : ExpectedOperationsBase<T, E> {
  using ExpectedOperationsBase<T, E>::ExpectedOperationsBase;

  ExpectedCopyBase() = default;
  ExpectedCopyBase(const ExpectedCopyBase& rhs)
      : ExpectedOperationsBase<T, E>(no_init)
  {
    if (rhs.has_value()) {
      this->construct_with(rhs);
    } else {
      this->construct_error(rhs.geterr());
    }
  }

  ExpectedCopyBase(ExpectedCopyBase&& rhs) = default;
  ExpectedCopyBase& operator=(const ExpectedCopyBase& rhs) = default;
  ExpectedCopyBase& operator=(ExpectedCopyBase&& rhs) = default;
};

// This class manages conditionally having a trivial move constructor
// Unfortunately there's no way to achieve this in GCC < 5 AFAIK, since it
// doesn't implement an analogue to std::is_trivially_move_constructible. We
// have to make do with a non-trivial move constructor even if T is trivially
// move constructible
template <class T, class E,
          bool = is_void_or<T, std::is_trivially_move_constructible<T>>::value&&
              std::is_trivially_move_constructible<E>::value>
struct ExpectedMoveBase : ExpectedCopyBase<T, E> {
  using ExpectedCopyBase<T, E>::ExpectedCopyBase;
};
template <class T, class E>
struct ExpectedMoveBase<T, E, false> : ExpectedCopyBase<T, E> {
  using ExpectedCopyBase<T, E>::ExpectedCopyBase;

  ExpectedMoveBase() = default;
  ExpectedMoveBase(const ExpectedMoveBase& rhs) = default;

  ExpectedMoveBase(ExpectedMoveBase&& rhs) noexcept(
      std::is_nothrow_move_constructible<T>::value)
      : ExpectedCopyBase<T, E>(no_init)
  {
    if (rhs.has_value()) {
      this->construct_with(std::move(rhs));
    } else {
      this->construct_error(std::move(rhs.geterr()));
    }
  }
  ExpectedMoveBase& operator=(const ExpectedMoveBase& rhs) = default;
  ExpectedMoveBase& operator=(ExpectedMoveBase&& rhs) = default;
};

// This class manages conditionally having a trivial copy assignment operator
template <class T, class E,
          bool = is_void_or<
              T, std::conjunction<std::is_trivially_copy_assignable<T>,
                                  std::is_trivially_copy_constructible<T>,
                                  std::is_trivially_destructible<T>>>::value&&
              std::is_trivially_copy_assignable_v<E>&&
                  std::is_trivially_copy_constructible_v<E>&&
                      std::is_trivially_destructible_v<E>>
struct ExpectedCopyAssignBase : ExpectedMoveBase<T, E> {
  using ExpectedMoveBase<T, E>::ExpectedMoveBase;
};

template <class T, class E>
struct ExpectedCopyAssignBase<T, E, false> : ExpectedMoveBase<T, E> {
  using ExpectedMoveBase<T, E>::ExpectedMoveBase;

  ExpectedCopyAssignBase() = default;
  ExpectedCopyAssignBase(const ExpectedCopyAssignBase& rhs) = default;

  ExpectedCopyAssignBase(ExpectedCopyAssignBase&& rhs) = default;
  ExpectedCopyAssignBase& operator=(const ExpectedCopyAssignBase& rhs)
  {
    this->assign(rhs);
    return *this;
  }
  ExpectedCopyAssignBase& operator=(ExpectedCopyAssignBase&& rhs) = default;
};

// This class manages conditionally having a trivial move assignment operator
// Unfortunately there's no way to achieve this in GCC < 5 AFAIK, since it
// doesn't implement an analogue to std::is_trivially_move_assignable. We have
// to make do with a non-trivial move assignment operator even if T is trivially
// move assignable
template <class T, class E,
          bool = is_void_or<
              T, std::conjunction<std::is_trivially_destructible<T>,
                                  std::is_trivially_move_constructible<T>,
                                  std::is_trivially_move_assignable<T>>>::
              value&& std::is_trivially_destructible<E>::value&&
                  std::is_trivially_move_constructible<E>::value&&
                      std::is_trivially_move_assignable<E>::value>
struct ExpectedMoveAssignBase : ExpectedCopyAssignBase<T, E> {
  using ExpectedCopyAssignBase<T, E>::ExpectedCopyAssignBase;
};

template <class T, class E>
struct ExpectedMoveAssignBase<T, E, false> : ExpectedCopyAssignBase<T, E> {
  using ExpectedCopyAssignBase<T, E>::ExpectedCopyAssignBase;

  ExpectedMoveAssignBase() = default;
  ExpectedMoveAssignBase(const ExpectedMoveAssignBase& rhs) = default;

  ExpectedMoveAssignBase(ExpectedMoveAssignBase&& rhs) = default;

  ExpectedMoveAssignBase&
  operator=(const ExpectedMoveAssignBase& rhs) = default;

  ExpectedMoveAssignBase& operator=(ExpectedMoveAssignBase&& rhs) noexcept(
      std::is_nothrow_move_constructible<T>::value&&
          std::is_nothrow_move_assignable<T>::value)
  {
    this->assign(std::move(rhs));
    return *this;
  }
};

// ExpectedDeleteCtorBase will conditionally delete copy and move
// constructors depending on whether T is copy/move constructible
template <class T, class E,
          bool EnableCopy = (is_copy_constructible_or_void<T>::value &&
                             std::is_copy_constructible<E>::value),
          bool EnableMove = (is_move_constructible_or_void<T>::value &&
                             std::is_move_constructible_v<E>)>
struct ExpectedDeleteCtorBase {
  ExpectedDeleteCtorBase() = default;
  ExpectedDeleteCtorBase(const ExpectedDeleteCtorBase&) = default;
  ExpectedDeleteCtorBase(ExpectedDeleteCtorBase&&) noexcept = default;
  ExpectedDeleteCtorBase& operator=(const ExpectedDeleteCtorBase&) = default;
  ExpectedDeleteCtorBase&
  operator=(ExpectedDeleteCtorBase&&) noexcept = default;
};

template <class T, class E> struct ExpectedDeleteCtorBase<T, E, true, false> {
  ExpectedDeleteCtorBase() = default;
  ExpectedDeleteCtorBase(const ExpectedDeleteCtorBase&) = default;
  ExpectedDeleteCtorBase(ExpectedDeleteCtorBase&&) noexcept = delete;
  ExpectedDeleteCtorBase& operator=(const ExpectedDeleteCtorBase&) = default;
  ExpectedDeleteCtorBase&
  operator=(ExpectedDeleteCtorBase&&) noexcept = default;
};

template <class T, class E> struct ExpectedDeleteCtorBase<T, E, false, true> {
  ExpectedDeleteCtorBase() = default;
  ExpectedDeleteCtorBase(const ExpectedDeleteCtorBase&) = delete;
  ExpectedDeleteCtorBase(ExpectedDeleteCtorBase&&) noexcept = default;
  ExpectedDeleteCtorBase& operator=(const ExpectedDeleteCtorBase&) = default;
  ExpectedDeleteCtorBase&
  operator=(ExpectedDeleteCtorBase&&) noexcept = default;
};

template <class T, class E> struct ExpectedDeleteCtorBase<T, E, false, false> {
  ExpectedDeleteCtorBase() = default;
  ExpectedDeleteCtorBase(const ExpectedDeleteCtorBase&) = delete;
  ExpectedDeleteCtorBase(ExpectedDeleteCtorBase&&) noexcept = delete;
  ExpectedDeleteCtorBase& operator=(const ExpectedDeleteCtorBase&) = default;
  ExpectedDeleteCtorBase&
  operator=(ExpectedDeleteCtorBase&&) noexcept = default;
};

// ExpectedDeleteAssignBase will conditionally delete copy and move
// constructors depending on whether T and E are copy/move constructible +
// assignable
template <class T, class E,
          bool EnableCopy = (is_copy_constructible_or_void<T>::value &&
                             std::is_copy_constructible<E>::value &&
                             is_copy_assignable_or_void<T>::value &&
                             std::is_copy_assignable<E>::value),
          bool EnableMove = (is_move_constructible_or_void<T>::value &&
                             std::is_move_constructible_v<E> &&
                             is_move_assignable_or_void<T>::value &&
                             std::is_move_assignable<E>::value)>
struct ExpectedDeleteAssignBase {
  ExpectedDeleteAssignBase() = default;
  ExpectedDeleteAssignBase(const ExpectedDeleteAssignBase&) = default;
  ExpectedDeleteAssignBase(ExpectedDeleteAssignBase&&) noexcept = default;
  ExpectedDeleteAssignBase&
  operator=(const ExpectedDeleteAssignBase&) = default;
  ExpectedDeleteAssignBase&
  operator=(ExpectedDeleteAssignBase&&) noexcept = default;
};

template <class T, class E> struct ExpectedDeleteAssignBase<T, E, true, false> {
  ExpectedDeleteAssignBase() = default;
  ExpectedDeleteAssignBase(const ExpectedDeleteAssignBase&) = default;
  ExpectedDeleteAssignBase(ExpectedDeleteAssignBase&&) noexcept = default;
  ExpectedDeleteAssignBase&
  operator=(const ExpectedDeleteAssignBase&) = default;
  ExpectedDeleteAssignBase&
  operator=(ExpectedDeleteAssignBase&&) noexcept = delete;
};

template <class T, class E> struct ExpectedDeleteAssignBase<T, E, false, true> {
  ExpectedDeleteAssignBase() = default;
  ExpectedDeleteAssignBase(const ExpectedDeleteAssignBase&) = default;
  ExpectedDeleteAssignBase(ExpectedDeleteAssignBase&&) noexcept = default;
  ExpectedDeleteAssignBase& operator=(const ExpectedDeleteAssignBase&) = delete;
  ExpectedDeleteAssignBase&
  operator=(ExpectedDeleteAssignBase&&) noexcept = default;
};

template <class T, class E>
struct ExpectedDeleteAssignBase<T, E, false, false> {
  ExpectedDeleteAssignBase() = default;
  ExpectedDeleteAssignBase(const ExpectedDeleteAssignBase&) = default;
  ExpectedDeleteAssignBase(ExpectedDeleteAssignBase&&) noexcept = default;
  ExpectedDeleteAssignBase& operator=(const ExpectedDeleteAssignBase&) = delete;
  ExpectedDeleteAssignBase&
  operator=(ExpectedDeleteAssignBase&&) noexcept = delete;
};

// This is needed to be able to construct the ExpectedDefaultCtorBase which
// follows, while still conditionally deleting the default constructor.
struct DefaultConstructorTag {
  explicit constexpr DefaultConstructorTag() = default;
};

// ExpectedDefaultCtorBase will ensure that Expected has a deleted default
// consturctor if T is not default constructible.
// This specialization is for when T is default constructible
template <class T, class E,
          bool Enable =
              std::is_default_constructible<T>::value || std::is_void<T>::value>
struct ExpectedDefaultCtorBase {
  constexpr ExpectedDefaultCtorBase() noexcept = default;
  constexpr ExpectedDefaultCtorBase(ExpectedDefaultCtorBase const&) noexcept =
      default;
  constexpr ExpectedDefaultCtorBase(ExpectedDefaultCtorBase&&) noexcept =
      default;
  ExpectedDefaultCtorBase&
  operator=(ExpectedDefaultCtorBase const&) noexcept = default;
  ExpectedDefaultCtorBase&
  operator=(ExpectedDefaultCtorBase&&) noexcept = default;

  constexpr explicit ExpectedDefaultCtorBase(DefaultConstructorTag) {}
};

// This specialization is for when T is not default constructible
template <class T, class E> struct ExpectedDefaultCtorBase<T, E, false> {
  constexpr ExpectedDefaultCtorBase() noexcept = delete;
  constexpr ExpectedDefaultCtorBase(ExpectedDefaultCtorBase const&) noexcept =
      default;
  constexpr ExpectedDefaultCtorBase(ExpectedDefaultCtorBase&&) noexcept =
      default;
  ExpectedDefaultCtorBase&
  operator=(ExpectedDefaultCtorBase const&) noexcept = default;
  ExpectedDefaultCtorBase&
  operator=(ExpectedDefaultCtorBase&&) noexcept = default;

  constexpr explicit ExpectedDefaultCtorBase(DefaultConstructorTag) {}
};
} // namespace detail

/// @brief `Expected<T, E>` is a type that represents either success (`T`) or
/// failure (`E`).
///
/// An `Expected<T, E>` object is an object that contains the storage for
/// another object and manages the lifetime of this contained object `T`.
/// Alternatively it could contain the storage for another Unexpected object
/// `E`. The contained object may not be initialized after the Expected object
/// has been initialized, and may not be destroyed before the Expected object
/// has been destroyed. The initialization state of the contained object is
/// tracked by the Expected object.
template <class T, class E>
class Expected : private detail::ExpectedMoveAssignBase<T, E>,
                 private detail::ExpectedDeleteCtorBase<T, E>,
                 private detail::ExpectedDeleteAssignBase<T, E>,
                 private detail::ExpectedDefaultCtorBase<T, E> {
  static_assert(!std::is_reference<T>::value, "T must not be a reference");
  static_assert(!std::is_same<T, std::remove_cv<std::in_place_t>>::value,
                "T must not be in_place_t");
  static_assert(!std::is_same<T, std::remove_cv<unexpect_t>>::value,
                "T must not be unexpect_t");
  static_assert(!std::is_same<T, std::remove_cv<Unexpected<E>>>::value,
                "T must not be Unexpected<E>");
  static_assert(!std::is_reference<E>::value, "E must not be a reference");

  T* valptr()
  {
    return std::addressof(this->val_);
  }
  const T* valptr() const
  {
    return std::addressof(this->val_);
  }
  Unexpected<E>* errptr()
  {
    return std::addressof(this->error_);
  }
  const Unexpected<E>* errptr() const
  {
    return std::addressof(this->error_);
  }

  template <class U = T, std::enable_if_t<!std::is_void<U>::value>* = nullptr>
  U& val()
  {
    return this->val_;
  }
  Unexpected<E>& err()
  {
    return this->error_;
  }

  template <class U = T, std::enable_if_t<!std::is_void<U>::value>* = nullptr>
  const U& val() const
  {
    return this->val_;
  }
  const Unexpected<E>& err() const
  {
    return this->error_;
  }

  using impl_base = detail::ExpectedMoveAssignBase<T, E>;
  using ctor_base = detail::ExpectedDefaultCtorBase<T, E>;

public:
  using value_type = T;
  using error_type = E;
  using Unexpected_type = Unexpected<E>;

  /**
   * @brief Carries out some operation which returns an Expected on the stored
   * object if there is one.
   *
   * This operation is commonly called a monadic `bind`.
   * Some languages call this operation `flatmap`.
   *
   * @return   Let `U` be the result of `std::invoke(std::forward<F>(f),
   * value())`. Returns an `Expected<U>`. The return value is empty if `*this`
   * is empty, otherwise the return value of `std::invoke(std::forward<F>(f),
   * value())`  is returned.
   */
  template <class F> constexpr auto and_then(F&& f) &
  {
    return and_then_impl(*this, std::forward<F>(f));
  }

  /// @overload
  template <class F> constexpr auto and_then(F&& f) &&
  {
    return and_then_impl(std::move(*this), std::forward<F>(f));
  }

  /// @overload
  template <class F> constexpr auto and_then(F&& f) const&
  {
    return and_then_impl(*this, std::forward<F>(f));
  }

  /// @overload
  template <class F> constexpr auto and_then(F&& f) const&&
  {
    return and_then_impl(std::move(*this), std::forward<F>(f));
  }

  /// @brief Carries out some operation on the stored object if there is one.
  ///
  /// Let `U` be the result of `std::invoke(std::forward<F>(f), value())`. If
  /// `U` is `void`, returns an `Expected<monostate,E>, otherwise
  ///  returns an `Expected<U,E>`. If `*this` is Unexpected, the
  /// result is `*this`, otherwise an `Expected<U,E>` is constructed from the
  /// return value of `std::invoke(std::forward<F>(f), value())` and is
  /// returned.
  template <class F> constexpr auto map(F&& f) &
  {
    return expected_map_impl(*this, std::forward<F>(f));
  }

  /// @overload
  template <class F> constexpr auto map(F&& f) &&
  {
    return expected_map_impl(std::move(*this), std::forward<F>(f));
  }

  /// @overload
  template <class F> constexpr auto map(F&& f) const&
  {
    return expected_map_impl(*this, std::forward<F>(f));
  }

  /// @overload
  template <class F> constexpr auto map(F&& f) const&&
  {
    return expected_map_impl(std::move(*this), std::forward<F>(f));
  }

  /// @brief Carries out some operation on the stored Unexpected object if there
  /// is one.
  ///
  /// Let `U` be the result of `std::invoke(std::forward<F>(f),
  /// value())`. If `U` is `void`, returns an `Expected<T,monostate>`, otherwise
  /// returns an `Expected<T,U>`. If `*this` has an Expected
  /// value, the result is `*this`, otherwise an `Expected<T,U>` is constructed
  /// from `make_unexpected(std::invoke(std::forward<F>(f), value()))` and is
  /// returned.
  template <class F> constexpr auto map_error(F&& f) &
  {
    return map_error_impl(*this, std::forward<F>(f));
  }

  /// @overload
  template <class F> constexpr auto map_error(F&& f) &&
  {
    return map_error_impl(std::move(*this), std::forward<F>(f));
  }

  /// @overload
  template <class F> constexpr auto map_error(F&& f) const&
  {
    return map_error_impl(*this, std::forward<F>(f));
  }

  /// @overload
  template <class F> constexpr auto map_error(F&& f) const&&
  {
    return map_error_impl(std::move(*this), std::forward<F>(f));
  }

  /// @brief Calls `f` if the expectd is in the Unexpected state
  /// @pre `F` is invokable with `E`, and `std::invoke_result_t<F>`
  /// must be void or convertible to `expcted<T,E>`.
  ///
  /// If `*this` has a value, returns `*this`. Otherwise, if `f` returns `void`,
  /// calls `std::forward<F>(f)(E)` and returns `std::nullopt`. Otherwise,
  /// returns `std::forward<F>(f)(E)`.
  template <class F> Expected constexpr or_else(F&& f) &
  {
    return or_else_impl(*this, std::forward<F>(f));
  }

  /// @overload
  template <class F> Expected constexpr or_else(F&& f) &&
  {
    return or_else_impl(std::move(*this), std::forward<F>(f));
  }

  /// @overload
  template <class F> Expected constexpr or_else(F&& f) const&
  {
    return or_else_impl(*this, std::forward<F>(f));
  }

  /// @overload
  template <class F> Expected constexpr or_else(F&& f) const&&
  {
    return or_else_impl(std::move(*this), std::forward<F>(f));
  }

  constexpr Expected() = default;

  template <
      class... Args,
      std::enable_if_t<std::is_constructible<T, Args&&...>::value>* = nullptr>
  explicit constexpr Expected(std::in_place_t, Args&&... args)
      : impl_base(std::in_place, std::forward<Args>(args)...),
        ctor_base(detail::DefaultConstructorTag{})
  {
  }

  template <class U, class... Args,
            std::enable_if_t<std::is_constructible<
                T, std::initializer_list<U>&, Args&&...>::value>* = nullptr>
  constexpr Expected(std::in_place_t, std::initializer_list<U> il,
                     Args&&... args)
      : impl_base(std::in_place, il, std::forward<Args>(args)...),
        ctor_base(detail::DefaultConstructorTag{})
  {
  }

  /// @overload
  template <
      class G = E,
      std::enable_if_t<std::is_constructible<E, const G&>::value>* = nullptr,
      std::enable_if_t<!std::is_convertible<const G&, E>::value>* = nullptr>
  explicit constexpr Expected(const Unexpected<G>& e)
      : impl_base(unexpect, e.value()),
        ctor_base(detail::DefaultConstructorTag{})
  {
  }

  /// @cond
  template <class G = E,
            std::enable_if_t<std::is_constructible_v<E, const G&>>* = nullptr,
            std::enable_if_t<std::is_convertible_v<const G&, E>>* = nullptr>
  constexpr Expected(Unexpected<G> const& e)
      : impl_base(unexpect, e.value()),
        ctor_base(detail::DefaultConstructorTag{}) // NOLINT
  {
  }
  /// @endcond

  /// @overload
  template <class G = E,
            std::enable_if_t<std::is_constructible<E, G&&>::value>* = nullptr,
            std::enable_if_t<!std::is_convertible<G&&, E>::value>* = nullptr>
  explicit constexpr Expected(Unexpected<G>&& e) noexcept(
      std::is_nothrow_constructible<E, G&&>::value)
      : impl_base(unexpect, std::move(e.value())),
        ctor_base(detail::DefaultConstructorTag{})
  {
  }

  /// @cond
  template <class G = E,
            std::enable_if_t<std::is_constructible<E, G&&>::value>* = nullptr,
            std::enable_if_t<std::is_convertible<G&&, E>::value>* = nullptr>
  constexpr Expected(Unexpected<G>&& e) noexcept(
      std::is_nothrow_constructible<E, G&&>::value)
      : impl_base(unexpect, std::move(e.value())),
        ctor_base(detail::DefaultConstructorTag{}) // NOLINT
  {
  }
  /// @endcond

  template <
      class... Args,
      std::enable_if_t<std::is_constructible<E, Args&&...>::value>* = nullptr>
  constexpr explicit Expected(unexpect_t, Args&&... args)
      : impl_base(unexpect, std::forward<Args>(args)...),
        ctor_base(detail::DefaultConstructorTag{})
  {
  }

  /// @cond
  template <class U, class... Args,
            std::enable_if_t<std::is_constructible<
                E, std::initializer_list<U>&, Args&&...>::value>* = nullptr>
  constexpr explicit Expected(unexpect_t, std::initializer_list<U> il,
                              Args&&... args)
      : impl_base(unexpect, il, std::forward<Args>(args)...),
        ctor_base(detail::DefaultConstructorTag{})
  {
  }
  /// @endcond

  template <
      class U, class G,
      std::enable_if_t<!(std::is_convertible<U const&, T>::value &&
                         std::is_convertible<G const&, E>::value)>* = nullptr,
      detail::expected_enable_from_other<T, E, U, G, const U&, const G&>* =
          nullptr>
  explicit constexpr Expected(const Expected<U, G>& rhs)
      : ctor_base(detail::DefaultConstructorTag{})
  {
    if (rhs.has_value()) {
      this->construct(*rhs);
    } else {
      this->construct_error(rhs.error());
    }
  }

  /// @cond
  template <
      class U, class G,
      std::enable_if_t<(std::is_convertible<U const&, T>::value &&
                        std::is_convertible<G const&, E>::value)>* = nullptr,
      detail::expected_enable_from_other<T, E, U, G, const U&, const G&>* =
          nullptr>
  constexpr Expected(const Expected<U, G>& rhs) // NOLINT
      : ctor_base(detail::DefaultConstructorTag{})
  {
    if (rhs.has_value()) {
      this->construct(*rhs);
    } else {
      this->construct_error(rhs.error());
    }
  }
  /// @endcond

  template <class U, class G,
            std::enable_if_t<!(std::is_convertible<U&&, T>::value &&
                               std::is_convertible<G&&, E>::value)>* = nullptr,
            detail::expected_enable_from_other<T, E, U, G, U&&, G&&>* = nullptr>
  explicit constexpr Expected(Expected<U, G>&& rhs)
      : ctor_base(detail::DefaultConstructorTag{})
  {
    if (rhs.has_value()) {
      this->construct(std::move(*rhs));
    } else {
      this->construct_error(std::move(rhs.error()));
    }
  }

  /// @cond
  template <class U, class G,
            std::enable_if_t<(std::is_convertible<U&&, T>::value &&
                              std::is_convertible<G&&, E>::value)>* = nullptr,
            detail::expected_enable_from_other<T, E, U, G, U&&, G&&>* = nullptr>
  constexpr Expected(Expected<U, G>&& rhs) // NOLINT
      : ctor_base(detail::DefaultConstructorTag{})
  {
    if (rhs.has_value()) {
      this->construct(std::move(*rhs));
    } else {
      this->construct_error(std::move(rhs.error()));
    }
  }
  /// @endcond

  template <class U = T,
            std::enable_if_t<!std::is_convertible<U&&, T>::value>* = nullptr,
            detail::expected_enable_forward_value<T, E, U>* = nullptr>
  explicit constexpr Expected(U&& v)
      : Expected(std::in_place, std::forward<U>(v))
  {
  }

  /// @cond
  template <class U = T,
            std::enable_if_t<std::is_convertible<U&&, T>::value>* = nullptr,
            detail::expected_enable_forward_value<T, E, U>* = nullptr>
  constexpr Expected(U&& v)
      : Expected(std::in_place, std::forward<U>(v)) // NOLINT
  {
  }

  /// @endcond

  template <
      class U = T, class G = T,
      std::enable_if_t<std::is_nothrow_constructible<T, U&&>::value>* = nullptr,
      std::enable_if_t<!std::is_void<G>::value>* = nullptr,
      std::enable_if_t<
          (!std::is_same<Expected<T, E>, std::decay_t<U>>::value &&
           !std::conjunction<std::is_scalar<T>,
                             std::is_same<T, std::decay_t<U>>>::value &&
           std::is_constructible<T, U>::value &&
           std::is_assignable<G&, U>::value &&
           std::is_nothrow_move_constructible<E>::value)>* = nullptr>
  Expected& operator=(U&& v)
  {
    if (has_value()) {
      val() = std::forward<U>(v);
    } else {
      err().~Unexpected<E>();
      ::new (valptr()) T(std::forward<U>(v));
      this->has_val_ = true;
    }

    return *this;
  }

  /// @cond
  template <class U = T, class G = T,
            std::enable_if_t<!std::is_nothrow_constructible<T, U&&>::value>* =
                nullptr,
            std::enable_if_t<!std::is_void<U>::value>* = nullptr,
            std::enable_if_t<
                (!std::is_same<Expected<T, E>, std::decay_t<U>>::value &&
                 !std::conjunction<std::is_scalar<T>,
                                   std::is_same<T, std::decay_t<U>>>::value &&
                 std::is_constructible<T, U>::value &&
                 std::is_assignable<G&, U>::value &&
                 std::is_nothrow_move_constructible<E>::value)>* = nullptr>
  Expected& operator=(U&& v)
  {
    if (has_value()) {
      val() = std::forward<U>(v);
    } else {
      auto tmp = std::move(err());
      err().~Unexpected<E>();

#ifdef BEYOND_EXPECTED_EXCEPTIONS_ENABLED
      try {
        ::new (valptr()) T(std::move(v)); // NOLINT
        this->has_val_ = true;
      } catch (...) {
        err() = std::move(tmp);
        throw;
      }
#else
      ::new (valptr()) T(std::move(v));
      this->has_val_ = true;
#endif
    }

    return *this;
  }
  /// @endcond

  template <class G = E,
            std::enable_if_t<std::is_nothrow_copy_constructible<G>::value &&
                             std::is_assignable<G&, G>::value>* = nullptr>
  Expected& operator=(const Unexpected<G>& rhs)
  {
    if (!has_value()) {
      err() = rhs;
    } else {
      this->destroy_val();
      ::new (errptr()) Unexpected<E>(rhs);
      this->has_val_ = false;
    }

    return *this;
  }

  template <class G = E,
            std::enable_if_t<std::is_nothrow_move_constructible<G>::value &&
                             std::is_move_assignable<G>::value>* = nullptr>
  Expected& operator=(Unexpected<G>&& rhs) noexcept
  {
    if (!has_value()) {
      err() = std::move(rhs);
    } else {
      this->destroy_val();
      ::new (errptr()) Unexpected<E>(std::move(rhs));
      this->has_val_ = false;
    }

    return *this;
  }

  template <class... Args, std::enable_if_t<std::is_nothrow_constructible<
                               T, Args&&...>::value>* = nullptr>
  void emplace(Args&&... args)
  {
    if (has_value()) {
      val() = T(std::forward<Args>(args)...);
    } else {
      err().~Unexpected<E>();
      ::new (valptr()) T(std::forward<Args>(args)...);
      this->has_val_ = true;
    }
  }

  /// @cond
  template <class... Args, std::enable_if_t<!std::is_nothrow_constructible<
                               T, Args&&...>::value>* = nullptr>
  void emplace(Args&&... args)
  {
    if (has_value()) {
      val() = T(std::forward<Args>(args)...);
    } else {
      auto tmp = std::move(err());
      err().~Unexpected<E>();

      try {
        ::new (valptr()) T(std::forward<Args>(args)...);
        this->has_val_ = true;
      } catch (...) {
        err() = std::move(tmp);
        throw;
      }
    }
  }
  /// @endcond

  template <class U, class... Args,
            std::enable_if_t<std::is_nothrow_constructible<
                T, std::initializer_list<U>&, Args&&...>::value>* = nullptr>
  void emplace(std::initializer_list<U> il, Args&&... args)
  {
    if (has_value()) {
      T t(il, std::forward<Args>(args)...);
      val() = std::move(t);
    } else {
      err().~Unexpected<E>();
      ::new (valptr()) T(il, std::forward<Args>(args)...);
      this->has_val_ = true;
    }
  }

  /// @cond
  template <class U, class... Args,
            std::enable_if_t<!std::is_nothrow_constructible<
                T, std::initializer_list<U>&, Args&&...>::value>* = nullptr>
  void emplace(std::initializer_list<U> il, Args&&... args)
  {
    if (has_value()) {
      T t(il, std::forward<Args>(args)...);
      val() = std::move(t);
    } else {
      auto tmp = std::move(err());
      err().~Unexpected<E>();

#ifdef BEYOND_EXPECTED_EXCEPTIONS_ENABLED
      try {
        ::new (valptr()) T(il, std::forward<Args>(args)...);
        this->has_val_ = true;
      } catch (...) {
        err() = std::move(tmp);
        throw;
      }
#else
      ::new (valptr()) T(il, std::forward<Args>(args)...);
      this->has_val_ = true;
#endif
    }
  }
  /// @endcond

  void swap(Expected& rhs) noexcept(
      std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_swappable_v<
          T&>&& std::is_nothrow_move_constructible_v<E>&&
          std::is_nothrow_swappable_v<E&>)
  {
    if (has_value()) {
      if (rhs.has_value()) {
        std::swap(val(), rhs.val());
      } else {
        auto temp = std::move(rhs.err());
        ::new (rhs.valptr()) T(val());
        ::new (errptr()) Unexpected_type(std::move(temp));
        std::swap(this->has_val_, rhs.has_val_);
      }
    } else {
      if (rhs.has_value()) {
        auto temp = std::move(this->err());
        ::new (valptr()) T(rhs.val());
        ::new (rhs.errptr()) Unexpected_type(std::move(temp));
        std::swap(this->has_val_, rhs.has_val_);
      } else {
        std::swap(err(), rhs.err());
      }
    }
  }

  /// @brief Returns a pointer to the stored value
  /// @pre a value is stored
  /// @warning The behavior of dereferencing without a value stored is undefined
  constexpr const T* operator->() const
  {
    return valptr();
  }
  /// @overload
  constexpr T* operator->()
  {
    return valptr();
  }

  /// @brief returns the stored value
  /// @pre a value is stored
  /// @warning The behavior of dereferencing without a value stored is undefined
  template <class U = T, std::enable_if_t<!std::is_void<U>::value>* = nullptr>
  constexpr const U& operator*() const&
  {
    BEYOND_ASSERT(has_value());
    return val();
  }
  /// @overload
  template <class U = T, std::enable_if_t<!std::is_void<U>::value>* = nullptr>
  constexpr U& operator*() &
  {
    BEYOND_ASSERT(has_value());
    return val();
  }
  /// @overload
  template <class U = T, std::enable_if_t<!std::is_void<U>::value>* = nullptr>
  constexpr const U&& operator*() const&&
  {
    BEYOND_ASSERT(has_value());
    return std::move(val());
  }

  /// @overload
  template <class U = T, std::enable_if_t<!std::is_void<U>::value>* = nullptr>
  constexpr U&& operator*() &&
  {
    BEYOND_ASSERT(has_value());
    return std::move(val());
  }

  /// @brief returns whether or not the optional has a value
  constexpr bool has_value() const noexcept
  {
    return this->has_val_;
  }

  /// @brief returns whether or not the optional has a value
  /// @see has_value
  constexpr explicit operator bool() const noexcept
  {
    return this->has_val_;
  }

  /**
   * @brief returns the contained value
   * @warning If the Expected does not contain a value, the result is undefined
   */
  template <class U = T, std::enable_if_t<!std::is_void<U>::value>* = nullptr>
  constexpr const U& value() const&
  {
    BEYOND_ASSERT(has_value());
    return val();
  }

  /// @overload
  template <class U = T, std::enable_if_t<!std::is_void<U>::value>* = nullptr>
  constexpr U& value() &
  {
    BEYOND_ASSERT(has_value());
    return val();
  }

  /// @overload
  template <class U = T, std::enable_if_t<!std::is_void<U>::value>* = nullptr>
  constexpr const U&& value() const&&
  {
    BEYOND_ASSERT(has_value());
    return std::move(val());
  }

  /// @overload
  template <class U = T, std::enable_if_t<!std::is_void<U>::value>* = nullptr>
  constexpr U&& value() &&
  {
    BEYOND_ASSERT(has_value());
    return std::move(val());
  }

  /// @brief returns the Unexpected value
  /// @pre there is an Unexpected value
  constexpr const E& error() const&
  {
    BEYOND_ASSERT(!has_value());
    return err().value();
  }

  /// @overload
  constexpr E& error() &
  {
    BEYOND_ASSERT(!has_value());
    return err().value();
  }

  /// @overload
  constexpr const E&& error() const&&
  {
    BEYOND_ASSERT(!has_value());
    return std::move(err().value());
  }

  /// @overload
  constexpr E&& error() &&
  {
    BEYOND_ASSERT(!has_value());
    return std::move(err().value());
  }

  /// @brief returns the stored value if there is one, otherwise returns `v`
  template <class U> constexpr T value_or(U&& v) const&
  {
    static_assert(std::is_copy_constructible<T>::value &&
                      std::is_convertible<U&&, T>::value,
                  "T must be copy-constructible and convertible to from U&&");
    return bool(*this) ? **this : static_cast<T>(std::forward<U>(v));
  }

  /// @overload
  template <class U> constexpr T value_or(U&& v) &&
  {
    static_assert(std::is_move_constructible_v<T> &&
                      std::is_convertible<U&&, T>::value,
                  "T must be move-constructible and convertible to from U&&");
    return bool(*this) ? std::move(**this) : static_cast<T>(std::forward<U>(v));
  }
};

namespace detail {
template <class Exp> using exp_t = typename std::decay_t<Exp>::value_type;
template <class Exp> using err_t = typename std::decay_t<Exp>::error_type;
template <class Exp, class Ret> using ret_t = Expected<Ret, err_t<Exp>>;

template <class Exp, class F,
          std::enable_if_t<!std::is_void<exp_t<Exp>>::value>* = nullptr,
          class Ret = decltype(std::invoke(std::declval<F>(),
                                           *std::declval<Exp>()))>
constexpr auto and_then_impl(Exp&& exp, F&& f)
{
  static_assert(detail::is_expected_v<Ret>, "F must return an Expected");

  return exp.has_value()
             ? std::invoke(std::forward<F>(f), *std::forward<Exp>(exp))
             : Ret(unexpect, exp.error());
}

template <class Exp, class F,
          std::enable_if_t<std::is_void<exp_t<Exp>>::value>* = nullptr,
          class Ret = decltype(std::invoke(std::declval<F>()))>
constexpr auto and_then_impl(Exp&& exp, F&& f)
{
  static_assert(detail::is_expected_v<Ret>, "F must return an Expected");

  return exp.has_value() ? std::invoke(std::forward<F>(f))
                         : Ret(unexpect, exp.error());
}

template <class Exp, class F,
          std::enable_if_t<!std::is_void<exp_t<Exp>>::value>* = nullptr,
          class Ret = decltype(std::invoke(std::declval<F>(),
                                           *std::declval<Exp>())),
          std::enable_if_t<!std::is_void<Ret>::value>* = nullptr>
constexpr auto expected_map_impl(Exp&& exp, F&& f)
{
  using result = ret_t<Exp, std::decay_t<Ret>>;
  return exp.has_value()
             ? result(std::invoke(std::forward<F>(f), *std::forward<Exp>(exp)))
             : result(unexpect, std::forward<Exp>(exp).error());
}

template <class Exp, class F,
          std::enable_if_t<!std::is_void<exp_t<Exp>>::value>* = nullptr,
          class Ret = decltype(std::invoke(std::declval<F>(),
                                           *std::declval<Exp>())),
          std::enable_if_t<std::is_void<Ret>::value>* = nullptr>
auto expected_map_impl(Exp&& exp, F&& f)
{
  using result = Expected<void, err_t<Exp>>;
  if (exp.has_value()) {
    std::invoke(std::forward<F>(f), *std::forward<Exp>(exp));
    return result();
  }

  return result(unexpect, std::forward<Exp>(exp).error());
}

template <class Exp, class F,
          std::enable_if_t<std::is_void<exp_t<Exp>>::value>* = nullptr,
          class Ret = decltype(std::invoke(std::declval<F>())),
          std::enable_if_t<!std::is_void<Ret>::value>* = nullptr>
constexpr auto expected_map_impl(Exp&& exp, F&& f)
{
  using result = ret_t<Exp, std::decay_t<Ret>>;
  return exp.has_value() ? result(std::invoke(std::forward<F>(f)))
                         : result(unexpect, std::forward<Exp>(exp).error());
}

template <class Exp, class F,
          std::enable_if_t<std::is_void<exp_t<Exp>>::value>* = nullptr,
          class Ret = decltype(std::invoke(std::declval<F>())),
          std::enable_if_t<std::is_void<Ret>::value>* = nullptr>
auto expected_map_impl(Exp&& exp, F&& f)
{
  using result = Expected<void, err_t<Exp>>;
  if (exp.has_value()) {
    std::invoke(std::forward<F>(f));
    return result();
  }

  return result(unexpect, std::forward<Exp>(exp).error());
}

template <class Exp, class F,
          std::enable_if_t<!std::is_void<exp_t<Exp>>::value>* = nullptr,
          class Ret = decltype(std::invoke(std::declval<F>(),
                                           std::declval<Exp>().error())),
          std::enable_if_t<!std::is_void<Ret>::value>* = nullptr>
constexpr auto map_error_impl(Exp&& exp, F&& f)
{
  using result = Expected<exp_t<Exp>, std::decay_t<Ret>>;
  return exp.has_value()
             ? result(*std::forward<Exp>(exp))
             : result(unexpect, std::invoke(std::forward<F>(f),
                                            std::forward<Exp>(exp).error()));
}
template <class Exp, class F,
          std::enable_if_t<!std::is_void<exp_t<Exp>>::value>* = nullptr,
          class Ret = decltype(std::invoke(std::declval<F>(),
                                           std::declval<Exp>().error())),
          std::enable_if_t<std::is_void<Ret>::value>* = nullptr>
auto map_error_impl(Exp&& exp, F&& f)
{
  using result = Expected<exp_t<Exp>, monostate>;
  if (exp.has_value()) {
    return result(*std::forward<Exp>(exp));
  }

  std::invoke(std::forward<F>(f), std::forward<Exp>(exp).error());
  return result(unexpect, monostate{});
}
template <class Exp, class F,
          std::enable_if_t<std::is_void<exp_t<Exp>>::value>* = nullptr,
          class Ret = decltype(std::invoke(std::declval<F>(),
                                           std::declval<Exp>().error())),
          std::enable_if_t<!std::is_void<Ret>::value>* = nullptr>
constexpr auto map_error_impl(Exp&& exp, F&& f)
{
  using result = Expected<exp_t<Exp>, std::decay_t<Ret>>;
  return exp.has_value()
             ? result()
             : result(unexpect, std::invoke(std::forward<F>(f),
                                            std::forward<Exp>(exp).error()));
}
template <class Exp, class F,
          std::enable_if_t<std::is_void<exp_t<Exp>>::value>* = nullptr,
          class Ret = decltype(std::invoke(std::declval<F>(),
                                           std::declval<Exp>().error())),
          std::enable_if_t<std::is_void<Ret>::value>* = nullptr>
auto map_error_impl(Exp&& exp, F&& f)
{
  using result = Expected<exp_t<Exp>, monostate>;
  if (exp.has_value()) {
    return result();
  }

  std::invoke(std::forward<F>(f), std::forward<Exp>(exp).error());
  return result(unexpect, monostate{});
}

template <class Exp, class F,
          class Ret = decltype(std::invoke(std::declval<F>(),
                                           std::declval<Exp>().error())),
          std::enable_if_t<!std::is_void<Ret>::value>* = nullptr>
constexpr auto or_else_impl(Exp&& exp, F&& f)
{
  static_assert(detail::is_expected_v<Ret>, "F must return an Expected");
  return exp.has_value()
             ? std::forward<Exp>(exp)
             : std::invoke(std::forward<F>(f), std::forward<Exp>(exp).error());
}

template <class Exp, class F,
          class Ret = decltype(std::invoke(std::declval<F>(),
                                           std::declval<Exp>().error())),
          std::enable_if_t<std::is_void<Ret>::value>* = nullptr>
std::decay_t<Exp> or_else_impl(Exp&& exp, F&& f)
{
  return exp.has_value()
             ? std::forward<Exp>(exp)
             : (std::invoke(std::forward<F>(f), std::forward<Exp>(exp).error()),
                std::forward<Exp>(exp));
}

} // namespace detail

template <class T, class E, class U, class F>
constexpr bool operator==(const Expected<T, E>& lhs, const Expected<U, F>& rhs)
{
  return (lhs.has_value() != rhs.has_value())
             ? false
             : (!lhs.has_value() ? lhs.error() == rhs.error() : *lhs == *rhs);
}
template <class T, class E, class U, class F>
constexpr bool operator!=(const Expected<T, E>& lhs, const Expected<U, F>& rhs)
{
  return (lhs.has_value() != rhs.has_value())
             ? true
             : (!lhs.has_value() ? lhs.error() != rhs.error() : *lhs != *rhs);
}

template <class T, class E, class U>
constexpr bool operator==(const Expected<T, E>& x, const U& v)
{
  return x.has_value() ? *x == v : false;
}
template <class T, class E, class U>
constexpr bool operator==(const U& v, const Expected<T, E>& x)
{
  return x.has_value() ? *x == v : false;
}
template <class T, class E, class U>
constexpr bool operator!=(const Expected<T, E>& x, const U& v)
{
  return x.has_value() ? *x != v : true;
}
template <class T, class E, class U>
constexpr bool operator!=(const U& v, const Expected<T, E>& x)
{
  return x.has_value() ? *x != v : true;
}

template <class T, class E>
constexpr bool operator==(const Expected<T, E>& x, const Unexpected<E>& e)
{
  return x.has_value() ? false : x.error() == e.value();
}
template <class T, class E>
constexpr bool operator==(const Unexpected<E>& e, const Expected<T, E>& x)
{
  return x.has_value() ? false : x.error() == e.value();
}
template <class T, class E>
constexpr bool operator!=(const Expected<T, E>& x, const Unexpected<E>& e)
{
  return x.has_value() ? true : x.error() != e.value();
}
template <class T, class E>
constexpr bool operator!=(const Unexpected<E>& e, const Expected<T, E>& x)
{
  return x.has_value() ? true : x.error() != e.value();
}

template <class T, class E,
          std::enable_if_t<std::is_move_constructible_v<T> &&
                           std::is_swappable_v<T&> && std::is_swappable_v<E&> &&
                           std::is_move_constructible_v<E>>* = nullptr>
void swap(Expected<T, E>& lhs,
          Expected<T, E>& rhs) noexcept(noexcept(lhs.swap(rhs)))
{
  lhs.swap(rhs);
}
} // namespace beyond

/** @}@} */

#endif // BEYOND_CORE_UTILS_EXPECTED_HPP
