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
 * @brief Template for the base class of a resource handle
 *
 * Resource handles act as none-owning references to a resource. It has
 * additional functionality to check for dangling.
 */
// clang-format off
template <typename Derived,
          typename StorageT,
          typename IndexT, std::size_t _index_bits,
          typename GenerationT, std::size_t _generation_bits>
// clang-format on
struct Handle : HandleBase {
public:
  using Storage = StorageT;
  using storage_type = StorageT;

  using Index = IndexT;
  using index_type = IndexT;
  static constexpr std::size_t index_bits = _index_bits;

  using Generation = GenerationT;
  using generation_type = GenerationT;
  static constexpr std::size_t generation_bits = _generation_bits;

  using DiffType = std::make_signed_t<Index>;

  static_assert(std::is_unsigned_v<Storage>,
                "The storage must an unsigned integer");
  static_assert(std::is_unsigned_v<Index>,
                "The storage must an unsigned integer");
  static_assert(std::is_unsigned_v<Generation>,
                "The storage must an unsigned integer");
  static_assert(index_bits <= 8 * sizeof(Index));
  static_assert(generation_bits <= 8 * sizeof(Generation));
  static_assert(index_bits + generation_bits == 8 * sizeof(Storage));

  explicit constexpr Handle(Index id = 0, Generation gen = 0)
      : index{id}, generation{gen}
  {
  }

  [[nodiscard]] friend constexpr auto operator==(Derived lhs, Derived rhs)
      -> bool
  {
    return lhs.index == rhs.index && lhs.generation == rhs.generation;
  }

  [[nodiscard]] friend constexpr auto operator!=(Derived lhs, Derived rhs)
      -> bool
  {
    return lhs.index != rhs.index || lhs.generation != rhs.generation;
  }

  Index index : index_bits;
  Generation generation : generation_bits;
};

/** @} @} */

} // namespace beyond

#endif // BEYOND_CORE_UTILS_HANLDE_HPP
