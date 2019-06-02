#include <catch2/catch.hpp>

#include <tuple>
#include <vector>

#include "beyond/core/utils/expected.hpp"

TEST_CASE("Constructors Expected", "[beyond.core.utils.expected]")
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
