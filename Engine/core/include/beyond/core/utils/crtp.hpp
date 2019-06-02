#ifndef BEYOND_CORE_UTILS_CRTP_HPP
#define BEYOND_CORE_UTILS_CRTP_HPP

/**
 * @file crtp.hpp
 * @brief Provides helper class for curious recurrence template pattern
 * @ingroup util
 */

namespace beyond {

/**
 * @addtogroup core
 * @{
 * @addtogroup util
 * @{
 */

/**
 * @brief A helper for the curiously recurring template pattern
 *
 * CRTP helps to define CRTP base classes. For example: This class support
 * multiple CRTP base classes. For example:
 *
 * @code
 * template <typename Derived>
 * class Base1 {
 *   ...
 * }
 *
 * class Derived : CRTP<Derived, Base1>, CRTP<Derived, Base2> {
 *   ...
 * }
 * @endcode
 *
 * The implementation of this class comes from Jonathan Boccara's
 * <a href="https://www.fluentcpp.com/2017/05/19/crtp-helper/">Removing
 * Duplicates in C++ CRTP Base Classes</a>
 */
template <typename Derived, template <typename> class Base> struct CRTP {
  [[nodiscard]] constexpr auto underlying() noexcept -> Derived&
  {
    return static_cast<Derived&>(*this);
  }

  [[nodiscard]] constexpr auto underlying() const noexcept -> const Derived&
  {
    return static_cast<const Derived&>(*this);
  }

private:
  constexpr CRTP() noexcept = default;
  friend Base<Derived>;
};

/** @} @} */

} // namespace beyond

#endif // BEYOND_CORE_UTILS_CRTP_HPP
