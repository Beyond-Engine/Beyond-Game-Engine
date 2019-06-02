#ifndef BEYOND_CORE_MATH_VECTOR_HPP
#define BEYOND_CORE_MATH_VECTOR_HPP

/**
 * @file vector.hpp
 * @brief Provides Vector and Point classes
 * @ingroup math
 */

#include <array>
#include <cmath>
#include <iostream>
#include <type_traits>
#include <utility>

#include "beyond/core/utils/assert.hpp"
#include "beyond/core/utils/bit_cast.hpp"

namespace beyond {

/**
 * @addtogroup core
 * @{
 * @addtogroup math
 * @{
 */

/**
 * @brief Fix dimensional vectors, supports a lot of common vector arithmetic
 * operations
 *
 * @see Vector<T, 2>
 * @see Vector<T, 3>
 * @see Vector<T, 4>
 */
template <typename T, std::size_t size> struct Vector;

/**
 * @brief Fix dimensional points
 *
 * @see Point<T, 2>
 * @see Point<T, 3>
 * @related Vector
 */
template <typename T, std::size_t size> struct Point;

/**
 * @brief Traits for vector like entities that holds information about
 components and subparts
 */
template <typename Vector> struct VectorTrait;

template <typename Value> struct VectorTrait<Vector<Value, 2>> {
  using ValueType = Value;
  using VectorType = Vector<Value, 2>;
  using Vector2Type = Vector<Value, 2>;
  static constexpr std::size_t size = 2;
};

template <typename Value> struct VectorTrait<Vector<Value, 3>> {
  using ValueType = Value;
  using VectorType = Vector<Value, 2>;
  using Vector2Type = Vector<Value, 2>;
  using Vector3Type = Vector<Value, 3>;
  static constexpr std::size_t size = 3;
};

template <typename Value> struct VectorTrait<Vector<Value, 4>> {
  using ValueType = Value;
  using VectorType = Vector<Value, 2>;
  using Vector2Type = Vector<Value, 2>;
  using Vector3Type = Vector<Value, 3>;
  using Vector4Type = Vector<Value, 4>;
  static constexpr std::size_t size = 4;
};

/**
 * @brief Storage of a vector-like object
 * @see Vector, Point, VectorBase
 *
 * This class storage class of vector-like objects, including
 * Vector, and Point.
 */
template <typename Derived, std::size_t size> struct VectorStorage;

namespace detail {

/**
 * @brief Abstraction of vector component
 */
template <typename Trait, size_t index> struct VectorComponent {
  using ValueType = typename Trait::ValueType;

  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  [[nodiscard]] constexpr operator ValueType&() noexcept
  {
    return (data_[index]);
  }

  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  [[nodiscard]] constexpr operator const ValueType&() const noexcept
  {
    return (data_[index]);
  }

  // NOLINTNEXTLINE(google-runtime-operator)
  [[nodiscard]] constexpr auto operator&() const noexcept -> const ValueType*
  {
    return data_ + index;
  }

private:
  ValueType data_[1];
};

template <typename Trait, std::size_t... indices> struct VectorConverter;

template <typename Trait, std::size_t index_x, std::size_t index_y>
struct VectorConverter<Trait, index_x, index_y> {
  using ValueType = typename Trait::ValueType;
  using VectorType = typename Trait::Vector2Type;

