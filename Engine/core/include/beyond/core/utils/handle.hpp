#pragma once

#ifndef BEYOND_CORE_UTILS_HANLDE_HPP
#define BEYOND_CORE_UTILS_HANLDE_HPP

#include <cstdint>
#include <type_traits>

/**
 * @file handle.hpp
 * @brief Provides helper class for `operator->` for proxy iterators of
 * container
 * @ingroup util
 */

namespace beyond {

/**
 * @addtogroup core
 * @{
 * @addtogroup ecs
 * @{
 */

struct HandleBase {
};

/**
 * @brief Template for the base class of a versioned resource handle
 *
 * Handles act as none-owning references to a resource. It has
 * additional functionality of storing a generation number to check collision.
 */
template <typename Derived, typename StorageT, std::size_t index_bits,
          std::size_t generation_bits>
struct Handle : HandleBase {
public:
  using Storage = StorageT;
  using Index = Storage;
  using Generation = Storage;
  using DiffType = std::make_signed_t<Index>;

  /// @brief The shift of index bits
  static constexpr std::size_t shift = index_bits;
  static constexpr StorageT index_mask = ~(~Storage{0} >> shift << shift);

  static_assert(std::is_unsigned_v<Storage>,
                "The storage must an unsigned integer");
  static_assert(index_bits + generation_bits == 8 * sizeof(Storage));

  explicit constexpr Handle(Storage id = 0, Storage gen = 0)
      : data_{id + (gen << shift)}
  {
  }

  /// @brief Return true if the index overflows the index range
  [[nodiscard]] static constexpr auto is_overflow(Storage index) -> bool
  {
    return (index >> shift) != 0;
  }

  [[nodiscard]] auto index() const -> Storage
  {
    return data_ & index_mask;
  }

  [[nodiscard]] auto generation() const -> Storage
  {
    return data_ >> shift;
  }

  [[nodiscard]] friend constexpr auto operator==(Derived lhs, Derived rhs)
      -> bool
  {
    return lhs.data_ == rhs.data_;
  }

  [[nodiscard]] friend constexpr auto operator!=(Derived lhs, Derived rhs)
      -> bool
  {
    return lhs.data_ != rhs.data_;
  }

private:
  Storage data_;
};

/** @} @} */

} // namespace beyond

#endif // BEYOND_CORE_UTILS_HANLDE_HPP
