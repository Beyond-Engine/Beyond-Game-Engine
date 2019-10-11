#ifndef BEYOND_CORE_ECS_SPARSE_MAP_HPP
#define BEYOND_CORE_ECS_SPARSE_MAP_HPP

#include <iterator>
#include <vector>

#include "beyond/core/ecs/sparse_set.hpp"
#include "beyond/core/utils/arrow_proxy.hpp"

/**
 * @file sparse_map.hpp
 * @brief Provides the SparseMap class
 *
 * Sparse maps are the internal data structure of the entity-component-system
 to
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
template <typename Handle, typename T> class SparseMap {
public:
  using SizeType = typename Handle::Index;
  using DiffType = typename Handle::DiffType;
  using MappedType = T;

  SparseMap() noexcept = default;

  /// @brief Returns true if the sparse map is empty
  [[nodiscard]] auto empty() const noexcept -> bool
  {
    return handles_.empty();
  }

  /// @brief Gets how many components are stored in the sparse map
  [[nodiscard]] auto size() const noexcept -> SizeType
  {
    return handles_.size();
  }

  /// @brief Gets the capacity of the sparse map
  [[nodiscard]] auto capacity() const noexcept -> SizeType
  {
    return handles_.capacity();
  }

  /// @brief Reserves the capacity of the sparse map to `capacity`
  auto reserve(SizeType capacity) -> void
  {
    handles_.reserve(capacity);
  }

  /**
   * @brief Inserts an entity and its corresponding data to the sparse map
   *
   * @warning Attempting to insert an entity that is already in the sparse map
   * leads to undefined behavior.
   *
   * @param handle A valid handle.
   * @param data The data attaches to an entity
   */
  auto insert(Handle handle, MappedType data) -> void
  {
    handles_.insert(handle);
    data_.push_back(std::move(data));
  }

  /**
   * @brief Removes an entity from the sparse map and destroys its
   corresponding
   * data
   *
   * @warning Attempting to erase an entity that is not in the sparse map
   leads
   * to undefined behavior.
   *
   * @param handle A valid handle.
   */
  auto erase(Handle handle) -> void
  {
    std::swap(data_[handles_.index_of(handle)], data_.back());
    data_.pop_back();
    handles_.erase(handle);
  }

  /**
   * @brief Checks if the sparse map contains an entity
   * @param handle A valid handle.
   * @return true if the sparse map contains the given antity, false otherwise
   */
  [[nodiscard]] auto contains(Handle handle) const noexcept -> bool
  {
    return handles_.contains(handle);
  }

  /**
   * @brief Gets the index of an entity in a sparse map.
   *
   * @warning Attempting to get the index of an entity that is not in the
   * sparse map leads to undefined behavior.
   *
   * @param handle A valid handle.
   *
   * @return The position of entity in the sparse map
   */
  [[nodiscard]] auto index_of(Handle handle) const noexcept -> SizeType
  {
    return handles_.index_of(handle);
  }

  /**
   * @brief Returns the data associated with an entity.
   *
   * @warning Attempting to use an entity that is not in the
   * sparse map leads to undefined behavior.
   *
   * @param handle A valid handle.
   *
   * @return The data associated with an entity
   */
  [[nodiscard]] auto get(Handle handle) const noexcept -> const MappedType&
  {
    return data_[handles_.index_of(handle)];
  }

  /// @overload
  [[nodiscard]] auto get(Handle handle) noexcept -> MappedType&
  {
    return data_[handles_.index_of(handle)];
  }

  /**
   * @brief Trys to get the data associated with an entity.
   *
   * @param handle A valid handle.
   *
   * @return A pointer to the data associated with an entity if the entity is
   in
   * the sparse map, nullptr otherwise
   */
  [[nodiscard]] auto try_get(Handle handle) const noexcept -> const MappedType*
  {
    return (handles_.contains(handle)) ? &data_[handles_.index_of(handle)]
                                       : nullptr;
  }

  /// @overload
  [[nodiscard]] auto try_get(Handle handle) noexcept -> MappedType*
  {
    return (handles_.contains(handle)) ? &data_[handles_.index_of(handle)]
                                       : nullptr;
  }

  /// @brief Direct accesses to the array of entites.
  [[nodiscard]] auto entities() const noexcept -> const Handle*
  {
    return handles_.entities();
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
        std::conditional_t<is_const, std::pair<Handle, const MappedType&>,
                           std::pair<Handle, MappedType&>>;
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
      return std::make_pair(map_->handles_.entities()[index_],
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
  SparseSet<Handle> handles_;
  std::vector<MappedType> data_;
};

/** @}
 *  @} */

} // namespace beyond

#endif // BEYOND_CORE_ECS_SPARSE_MAP_HPP
