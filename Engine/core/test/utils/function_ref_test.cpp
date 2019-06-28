#include <catch2/catch.hpp>

#include "beyond/core/utils/function_ref.hpp"

TEST_CASE("function_ref assignments", "[beyond.core.function_ref]")
{
  const auto f = [] {};
  struct b {
    void baz() {}
    void qux() {}
  };

  {
    beyond::function_ref<auto()->void> fr = f;
    fr = [] {};
  }

  {
    beyond::function_ref<auto(b)->void> fr = &b::baz;
    fr = &b::qux;
  }
}

namespace {
bool f_called = false;
void f2()
{
  f_called = true;
}
struct b2 {
  bool baz_called = false;
  void baz()
  {
    baz_called = true;
  }
  bool qux_called = false;
  void qux()
  {
    qux_called = true;
  }
};
} // namespace

TEST_CASE("function_ref calls", "[beyond.core.function_ref]")
{
  {
    beyond::function_ref<void(void)> fr = f2;
    fr();
    REQUIRE(f_called);
  }

  {
    b2 o;
    auto x = &b2::baz;
    beyond::function_ref<void(b2&)> fr = x;
    fr(o);
    REQUIRE(o.baz_called);
    x = &b2::qux;
    fr = x;
    fr(o);
    REQUIRE(o.qux_called);
  }

  {
    auto x = [] { return 42; };
    beyond::function_ref<int()> fr = x;
    REQUIRE(fr() == 42);
  }

  {
    int i = 0;
    auto x = [&i] { i = 42; };
    beyond::function_ref<void()> fr = x;
    fr();
    REQUIRE(i == 42);
  }
}

static void foo() {}
TEST_CASE("function_ref Constructors", "[beyond.core.function_ref]")
{
  beyond::function_ref<auto()->void> fr1 = [] {};
  beyond::function_ref<auto()->void> fr2 = foo;

  fr1();
  fr2();
}

TEST_CASE("Implicit conversion from function that returns child to "
          "function_ref that returns a Parent",
          "[beyond.core.function_ref]")
{
  struct Fruit {
  };
  struct Apple : Fruit {
  };

  auto get_apple = []() -> Apple* { return nullptr; };
  auto bar = [get_apple]() -> beyond::function_ref<Fruit*()> {
    return get_apple;
  };

  REQUIRE(bar()() == nullptr);
}

void foo(const beyond::function_ref<int(const std::vector<int>)>& func)
{
  REQUIRE(func({12}) == 144);
}

static_assert(std::is_trivially_copyable_v<beyond::function_ref<void()>>);
