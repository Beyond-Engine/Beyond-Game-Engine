#include "core/ecs/sparse_set.hpp"

namespace beyond {

template <typename Entity, typename T> class SparseMap {
public:
  using Trait = EntityTrait<Entity>;
  using SizeType = typename Trait::Id;
  using DiffType = typename Trait::DiffType;
  using ValueType = T;

  [[nodiscard]] auto empty() const noexcept -> bool
  {
    return entities_set_.empty();
  }

  [[nodiscard]] auto size() const noexcept -> SizeType
  {
    return entities_set_.size();
  }

  [[nodiscard]] auto capacity() const noexcept -> SizeType
  {
    return entities_set_.capacity();
  }

  auto reserve(SizeType size) -> void
  {
    return entities_set_.reserve(size);
  }

  auto insert(Entity entity, ValueType /*data*/) -> void
  {
    return entities_set_.insert(entity);
  }

  [[nodiscard]] auto contains(Entity entity) const noexcept -> bool
  {
    return entities_set_.contains(entity);
  }

  [[nodiscard]] auto get(Entity entity) const noexcept -> SizeType
  {
    return entities_set_.get(entity);
  }

  [[nodiscard]] auto entities() const noexcept -> const Entity*
  {
    return entities_set_.entities();
  }

private:
  SparseSet<Entity> entities_set_;
};

} // namespace beyond

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

  GIVEN("An entity and its corresponding data")
  {

    const Entity entity = 42;
    const float data = 3.14f;

    WHEN("Insert that entity to the sparse map")
    {
      sm.insert(entity, data);

      THEN("You can find this entity in the sparse set")
      {
        REQUIRE(sm.contains(entity));
        const auto index = sm.get(entity);
        REQUIRE(index == 0);
        REQUIRE(sm.entities()[index] == entity);
      }
    }
  }
}
