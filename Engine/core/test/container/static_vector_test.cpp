#include <catch2/catch.hpp>

#include <array>
#include <beyond/core/container/static_vector.hpp>
#include <string>

using namespace beyond;

TEST_CASE("static_vector", "[container]")
{
  GIVEN("A default constructed static_vector")
  {
    static_vector<int, 10> v1;
    REQUIRE(v1.capacity() == 10);
    THEN("it is empty")
    {
      REQUIRE(v1.empty());
      REQUIRE(v1.size() == 0);
    }

    WHEN("Adds an element to it")
    {
      const int first = 42;
      v1.emplace_back(first);

      THEN("Can find that element at front")
      {
        REQUIRE(v1[0] == first);
      }

      THEN("It is no longer empty")
      {
        REQUIRE(!v1.empty());
        REQUIRE(v1.size() == 1);
      }

      AND_WHEN("Adds another element to it")
      {
        const int second = 21;
        v1.push_back(second);
        REQUIRE(v1.size() == 2);
        REQUIRE(v1[1] == second);

        AND_WHEN("pops the last element from it")
        {
          v1.pop_back();
          REQUIRE(v1.size() == 1);
          REQUIRE(v1[0] == first);
        }
      }
    }
  }
}

TEST_CASE("static_vector constructors", "[container]")
{
  SECTION("construct by size")
  {
    static_vector<int, 10> v(8);
    REQUIRE(v.size() == 8);
    REQUIRE(v[1] == int{});
  }

  SECTION("construct by size and a value")
  {
    static_vector<int, 10> v(8, 42);
    REQUIRE(v.size() == 8);
    REQUIRE(v[7] == 42);
  }

  SECTION("construct by a pair of iterator")
  {
    std::array a{1, 2, 3, 4, 5};
    static_vector<int, 10> v{a.begin(), a.end()};
    REQUIRE(v.size() == 5);
    REQUIRE(v[3] == 4);
  }

  SECTION("construct by an initializer list")
  {
    static_vector<int, 10> v{1, 2, 3, 4, 5};
    REQUIRE(v.size() == 5);
    REQUIRE(v[3] == 4);
  }
}

TEST_CASE("static_vector iterators", "[container]")
{
  static_vector<std::string, 10> v;
  REQUIRE(v.begin() == v.end());

  const std::string first{"hello"};
  v.push_back(first);
  REQUIRE(v.begin() != v.end());

  SECTION("Elements access")
  {
    REQUIRE(*v.begin() == first);
    REQUIRE(v.begin()->size() == first.size());
  }

  SECTION("Pre and post fix increment & decrement")
  {
    const std::string second{"world"};
    v.push_back(second);
    auto i = v.begin();
    REQUIRE(*(++i) == second);
    REQUIRE(*i == second);
    REQUIRE(*(i++) == second);
    REQUIRE(i == v.end());
    REQUIRE((i--) == v.end());
    REQUIRE(*i == second);
    REQUIRE(*(--i) == first);
    REQUIRE(*i == first);
  }

  SECTION("operator[]")
  {
    // TODO
  }

  SECTION("Iterator ordering")
  {
    // TODO
  }

  SECTION("Random access iterator affine space operations")
  {
    // TODO
  }
}
