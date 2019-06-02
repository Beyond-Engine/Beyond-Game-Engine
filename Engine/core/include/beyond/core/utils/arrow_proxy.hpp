#ifndef BEYOND_CORE_UTILS_ARROW_PROXY_HPP
#define BEYOND_CORE_UTILS_ARROW_PROXY_HPP

/**
 * @file arrow_proxy.hpp
 * @brief Provides helper class for `operator->` for proxy iterators of
 * container
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
 * @brief Helper class for implementing `operator->` of proxy iterators of
 * container
 *
 * Helper class to implement `operator->` for the proxy iterators for
 * containers. See <a
 * href="https://quuxplusone.github.io/blog/2019/02/06/arrow-proxy/">C++ idiom
 * of the day: arrow_proxy</a>.
 */
template <class Reference> struct ArrowProxy {
  Reference r;
  [[nodiscard]] auto operator-> () noexcept -> Reference*
  {
    return &r;
  }
};

/** @} @} */

} // namespace beyond

#endif // BEYOND_CORE_UTILS_ARROW_PROXY_HPP
