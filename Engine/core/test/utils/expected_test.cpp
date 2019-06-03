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
    REQUIRE(std::get<0>(*e) == 0);
    REQUIRE(std::get<1>(*e) == 1);
  }

  SECTION("In place construction of Expected of vector")
  {
    beyond::Expected<std::vector<int>, int> e(std::in_place, {0, 1});
    REQUIRE(e);
    REQUIRE((*e)[0] == 0);
    REQUIRE((*e)[1] == 1);
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
  REQUIRE(*e1 == 17);
  REQUIRE(e2);
  REQUIRE(*e2 == 17);

  e1 = e2;
  REQUIRE(e1);
  REQUIRE(*e1 == 17);
  REQUIRE(e2);
  REQUIRE(*e2 == 17);

  e1 = 42;
  REQUIRE(e1);
  REQUIRE(*e1 == 42);

  auto unex = beyond::make_unexpected(12);
  e1 = unex;
  REQUIRE(!e1);
  REQUIRE(e1.error() == 12);

  e1 = beyond::make_unexpected(42);
  REQUIRE(!e1);
  REQUIRE(e1.error() == 42);

  e1 = e3;
  REQUIRE(e1);
  REQUIRE(*e1 == 21);

  e4 = e5;
  REQUIRE(!e4);
  REQUIRE(e4.error() == 17);

  e4 = e6;
  REQUIRE(!e4);
  REQUIRE(e4.error() == 21);

  e4 = e1;
  REQUIRE(e4);
  REQUIRE(*e4 == 21);
}