  [[nodiscard]] static constexpr auto convert(const ValueType* data) noexcept
      -> VectorType
  {
    return VectorType(data[index_x], data[index_y]);
  }
};

template <typename Trait> struct VectorConverter<Trait, 0, 1> {
  using ValueType = typename Trait::ValueType;
  using VectorType = typename Trait::Vector2Type;

  [[nodiscard]] static constexpr auto convert(const ValueType* data) noexcept
      -> const VectorType&
  {
    return (*bit_cast<const VectorType*>(data));
  }

  [[nodiscard]] static constexpr auto convert(ValueType* data) -> VectorType&
  {
    return (*bit_cast<VectorType*>(data));
  }
};

template <typename Trait> struct VectorConverter<Trait, 1, 2> {
  using ValueType = typename Trait::ValueType;
  using VectorType = typename Trait::Vector2Type;

  [[nodiscard]] static constexpr auto convert(const ValueType* data) noexcept
      -> const VectorType&
  {
    return (*bit_cast<const VectorType*>(data + 1)); // NOLINT
  }

  [[nodiscard]] static constexpr auto convert(ValueType* data) -> VectorType&
  {
    return (bit_cast<VectorType>(data + 1)); // NOLINT
  }
};

template <typename Trait> struct VectorConverter<Trait, 2, 3> {
  using ValueType = typename Trait::ValueType;
  using VectorType = typename Trait::Vector2Type;

  [[nodiscard]] static constexpr auto convert(const ValueType* data) noexcept
      -> const VectorType&
  {
    return (*bit_cast<const VectorType*>(data + 2)); // NOLINT
  }

  [[nodiscard]] static constexpr auto convert(ValueType* data) -> VectorType&
  {
    return (bit_cast<VectorType>(data + 2)); // NOLINT
  }
};

template <typename Trait, std::size_t dimensions, std::size_t... indices>
struct Subvector {
  using ValueType = typename Trait::ValueType;
  using VectorType = typename Trait::VectorType;

  std::array<ValueType, dimensions> elem;

  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  [[nodiscard]] constexpr operator VectorType() noexcept
  {
    return (detail::VectorConverter<Trait, indices...>::convert(elem.data()));
  }

  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  [[nodiscard]] constexpr operator VectorType() const noexcept
  {
    return (detail::VectorConverter<Trait, indices...>::convert(elem.data()));
  }

  constexpr auto operator=(VectorType value) noexcept -> Subvector&
  {
    std::size_t i = 0;
    ((elem[indices] = value[i++]), ...);
    return (*this);
  }

