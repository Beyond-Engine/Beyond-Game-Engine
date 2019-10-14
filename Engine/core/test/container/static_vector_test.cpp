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
  using size_type = std::size_t;
  using value_type = T;
  using reference = T&;

  // TODO(lesley): Other constructors supported by std::vector
  static_vector() = default;
  ~static_vector()
  {
    for (std::size_t pos = 0; pos < size_; ++pos) {
      reinterpret_cast<T*>(data_)[pos].~T();
    }
  }

  static_vector(const static_vector&) = default;
  auto operator=(const static_vector&) & noexcept -> static_vector& = default;
  static_vector(static_vector&&) noexcept = default;
  auto operator=(static_vector&&) & noexcept -> static_vector& = default;

  /**
   * @brief Gets the capacity of the `static_vector`
   *
   * Complexity: O(1)
   */
  [[nodiscard]] constexpr auto capacity() const -> size_type
  {
    return N;
  }

  /**
   * @brief Gets the size of the `static_vector`
   *
   * Complexity: O(1)
   */
  [[nodiscard]] constexpr auto size() const -> size_type
  {
    return size_;
  }

  /**
   * @brief Returns if the `static_vector` is empty or not
   *
   * Complexity: O(1)
   */
  [[nodiscard]] constexpr auto empty() const -> size_type
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
  [[nodiscard]] constexpr auto operator[](std::size_t pos) const -> const T&
  {
    return reinterpret_cast<T*>(data_)[pos];
  }

  /// @overload
  [[nodiscard]] constexpr auto operator[](std::size_t pos) -> T&
  {
    return reinterpret_cast<T*>(data_)[pos];
  }

  // TODO(lesley): clear, erase, insert, resize, asign, swap
  // TODO(lesley): front, back, data, at
  template <bool is_const = false> class I {
    using iterator_category = std::random_access_iterator_tag;
    using value_type = std::conditional_t<is_const, const T&, T&>;
    using difference_type = std::ptrdiff_t;
    using reference = value_type;
    using pointer = std::conditional_t<is_const, T* const, T*>;

  public:
    explicit constexpr I(pointer data = nullptr) : data_{data} {}

    [[nodiscard]] constexpr auto operator*() const noexcept -> reference
    {
      return *data_;
    }

    [[nodiscard]] constexpr auto operator-> () const noexcept -> pointer
    {
      return data_;
    }

    [[nodiscard]] constexpr auto operator++() noexcept -> I&
    {
      ++data_;
      return *this;
    }

    [[nodiscard]] constexpr auto operator--() noexcept -> I&
    {
      --data_;
      return *this;
    }

    [[nodiscard]] constexpr auto operator++(int) noexcept -> I
    {
      return I{data_++};
    }

    [[nodiscard]] constexpr auto operator--(int) noexcept -> I
    {
      return I{data_--};
    }

    [[nodiscard]] friend constexpr auto operator==(I lhs, I rhs) noexcept
        -> bool
    {
      return lhs.data_ == rhs.data_;
    }

    [[nodiscard]] friend constexpr auto operator!=(I lhs, I rhs) noexcept
        -> bool
    {
      return !(lhs == rhs);
    }

  private:
    pointer data_ = nullptr;
  };

  using iterator = I<false>;
  using const_iterator = I<true>;

  // TODO(lesley): other begin and end family of functions
  [[nodiscard]] constexpr auto begin() noexcept -> iterator
  {
    return iterator{reinterpret_cast<T*>(data_)};
  }

  [[nodiscard]] constexpr auto end() noexcept -> iterator
  {
    return iterator{reinterpret_cast<T*>(data_) + size_};
  }

private:
  std::size_t size_ = 0;
  alignas(T) std::byte data_[sizeof(T) * N];
};

/** @}@} */

// TODO(lesley): lexicographically compares
// Free functions TODO(lesley): swap, erase, erase_if
// TODO(lesley): deduction guide

} // namespace beyond

#include <catch2/catch.hpp>

#include <string>

using namespace beyond;

TEST_CASE("static_vector", "[container]")
{
  GIVEN("A default constructed static_vector")
  {
    static_vector<int, 10> v1;
    THEN("it is empty")
    {
      REQUIRE(v1.empty());
      REQUIRE(v1.size() == 0);
    }

    WHEN("Adds an element to it")
    {
      const int first = 42;
      v1.emplace_back(first);

      THEN("Can find that element at front")
      {
        REQUIRE(v1[0] == first);
      }

      THEN("It is no longer empty")
      {
        REQUIRE(!v1.empty());
        REQUIRE(v1.size() == 1);
      }

      AND_WHEN("Adds another element to it")
      {
        const int second = 21;
        v1.push_back(second);
        REQUIRE(v1.size() == 2);
        REQUIRE(v1[1] == second);

        AND_WHEN("pops the last element from it")
        {
          v1.pop_back();
          REQUIRE(v1.size() == 1);
          REQUIRE(v1[0] == first);
        }
      }
    }
  }
}

TEST_CASE("static_vector iterators", "[container]")
{
  static_vector<std::string, 10> v;
  REQUIRE(v.begin() == v.end());

  const std::string first{"hello"};
  v.push_back(first);
  REQUIRE(v.begin() != v.end());

  SECTION("Elements access")
  {
    REQUIRE(*v.begin() == first);
    REQUIRE(v.begin()->size() == first.size());
  }

  SECTION("Pre and post fix increment & decrement")
  {
    const std::string second{"world"};
    v.push_back(second);
    auto i = v.begin();
    REQUIRE(*(++i) == second);
    REQUIRE(*i == second);
    REQUIRE(*(i++) == second);
    REQUIRE(i == v.end());
    REQUIRE((i--) == v.end());
    REQUIRE(*i == second);
    REQUIRE(*(--i) == first);
    REQUIRE(*i == first);
  }

  SECTION("operator[]")
  {
    // TODO
  }

  SECTION("Iterator ordering")
  {
    // TODO
  }

  SECTION("Random access iterator affine space operations")
  {
    // TODO
  }
}
