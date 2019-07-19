#ifndef BEYOND_CORE_ECS_ENTITY_HPP
#define BEYOND_CORE_ECS_ENTITY_HPP

/**
 * @file entity.hpp
 * @brief Provides Definition and operation of a Entity handle
 *
 * @ingroup ecs
 */

#include <cstdint>

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
 * @brief Type of an entity handle
 */
using Entity = std::uint32_t;

constexpr auto entity_shift = 20u;
constexpr Entity entity_mask = 0xFFFFF;

/// @brief Get the ID part of an entity handle
constexpr auto entity_id(Entity e) noexcept -> Entity
{
  return e & entity_mask;
}

/// @brief Get the version part of the entity handle, unshifted
constexpr auto entity_version(Entity e) noexcept -> Entity
{
  return e & ~entity_mask;
}

/** @}
 *  @} */

}

#endif // BEYOND_CORE_ECS_ENTITY_HPP
