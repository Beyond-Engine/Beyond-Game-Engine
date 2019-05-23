#include <vector>

namespace beyond {

template <typename T, typename Key = std::size_t> class SlotMap {
public:
  SlotMap() = default;

  /// @brief Gets how many components are stored in the slot map
  [[nodiscard]] auto size() const noexcept -> std::size_t
  {
    return data_.size();
  }

  /// @brief Gets the capacity of the slot map
  [[nodiscard]] auto capacity() const noexcept -> std::size_t
  {
    return data_.capacity();
  }

  /// @brief Reserves the capacity of the slot map to `capacity`
  auto reserve(std::size_t capacity) -> void
  {
    data_.reserve(capacity);
  }

  /// @brief insert Inserts the value into a position specified by the key
  auto insert(Key key, T value) -> void
  {
    if (indices_.size() < (key + 1)) {
      indices_.resize(key + 1);
    }

    data_.push_back(std::move(value));
    indices_[key] = data_.size() - 1;
  }

  /// @brief Gets the element inside the key
  auto operator[](Key key) const noexcept -> const T&
  {
    return data_[indices_[key]];
  }

  /// @brief Gets the element inside the key
  auto operator[](Key key) noexcept -> T&
  {
    return data_[indices_[key]];
  }

private:
  std::vector<std::size_t> indices_;
  std::vector<T> data_;
};

} // namespace beyond

#include <catch2/catch.hpp>

using namespace beyond;

TEST_CASE("SlotMap Test", "[beyond.core.math.angle]")
{
  SlotMap<int> sm;
  REQUIRE(sm.size() == 0);
  REQUIRE(sm.capacity() == 0);

  sm.reserve(16);
  REQUIRE(sm.size() == 0);
  REQUIRE(sm.capacity() >= 16);

  sm.insert(3, 100);
  REQUIRE(sm.size() == 1);
  REQUIRE(sm[3] == 100);
}
