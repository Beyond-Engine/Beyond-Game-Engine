#include <catch2/catch.hpp>

#include <string>
#include <tuple>
#include <vector>

#include "beyond/core/utils/expected.hpp"

template class beyond::Unexpected<int>;
template class beyond::Expected<int, int>;
template class beyond::Expected<int, std::string>;
template class beyond::Expected<std::string, int>;
template class beyond::Expected<std::string, std::string>;

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
    {
      beyond::Expected<int, int> e{1};
      REQUIRE(e);
      REQUIRE(e == 1);
    }

    {
      beyond::Expected<std::string, int> e{"value"};
      REQUIRE(e);
      REQUIRE(e == "value");
    }
  }

  SECTION("Expected constructed with an Unexpected")
  {
    {
      beyond::Expected<int, int> e = beyond::make_unexpected(1);
      REQUIRE(!e);
      REQUIRE(e.error() == 1);
    }

    {
      beyond::Expected<int, std::string> e = beyond::make_unexpected("error");
      REQUIRE(!e);
      REQUIRE(e.error() == "error");
    }
  }

  SECTION("Expected constructed with unexpect tag")
  {
    {
      beyond::Expected<int, int> e{beyond::unexpect, 1};
      REQUIRE(!e);
      REQUIRE(e.error() == 1);
    }

    {
      beyond::Expected<int, std::string> e{beyond::unexpect, "error"};
      REQUIRE(!e);
      REQUIRE(e.error() == "error");
    }
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

  SECTION("Expected of void value")
  {
    beyond::Expected<void, int> e;
    REQUIRE(e);
  }
}

TEST_CASE("Assignments", "[beyond.core.utils.expected]")
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

TEST_CASE("Triviality", "[beyond.core.utils.expected]")
{
  STATIC_REQUIRE(
      std::is_trivially_copy_constructible_v<beyond::Expected<int, int>>);
  STATIC_REQUIRE(
      std::is_trivially_copy_assignable_v<beyond::Expected<int, int>>);
  STATIC_REQUIRE(
      std::is_trivially_move_constructible_v<beyond::Expected<int, int>>);
  STATIC_REQUIRE(
      std::is_trivially_move_assignable_v<beyond::Expected<int, int>>);
  STATIC_REQUIRE(std::is_trivially_destructible_v<beyond::Expected<int, int>>);

  STATIC_REQUIRE(
      std::is_trivially_copy_constructible_v<beyond::Expected<void, int>>);
  STATIC_REQUIRE(
      std::is_trivially_move_constructible_v<beyond::Expected<void, int>>);
  STATIC_REQUIRE(std::is_trivially_destructible_v<beyond::Expected<void, int>>);

  {
    struct T {
      T(const T&) = default;
      T(T&&) = default;
      T& operator=(const T&) = default;
      T& operator=(T&&) = default;
      ~T() = default;
    };
    STATIC_REQUIRE(
        std::is_trivially_copy_constructible_v<beyond::Expected<T, int>>);
    STATIC_REQUIRE(
        std::is_trivially_copy_assignable_v<beyond::Expected<T, int>>);
    STATIC_REQUIRE(
        std::is_trivially_move_constructible_v<beyond::Expected<T, int>>);
    STATIC_REQUIRE(
        std::is_trivially_move_assignable_v<beyond::Expected<T, int>>);
    STATIC_REQUIRE(std::is_trivially_destructible_v<beyond::Expected<T, int>>);
  }

  struct NoneTrivial {
    // NOLINTNEXTLINE(hicpp-use-equals-default, modernize-use-equals-default)
    NoneTrivial(const NoneTrivial&) {}
    NoneTrivial(NoneTrivial&&) {} //  // NOLINT(hicpp-noexcept-move)
    // NOLINTNEXTLINE(hicpp-use-equals-default, modernize-use-equals-default)
    NoneTrivial& operator=(const NoneTrivial&)
    {
      return *this;
    }
    NoneTrivial& operator=(NoneTrivial&&) // NOLINT(hicpp-use-equals-default,
                                          // hicpp-noexcept-move)
    {
      return *this;
    }

    ~NoneTrivial() {} // NOLINT
  };
  STATIC_REQUIRE(!std::is_trivially_copy_constructible_v<
                 beyond::Expected<NoneTrivial, int>>);
  STATIC_REQUIRE(
      !std::is_trivially_copy_assignable_v<beyond::Expected<NoneTrivial, int>>);
  STATIC_REQUIRE(!std::is_trivially_move_constructible_v<
                 beyond::Expected<NoneTrivial, int>>);
  STATIC_REQUIRE(
      !std::is_trivially_move_assignable_v<beyond::Expected<NoneTrivial, int>>);
  STATIC_REQUIRE(
      !std::is_trivially_destructible_v<beyond::Expected<NoneTrivial, int>>);
}

