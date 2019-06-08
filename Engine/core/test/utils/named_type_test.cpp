#include <catch2/catch.hpp>

#include "beyond/core/utils/named_type.hpp"

using NamedDouble = beyond::NamedType<double, struct NamedDoubleTag>;
using NamedDoubleR = beyond::NamedType<double&, struct NamedDoubleTag>;
using NamedDoubleCR = beyond::NamedType<const double&, struct NamedDoubleTag>;
TEST_CASE("NamedType basic usage", "[beyond.core.utils.named_type]")
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

TEST_CASE("NamedType of References", "[beyond.core.utils.named_type]")
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

TEST_CASE("Comparison Operations", "[beyond.core.utils.named_type]")
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
