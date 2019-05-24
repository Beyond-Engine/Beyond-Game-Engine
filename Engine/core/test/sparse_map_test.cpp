#include "core/ecs/sparse_set.hpp"

namespace beyond {

template <typename Entity, typename T> class SparseMap {
public:
  using Trait = EntityTrait<Entity>;
  using SizeType = typename Trait::Id;
  using DiffType = typename Trait::DiffType;

  [[nodiscard]] auto empty() const noexcept -> bool
  {
    return entities_.empty();
  }

  [[nodiscard]] auto size() const noexcept -> SizeType
  {
    return entities_.size();
  }

  [[nodiscard]] auto capacity() const noexcept -> SizeType
  {
    return entities_.capacity();
  }

private:
  SparseSet<Entity> entities_;
};

} // namespace beyond

#include <catch2/catch.hpp>

using namespace beyond;

TEST_CASE("SparseMap", "[beyond.core.ecs.sparse_set]")
{
  using Entity = std::uint32_t;
  SparseMap<Entity, float> sm;
  REQUIRE(sm.empty());
  REQUIRE(sm.size() == 0);
  REQUIRE(sm.capacity() == 0);
}