TEST_CASE("Deletion", "[beyond.core.utils.expected]")
{
  STATIC_REQUIRE(std::is_copy_constructible_v<beyond::Expected<int, int>>);
  STATIC_REQUIRE(std::is_copy_assignable_v<beyond::Expected<int, int>>);
  STATIC_REQUIRE(std::is_move_constructible_v<beyond::Expected<int, int>>);
  STATIC_REQUIRE(std::is_move_assignable_v<beyond::Expected<int, int>>);
  STATIC_REQUIRE(std::is_destructible_v<beyond::Expected<int, int>>);

  {
    struct T {
      T() = default;
    };
    STATIC_REQUIRE(std::is_default_constructible_v<beyond::Expected<T, int>>);
  }

  {
    struct T {
      T(int) {} // NOLINT
    };
    STATIC_REQUIRE(!std::is_default_constructible_v<beyond::Expected<T, int>>);
  }

  {
    struct T {
      T(const T&) = default;
      T(T&&) = default;
      T& operator=(const T&) = default;
      T& operator=(T&&) = default;
      ~T() = default;
    };
    STATIC_REQUIRE(std::is_copy_constructible_v<beyond::Expected<T, int>>);
    STATIC_REQUIRE(std::is_copy_assignable_v<beyond::Expected<T, int>>);
    STATIC_REQUIRE(std::is_move_constructible_v<beyond::Expected<T, int>>);
    STATIC_REQUIRE(std::is_move_assignable_v<beyond::Expected<T, int>>);
    STATIC_REQUIRE(std::is_destructible_v<beyond::Expected<T, int>>);
  }

  {
    struct T {
      T(const T&) = delete;
      T(T&&) = delete;
      T& operator=(const T&) = delete;
      T& operator=(T&&) = delete;
    };
    STATIC_REQUIRE(!std::is_copy_constructible_v<beyond::Expected<T, int>>);
    STATIC_REQUIRE(!std::is_copy_assignable_v<beyond::Expected<T, int>>);
    STATIC_REQUIRE(!std::is_move_constructible_v<beyond::Expected<T, int>>);
    STATIC_REQUIRE(!std::is_move_assignable_v<beyond::Expected<T, int>>);
  }

  {
    struct T {
      T(const T&) = delete;
      T(T&&) = default;
      T& operator=(const T&) = delete;
      T& operator=(T&&) = default;
    };
    STATIC_REQUIRE(!std::is_copy_constructible_v<beyond::Expected<T, int>>);
    STATIC_REQUIRE(!std::is_copy_assignable_v<beyond::Expected<T, int>>);
    STATIC_REQUIRE(std::is_move_constructible_v<beyond::Expected<T, int>>);
    STATIC_REQUIRE(std::is_move_assignable_v<beyond::Expected<T, int>>);
  }

  {
    struct T {
      T(const T&) = default;
      T(T&&) = delete;
      T& operator=(const T&) = default;
      T& operator=(T&&) = delete;
    };
    STATIC_REQUIRE(std::is_copy_constructible_v<beyond::Expected<T, int>>);
    STATIC_REQUIRE(std::is_copy_assignable_v<beyond::Expected<T, int>>);
  }

  {
    beyond::Expected<int, int> e;
    STATIC_REQUIRE(std::is_default_constructible_v<decltype(e)>);
    STATIC_REQUIRE(std::is_copy_constructible_v<decltype(e)>);
    STATIC_REQUIRE(std::is_move_constructible_v<decltype(e)>);
    STATIC_REQUIRE(std::is_copy_assignable_v<decltype(e)>);
    STATIC_REQUIRE(std::is_move_assignable_v<decltype(e)>);
    STATIC_REQUIRE(std::is_trivially_copy_constructible_v<decltype(e)>);
    STATIC_REQUIRE(std::is_trivially_copy_assignable_v<decltype(e)>);
    STATIC_REQUIRE(std::is_trivially_move_constructible_v<decltype(e)>);
    STATIC_REQUIRE(std::is_trivially_move_assignable_v<decltype(e)>);
  }

  {
    beyond::Expected<int, std::string> e;
    STATIC_REQUIRE(std::is_default_constructible_v<decltype(e)>);
    STATIC_REQUIRE(std::is_copy_constructible_v<decltype(e)>);
    STATIC_REQUIRE(std::is_move_constructible_v<decltype(e)>);
    STATIC_REQUIRE(std::is_copy_assignable_v<decltype(e)>);
    STATIC_REQUIRE(std::is_move_assignable_v<decltype(e)>);
    STATIC_REQUIRE(!std::is_trivially_copy_constructible_v<decltype(e)>);
    STATIC_REQUIRE(!std::is_trivially_copy_assignable_v<decltype(e)>);
    STATIC_REQUIRE(!std::is_trivially_move_constructible_v<decltype(e)>);
    STATIC_REQUIRE(!std::is_trivially_move_assignable_v<decltype(e)>);
  }

  {
    beyond::Expected<std::string, int> e;
    STATIC_REQUIRE(std::is_default_constructible_v<decltype(e)>);
    STATIC_REQUIRE(std::is_copy_constructible_v<decltype(e)>);
    STATIC_REQUIRE(std::is_move_constructible_v<decltype(e)>);
    STATIC_REQUIRE(std::is_copy_assignable_v<decltype(e)>);
    STATIC_REQUIRE(std::is_move_assignable_v<decltype(e)>);
    STATIC_REQUIRE(!std::is_trivially_copy_constructible_v<decltype(e)>);
    STATIC_REQUIRE(!std::is_trivially_copy_assignable_v<decltype(e)>);
    STATIC_REQUIRE(!std::is_trivially_move_constructible_v<decltype(e)>);
    STATIC_REQUIRE(!std::is_trivially_move_assignable_v<decltype(e)>);
  }

  {
    beyond::Expected<std::string, std::string> e;
    STATIC_REQUIRE(std::is_default_constructible_v<decltype(e)>);
    STATIC_REQUIRE(std::is_copy_constructible_v<decltype(e)>);
    STATIC_REQUIRE(std::is_move_constructible_v<decltype(e)>);
    STATIC_REQUIRE(std::is_copy_assignable_v<decltype(e)>);
    STATIC_REQUIRE(std::is_move_assignable_v<decltype(e)>);
    STATIC_REQUIRE(!std::is_trivially_copy_constructible_v<decltype(e)>);
    STATIC_REQUIRE(!std::is_trivially_copy_assignable_v<decltype(e)>);
    STATIC_REQUIRE(!std::is_trivially_move_constructible_v<decltype(e)>);
    STATIC_REQUIRE(!std::is_trivially_move_assignable_v<decltype(e)>);
  }
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

  SECTION("Compare with Expected")
  {
    CHECK(e1 == e2);
    CHECK(e1 != e3);
    CHECK(e1 != e4);
    CHECK(e4 != e1);
    CHECK(e4 == e5);
    CHECK(e4 != e6);
  }

  SECTION("Compare with T")
  {
    CHECK(e1 == i1);
    CHECK(i1 == e1);
    CHECK(e1 != i2);
    CHECK(i2 != e1);
  }

  SECTION("Compare with Unexpected<E>")
  {
    CHECK(e1 != beyond::make_unexpected(i1));
    CHECK(beyond::make_unexpected(i1) != e1);
    CHECK(e4 == beyond::make_unexpected(i1));
    CHECK(beyond::make_unexpected(i1) == e4);
    CHECK(e4 != beyond::make_unexpected(i2));
    CHECK(beyond::make_unexpected(i2) != e4);
  }
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

TEST_CASE("Expected Observers", "[beyond.core.utils.expected]")
{
  beyond::Expected<int, int> o1 = 42;
  beyond::Expected<int, int> o2{beyond::unexpect, 0};
  const beyond::Expected<int, int> o3 = 42;

  REQUIRE(*o1 == 42);
  REQUIRE(*o1 == o1.value());
  REQUIRE(o1.value_or(21) == 42);
  REQUIRE(o2.value_or(42) == 42);
  REQUIRE(o2.error() == 0);
  REQUIRE(o3.value() == 42);
  STATIC_REQUIRE(std::is_same_v<decltype(o1.value()), int&>);
  STATIC_REQUIRE(std::is_same_v<decltype(o3.value()), const int&>);
  STATIC_REQUIRE(std::is_same_v<decltype(std::move(o1).value()), int&&>);
  STATIC_REQUIRE(std::is_same_v<decltype(std::move(o3).value()), // NOLINT
                                const int&&>);

  struct move_detector {
    move_detector() = default;
    move_detector(move_detector&& rhs) noexcept
    {
      rhs.been_moved = true;
    }
    bool been_moved = false;
  };

  beyond::Expected<move_detector, int> o4{std::in_place};
  move_detector o5 = std::move(o4).value();
  REQUIRE(o4->been_moved); // NOLINT
  REQUIRE(!o5.been_moved);
}

TEST_CASE("Expected.map", "[beyond.core.utils.expected]")
{
  auto mul2 = [](auto t) { return t * 2; };
  auto ret_void = [](int) {};

  SECTION("double the value of Expected by map")
  {
    const auto v = 21;
    const auto v2 = 42;

    {
      beyond::Expected<int, int> e = v;
      auto ret = e.map(mul2);
      REQUIRE(ret);
      REQUIRE(*ret == v2);
    }

    {
      const beyond::Expected<int, int> e(v);
      const auto e2 = e.map(mul2);
      REQUIRE(e2);
      REQUIRE(e2 == v2);
    }

    {
      beyond::Expected<int, int> e = v;
      auto ret = std::move(e).map(mul2);
      REQUIRE(ret);
      REQUIRE(*ret == v2);
    }

    {
      const beyond::Expected<int, int> e = v;
      auto ret = std::move(e).map(mul2);
      REQUIRE(ret);
      REQUIRE(*ret == v2);
    }
  }

  SECTION("map does not impact the error value")
  {
    const auto v = 21;

    {
      beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = e.map(mul2);
      REQUIRE(!ret);
      REQUIRE(ret.error() == v);
    }

    {
      const beyond::Expected<int, int> e(beyond::unexpect, v);
      const auto e2 = e.map(mul2);
      REQUIRE(!e2);
      REQUIRE(e2.error() == v);
    }

    {
      beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = std::move(e).map(mul2);
      REQUIRE(!ret);
      REQUIRE(ret.error() == v);
    }

    {
      const beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = std::move(e).map(mul2);
      REQUIRE(!ret);
      REQUIRE(ret.error() == v);
    }
  }

  SECTION("map with a function that returns void")
  {
    const auto v = 21;
    {
      beyond::Expected<int, int> e = v;
      auto ret = e.map(ret_void);
      REQUIRE(ret);
      STATIC_REQUIRE(
          std::is_same_v<decltype(ret), beyond::Expected<void, int>>);
    }

    {
      const beyond::Expected<int, int> e = v;
      auto ret = e.map(ret_void);
      REQUIRE(ret);
      STATIC_REQUIRE(
          (std::is_same_v<decltype(ret), beyond::Expected<void, int>>));
    }

    {
      beyond::Expected<int, int> e = v;
      auto ret = std::move(e).map(ret_void);
      REQUIRE(ret);
      STATIC_REQUIRE(
          (std::is_same_v<decltype(ret), beyond::Expected<void, int>>));
    }

    {
      const beyond::Expected<int, int> e = v;
      auto ret = std::move(e).map(ret_void);
      REQUIRE(ret);
      STATIC_REQUIRE(
          (std::is_same_v<decltype(ret), beyond::Expected<void, int>>));
    }
  }

  SECTION("map with a function that returns void does not impact the error")
  {
    const auto v = 21;
    {
      beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = e.map(ret_void);
      REQUIRE(!ret);
      STATIC_REQUIRE(
          (std::is_same_v<decltype(ret), beyond::Expected<void, int>>));
    }

    {
      const beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = e.map(ret_void);
      REQUIRE(!ret);
      STATIC_REQUIRE(
          (std::is_same_v<decltype(ret), beyond::Expected<void, int>>));
    }

    {
      beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = std::move(e).map(ret_void);
      REQUIRE(!ret);
      STATIC_REQUIRE(
          (std::is_same_v<decltype(ret), beyond::Expected<void, int>>));
    }

    {
      const beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = std::move(e).map(ret_void);
      REQUIRE(!ret);
      STATIC_REQUIRE(
          (std::is_same_v<decltype(ret), beyond::Expected<void, int>>));
    }
  }

  SECTION("mapping functions which return references")
  {
    const auto v = 42;
    beyond::Expected<int, int> e(v);
    auto ret = e.map([](int& i) -> int& { return i; });
    REQUIRE(ret);
    REQUIRE(ret == v);
  }
}

TEST_CASE("Expected.and_then", "[beyond.core.utils.expected]")
{
  const auto fail_v = 17;
  auto succeed = [](int a) { return beyond::Expected<int, int>(a * 2); };
  auto fail = [fail_v](int) {
    return beyond::Expected<int, int>(beyond::unexpect, fail_v);
  };

  SECTION("and_then pip this expected with a function")
  {
    const auto v = 21;
    const auto v2 = 42;

    {
      beyond::Expected<int, int> e = v;
      auto ret = e.and_then(succeed);
      REQUIRE(ret);
      REQUIRE(*ret == v2);
    }

    {
      const beyond::Expected<int, int> e = v;
      auto ret = e.and_then(succeed);
      REQUIRE(ret);
      REQUIRE(*ret == v2);
    }

    {
      beyond::Expected<int, int> e = v;
      auto ret = std::move(e).and_then(succeed);
      REQUIRE(ret);
      REQUIRE(*ret == v2);
    }

    {
      const beyond::Expected<int, int> e = v;
      auto ret = std::move(e).and_then(succeed);
      REQUIRE(ret);
      REQUIRE(*ret == v2);
    }
  }

  SECTION("and_then returns the expected of failing value")
  {
    const auto v = 21;
    {
      beyond::Expected<int, int> e = v;
      auto ret = e.and_then(fail);
      REQUIRE(!ret);
      REQUIRE(ret.error() == fail_v);
    }
    {
      const beyond::Expected<int, int> e = v;
      auto ret = e.and_then(fail);
      REQUIRE(!ret);
      REQUIRE(ret.error() == fail_v);
    }
    {
      beyond::Expected<int, int> e = v;
      auto ret = std::move(e).and_then(fail);
      REQUIRE(!ret);
      REQUIRE(ret.error() == fail_v);
    }
    {
      const beyond::Expected<int, int> e = v;
      auto ret = std::move(e).and_then(fail);
      REQUIRE(!ret);
      REQUIRE(ret.error() == fail_v);
    }
  }

  SECTION("and_then pass the error of expected if it already contains one")
  {
    const auto v = 21;

    {
      beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = e.and_then(succeed);
      REQUIRE(!ret);
      REQUIRE(ret.error() == v);
    }

    {
      const beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = e.and_then(succeed);
      REQUIRE(!ret);
      REQUIRE(ret.error() == v);
    }

    {
      beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = std::move(e).and_then(succeed);
      REQUIRE(!ret);
      REQUIRE(ret.error() == v);
    }

    {
      const beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = std::move(e).and_then(succeed);
      REQUIRE(!ret);
      REQUIRE(ret.error() == v);
    }
  }
}

TEST_CASE("Expected.map_error", "[beyond.core.utils.expected]")
{
  auto mul2 = [](int a) { return a * 2; };
  auto ret_void = [](int) {};

  SECTION("map_error does not impact Expected contains a value")
  {
    const auto v = 21;

    {
      beyond::Expected<int, int> e = v;
      auto ret = e.map_error(mul2);
      REQUIRE(ret);
      REQUIRE(*ret == v);
    }

    {
      const beyond::Expected<int, int> e = v;
      auto ret = e.map_error(mul2);
      REQUIRE(ret);
      REQUIRE(*ret == v);
    }

    {
      beyond::Expected<int, int> e = v;
      auto ret = std::move(e).map_error(mul2);
      REQUIRE(ret);
      REQUIRE(*ret == v);
    }

    {
      const beyond::Expected<int, int> e = v;
      auto ret = std::move(e).map_error(mul2);
      REQUIRE(ret);
      REQUIRE(*ret == v);
    }
  }

  SECTION("map_error will map the error value of Expected")
  {
    const auto error = 21;
    const auto error2 = 42;

    {
      beyond::Expected<int, int> e(beyond::unexpect, error);
      auto ret = e.map_error(mul2);
      REQUIRE(!ret);
      REQUIRE(ret.error() == error2);
    }

    {
      const beyond::Expected<int, int> e(beyond::unexpect, error);
      auto ret = e.map_error(mul2);
      REQUIRE(!ret);
      REQUIRE(ret.error() == error2);
    }

    {
      beyond::Expected<int, int> e(beyond::unexpect, error);
      auto ret = std::move(e).map_error(mul2);
      REQUIRE(!ret);
      REQUIRE(ret.error() == error2);
    }

    {
      const beyond::Expected<int, int> e(beyond::unexpect, error);
      auto ret = std::move(e).map_error(mul2);
      REQUIRE(!ret);
      REQUIRE(ret.error() == error2);
    }
  }

  SECTION("map_error of void does not impact Expected contains a value")
  {
    const auto v = 21;

    {
      beyond::Expected<int, int> e = v;
      auto ret = e.map_error(ret_void);
      REQUIRE(ret);
    }

    {
      const beyond::Expected<int, int> e = v;
      auto ret = e.map_error(ret_void);
      REQUIRE(ret);
    }

    {
      beyond::Expected<int, int> e = v;
      auto ret = std::move(e).map_error(ret_void);
      REQUIRE(ret);
    }

    {
      const beyond::Expected<int, int> e = v;
      auto ret = std::move(e).map_error(ret_void);
      REQUIRE(ret);
    }
  }

  SECTION("map_error of void map the Expected contains of error to Expected<T, "
          "void>")
  {
    const auto unex = 21;
    {
      beyond::Expected<int, int> e(beyond::unexpect, unex);
      auto ret = e.map_error(ret_void);
      REQUIRE(!ret);
    }

    {
      const beyond::Expected<int, int> e(beyond::unexpect, unex);
      auto ret = e.map_error(ret_void);
      REQUIRE(!ret);
    }

    {
      beyond::Expected<int, int> e(beyond::unexpect, unex);
      auto ret = std::move(e).map_error(ret_void);
      REQUIRE(!ret);
    }

    {
      const beyond::Expected<int, int> e(beyond::unexpect, unex);
      auto ret = std::move(e).map_error(ret_void);
      REQUIRE(!ret);
    }
  }
}

TEST_CASE("Expected.or_else", "[beyond.core.utils.expected]")
{
  const auto error_v = 17;
  using eptr = std::unique_ptr<int>;
  auto succeed = [](int v) { return beyond::Expected<int, int>(v * 2); };
  auto succeedptr = [](eptr ptr) {
    return beyond::Expected<int, eptr>(*ptr * 2);
  };
  auto fail = [error_v](int) {
    return beyond::Expected<int, int>(beyond::unexpect, error_v);
  };
  auto efail = [error_v](eptr e) {
    *e = error_v;
    return beyond::Expected<int, eptr>(beyond::unexpect, std::move(e));
  };
  auto failvoid = [](int) {};
  auto failvoidptr = [](const eptr&) { /* don't consume */ };
  auto consumeptr = [](eptr) {};
  auto make_u_int = [](int n) { return std::unique_ptr<int>(new int(n)); };

  SECTION("or_else does not impact an Expected with a value")
  {
    const auto v = 21;
    {
      beyond::Expected<int, int> e = v;
      auto ret = e.or_else(succeed);
      REQUIRE(ret);
      REQUIRE(*ret == v);
    }

    {
      const beyond::Expected<int, int> e = v;
      auto ret = e.or_else(succeed);
      REQUIRE(ret);
      REQUIRE(*ret == v);
    }

    {
      beyond::Expected<int, int> e = v;
      auto ret = std::move(e).or_else(succeed);
      REQUIRE(ret);
      REQUIRE(*ret == v);
    }

    {
      beyond::Expected<int, eptr> e = v;
      auto ret = std::move(e).or_else(succeedptr);
      REQUIRE(ret);
      REQUIRE(*ret == v);
    }

    {
      const beyond::Expected<int, int> e = v;
      auto ret = std::move(e).or_else(succeed);
      REQUIRE(ret);
      REQUIRE(*ret == v);
    }

    {
      beyond::Expected<int, int> e = v;
      auto ret = e.or_else(fail);
      REQUIRE(ret);
      REQUIRE(*ret == v);
    }

    {
      const beyond::Expected<int, int> e = v;
      auto ret = e.or_else(fail);
      REQUIRE(ret);
      REQUIRE(*ret == v);
    }

    {
      beyond::Expected<int, int> e = v;
      auto ret = std::move(e).or_else(fail);
      REQUIRE(ret);
      REQUIRE(ret == v);
    }

    {
      beyond::Expected<int, eptr> e = v;
      auto ret = std::move(e).or_else(efail);
      REQUIRE(ret);
      REQUIRE(ret == v);
    }

    {
      const beyond::Expected<int, int> e = v;
      auto ret = std::move(e).or_else(fail);
      REQUIRE(ret);
      REQUIRE(*ret == v);
    }
  }

  SECTION("or_else with succeeding function will convert an Expected with "
          "error value to an Expected that contains a value")
  {
    const auto v = 21;
    const auto v2 = 42;

    {
      beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = e.or_else(succeed);
      REQUIRE(ret);
      REQUIRE(*ret == v2);
    }

    {
      const beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = e.or_else(succeed);
      REQUIRE(ret);
      REQUIRE(*ret == v2);
    }

    {
      beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = std::move(e).or_else(succeed);
      REQUIRE(ret);
      REQUIRE(*ret == v2);
    }

    {
      beyond::Expected<int, eptr> e(beyond::unexpect, make_u_int(v));
      auto ret = std::move(e).or_else(succeedptr);
      REQUIRE(ret);
      REQUIRE(*ret == v2);
    }

    {
      const beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = std::move(e).or_else(succeed);
      REQUIRE(ret);
      REQUIRE(*ret == v2);
    }
  }

  SECTION("or_else with failing function will returns an Expected with the "
          "error value")
  {
    const auto v = 21;

    {
      beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = e.or_else(fail);
      REQUIRE(!ret);
      REQUIRE(ret.error() == error_v);
    }

    {
      beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = e.or_else(failvoid);
      REQUIRE(!ret);
      REQUIRE(ret.error() == v);
    }

    {
      const beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = e.or_else(fail);
      REQUIRE(!ret);
      REQUIRE(ret.error() == error_v);
    }

    {
      const beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = e.or_else(failvoid);
      REQUIRE(!ret);
      REQUIRE(ret.error() == v);
    }

    {
      beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = std::move(e).or_else(fail);
      REQUIRE(!ret);
      REQUIRE(ret.error() == error_v);
    }

    {
      beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = std::move(e).or_else(failvoid);
      REQUIRE(!ret);
      REQUIRE(ret.error() == v);
    }

    {
      beyond::Expected<int, eptr> e(beyond::unexpect, make_u_int(v));
      auto ret = std::move(e).or_else(failvoidptr);
      REQUIRE(!ret);
      REQUIRE(*ret.error() == v);
    }

    {
      beyond::Expected<int, eptr> e(beyond::unexpect, make_u_int(v));
      auto ret = std::move(e).or_else(consumeptr);
      REQUIRE(!ret);
      REQUIRE(ret.error() == nullptr);
    }

    {
      const beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = std::move(e).or_else(fail);
      REQUIRE(!ret);
      REQUIRE(ret.error() == error_v);
    }

    {
      const beyond::Expected<int, int> e(beyond::unexpect, v);
      auto ret = std::move(e).or_else(failvoid);
      REQUIRE(!ret);
      REQUIRE(ret.error() == v);
    }
  }
}

TEST_CASE("Unexpected construction", "[beyond.core.utils.expected]")
{
  REQUIRE(beyond::make_unexpected(2).value() == 2);
}

TEST_CASE("Unexpected Equality", "[beyond.core.utils.expected]")
{
  const auto v1 = 2;
  const auto v2 = 3;

  REQUIRE(beyond::make_unexpected(v1) == beyond::make_unexpected(v1));
  REQUIRE(beyond::make_unexpected(v1) != beyond::make_unexpected(v2));
  REQUIRE(beyond::make_unexpected(v1) < beyond::make_unexpected(v2));
  REQUIRE(beyond::make_unexpected(v1) <= beyond::make_unexpected(v2));
  REQUIRE(beyond::make_unexpected(v2) > beyond::make_unexpected(v1));
  REQUIRE(beyond::make_unexpected(v2) >= beyond::make_unexpected(v1));
}
