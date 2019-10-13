#ifndef BEYOND_CORE_ECS_SPARSE_SET_HPP
#define BEYOND_CORE_ECS_SPARSE_SET_HPP

#include <array>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <beyond/core/utils/assert.hpp>
#include <beyond/core/utils/crtp.hpp>
#include <beyond/core/utils/handle.hpp>

/**
 * @file sparse_set.hpp
 * @brief Provides the SparseSet class
 *
 * Sparse sets are the internal data structure of the entity-component-system to
 * store entities
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

template <typename Derived>
class SparseSetBase : public CRTP<Derived, SparseSetBase> {
public:
  SparseSetBase() = default;

  // It is impossible to have a sparse set bigger than the total number of
  // entities
};

/**
 * @brief SparseSet stores entities
 */

namespace detail {
constexpr std::size_t page_shift = 12;

}

template <typename Handle>
class SparseSet : public SparseSetBase<SparseSet<Handle>> {
public:
  using SparseSetBase<SparseSet<Handle>>::SparseSetBase;
  using SizeType = typename Handle::Index;
  using DiffType = typename Handle::DiffType;

private:
  static constexpr SizeType page_count =
      1u << (Handle::shift - detail::page_shift);
  static constexpr SizeType page_size =
      1u << detail::page_shift; // Handles per page
  using Page = std::array<std::optional<SizeType>, page_size>;

  static_assert(std::is_base_of_v<beyond::HandleBase, Handle>);
  static_assert(
      Handle::shift > detail::page_shift,
      "The maximum indices of a handle should be larger than page size");

  using Iterator = const Handle*;

public:
  SparseSet() noexcept = default;

  /// @brief Returns true if the SparseSet does not contains any elements
  [[nodiscard]] auto empty() const noexcept -> bool
  {
    return direct_.empty();
  }

  /// @brief Gets how many entities are stored in the sparse set
  [[nodiscard]] auto size() const noexcept -> SizeType
  {
    return static_cast<SizeType>(direct_.size());
  }

  /// @brief Gets the capacity of the sparse set
  [[nodiscard]] auto capacity() const noexcept -> SizeType
  {
    return static_cast<SizeType>(direct_.capacity());
  }

  /// @brief Reserves the capacity of the sparse set to `capacity`
  auto reserve(std::size_t capacity) -> void
  {
    direct_.reserve(capacity);
  }

  /**
   * @brief Inserts a handle to the sparse set
   *
   * @warning
   * Attempting to assign a handle that already belongs to the sparse set
   * results in undefined behavior.
   *
   * @param handle A valid handle
   */
  auto insert(Handle handle) -> void
  {
    BEYOND_ASSERT(!contains(handle));
    const auto [page, offset] = page_index_of(handle);
    if (reverse_[page] == nullptr) {
      reverse_[page] = std::make_unique<Page>();
    }
    (*reverse_[page])[offset] = static_cast<SizeType>(direct_.size());
    direct_.push_back(handle);
  }

  /**
   * @brief Removes a handle from the sparse set
   *
   * @warning Attempting to erase an handle that is not in the sparse set leads
   * to undefined behavior.
   *
   * @param handle A valid handle identifier.
   */
  auto erase(Handle handle) -> void
  {
    BEYOND_ASSERT(contains(handle));
    // Swaps the to be delete alement with the last element
    const auto [from_page, from_offset] = page_index_of(handle);
    const auto [to_page, to_offset] = page_index_of(direct_.back());

    const auto handle_from_index = *(*reverse_[from_page])[from_offset];
    BEYOND_ASSERT(direct_[handle_from_index] == handle);

    (*reverse_[from_page])[from_offset] = std::nullopt;
    *(*reverse_[to_page])[to_offset] = handle_from_index;

    direct_[handle_from_index] = direct_.back();
    direct_.pop_back();
  }

  /**
   * @brief Gets the position of an handle in a sparse set.
   *
   * @warning Attempting to get the index of an handle that is not in the
   * sparse set leads to undefined behavior.
   *
   * @param handle A valid handle.
   *
   * @return The position of handle in the sparse set
   */
  [[nodiscard]] auto index_of(Handle handle) const noexcept -> SizeType
  {
    BEYOND_ASSERT(contains(handle));
    const auto [page, offset] = page_index_of(handle);
    return *(*reverse_[page])[offset];
  }

  /**
   * @brief Checks if the sparse set contains a handle
   * @param handle A valid handle
   * @return true if the sparse set contains the given handle, false otherwise
   */
  [[nodiscard]] auto contains(Handle handle) const noexcept -> bool
  {
    const auto [page, offset] = page_index_of(handle);
    if (reverse_[page]) {
      return (*reverse_[page])[offset] != std::nullopt;
    }
    return false;
  }

  /**
   * @brief Returns a raw pointer to the underlying entities array
   */
  [[nodiscard]] auto entities() const noexcept -> const Handle*
  {
    return direct_.data();
  }

  /// @brief Gets the iterator to the beginning of the sparse set
  /// @return An iterator to the first handle
  [[nodiscard]] auto begin() const noexcept -> Iterator
  {
    return Iterator{direct_.data()};
  }

  /// @brief Gets an iterator to the end of sparse set
  /// @return An iterator to the handle following the last handle
  [[nodiscard]] auto end() const noexcept -> Iterator
  {
    return Iterator{direct_.data() + direct_.size()};
  }

  /// @copydoc begin
  [[nodiscard]] auto cbegin() const noexcept -> Iterator
  {
    return Iterator{direct_.data()};
  }

  /// @copydoc end
  [[nodiscard]] auto cend() const noexcept -> Iterator
  {
    return Iterator{direct_.data() + direct_.size()};
  }

private:
  std::array<std::unique_ptr<Page>, page_count> reverse_;
  std::vector<Handle> direct_; // The packed array of entities

  // Given an handle, get its location inside the reverse array
  [[nodiscard]] auto page_index_of(Handle handle) const noexcept
  {
    const auto index = handle.index();
    const SizeType page = index / page_size;
    const SizeType offset = index & (page_size - 1);
    return std::make_pair(page, offset);
  }
};

/** @}
 *  @} */

} // namespace beyond

#endif // BEYOND_CORE_ECS_SPARSE_SET_HPP
