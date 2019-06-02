#include <functional>
#include <utility>

#include "core/crtp.hpp"
#include "core/type_traits.hpp"

namespace beyond {

// Enable empty base class optimization with multiple inheritance on Visual
// Studio.
#if defined(_MSC_VER)
#define BEYOND_EBCO __declspec(empty_bases)
#else
#define BEYOND_EBCO
#endif

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

} // namespace beyond

#include <catch2/catch.hpp>

using NamedDouble = beyond::NamedType<double, struct NamedDoubleTag>;
using NamedDoubleR = beyond::NamedType<double&, struct NamedDoubleTag>;
using NamedDoubleCR = beyond::NamedType<const double&, struct NamedDoubleTag>;
TEST_CASE("NamedType basic usage", "[beyond.core.meta.named_type]")
{
  SECTION("Constructs from prvalue")
  {
    const NamedDouble nd{1.2};
    REQUIRE(nd.get() == Approx(1.2));
  }

  SECTION("Constructs from lvalue")
  {
    const auto value = 1.4;
    NamedDouble nd{value};
    REQUIRE(nd.get() == Approx(value));

    SECTION("Modify the underlying value")
    {
      nd.get() = 2;
      REQUIRE(nd.get() == Approx(2));
    }
  }
}

TEST_CASE("NamedType of References", "[beyond.core.meta.named_type]")
{
  SECTION("Constructs from lvalue")
  {
    auto value = 1.4;
    NamedDoubleR nd{value};
    REQUIRE(nd.get() == Approx(value));
  }

  SECTION("Creates from None-Reference NamedType")
  {
    auto nd1 = NamedDouble{1.2};
    NamedDoubleR nd2 = nd1;
    REQUIRE(nd2.get() == Approx(1.2));
  }

  SECTION("Creates from const None-Reference NamedType")
  {
    const auto nd1 = NamedDouble{1.2};
    NamedDoubleCR nd2 = nd1;
    REQUIRE(nd2.get() == Approx(1.2));
  }
}

TEST_CASE("Arithmatic Operations", "[beyond.core.meta.named_type]")
{

  SECTION("Incrementation")
  {
    using Incrementable =
        beyond::NamedType<int, struct NamedIntTag, beyond::IncrementableBase>;
    constexpr auto i = 1;
    Incrementable n{i};
    CHECK((++n).get() == 2);
    CHECK((n++).get() == 2);
    CHECK(n.get() == 3);
  }

  SECTION("Decrementation")
  {
    using Decrementable =
        beyond::NamedType<int, struct NamedIntTag, beyond::DecrementableBase>;
    constexpr auto i = 1;
    Decrementable n{i};
    CHECK((--n).get() == 0);
    CHECK((n--).get() == 0);
    CHECK(n.get() == -1);
  }

  SECTION("Addition")
  {
    using Addable =
        beyond::NamedType<double, struct NamedDoubleTag, beyond::AddableBase>;
    constexpr auto d1 = 1., d2 = 3.;
    Addable n1{d1};
    Addable n2{d2};
    REQUIRE((n1 + n2).get() == Approx(d1 + d2));
  }

  SECTION("Subtraction")
  {
    using Subtractble = beyond::NamedType<double, struct NamedDoubleTag,
                                          beyond::SubtractableBase>;
    constexpr auto d1 = 1., d2 = 3.;
    Subtractble n1{d1};
    Subtractble n2{d2};
    REQUIRE((n1 - n2).get() == Approx(d1 - d2));
  }

  SECTION("Negation")
  {
    using Negatabe =
        beyond::NamedType<double, struct NamedDoubleTag, beyond::NegatabeBase>;
    constexpr auto d1 = 1.;
    Negatabe n1{d1};
    REQUIRE((-n1).get() == Approx(-d1));
  }
}

TEST_CASE("Comparison Operations", "[beyond.core.meta.named_type]")
{
  SECTION("Equality")
  {
    using Equable =
        beyond::NamedType<int, struct NamedIntTag, beyond::EquableBase>;
    Equable n1{1};
    Equable n2{2};
    CHECK(!(n1 == n2));
    CHECK(n1 != n2);
  }

  SECTION("comparable")
  {
    using Comparable =
        beyond::NamedType<int, struct NamedIntTag, beyond::ComparableBase>;
    Comparable n1{1};
    Comparable n2{2};
    CHECK(!(n1 == n2));
    CHECK(n1 != n2);
    CHECK(n1 < n2);
    CHECK(n1 <= n2);
    CHECK(!(n1 > n2));
    CHECK(!(n1 >= n2));
  }
}

struct Meter
    : beyond::NamedType<double, struct MeterTag, beyond::IncrementableBase,
                        beyond::DecrementableBase, beyond::AddableBase,
                        beyond::SubtractableBase> {
  explicit Meter(double d) : NamedType{d} {}
  Meter(NamedType n) : NamedType{n} {}
};

TEST_CASE("Inheritance from NamedType")
{
  constexpr auto d1 = 1., d2 = 3.;
  Meter n1{d1};
  Meter n2{d2};
  Meter result = n1 - n2;
  REQUIRE(result.get() == Approx(d1 - d2));
}

TEST_CASE("Empty base class optimization")
{
  static_assert(sizeof(Meter) == sizeof(double));
}