  template <std::size_t... indices2>
  constexpr auto
  operator=(Subvector<Trait, dimensions, indices2...> value) noexcept
      -> Subvector&
  {
    ((elem[indices] = value.elem[indices2]), ...);
    return (*this);
  }
};

template <typename Trait, std::size_t index_x, std::size_t index_y>
using Subvector2 = Subvector<Trait, 2, index_x, index_y>;

template <typename Trait, std::size_t index_x, std::size_t index_y,
          std::size_t index_z>
using Subvector3 = Subvector<Trait, 3, index_x, index_y, index_z>;

template <typename Trait, std::size_t index_x, std::size_t index_y,
          std::size_t index_z, std::size_t index_w>
using Subvector4 = Subvector<Trait, 4, index_x, index_y, index_z, index_w>;

template <typename Trait, std::size_t index_x1, std::size_t index_y1,
          std::size_t index_x2, std::size_t index_y2>
[[nodiscard]] constexpr auto
operator==(const Subvector2<Trait, index_x1, index_y1>& v1,
           const Subvector2<Trait, index_x2, index_y2>& v2) noexcept -> bool
{
  return v1.elem[index_x1] == v2.elem[index_x2] &&
         v1.elem[index_y1] == v1.elem[index_y2];
}

template <typename Trait, std::size_t index_x1, std::size_t index_y1,
          std::size_t index_x2, std::size_t index_y2>
[[nodiscard]] constexpr auto
operator!=(const Subvector2<Trait, index_x1, index_y1>& v1,
           const Subvector2<Trait, index_x2, index_y2>& v2) noexcept -> bool
{
  return v1.elem[index_x1] != v2.elem[index_x2] ||
         v1.elem[index_y1] != v1.elem[index_y2];
}

} // namespace detail

template <typename Derived> struct VectorStorage<Derived, 2> {
  using Trait = VectorTrait<Derived>;
  using ValueType = typename Trait::ValueType;

  constexpr VectorStorage() noexcept : elem{{}} {}
  explicit constexpr VectorStorage(
      const std::array<ValueType, 2>& data) noexcept
      : elem{data}
  {
  }

  union {
    std::array<ValueType, 2> elem;
    detail::VectorComponent<Trait, 0> x;
    detail::VectorComponent<Trait, 1> y;
    detail::Subvector2<Trait, 0, 1> xy;
    detail::Subvector2<Trait, 1, 0> yx;
  };
};

template <typename Derived> struct VectorStorage<Derived, 3> {
  using Trait = VectorTrait<Derived>;
  using ValueType = typename Trait::ValueType;

  constexpr VectorStorage() noexcept : elem{{}} {}
  explicit constexpr VectorStorage(
      const std::array<ValueType, 3>& data) noexcept
      : elem{data}
  {
  }

  union {
    std::array<ValueType, 3> elem;
    detail::VectorComponent<Trait, 0> x;
    detail::VectorComponent<Trait, 1> y;
    detail::VectorComponent<Trait, 2> z;
    /*
      detail::Subvector2<VectorTrait<Vector<T, 2>>, 0, 1> xy;
      detail::Subvector2<VectorTrait<Vector<T, 2>>, 1, 0> yx;
      detail::Subvector2<VectorTrait<Vector<T, 2>>, 0, 2> xz;
      detail::Subvector2<VectorTrait<Vector<T, 2>>, 2, 0> zx;
      detail::Subvector2<VectorTrait<Vector<T, 2>>, 1, 2> yz;
      detail::Subvector2<VectorTrait<Vector<T, 2>>, 2, 1> zy;*/
  };
};

template <typename Derived> struct VectorStorage<Derived, 4> {
  using Trait = VectorTrait<Derived>;
  using ValueType = typename Trait::ValueType;

  constexpr VectorStorage() noexcept : elem{{}} {}
  explicit constexpr VectorStorage(
      const std::array<ValueType, 4>& data) noexcept
      : elem{data}
  {
  }
  union {
    std::array<ValueType, 4> elem;
    detail::VectorComponent<Trait, 0> x;
    detail::VectorComponent<Trait, 1> y;
    detail::VectorComponent<Trait, 2> z;
    detail::VectorComponent<Trait, 3> w;
  };
};

/**
 * @brief This class serve as base class for Vector, Bivector, and Trivectors.
 *
 * This class contains all the possible operations on vectors and anti-vectors
 * (Bivector and Trivector).
 *
 * @see Vector, VectorStorage
 */
template <typename T, std::size_t... Ns>
struct VectorBase : VectorStorage<Vector<T, sizeof...(Ns)>, sizeof...(Ns)> {
  using ValueType = T;
  using Storage = VectorStorage<Vector<T, sizeof...(Ns)>, sizeof...(Ns)>;

  /**
   * @brief Gets the dimensionality of a vector
   */
  [[nodiscard]] static constexpr auto size() noexcept -> std::size_t
  {
    return sizeof...(Ns);
  }

  constexpr VectorBase() noexcept = default;

  template <typename A0, typename... Args>
  explicit constexpr VectorBase(A0 a0, Args... args) noexcept
      : Storage{{a0, args...}}
  {
  }

  /**
   * @brief Gets the length of the vector
   * @note Only vectors of floating point components support this operation
   * @see length_squared()
   */
  template <typename U = ValueType>
  [[nodiscard]] auto length() const noexcept
      -> std::enable_if_t<std::is_floating_point_v<U>, U>
  {
    return std::sqrt(length_squared());
  }

  /**
   * @brief Gets the i-th component of the vector
   * @warning Behavior of out of index is undefined
   */
  auto operator[](std::size_t i) const noexcept -> ValueType
  {
    BEYOND_ASSERT_MSG(i < size(), "Invalid index");
    return Storage::elem[i];
  }

  /// @overload
  auto operator[](std::size_t i) noexcept -> ValueType&
  {
    BEYOND_ASSERT_MSG(i < size(), "Invalid index");
    return Storage::elem[i];
  }

