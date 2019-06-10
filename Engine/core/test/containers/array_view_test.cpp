#include <catch2/catch.hpp>

#include <array>
#include <vector>

#include "beyond/core/containers/array_view.hpp"

TEST_CASE("ArrayView constructors", "[beyond.core.containers.array_view]")
{
  SECTION("Default constructor")
  {
    beyond::ArrayView<int> view;
    REQUIRE(view.empty());
    REQUIRE(view.size() == 0);
  }

  SECTION("Create from a pointer and size")
  {
    constexpr std::size_t size = 3;
    const int data[size] = {0, 1, 2};

    beyond::ArrayView view{static_cast<const int*>(data), size};
    REQUIRE(!view.empty());
    REQUIRE(view.size() == size);
    CHECK(view.data()[0] == 0);
    CHECK(view.data()[1] == 1);
    CHECK(view.data()[2] == 2);
  }

  SECTION("Create from a raw array")
  {
    constexpr std::size_t size = 3;
    constexpr int data[size] = {0, 1, 2};

    beyond::ArrayView view{data};
    REQUIRE(view.size() == size);
    CHECK(view.data()[0] == 0);
    CHECK(view.data()[1] == 1);
    CHECK(view.data()[2] == 2);
  }

  SECTION("Create from a std::array")
  {
    std::array<int, 3> v{0, 1, 2};

    beyond::ArrayView view{v};
    REQUIRE(view.size() == v.size());
    CHECK(view.data()[0] == v[0]);
    CHECK(view.data()[1] == v[1]);
    CHECK(view.data()[2] == v[2]);
  }

  SECTION("Create from a std::vector")
  {
    std::vector<int> v{0, 1, 2};

    beyond::ArrayView view{v};
    REQUIRE(view.size() == v.size());
    CHECK(view.data()[0] == v[0]);
    CHECK(view.data()[1] == v[1]);
    CHECK(view.data()[2] == v[2]);
  }
}

TEST_CASE("Fix-extends ArrayView")
{
  std::vector<int> v{0, 1, 2};
  std::array a{2, 1};

  GIVEN("A fixed view to array a=[2, 1]")
  {
    beyond::ArrayView<int, a.size()> fixed_view{a};
    static_assert(sizeof(fixed_view) == sizeof(void*),
                  "fixed ArrayView only stores a pointer");

    THEN("Can constructs an dynamic-extends ArrayView from it")
    {
      beyond::ArrayView<int, beyond::dynamic_extent> dynamic_view{fixed_view};
      static_assert(sizeof(dynamic_view) == sizeof(void*) * 2,
                    "dynamic ArrayView stores a pointer and a size");

      REQUIRE(fixed_view.size() == dynamic_view.size());
      REQUIRE(fixed_view.data() == dynamic_view.data());
    }
  }
}

TEST_CASE("ArrayView accessors", "[beyond.core.containers.array_view]")
{

  std::array a{0, 1, 2};
  beyond::ArrayView<int> view{a};

  SECTION("index accessor")
  {
    REQUIRE(view[0] == 0);
    REQUIRE(view(1) == 1);
  }

  SECTION("front and back")
  {
    REQUIRE(view.front() == 0);
    REQUIRE(view.back() == 2);
  }
}

TEST_CASE("ArrayView iterator", "[beyond.core.containers.array_view]")
{
  std::array a{0, 1, 2};
  beyond::ArrayView<int> view{a};

  CHECK(*view.begin() == 0);
  CHECK(*(view.end() - 1) == 2);
  CHECK(view.end() - view.begin() == 3);
}
