#include "core/ecs/sparse_map.hpp"

#include <catch2/catch.hpp>

using namespace beyond;

TEST_CASE("SparseMap", "[beyond.core.ecs.sparse_set]")
{
  using Entity = std::uint32_t;
  SparseMap<Entity, float> sm;
  REQUIRE(sm.empty());
  REQUIRE(sm.size() == 0);
  REQUIRE(sm.capacity() == 0);

  WHEN("Reserve space of 16 elements")
  {
    sm.reserve(16);
    REQUIRE(sm.empty());
    REQUIRE(sm.capacity() >= 16);
  }

  GIVEN("An entity 42 and its corresponding data")
  {

    const Entity entity = 42;
    const float data = 3.14f;
    const float data2 = 2.16f;

    WHEN("Insert that entity to the sparse map")
    {
      sm.insert(entity, data);

      THEN("You cannot find entity 0 in the sparse map")
      {
        REQUIRE(!sm.try_get(0));
      }

      THEN("You can find this entity and its data in the sparse map")
      {
        REQUIRE(sm.contains(entity));
        REQUIRE(sm.try_get(entity));
        REQUIRE(*sm.try_get(entity) == Approx(data));
        REQUIRE(sm.get(entity) == Approx(data));

        AND_THEN("You can modify the data attached to this entity")
        {
          sm.get(entity) = 4;
          REQUIRE(sm.get(entity) == Approx(4));
          *sm.try_get(entity) = 2;
          REQUIRE(sm.get(entity) == Approx(2));
        }
      }

      THEN("You can find this only entity has index 0")
      {
        const auto index = sm.index_of(entity);
        REQUIRE(index == 0);
        REQUIRE(sm.entities()[index] == entity);
        REQUIRE(sm.data()[index] == Approx(data));
      }

      AND_WHEN("Delete that entity in the sparse map")
      {
        sm.erase(entity);
        THEN("The sparse map will no longer contain that entity")
        {
          REQUIRE(sm.size() == 0);
          REQUIRE(!sm.contains(entity));
        }

        AND_WHEN(
            "Reconstructs that entity in the sparse map with different data")
        {
          sm.insert(entity, data2);

          THEN("You can find this entity in the sparse map")
          {
            REQUIRE(sm.contains(entity));
            REQUIRE(sm.get(entity) == Approx(data2));
          }
        }
      }
    }
  }
}
