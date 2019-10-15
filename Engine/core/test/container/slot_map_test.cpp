#include <type_traits>
#include <utility>
#include <vector>

#include <beyond/core/utils/assert.hpp>

namespace beyond {

/**
 * @addtogroup core
 * @{
 * @addtogroup container
 * @{
 */

template <typename Key, typename T,
          template <typename...> typename Container = std::vector>
class slot_map {
public:
  using key_size_type =
      std::remove_reference_t<decltype(std::declval<Key>().index())>;
  using key_generation_type =
      std::remove_reference_t<decltype(std::declval<Key>().generation())>;
  using value_type = typename Container<T>::value_type;
  using key_type = Key;
  using size_type = typename Container<T>::size_type;
  using reference = typename Container<value_type>::reference;
  using const_reference = typename Container<value_type>::const_reference;
  using pointer = typename Container<value_type>::pointer;
  using const_pointer = typename Container<value_type>::const_pointer;
  using container_type = Container<value_type>;

  using iterator = typename Container<value_type>::iterator;
  using const_iterator = typename Container<value_type>::const_iterator;
  using reverse_iterator = typename Container<value_type>::reverse_iterator;
  using const_reverse_iterator =
      typename Container<value_type>::const_reverse_iterator;

  constexpr slot_map() = default;

  /**
   * @brief Returns whether the `slot_map` is empty
   *
   * Complexity: O(1)
   */
  [[nodiscard]] constexpr auto empty() const noexcept -> bool
  {
    return data_.empty();
  }

  /**
   * @brief Returns the size of the `slot_map`
   *
   * Complexity: O(1)
   */
  [[nodiscard]] constexpr auto size() const noexcept -> size_type
  {
    return data_.size();
  }

  /**
   * @brief Returns the capacity of the `slot_map`
   *
   * Complexity: O(1)
   */
  [[nodiscard]] constexpr auto capacity() const noexcept -> size_type
  {
    return data_.capacity();
  }

  /**
   * @brief Reserve the capacity of the `slot_map`
   *
   * Increase the capacity of the `slot_map` to a value that's greater or
   equal
   * to `n`. If `n` is greater than the current capacity(), new storage is
   * allocated, otherwise the function does nothing.
   *
   * `reserve` dose not change the size of the `slot_map`.
   *
   * Complexity: O(n) (To initialize the free list)
   */
  constexpr auto reserve(size_type n) -> void
  {
    if (n > capacity()) {
      slots_.reserve(n);
      // TODO(llai): Current grow_slot will assertion fail if there is already a
      // free list because it will lost all the previous free list info
      // which means this function will not work when the `slot_map` already
      // have a capacity and we want larger
      this->grow_slots();
      data_.reserve(n);
      erase_helper_.reserve(n);
    }
  }

  /**
   * @brief Inserts a value into the `slot_map`
   * @return The key that refer to the value inserted
   *
   * Complexity: O(1), except when an allocation is required, where it is the
   * allocation cost of `container_type`.
   */
  [[nodiscard]] constexpr auto insert(const value_type& value) -> key_type
  {
    check_size_and_grow();
    data_.push_back(value);

    // TODO(llai): special treatment for only one free element
    BEYOND_ASSERT(free_head_ != free_tail_);
    const auto index = static_cast<key_size_type>(data_.size() - 1);
    auto& slot = slots_[free_head_];
    const auto next_free_head =
        slot.index(); // The index is what free list point to
    slot = Key{index, slot.generation() + 1};
    free_head_ = next_free_head;

    return slot;
  }

  /**
   * @brief Find the element that mapped by the `key`
   *
   * The find() functions have generation counter checking. If the check
   fails,
   * the result of end() is returned.
   *
   * Complexity: O(1)
   */
  [[nodiscard]] constexpr auto find(const key_type& key) noexcept -> iterator
  {
    const auto index = key.index();
    if (index >= data_.size()) {
      return std::end(data_);
    }

    return std::begin(data_) + index;
  }

  // @overload
  //  constexpr auto find(const key_type& key) const noexcept -> const_iterator

  [[nodiscard]] constexpr auto begin() noexcept -> iterator
  {
    return data_.begin();
  }

  [[nodiscard]] constexpr auto end() noexcept -> iterator
  {
    return data_.end();
  }

  [[nodiscard]] constexpr auto begin() const noexcept -> const_iterator
  {
    return data_.begin();
  }

  [[nodiscard]] constexpr auto end() const noexcept -> const_iterator
  {
    return data_.end();
  }

  [[nodiscard]] constexpr auto cbegin() const noexcept -> const_iterator
  {
    return data_.cbegin();
  }

  [[nodiscard]] constexpr auto cend() const noexcept -> const_iterator
  {
    return data_.cend();
  }

  [[nodiscard]] constexpr auto rbegin() noexcept -> reverse_iterator
  {
    return data_.rbegin();
  }

