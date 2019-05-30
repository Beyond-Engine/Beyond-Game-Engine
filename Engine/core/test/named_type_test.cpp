#include "core/crtp.hpp"
#include "core/type_traits.hpp"

#include <catch2/catch.hpp>

namespace beyond {

template <typename T> class Addable {
};
template <typename T> class Comparable {
};

template <typename T, typename Tag, template <typename> typename... Mixins>
struct NamedType : public Mixins<NamedType<T, Tag, Mixins...>>... {
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
