#pragma once

#ifndef BEYOND_CORE_CONTAINER_STATIC_VECTOR_HPP
#define BEYOND_CORE_CONTAINER_STATIC_VECTOR_HPP

#include <algorithm>
#include <iterator>
#include <type_traits>

#include <beyond/core/utils/assert.hpp>
#include <beyond/core/utils/bit_cast.hpp>

namespace beyond {

/**
 * @addtogroup core
 * @{
 * @addtogroup container
 * @{
 */

template <class T, std::size_t N> class static_vector {
public:
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using size_type = std::size_t;
  using difference_type = std::make_signed_t<size_type>;

  static_vector() noexcept = default;

  /**
   * @brief constructs a static_vector with n default-inserted elements.
   * @pre `n <= capacity()`
   *
   * Complexity: O(n)
   */
  constexpr explicit static_vector(size_type n) noexcept(
      std::is_nothrow_constructible_v<value_type>)
      : size_{n}
  {
    BEYOND_ASSERT(n <= capacity());
    std::uninitialized_value_construct_n(reinterpret_cast<T*>(data_), size_);
  }

  /**
   * @brief Constructs a `static_vector` with `n` copies of value
   * @pre `n <= capacity()`
   *
   * Complexity: O(n)
   */
  constexpr explicit static_vector(size_type n, value_type v) noexcept(
      std::is_nothrow_copy_constructible_v<value_type>)
      : size_{n}
  {
    BEYOND_ASSERT(n <= capacity());
    std::uninitialized_fill_n(reinterpret_cast<T*>(data_), size_, v);
  }

  /**
   * @brief Constructs a `static_vector` equal to the range `[first, last)`
   *
   * Complexity: Initializing distance(first, last) <= capacity() of
   * `value_type`s
   */
  template <class InputIterator,
            typename = decltype(*std::declval<InputIterator&>(), void(),
                                ++std::declval<InputIterator&>(), void())>
  constexpr static_vector(InputIterator first, InputIterator last) noexcept(
      std::is_nothrow_copy_constructible_v<value_type>)
  {
    const auto distance = std::distance(first, last);
    BEYOND_ASSERT(static_cast<size_type>(distance) <= capacity());
    size_ = distance;
    std::uninitialized_copy(first, last, reinterpret_cast<T*>(data_));
  }

  constexpr static_vector(std::initializer_list<value_type> il)
      : size_{il.size()}
  {
    BEYOND_ASSERT(size_ <= capacity());
    std::uninitialized_copy(std::begin(il), std::end(il),
                            reinterpret_cast<T*>(data_));
  }

  ~static_vector() noexcept(std::is_nothrow_destructible_v<value_type>)
  {
    std::destroy_n(reinterpret_cast<T*>(data_), size_);
  }

  static_vector(const static_vector&) noexcept(
      std::is_nothrow_copy_constructible_v<value_type>) = default;
  auto operator=(const static_vector&) &
      noexcept(std::is_nothrow_copy_assignable_v<value_type>)
          -> static_vector& = default;
  static_vector(static_vector&&) noexcept(
      std::is_nothrow_move_constructible_v<value_type>) = default;
  auto operator=(static_vector&&) &
      noexcept(std::is_nothrow_move_assignable_v<value_type>)
          -> static_vector& = default;

  /**
   * @brief Gets the capacity of the `static_vector`
   *
   * Complexity: O(1)
   */
  [[nodiscard]] constexpr auto capacity() const noexcept -> size_type
  {
    return N;
  }

  /**
   * @brief Gets the size of the `static_vector`
   *
   * Complexity: O(1)
   */
  [[nodiscard]] constexpr auto size() const noexcept -> size_type
  {
    return size_;
  }

  /**
   * @brief Returns if the `static_vector` is empty or not
   *
   * Complexity: O(1)
   */
  [[nodiscard]] constexpr auto empty() const noexcept -> size_type
  {
    return size_ == 0;
  }

  /**
   * @brief Pushes an object into the end of the static_vector
   *
   * @warning If `size() == capacity()`, the result is undefined
   * @return A reference to the created object
   *
   * Complexity: O(1)
   */
  template <typename... Args>
  auto push_back(const value_type& value) -> reference
  {
    return emplace_back(value);
  }

  /// @overload
  template <typename... Args> auto push_back(value_type&& value) -> reference
  {
    return emplace_back(std::move(value));
  }

  /**
   * @brief Inplace constructs an object into the end of the static_vector
   *
   * @warning If `size() == capacity()`, the result is undefined
   * @return A reference to the created object
   *
   * Complexity: O(1)
   */
  template <typename... Args> auto emplace_back(Args&&... args) -> reference
  {
    BEYOND_ASSERT(size_ < N);

    new (reinterpret_cast<T*>(data_) + size_) T(std::forward<Args>(args)...);
    ++size_;
    return reinterpret_cast<T*>(data_)[size_ - 1];
  }

  /**
   * @brief Removes the last element of the container
   *
   * @warning If `size() == 0`, the result is undefined
   *
   * Complexity: O(1)
   */
  auto pop_back() -> void
  {
    BEYOND_ASSERT(size_ != 0);
    --size_;
  }

  /**
   * @brief Access an object at index `pos`
   * @warning If `pos > size()`, the result is undefined
   *
   * Complexity: O(1)
   */
  [[nodiscard]] constexpr auto operator[](std::size_t pos) const
      -> const_reference
  {
    return reinterpret_cast<const T*>(data_)[pos];
  }

  /// @overload
  [[nodiscard]] constexpr auto operator[](std::size_t pos) -> reference
  {
    return reinterpret_cast<T*>(data_)[pos];
  }

  /**
   * @brief access the first element
   * @warning Calling `front` on an empty container is undefined.
   *
   * Complexity: O(1)
   */
  [[nodiscard]] constexpr auto front() noexcept -> reference
  {
    return reinterpret_cast<T*>(data_)[0];
  }

  /// @overload
  [[nodiscard]] constexpr auto front() const noexcept -> const_reference
  {
    return reinterpret_cast<const T*>(data_)[0];
  }

  /**
   * @brief access the last element
   * @warning Calling `back` on an empty container is undefined.
   *
   * Complexity: O(1)
   */
  [[nodiscard]] constexpr auto back() noexcept -> reference
  {
    return reinterpret_cast<T*>(data_)[size_ - 1];
  }

  /// @overload
  [[nodiscard]] constexpr auto back() const noexcept -> const_reference
  {
    return reinterpret_cast<const T*>(data_)[size_ - 1];
  }

  /**
   * @brief Gets the underlying raw data pointer of the `static_vector`
   *
   * Complexity: O(1)
   */
  [[nodiscard]] constexpr auto data() noexcept -> pointer
  {
    return reinterpret_cast<T*>(data_);
  }

  /// @overload
  [[nodiscard]] constexpr auto data() const noexcept -> const_pointer
  {
    return reinterpret_cast<const T*>(data_);
  }

  /**
   * @brief Access an object at index `pos` with bounds checking
   * @throw `std::out_of_range` if `pos >= size()`
   *
   * Complexity: O(1)
   */
  [[nodiscard]] constexpr auto at(std::size_t pos) const -> const_reference
  {
    if (pos >= size()) {
      throw std::out_of_range{fmt::format(
          "static_vector `at` out of index, at {}, size {}", pos, size_)};
    }

    return reinterpret_cast<const T*>(data_)[pos];
  }

  /// @overload
  [[nodiscard]] constexpr auto at(std::size_t pos) -> reference
  {
    if (pos >= size()) {
      throw std::out_of_range{fmt::format(
          "static_vector `at` out of index, at {}, size {}", pos, size_)};
    }

    return reinterpret_cast<T*>(data_)[pos];
  }

  // TODO(lesley): clear, erase, insert, resize, asign
  template <bool is_const = false> class Itr {
  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using reference = std::conditional_t<is_const, const T&, T&>;
    using pointer = std::conditional_t<is_const, const T*, T*>;

    explicit constexpr Itr(pointer data = nullptr) : data_{data} {}

    [[nodiscard]] constexpr auto operator*() const noexcept -> reference
    {
      return *data_;
    }

    [[nodiscard]] constexpr auto operator-> () const noexcept -> pointer
    {
      return data_;
    }

    [[nodiscard]] constexpr auto operator++() noexcept -> Itr&
    {
      ++data_;
      return *this;
    }

    [[nodiscard]] constexpr auto operator--() noexcept -> Itr&
    {
      --data_;
      return *this;
    }

    [[nodiscard]] constexpr auto operator++(int) noexcept -> Itr
    {
      return Itr{data_++};
    }

    [[nodiscard]] constexpr auto operator--(int) noexcept -> Itr
    {
      return Itr{data_--};
    }

    [[nodiscard]] friend constexpr auto operator==(Itr lhs, Itr rhs) noexcept
        -> bool
    {
      return lhs.data_ == rhs.data_;
    }

    [[nodiscard]] friend constexpr auto operator!=(Itr lhs, Itr rhs) noexcept
        -> bool
    {
      return !(lhs == rhs);
    }

    [[nodiscard]] friend constexpr auto operator-(Itr lhs, Itr rhs) noexcept
        -> difference_type
    {
      return lhs.data_ - rhs.data_;
    }

    [[nodiscard]] friend constexpr auto operator+(Itr lhs,
                                                  difference_type rhs) noexcept
        -> Itr
    {
      return Itr{lhs.data_ + rhs};
    }

    [[nodiscard]] friend constexpr auto operator+(difference_type lhs,
                                                  Itr rhs) noexcept -> Itr
    {
      return Itr{lhs + rhs.data_};
    }

  private:
    pointer data_ = nullptr;
  };

  using iterator = Itr<false>;
  using const_iterator = Itr<true>;

  // TODO(lesley): other begin and end family of functions
  [[nodiscard]] constexpr auto begin() noexcept -> iterator
  {
    return iterator{reinterpret_cast<T*>(data_)};
  }

  [[nodiscard]] constexpr auto end() noexcept -> iterator
  {
    return iterator{reinterpret_cast<T*>(data_) + size_};
  }

  [[nodiscard]] constexpr auto begin() const noexcept -> const_iterator
  {
    return const_iterator{reinterpret_cast<const T*>(data_)};
  }

  [[nodiscard]] constexpr auto end() const noexcept -> const_iterator
  {
    return const_iterator{reinterpret_cast<const T*>(data_) + size_};
  }

  [[nodiscard]] constexpr auto cbegin() const noexcept -> const_iterator
  {
    return const_iterator{reinterpret_cast<const T*>(data_)};
  }

  [[nodiscard]] constexpr auto cend() const noexcept -> const_iterator
  {
    return const_iterator{reinterpret_cast<const T*>(data_) + size_};
  }

  /**
   * @brief Swap two `static_vector`s
   *
   * Complexity: Linear on the size() of both vector
   */
  constexpr auto
  swap(static_vector& other) noexcept(std::is_nothrow_swappable_v<value_type>)
      -> void
  {
    std::swap(size_, other.size_);
    std::swap(data_, other.data_);
  }

private:
  std::size_t size_ = 0;
  alignas(T) std::byte data_[sizeof(T) * N];
};

template <typename T, std::size_t N>
constexpr auto
swap(static_vector<T, N>& lhs,
     static_vector<T, N>& rhs) noexcept(std::is_nothrow_swappable_v<T>) -> void
{
  lhs.swap(rhs);
}

/** @}@} */

// TODO(lesley): lexicographically compares
// Free functions TODO(lesley): erase, erase_if
// TODO(lesley): deduction guide

} // namespace beyond

#endif // BEYOND_CORE_CONTAINER_STATIC_VECTOR_HPP
