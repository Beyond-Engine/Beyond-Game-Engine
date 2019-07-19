#include "beyond/core/ecs/entity.hpp"
#include <catch2/catch.hpp>

using beyond::Entity;

TEST_CASE("Entity Test", "[beyond.core.ecs.entity]")
{
  STATIC_REQUIRE(sizeof(Entity) == 4); // The size of an entity handle is 4 byte

  constexpr Entity e1 = 0x42000011;
  STATIC_REQUIRE(beyond::entity_id(e1) == 0x11);
  STATIC_REQUIRE(beyond::entity_version(e1) >> beyond::entity_shift == 0x420);
}