  [[nodiscard]] constexpr auto rend() noexcept -> reverse_iterator
  {
    return data_.rend();
  }

  [[nodiscard]] constexpr auto rbegin() const noexcept -> const_reverse_iterator
  {
    return data_.rbegin();
  }

  [[nodiscard]] constexpr auto rend() const noexcept -> const_reverse_iterator
  {
    return data_.rend();
  }

  [[nodiscard]] constexpr auto crbegin() const noexcept
      -> const_reverse_iterator
  {
    return data_.crbegin();
  }

  [[nodiscard]] constexpr auto crend() const noexcept -> const_reverse_iterator
  {
    return data_.crend();
  }

private:
  Container<key_type> slots_;
  Container<T> data_;
  Container<key_size_type> erase_helper_;

  key_size_type free_head_ = 0;
  key_size_type free_tail_ = 0;

  static constexpr auto growth_rate = 1.6;
  static constexpr size_type initial_alloc_size{16};

  // Grow slot and fill the list of free elements
  // Precondition: the free list have no free slots
  //   equivalently (this->size() == this-> capacity())
  constexpr auto grow_slots()
  {
    BEYOND_ASSERT(free_head_ == free_tail_);
    BEYOND_ASSERT(slots_.empty() || slots_[free_head_].index() == free_head_);
    BEYOND_ASSERT(this->size() == this->capacity());

    const size_type old_capacity = static_cast<size_type>(slots_.capacity());
    slots_.reserve(std::max(static_cast<size_type>(growth_rate * old_capacity),
                            initial_alloc_size));
    const auto new_capacity = static_cast<size_type>(slots_.capacity());

    // At least increase by 2
    BEYOND_ASSERT(new_capacity > old_capacity + 1);

    // Fill the free list, where all except the last point to the next free
    // element
    // {
    const auto last_index = static_cast<key_size_type>(new_capacity - 1);
    for (auto i = static_cast<key_size_type>(old_capacity); i < last_index;
         ++i) {
      slots_.emplace(std::end(slots_), i + 1, 0);
    }
    slots_.emplace(std::end(slots_), last_index, 0);
    // }

    free_head_ = static_cast<key_size_type>(old_capacity);
    free_tail_ = static_cast<key_size_type>(last_index);
  }

  // If size is equal to capacity, then grow
  constexpr auto check_size_and_grow()
  {
    if (this->size() == this->capacity()) {
      this->grow();
    }
  }

  constexpr auto grow()
  {
    auto new_capacity = std::max(
        initial_alloc_size, static_cast<size_type>(growth_rate * this->size()));
    if (new_capacity < this->capacity()) { // Overflow
      new_capacity = std::numeric_limits<size_type>::max();
    }

    grow_slots();
    data_.reserve(new_capacity);
    erase_helper_.reserve(new_capacity);
  }
};

/** @}@} */

} // namespace beyond

#include <catch2/catch.hpp>

#include <beyond/core/utils/handle.hpp>

namespace {

struct DummyHandle : beyond::Handle<DummyHandle, std::uint32_t, 20, 12> {
  using Handle::Handle;
};

} // anonymous namespace

TEST_CASE("slot_map", "[beyond.core.container.slot_map]")
{
  GIVEN("A default constructed slot_map")
  {
    beyond::slot_map<DummyHandle, int> ints;
    REQUIRE(ints.empty());
    REQUIRE(ints.size() == 0);

    WHEN("Reserve 42 elements to it")
    {
      ints.reserve(42);
      const auto capacity = ints.capacity();
      THEN("Its capacity should >= 42")
      {
        REQUIRE(capacity >= 42);
      }
      THEN("Its size is still 0")
      {
        REQUIRE(ints.size() == 0);
      }
      AND_WHEN("Reserve to a number less than current capacity")
      {
        ints.reserve(21);
        THEN("The capacity does not change")
        {
          REQUIRE(capacity == ints.capacity());
        }
      }
    }

    WHEN("Inserts 42 to it")
    {
      const auto value = 42;
      const auto k = ints.insert(value);

      THEN("Its size is 1")
      {
        REQUIRE(!ints.empty());
        REQUIRE(ints.size() == 1);
      }

      THEN("Can find 42 by the key returned")
      {
        const auto itr = ints.find(k);
        REQUIRE(itr != std::end(ints));
        REQUIRE(*itr == value);
      }

      AND_WHEN("Inserts 21 to it")
      {
        const auto value2 = 21;
        const auto k2 = ints.insert(value2);

        THEN("Its size is 2")
        {
          REQUIRE(ints.size() == 2);
        }

        THEN("Can find 21 by the key returned")
        {
          const auto itr = ints.find(k2);
          REQUIRE(itr != std::end(ints));
          REQUIRE(*itr == value2);
        }
      }
    }
  }
}
