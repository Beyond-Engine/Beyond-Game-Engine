#pragma once

#ifndef BEYOND_CORE_UTILS_RESOURCE_HANLDE_HPP
#define BEYOND_CORE_UTILS_RESOURCE_HANLDE_HPP

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
 * @addtogroup handle
 * @{
 */

struct HandleBase {
};

/**
 * @brief Template for the base class of a resource handle
 *
 * Resource handles act as none-owning references to a resource. It has
 * additional functionality to check for dangling.
 */
template <typename Resource, typename Storage, std::size_t index_bits,
          std::size_t generation_bits>
struct Handle : HandleBase {
public:
  static_assert(std::is_unsigned_v<Storage>,
                "The storage must an unsigned integer");
  static_assert(index_bits + generation_bits == 8 * sizeof(Storage));

  explicit constexpr Handle(Storage id = 0, Storage gen = 0)
      : data_{id + (gen << index_bits)}
  {
  }

  [[nodiscard]] auto index() const -> Storage
  {
    static constexpr auto index_mask = ~(~0 >> index_bits << index_bits);
    return data_ & index_mask;
  }

  [[nodiscard]] auto generation() const -> Storage
  {
    return data_ >> index_bits;
  }

  [[nodiscard]] friend constexpr auto operator==(Handle lhs, Handle rhs) -> bool
  {
    return lhs.data_ == rhs.data_;
  }

  [[nodiscard]] friend constexpr auto operator!=(Handle lhs, Handle rhs) -> bool
  {
    return lhs.data_ != rhs.data_;
  }

private:
  Storage data_;
};

/** @} @} */

} // namespace beyond

#endif // BEYOND_CORE_UTILS_RESOURCE_HANLDE_HPP
