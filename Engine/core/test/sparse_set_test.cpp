#include <array>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "core/assert.hpp"
#include "core/type_traits.hpp"

namespace beyond {

/// @brief Entity traits.
template <typename Entity> struct EntityTrait;

/**
 * @brief Entity traits for a 32 bits entity identifier.
 *
 * A 32 bits entity identifier guarantees:
 *
 * * 20 bits for the entity number (suitable for almost all the games).
 * * 12 bit for the version (resets in [0-4095]).
 */
template <> struct EntityTrait<std::uint32_t> {
  /// @brief Underlying entity type.
  using Entity = std::uint32_t;

  /// @brief Underlying entity id type without its version
  using Id = std::uint32_t;

  /// @brief Extent of the entity number within an identifier.
  static constexpr std::size_t entity_shift = 20;
  static constexpr std::uint32_t entity_mask = 0xFFFFF;
};

template <typename Entity> class SparseSet {
public:
  static_assert(beyond::is_complete<EntityTrait<Entity>>(),
                "The Entity class must has its corresponding EntityTrait");

  using Trait = EntityTrait<Entity>;
  // It is impossible to have a sparse set bigger than the total number of
  // entities
  using SizeType = typename Trait::Id;

private:
  static constexpr std::size_t page_shift = 12;
  static_assert(Trait::entity_shift > page_shift);
  static constexpr SizeType page_count = 1
                                         << (Trait::entity_shift - page_shift);
  static constexpr SizeType page_size = 1 << page_shift; // Entity per page
  using Page = std::array<std::optional<SizeType>, page_size>;

public:
  SparseSet() = default;

  /// @brief Returns true if the SparseSet does not contains any element
  [[nodiscard]] auto empty() const noexcept -> bool
  {
    return direct_.empty();
  }

  /// @brief Gets how many components are stored in the slot map
  [[nodiscard]] auto size() const noexcept -> SizeType
  {
    return static_cast<SizeType>(direct_.size());
  }

  /// @brief Gets the capacity of the slot map
  [[nodiscard]] auto capacity() const noexcept -> SizeType
  {
    return static_cast<SizeType>(direct_.capacity());
  }

  /// @brief Reserves the capacity of the slot map to `capacity`
  auto reserve(std::size_t capacity) -> void
  {
    direct_.reserve(capacity);
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
    BEYOND_ASSERT(!contains(entity));
    const auto [page, offset] = index_of(entity);
    if (reverse_[page] == nullptr) {
      reverse_[page] = std::make_unique<Page>();
    }
    (*reverse_[page])[offset] = static_cast<SizeType>(direct_.size());
    direct_.push_back(entity);
  }

  /**
   * @brief Removes an entity from the sparse set
   *
   * @warning Attempting to erase an entity that is not in the sparse set leads
   * to undefined behavior.
   *
   * @param entity A valid entity identifier.
   */
  auto erase(Entity entity) -> void
  {
    BEYOND_ASSERT(contains(entity));
    // Swaps the to be delete alement with the last element
    const auto [from_page, from_offset] = index_of(entity);
    const auto [to_page, to_offset] = index_of(direct_.back());

    const auto entity_from_index = *(*reverse_[from_page])[from_offset];
    BEYOND_ASSERT(direct_[entity_from_index] == entity);

    (*reverse_[from_page])[from_offset] = std::nullopt;
    *(*reverse_[to_page])[to_offset] = entity_from_index;

    direct_[entity_from_index] = direct_.back();
    direct_.pop_back();
  }

  /**
   * @brief Gets the position of an entity in a sparse set.
   *
   * @warning Attempting to get the index of an entity that is not in the
   * sparse set leads to undefined behavior.
   *
   * @param entity A valid entity identifier.
   *
   * @return The position of entity in the sparse set
   */
  [[nodiscard]] auto get(Entity entity) const noexcept -> SizeType
  {
    BEYOND_ASSERT(contains(entity));
    const auto [page, offset] = index_of(entity);
    return *(*reverse_[page])[offset];
  }

  /**
   * @brief Checks if the sparse set contains an entity
   * @param entity A valid entity identifier.
   * @return true if the sparse set contains the given antity, false otherwise
   */
  [[nodiscard]] auto contains(Entity entity) const noexcept -> bool
  {
    const auto [page, offset] = index_of(entity);
    if (reverse_[page]) {
      return (*reverse_[page])[offset] != std::nullopt;
    }
    return false;
  }

  /**
   * @brief Returns a raw pointer to the underlying entities array
   */
  [[nodiscard]] auto entities() const noexcept -> const Entity*
  {
    return direct_.data();
  }

  /** @brief Iterator of the sparse set
   * @note This Iterator cannot modify the underlying values of the sparse set,
   * and SparseSet only have this `const` version of iterators
   *
   * @tparam Entity A valid entity handle type
   */
  class Iterator {
  public:
    friend class SparseSet<Entity>;
    constexpr Iterator() noexcept = default;
    explicit constexpr Iterator(const Entity* entity) noexcept : entity_{entity}
    {
    }

    [[nodiscard]] constexpr auto operator==(const Iterator& other) const
        noexcept -> bool
    {
      return entity_ == other.entity_;
    }

    [[nodiscard]] constexpr auto operator!=(const Iterator& other) const
        noexcept -> bool
    {
      return !(*this == other);
    }

    [[nodiscard]] constexpr auto operator*() const noexcept -> Entity
    {
      return *entity_;
    }

    constexpr auto operator++() noexcept
    {
      ++entity_;
    }

  private:
    const Entity* entity_;
  };

  /// @brief Gets the iterator to the beginning of the sparse set
  /// @return An iterator to the first entity
  [[nodiscard]] auto begin() const noexcept -> Iterator
  {
    return Iterator{direct_.data()};
  }

  /// @brief Gets an iterator to the end of sparse set
  /// @return An iterator to the entity following the last entity
  [[nodiscard]] auto end() const noexcept -> Iterator
  {
    return Iterator{direct_.data() + direct_.size()};
  }

private:
  std::array<std::unique_ptr<Page>, page_count> reverse_;
  std::vector<Entity> direct_; // The packed array of entities

  // Given an entity, get its location inside the reverse array
  [[nodiscard]] auto index_of(Entity entity) const noexcept
  {
    const SizeType identifier =
        static_cast<SizeType>(entity & Trait::entity_mask);
    const SizeType page = identifier / page_size;
    const SizeType offset = identifier & (page_size - 1);
    return std::make_pair(page, offset);
  }
};

} // namespace beyond

#include <catch2/catch.hpp>

using namespace beyond;

TEST_CASE("SparseSet", "[beyond.core.ecs.sparse_set]")
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

    AND_GIVEN("An entity")
    {
      const Entity entity = 42;

      WHEN("Insert that entity to the sparse set")
      {
        ss.insert(entity);

        THEN("You cannot find 0 in the sparse set")
        {
          REQUIRE(!ss.contains(0));
        }

        THEN("You can find this entity in the sparse set")
        {
          REQUIRE(ss.contains(entity));
          const auto index = ss.get(entity);
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
              REQUIRE(ss.entities()[ss.get(entity)] == entity);
            }
          }
        }

        AND_GIVEN("begin() and end() iterators of the "
                  "sparse set")
        {
          auto begin = ss.begin();
          auto end = ss.end();
          REQUIRE(begin != end);
          AND_THEN("begin() points to the only entity in the sparse set")
          {
            REQUIRE(*ss.begin() == entity);
          }
          AND_WHEN("Do prefix increment of begin")
          {
            ++begin;
            AND_THEN("It should equal to end")
            {
              REQUIRE(begin == end);
            }
          }
        }
      }
    }
  }
}
