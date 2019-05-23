#include <unordered_map>
#include <vector>

#include "core/assert.hpp"

namespace beyond {

template <typename Entity> class SparseSet {
public:
  SparseSet() = default;

  /// @brief Returns true if the SparseSet does not contains any element
  [[nodiscard]] auto empty() const noexcept -> std::size_t
  {
    return direct_.empty();
  }

  /// @brief Gets how many components are stored in the slot map
  [[nodiscard]] auto size() const noexcept -> std::size_t
  {
    return direct_.size();
  }

  /// @brief Gets the capacity of the slot map
  [[nodiscard]] auto capacity() const noexcept -> std::size_t
  {
    return direct_.capacity();
  }

  /// @brief Reserves the capacity of the slot map to `capacity`
  auto reserve(std::size_t capacity) -> void
  {
    direct_.reserve(capacity);
  }

  /**
   * @brief Gets the position of an entity in a sparse set.
   *
   * @warning Attempting to get the index of an entity that are not in the
   * sparse set leads to undefined behavior.
   *
   * @param entity A valid entity identifier.
   *
   * @return The position of entity in the sparse set
   */
  [[nodiscard]] auto get(Entity entity) const noexcept -> std::size_t
  {
    BEYOND_ASSERT(contains(entity), "Attempting to get the index of an entity "
                                    "that are not in the sparse set");
    return reverse_.find(entity)->second;
  }

  /**
   * @brief Checks if the sparse set contains an entity
   * @param entity A valid entity identifier.
   * @return true if the sparse set contains the given antity, false otherwise
   */
  [[nodiscard]] auto contains(Entity entity) const noexcept -> bool
  {
    return reverse_.find(entity) != reverse_.end();
  }

  /**
   * @brief Inserts an entity to the sparse set
   *
   * @warning
   * Attempting to assign an entity that already belongs to the sparse set
   * results in undefined behavior.
   *
   * @param entity A valid entity identifier.
   */
  auto insert(Entity entity) -> void
  {
    BEYOND_ASSERT(!contains(entity), "Attempting to assign an entity that "
                                     "already belongs to the sparse set");
    reverse_.emplace(entity, direct_.size());
    direct_.push_back(entity);
  }

private:
  std::unordered_map<Entity, std::size_t>
      reverse_;                // Map from an entity to its location
  std::vector<Entity> direct_; // The packed array of entities
};

} // namespace beyond

#include <catch2/catch.hpp>

using namespace beyond;

TEST_CASE("SparseSet Test", "[beyond.core.ecs.SparseSet]")
{
  using Entity = std::uint32_t;

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

    AND_GIVEN("An entity 3")
    {
      const Entity entity = 3;

      WHEN("Insert that entity to the sparse set")
      {
        ss.insert(entity);

        THEN("You can find this entity at the sparse set's first spot")
        {
          REQUIRE(ss.contains(entity));
          REQUIRE(ss.get(entity) == 0);
        }
      }
    }
  }
}