  /**
   * @brief Gets the i-th component of the vector
   * @warning Behavior of out of index is undefined
   */
  auto operator()(std::size_t i) const noexcept -> ValueType
  {
    BEYOND_ASSERT_MSG(i < size(), "Invalid index");
    return Storage::elem[i];
  }

  /// @overload
  auto operator()(std::size_t i) noexcept -> ValueType&
  {
    BEYOND_ASSERT_MSG(i < size(), "Invalid index");
    return Storage::elem[i];
  }

  /**
   * @brief Negates this vector
   */
  constexpr auto operator-() const noexcept -> VectorBase
  {
    return VectorBase{(-Storage::elem[Ns])...};
  }

  /**
   * @brief Adds another vector to this vector
   */
  constexpr auto operator+=(const VectorBase& rhs) noexcept -> VectorBase&
  {
    ((Storage::elem[Ns] += rhs[Ns]), ...);
    return *this;
  }

  /**
   * @brief Subtract another vector to this vector
   */
  constexpr auto operator-=(const VectorBase& rhs) noexcept -> VectorBase&
  {
    ((Storage::elem[Ns] -= rhs[Ns]), ...);
    return *this;
  }

  /**
   * @brief Multiply a scalar to this vector
   */
  constexpr auto operator*=(ValueType rhs) noexcept -> VectorBase&
  {
    ((Storage::elem[Ns] *= rhs), ...);
    return *this;
  }

  /**
   * @brief Divides this vector by a scalar
   */
  template <typename U = ValueType>
  constexpr auto operator/=(U rhs) noexcept
      -> std::enable_if_t<std::is_floating_point_v<U>, VectorBase&>
  {
    BEYOND_ASSERT_MSG(rhs != 0, "Devide by zero");
    const auto inv = static_cast<ValueType>(1) / rhs;
    ((Storage::elem[Ns] *= inv), ...);
    return *this;
  }

  /**
   * @brief Returns the squared length of this vector.
   *
   * The magnitude of vector `v` is calculated by `sqrt(dot(v, v))`. However,
   * square root function is relatively expensive.  If you are using `length`
   * simply to compare distances, then you can just as well compare squared
   * length.
   * @see length()
   */
  [[nodiscard]] auto length_squared() const noexcept -> ValueType
  {
    return dot(*this, *this);
  }

  /**
   * @brief Returns true if the given vector `v` is exactly equal to this
   * vector.
   * @warning Due to floating point inaccuracies, this might return false for
   * vectors which are essentially (but not exactly) equal.
   */
  [[nodiscard]] constexpr auto operator==(const VectorBase& v) const noexcept
      -> bool
  {
    return ((Storage::elem[Ns] == v.elem[Ns]) && ...);
  }

