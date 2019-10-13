#include "beyond/core/ecs/sparse_set.hpp"
#include <catch2/catch.hpp>

using namespace beyond;

struct Entity : Handle<Entity, std::uint32_t, 24, 8> {
  using Handle::Handle;
};

TEST_CASE("SparseSet", "[beyond.core.ecs.sparse_set]")
{
  GIVEN("An empty sparse set")
  {
    SparseSet<Entity> ss;
    REQUIRE(ss.empty());
    REQUIRE(ss.size() == 0);
    REQUIRE(ss.capacity() == 0);

    WHEN("Reserve space of 16 elements")
    {
      ss.reserve(16);
      REQUIRE(ss.empty());
      REQUIRE(ss.capacity() >= 16);
    }

    AND_GIVEN("An entity")
    {
      const Entity entity{42};

      WHEN("Insert that entity to the sparse set")
      {
        ss.insert(entity);

        THEN("You cannot find 0 in the sparse set")
        {
          REQUIRE(!ss.contains(Entity{0}));
        }

        THEN("You can find this entity in the sparse set")
        {
          REQUIRE(ss.contains(entity));
          const auto index = ss.index_of(entity);
          REQUIRE(index == 0);
          REQUIRE(ss.entities()[index] == entity);
        }

        AND_WHEN("Delete that entity in the sparse set")
        {
          ss.erase(entity);
          THEN("The sparse set will no longer contain that entity")
          {
            REQUIRE(ss.size() == 0);
            REQUIRE(!ss.contains(entity));
          }

          AND_WHEN("Reconstructs that entity in the sparse set")
          {
            ss.insert(entity);

            THEN("You can find this entity in the sparse set")
            {
              REQUIRE(ss.contains(entity));
              REQUIRE(ss.entities()[ss.index_of(entity)] == entity);
            }
          }
        }

        AND_GIVEN("begin() and end() iterators of the "
                  "sparse set")
        {
          auto begin = ss.begin();
          auto end = ss.end();
          REQUIRE(begin == ss.cbegin());
          REQUIRE(end == ss.cend());

          REQUIRE(!(begin == end));
          REQUIRE(begin != end);
          REQUIRE(begin < end);
          REQUIRE(begin <= end);
          REQUIRE(end > begin);
          REQUIRE(end >= begin);

          AND_THEN("begin() points to the only entity in the sparse set")
          {
            REQUIRE(*ss.begin() == entity);
          }
          AND_THEN("++begin = end")
          {
            REQUIRE(++begin == end);
          }
          AND_THEN("--end == begin")
          {
            REQUIRE(--end == begin);
          }
          AND_WHEN("begin += 1")
          {
            begin += 1;
            AND_THEN("begin == end")
            {
              REQUIRE(begin == end);
            }
          }
          AND_WHEN("end -= 1")
          {
            end -= 1;
            AND_THEN("begin == end")
            {
              REQUIRE(begin == end);
            }
          }
          AND_THEN("begin + 1 == end")
          {
            REQUIRE(begin + 1 == end);
            REQUIRE(1 + begin == end);
          }
          AND_THEN("end - 1 == begin")
          {
            REQUIRE(end - 1 == begin);
          }
          AND_THEN("end - begin == 1")
          {
            REQUIRE(end - begin == 1);
          }
        }
      }
    }
  }
}
