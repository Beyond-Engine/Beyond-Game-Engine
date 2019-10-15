#include <catch2/catch.hpp>

#include <vector>

#include <beyond/core/utils/handle.hpp>

using namespace beyond;

namespace {

struct DummyHandle : Handle<DummyHandle, std::uint32_t, 12, 20> {
  using Handle::Handle;
};

} // anonymous namespace

TEST_CASE("Resource handle", "[beyond.core.util.handle]")
{
  STATIC_REQUIRE(sizeof(DummyHandle) == sizeof(std::uint32_t));

  const DummyHandle hd1;
  REQUIRE(hd1.index() == 0);
  REQUIRE(hd1.generation() == 0);

  const DummyHandle hd2{10};
  REQUIRE(hd2.index() == 10);
  REQUIRE(hd2.generation() == 0);

  const DummyHandle hd3{10, 10};
  REQUIRE(hd3.index() == 10);
  REQUIRE(hd3.generation() == 10);

  REQUIRE(hd1 == hd1);
  REQUIRE(hd1 != hd2);
}

TEST_CASE("Handle index overflow test", "[beyond.core.util.handle]") {}
