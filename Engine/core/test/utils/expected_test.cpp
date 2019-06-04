#include <catch2/catch.hpp>

#include <tuple>
#include <vector>

#include "beyond/core/utils/expected.hpp"

template class beyond::Unexpected<int>;
template class beyond::Expected<int, int>;

TEST_CASE("Expected Constructors", "[beyond.core.utils.expected]")
{
  SECTION("A default constructed Expected")
  {
    beyond::Expected<int, int> e;

    REQUIRE(e);
    REQUIRE(e.has_value());
    REQUIRE(e == 0);
  }

  SECTION("Expected constructed with a value")
  {
    beyond::Expected<int, int> e{1};
    REQUIRE(e);
    REQUIRE(e == 1);
  }

  SECTION("Expected constructed with an Unexpected")
  {
    beyond::Expected<int, int> e = beyond::make_unexpected(1);
    REQUIRE(!e);
    REQUIRE(e.error() == 1);
  }

  SECTION("Expected constructed with unexpect tag")
  {
    beyond::Expected<int, int> e{beyond::unexpect, 1};
    REQUIRE(!e);
    REQUIRE(e.error() == 1);
  }

  SECTION("In place construction of Expected of tuple")
  {
    beyond::Expected<std::tuple<int, int>, int> e(std::in_place, 0, 1);
    REQUIRE(e);
    CHECK(std::get<0>(*e) == 0);
    CHECK(std::get<1>(*e) == 1);
  }

  SECTION("In place construction of Expected of vector")
  {
    beyond::Expected<std::vector<int>, int> e(std::in_place, {0, 1});
    REQUIRE(e);
    CHECK((*e)[0] == 0);
    CHECK((*e)[1] == 1);
  }
}

TEST_CASE("Expected assignments", "[beyond.core.utils.expected]")
{
  beyond::Expected<int, int> e1 = 42;
  beyond::Expected<int, int> e2 = 17;
  beyond::Expected<int, int> e3 = 21;
  beyond::Expected<int, int> e4 = beyond::make_unexpected(42);
  beyond::Expected<int, int> e5 = beyond::make_unexpected(17);
  beyond::Expected<int, int> e6 = beyond::make_unexpected(21);

  e1 = e2;
  REQUIRE(e1);
  CHECK(*e1 == 17);
  REQUIRE(e2);
  CHECK(*e2 == 17);

  e1 = e2;
  REQUIRE(e1);
  CHECK(*e1 == 17);
  REQUIRE(e2);
  CHECK(*e2 == 17);

  e1 = 42;
  REQUIRE(e1);
  CHECK(*e1 == 42);

  auto unex = beyond::make_unexpected(12);
  e1 = unex;
  REQUIRE(!e1);
  CHECK(e1.error() == 12);

  e1 = beyond::make_unexpected(42);
  REQUIRE(!e1);
  CHECK(e1.error() == 42);

  e1 = e3;
  REQUIRE(e1);
  CHECK(*e1 == 21);

  e4 = e5;
  REQUIRE(!e4);
  CHECK(e4.error() == 17);

  e4 = e6;
  REQUIRE(!e4);
  CHECK(e4.error() == 21);

  e4 = e1;
  REQUIRE(e4);
  CHECK(*e4 == 21);
}

TEST_CASE("Expected equality comparison", "[beyond.core.utils.expected]")
{
  const auto i1 = 42, i2 = 17;
  const beyond::Expected<int, int> e1 = i1;
  const beyond::Expected<int, int> e2 = i1;
  const beyond::Expected<int, int> e3 = i2;
  const beyond::Expected<int, int> e4 = beyond::make_unexpected(i1);
  const beyond::Expected<int, int> e5 = beyond::make_unexpected(i1);
  const beyond::Expected<int, int> e6 = beyond::make_unexpected(i2);

  // Expected equality operators
  CHECK(e1 == e2);
  CHECK(e1 != e3);
  CHECK(e1 != e4);
  CHECK(e4 != e1);
  CHECK(e4 == e5);
  CHECK(e4 != e6);

  // Compare with T
  CHECK(e1 == i1);
  CHECK(i1 == e1);
  CHECK(e1 != i2);
  CHECK(i2 != e1);

  // Compare with Unexpected<E>
  CHECK(e1 != beyond::make_unexpected(i1));
  CHECK(beyond::make_unexpected(i1) != e1);
  CHECK(e4 == beyond::make_unexpected(i1));
  CHECK(beyond::make_unexpected(i1) == e4);
  CHECK(e4 != beyond::make_unexpected(i2));
  CHECK(beyond::make_unexpected(i2) != e4);
}

TEST_CASE("Expected swap", "[beyond.core.utils.expected]")
{
  const auto i1 = 42, i2 = 17;
  beyond::Expected<int, int> e1 = i1;
  beyond::Expected<int, int> e2 = i2;
  beyond::Expected<int, int> e3 = beyond::make_unexpected(i1);
  beyond::Expected<int, int> e4 = beyond::make_unexpected(i2);

  SECTION("Swap e1, e2")
  {
    swap(e1, e2);
    REQUIRE(e1);
    REQUIRE(e2);
    CHECK(e1 == i2);
    CHECK(e2 == i1);
  }

  SECTION("Swap e1, e3")
  {
    swap(e1, e3);
    REQUIRE(!e1);
    REQUIRE(e3);
    CHECK(e1.error() == i1);
    CHECK(e3 == i1);
  }

  SECTION("Swap e4, e1")
  {
    swap(e4, e1);
    REQUIRE(e4);
    REQUIRE(!e1);
    CHECK(e4 == i1);
    CHECK(e1.error() == i2);
  }

  SECTION("Swap e3, e4")
  {
    swap(e3, e4);
    REQUIRE(!e3);
    REQUIRE(!e4);
    CHECK(e3.error() == i2);
    CHECK(e4.error() == i1);
  }

  SECTION("Swap two good expected")
  {
    swap(e1, e2);
    CHECK(e1 == i2);
    CHECK(e2 == i1);
  }
}

TEST_CASE("Expected.map", "[beyond.core.utils.expected]")
{
  auto times2 = [](auto t) { return t * 2; };

  SECTION("double the value of Expected by map")
  {
    const beyond::Expected<int, int> e(21);
    const auto e2 = e.map(times2);
    REQUIRE(e2);
    REQUIRE(e2 == 42);
  }
}
