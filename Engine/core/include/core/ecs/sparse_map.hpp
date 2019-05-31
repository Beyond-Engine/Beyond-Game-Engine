#ifndef BEYOND_CORE_SPARSE_MAP_HPP
#define BEYOND_CORE_SPARSE_MAP_HPP

#include <unordered_map>
#include <vector>

#include "core/ecs/sparse_set.hpp"

/**
 * @file sparse_map.hpp
 * @brief Provides the SparseMap class
 *
 * Sparse maps are the internal data structure of the entity-component-system to
 * store entities and their associated data.
 * @ingroup ecs
 */

namespace beyond {

/**
 * @addtogroup core
 * @{
 */

/**
 * @defgroup ecs Entity Component System
 * @ingroup core
 *
 * @{
 */

/**
 * @brief Implements data storage for entities.
 *
 * This class builds on SparseSet and associates data to an entity. It is used
 * for storing components of entities in an EntityRegistry.
 *
 * @tparam Entity A valid entity handle
 * @tparam T The type of data to store in this SparseMap
 */
template <typename Entity, typename T> class SparseMap {
public:
  using Trait = EntityTrait<Entity>;
  using SizeType = typename Trait::Id;
  using DiffType = typename Trait::DiffType;
  using ValueType = T;

  SparseMap() noexcept = default;

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

  class Iterator {
  public:
    [[nodiscard]] auto operator!=(const Iterator& other) const noexcept -> bool
    {
      return (map_ != other.map_) || (index_ != other.index_);
    }

    [[nodiscard]] auto operator*() const noexcept
        -> std::pair<Entity, ValueType&>
    {
      return std::make_pair(map_->entities_set_.entities()[index_],
                            std::ref(map_->data_[index_]));
    }

    [[nodiscard]] auto operator-> () const noexcept
        -> const std::pair<Entity, ValueType&>*
    {
      auto pair = std::make_pair(map_->entities_set_.entities()[index_],
                                 map_->data_[index_]);
      return &pair;
    }

  private:
    SparseMap* map_;
    std::size_t index_;

    friend SparseMap;

    constexpr Iterator(SparseMap* map, std::size_t index)
        : map_{map}, index_{index}
    {
    }
  };

  [[nodiscard]] auto begin() noexcept -> Iterator
  {
    return {this, 0};
  }

  [[nodiscard]] auto end() noexcept -> Iterator
  {
    return {this, data_.size()};
  }

private:
  SparseSet<Entity> entities_set_;
  std::vector<ValueType> data_;
};

/** @}
 *  @} */

} // namespace beyond

#endif // BEYOND_CORE_SPARSE_MAP_HPP
