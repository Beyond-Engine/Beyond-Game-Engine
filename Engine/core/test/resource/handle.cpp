#include <catch2/catch.hpp>

#include <vector>

#include <beyond/core/resource/handle.hpp>

namespace beyond {

struct PointHandle : Handle<PointHandle, std::uint32_t, 12, 20> {
  using Handle::Handle;
};

} // namespace beyond

using namespace beyond;

TEST_CASE("Resource handle", "[resource]")
{
  STATIC_REQUIRE(sizeof(PointHandle) == sizeof(std::uint32_t));

  const PointHandle pt1;
  REQUIRE(pt1.index() == 0);
  REQUIRE(pt1.generation() == 0);

  const PointHandle pt2{10};
  REQUIRE(pt2.index() == 10);
  REQUIRE(pt2.generation() == 0);

  const PointHandle pt3{10, 10};
  REQUIRE(pt3.index() == 10);
  REQUIRE(pt3.generation() == 10);

  REQUIRE(pt1 == pt1);
  REQUIRE(pt1 != pt2);
}
