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
    entities_set_.reserve(size);
  }

  auto insert(Entity entity, ValueType data) -> void
  {
    entities_set_.insert(entity);
    data_.push_back(std::move(data));
  }

  auto erase(Entity entity) -> void
  {
    const auto index = entities_set_.get(entity);
    entities_set_.erase(entity);
    data_[index] = data_.back();
    data_.pop_back();
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

  [[nodiscard]] auto data() const noexcept -> const ValueType*
  {
    return data_.data();
  }

private:
  SparseSet<Entity> entities_set_;
  std::vector<ValueType> data_;
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
    const float data2 = 2.16f;

    WHEN("Insert that entity to the sparse map")
    {
      sm.insert(entity, data);

      THEN("You can find this entity and its data in the sparse map")
      {
        REQUIRE(sm.contains(entity));
        const auto index = sm.get(entity);
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
            const auto index = sm.get(entity);
            REQUIRE(sm.entities()[index] == entity);
            REQUIRE(sm.data()[index] == Approx(data2));
          }
        }
      }
    }
  }
}