  /**
   * @brief Returns false if the given vector `v` is exactly equal to this
   * vector.
   * @warning Due to floating point inaccuracies, this might return true for
   * vectors which are essentially (but not exactly) equal.
   */
  [[nodiscard]] constexpr auto operator!=(const VectorBase& v) const noexcept
      -> bool
  {
    return ((Storage::elem[Ns] != v.elem[Ns]) || ...);
  }
};

/**
 * @brief Adds two vectors
 * @related VectorBase
 */
template <typename T, std::size_t... Ns>
[[nodiscard]] constexpr auto operator+(VectorBase<T, Ns...> lhs,
                                       const VectorBase<T, Ns...>& rhs) noexcept
    -> VectorBase<T, Ns...>
{
  lhs += rhs;
  return lhs;
}

/**
 * @brief Subtracts two vectors
 * @related VectorBase
 */
template <typename T, std::size_t... Ns>
[[nodiscard]] constexpr auto operator-(VectorBase<T, Ns...> lhs,
                                       const VectorBase<T, Ns...>& rhs) noexcept
    -> VectorBase<T, Ns...>
{
  lhs -= rhs;
  return lhs;
}

/**
 * @brief Divides a vector by a scalar
 * @related VectorBase
 */
template <typename T, std::size_t... Ns>
[[nodiscard]] constexpr auto operator/(VectorBase<T, Ns...> lhs,
                                       T scalar) noexcept
    -> std::enable_if_t<std::is_floating_point_v<T>, VectorBase<T, Ns...>>
{
  lhs /= scalar;
  return lhs;
}

/// @brief Normalizes a vector into a unit vector
/// @note Only available for floating point vectors
template <typename T, std::size_t... Ns>
[[nodiscard]] constexpr auto normalize(const VectorBase<T, Ns...>& v) noexcept
    -> std::enable_if_t<std::is_floating_point_v<T>, VectorBase<T, Ns...>>
{
  return v / v.length();
}

template <typename T> struct Vector<T, 2> : VectorBase<T, 0, 1> {
  using ValueType = T;
  using Base = VectorBase<T, 0, 1>;

  constexpr Vector() noexcept = default;
  constexpr Vector(T xx, T yy) noexcept : Base{xx, yy} {}

  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  constexpr Vector(const Base& base) noexcept : Base{base} {}
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  constexpr Vector(Base&& base) noexcept : Base{std::forward<Base>(base)} {}
};

template <typename T> struct Vector<T, 3> : VectorBase<T, 0, 1, 2> {
  using ValueType = T;
  using Base = VectorBase<T, 0, 1, 2>;

  constexpr Vector() noexcept = default;
  constexpr Vector(const Vector<T, 2>& v, T zz) noexcept : Base{v[0], v[1], zz}
  {
  }
  constexpr Vector(T xx, T yy, T zz) noexcept : Base{xx, yy, zz} {}

  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  constexpr Vector(const Base& base) noexcept : Base{base} {}
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  constexpr Vector(Base&& base) noexcept : Base{std::forward<Base>(base)} {}
};

template <typename T> struct Vector<T, 4> : VectorBase<T, 0, 1, 2, 3> {
  using ValueType = T;
  using Base = VectorBase<T, 0, 1, 2, 3>;

  constexpr Vector() noexcept = default;
  constexpr Vector(T xx, T yy, T zz, T ww) noexcept : Base{xx, yy, zz, ww} {}

  constexpr Vector(const Vector<T, 2>& v, T zz, T ww) noexcept
      : Base{v[0], v[1], zz, ww}
  {
  }

  constexpr Vector(const Vector<T, 3>& v, T ww) noexcept
      : Base{v[0], v[1], v[2], ww}
  {
  }

  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  constexpr Vector(const Base& base) noexcept : Base{base} {}
  // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
  constexpr Vector(Base&& base) noexcept : Base{std::forward<Base>(base)} {}
};

/**
 * @brief Multiplies vector `v` by a scalar
 * @related VectorBase
 */
template <typename T, typename U, std::size_t... Ns>
[[nodiscard]] constexpr auto operator*(const VectorBase<T, Ns...>& v,
                                       U scalar) noexcept
{
  return VectorBase<std::common_type_t<T, U>, Ns...>{(scalar * v.elem[Ns])...};
}

/**
 * @brief Multiplies vector `v` by a scalar
 * @related VectorBase
 * @overload
 */
template <typename T, typename U, std::size_t... Ns>
[[nodiscard]] constexpr auto operator*(U scalar,
                                       const VectorBase<T, Ns...>& v) noexcept
{
  return VectorBase<std::common_type_t<T, U>, Ns...>{(scalar * v.elem[Ns])...};
}

template <typename T>
inline auto operator<<(std::ostream& os, const Vector<T, 2>& v) -> std::ostream&
{
  os << "vec(" << v.x << ", " << v.y << ')';
  return os;
}

template <typename T>
inline auto operator<<(std::ostream& os, const Vector<T, 3>& v) -> std::ostream&
{
  os << "vec(" << v.x << ", " << v.y << ", " << v.z << ')';
  return os;
}

template <typename T>
inline auto operator<<(std::ostream& os, const Vector<T, 4>& v) -> std::ostream&
{
  os << "vec(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ')';
  return os;
}

/**
 * @brief Gets the Dot Product of two vectors
 * @related VectorBase
 *
 * Dot product takes two vectors of the same dimensionality and sum the products
 * of the corresponding components of the vectors. The result is a scalar.
 */
template <typename T, typename U, std::size_t... Ns,
          typename Scalar = std::common_type_t<T, U>>
[[nodiscard]] constexpr auto dot(const VectorBase<T, Ns...>& v1,
                                 const VectorBase<U, Ns...>& v2) noexcept
    -> Scalar
{
  return ((v1.elem[Ns] * v2.elem[Ns]) + ...);
}

/**
 * @brief Calculates cross product of two 3d vectors
 * @related Vector
 */
template <typename T, typename U>
[[nodiscard]] constexpr auto cross(const Vector<T, 3>& v1,
                                   const Vector<U, 3>& v2) noexcept
    -> Vector<std::common_type_t<T, U>, 3>
{
  return {(v1.y * v2.z) - (v1.z * v2.y), (v1.z * v2.x) - (v1.x * v2.z),
          (v1.x * v2.y) - (v1.y * v2.x)};
}

template <typename T> struct Point<T, 2> : Vector<T, 2> {
  using Vector<T, 2>::Vector;
};

template <typename T> struct Point<T, 3> : Vector<T, 3> {
  using Vector<T, 3>::Vector;

  constexpr Point(const Point<T, 2>& p, T zz) noexcept : Point{p.x, p.y, zz} {}
};

/**
 * @brief Point + Vector = Point
 * @related Point
 */
template <typename T, std::size_t size>
[[nodiscard]] constexpr auto operator+(const Point<T, size>& p,
                                       const Vector<T, size>& v) noexcept
    -> Point<T, size>
{
  return static_cast<Point<T, size>>((bit_cast<Vector<T, size>>(p) + v));
}

/**
 * @brief Vector + Point = Point
 * @related Point
 */
template <typename T, std::size_t size>
[[nodiscard]] constexpr auto operator+(const Vector<T, size>& v,
                                       const Point<T, size>& p) noexcept
    -> Point<T, size>
{
  return static_cast<Point<T, size>>((bit_cast<Vector<T, size>>(p) + v));
}

/**
 * @brief Gets the squared distance between two points
 */
template <typename T, typename U, std::size_t size>
[[nodiscard]] constexpr auto distance_squared(const Point<T, size>& p1,
                                              const Point<U, size>& p2) noexcept
    -> std::common_type_t<T, U>
{
  const auto dx = p2 - p1;
  return dot(dx, dx);
}

/**
 * @brief Gets the distance between two points
 */
template <typename T, typename U, std::size_t size>
[[nodiscard]] auto distance(const Point<T, size>& p1,
                            const Point<U, size>& p2) noexcept
    -> std::common_type_t<T, U>
{
  return std::sqrt(distance_squared(p1, p2));
}

template <typename T> using Vector2 = Vector<T, 2>;
template <typename T> using Vector3 = Vector<T, 3>;
template <typename T> using Vector4 = Vector<T, 4>;
using Vector2f = Vector2<float>;
using Vector2i = Vector2<int>;
using Vector3f = Vector3<float>;
using Vector3i = Vector3<int>;
using Vector4f = Vector4<float>;
using Vector4i = Vector4<int>;

template <typename T> using Point2 = Point<T, 2>;
template <typename T> using Point3 = Point<T, 3>;
using Point2f = Point2<float>;
using Point2i = Point2<int>;
using Point3f = Point3<float>;
using Point3i = Point3<int>;

/** @}
 *  @} */

} // namespace beyond

#endif // BEYOND_CORE_MATH_VECTOR_HPP
