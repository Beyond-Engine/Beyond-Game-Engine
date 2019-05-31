#ifndef BEYOND_CORE_SPARSE_MAP_HPP
#define BEYOND_CORE_SPARSE_MAP_HPP

#include <iterator>
#include <vector>

#include "core/arrow_proxy.hpp"
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
  using MappedType = T;

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
  auto insert(Entity entity, MappedType data) -> void
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
  [[nodiscard]] auto get(Entity entity) const noexcept -> const MappedType&
  {
    return data_[entities_set_.index_of(entity)];
  }

  /// @overload
  [[nodiscard]] auto get(Entity entity) noexcept -> MappedType&
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
  [[nodiscard]] auto try_get(Entity entity) const noexcept -> const MappedType*
  {
    return (entities_set_.contains(entity))
               ? &data_[entities_set_.index_of(entity)]
               : nullptr;
  }

  /// @overload
  [[nodiscard]] auto try_get(Entity entity) noexcept -> MappedType*
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
  [[nodiscard]] auto data() const noexcept -> const MappedType*
  {
    return data_.data();
  }

  template <bool is_const = false> class I {
  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type =
        std::conditional_t<is_const, std::pair<Entity, const MappedType&>,
                           std::pair<Entity, MappedType&>>;
    using difference_type = std::ptrdiff_t;
    using reference = value_type;
    using pointer = ArrowProxy<reference>;

    [[nodiscard]] auto operator==(const I& other) const noexcept -> bool
    {
      return (map_ == other.map_) && (index_ == other.index_);
    }

    [[nodiscard]] auto operator!=(const I& other) const noexcept -> bool
    {
      return (map_ != other.map_) || (index_ != other.index_);
    }

    [[nodiscard]] auto operator*() const noexcept -> reference
    {
      return std::make_pair(map_->entities_set_.entities()[index_],
                            std::ref(map_->data_[index_]));
    }

    [[nodiscard]] auto operator-> () const noexcept -> pointer
    {
      return pointer{operator*()};
    }

    auto operator++() noexcept -> I&
    {
      ++index_;
      return *this;
    }

    auto operator--() noexcept -> I&
    {
      --index_;
      return *this;
    }

    auto operator+=(difference_type i) noexcept -> I&
    {
      index_ += i;
      return *this;
    }

    auto operator-=(difference_type i) noexcept -> I&
    {
      index_ -= i;
      return *this;
    }

    [[nodiscard]] friend auto operator+(const I& lhs, difference_type rhs) -> I
    {
      return I{lhs.map_, lhs.index_ + rhs};
    }

    [[nodiscard]] friend auto operator+(difference_type lhs, I rhs) -> I
    {
      return I{rhs.map_, rhs.index_ + lhs};
    }

    [[nodiscard]] friend auto operator-(const I& lhs, difference_type rhs) -> I
    {
      return I{lhs.map_, lhs.index_ - rhs};
    }

    [[nodiscard]] friend auto operator-(const I& lhs, const I& rhs)
        -> difference_type
    {
      BEYOND_ASSERT(lhs.map_ == rhs.map_);
      return lhs.index_ - rhs.index_;
    }

  private:
    using MapPtr = std::conditional_t<is_const, const SparseMap*, SparseMap*>;
    MapPtr map_;
    difference_type index_;

    friend SparseMap;

    constexpr I(MapPtr map, difference_type index) : map_{map}, index_{index} {}
  };

  /// @brief Non-constant Iterator of SparseSet
  using Iterator = I<false>;

  /// @brief Constant Iterator of SparseSet
  using ConstIterator = I<true>;

  /// @brief Gets the iterator to the beginning of the sparse map
  /// @return An iterator to the first entity
  [[nodiscard]] auto begin() const noexcept -> ConstIterator
  {
    return {this, 0};
  }

  /// @overload
  [[nodiscard]] auto begin() noexcept -> Iterator
  {
    return {this, 0};
  }

  /// @copydoc begin
  [[nodiscard]] auto cbegin() const noexcept -> ConstIterator
  {
    return {this, 0};
  }

  /// @brief Gets an iterator to the end of sparse set
  /// @return An iterator to the entity following the last entity
  [[nodiscard]] auto end() const noexcept -> ConstIterator
  {
    return {this,
            static_cast<typename ConstIterator::difference_type>(data_.size())};
  }

  /// @overload
  [[nodiscard]] auto end() noexcept -> Iterator
  {
    return {this,
            static_cast<typename Iterator::difference_type>(data_.size())};
  }

  /// @copydoc end
  [[nodiscard]] auto cend() const noexcept -> ConstIterator
  {
    return {this,
            static_cast<typename ConstIterator::difference_type>(data_.size())};
  }

private:
  SparseSet<Entity> entities_set_;
  std::vector<MappedType> data_;
};

/** @}
 *  @} */

} // namespace beyond

#endif // BEYOND_CORE_SPARSE_MAP_HPP
