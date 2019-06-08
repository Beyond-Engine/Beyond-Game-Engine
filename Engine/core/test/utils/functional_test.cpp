#include <catch2/catch.hpp>

#include "beyond/core/utils/functional.hpp"

TEST_CASE("Assign function object", "[beyond.core.util.funtional]")
{
  auto i = 1;
  CHECK(beyond::Assign<int>{}(i, 2) == 2);

  const auto r = 2;
  CHECK(beyond::Assign<int>{}(i, r) == r);
}

TEST_CASE("PlusEqual function object", "[beyond.core.util.funtional]")
{
  auto i = 1;
  CHECK(beyond::PlusEqual<int>{}(i, 2) == 3);

  const auto r = 2;
  CHECK(beyond::PlusEqual<int>{}(i, r) == 5);
}

TEST_CASE("MinusEqual function object", "[beyond.core.util.funtional]")
{
  auto i = 1;
  CHECK(beyond::MinusEqual<int>{}(i, 2) == -1);

  const auto r = 2;
  CHECK(beyond::MinusEqual<int>{}(i, r) == -3);
}
