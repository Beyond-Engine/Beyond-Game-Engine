#include "core/ecs/sparse_set.hpp"

namespace beyond {

template <typename Entity, typename T> class SparseMap {
public:
  using Trait = EntityTrait<Entity>;
  using SizeType = typename Trait::Id;
  using DiffType = typename Trait::DiffType;
  using ValueType = T;

  SparseMap() = default;

  /// @brief Returns true if the sparse map is empty
  [[nodiscard]] auto empty() const noexcept -> bool
  {
    return entities_set_.empty();
  }

  /// @brief Gets how many components are stored in the sparse map
  [[nodiscard]] auto size() const noexcept -> SizeType
  {
    return entities_set_.size();
  }

  /// @brief Gets the capacity of the sparse map
  [[nodiscard]] auto capacity() const noexcept -> SizeType
  {
    return entities_set_.capacity();
  }

  /// @brief Reserves the capacity of the sparse map to `capacity`
  auto reserve(SizeType capacity) -> void
  {
    entities_set_.reserve(capacity);
  }

  /**
   * @brief Inserts an entity and its corresponding data to the sparse map
   *
   * @warning Attempting to insert an entity that is already in the sparse map
   * leads to undefined behavior.
   *
   * @param entity A valid entity identifier.
   * @param data The data attaches to an entity
   */
  auto insert(Entity entity, ValueType data) -> void
  {
    entities_set_.insert(entity);
    data_.push_back(std::move(data));
  }

  /**
   * @brief Removes an entity from the sparse map and destroys its corresponding
   * data
   *
   * @warning Attempting to erase an entity that is not in the sparse map leads
   * to undefined behavior.
   *
   * @param entity A valid entity identifier.
   */
  auto erase(Entity entity) -> void
  {
    std::swap(data_[entities_set_.index_of(entity)], data_.back());
    data_.pop_back();
    entities_set_.erase(entity);
  }

  /**
   * @brief Checks if the sparse map contains an entity
   * @param entity A valid entity identifier.
   * @return true if the sparse map contains the given antity, false otherwise
   */
  [[nodiscard]] auto contains(Entity entity) const noexcept -> bool
  {
    return entities_set_.contains(entity);
  }

  /**
   * @brief Gets the index of an entity in a sparse map.
   *
   * @warning Attempting to get the index of an entity that is not in the
   * sparse map leads to undefined behavior.
   *
   * @param entity A valid entity identifier.
   *
   * @return The position of entity in the sparse map
   */
  [[nodiscard]] auto index_of(Entity entity) const noexcept -> SizeType
  {
    return entities_set_.index_of(entity);
  }

  /**
   * @brief Returns the data associated with an entity.
   *
   * @warning Attempting to use an entity that is not in the
   * sparse map leads to undefined behavior.
   *
   * @param entity A valid entity identifier
   *
   * @return The data associated with an entity
   */
  [[nodiscard]] auto get(Entity entity) const noexcept -> const ValueType&
  {
    return data_[entities_set_.index_of(entity)];
  }

  /// @overload
  [[nodiscard]] auto get(Entity entity) noexcept -> ValueType&
  {
    return data_[entities_set_.index_of(entity)];
  }

  /**
   * @brief Trys to get the data associated with an entity.
   *
   * @param entity An entity identifier
   *
   * @return A pointer to the data associated with an entity if the entity is in
   * the sparse map, nullptr otherwise
   */
  [[nodiscard]] auto try_get(Entity entity) const noexcept -> const ValueType*
  {
    return (entities_set_.contains(entity))
               ? &data_[entities_set_.index_of(entity)]
               : nullptr;
  }

  /// @overload
  [[nodiscard]] auto try_get(Entity entity) noexcept -> ValueType*
  {
    return (entities_set_.contains(entity))
               ? &data_[entities_set_.index_of(entity)]
               : nullptr;
  }

  /// @brief Direct accesses to the array of entites.
  [[nodiscard]] auto entities() const noexcept -> const Entity*
  {
    return entities_set_.entities();
  }

  /// @brief Direct accesses to the array of data.
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
