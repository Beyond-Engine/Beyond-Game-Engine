#include <catch2/catch.hpp>

#include <beyond/graphics/backend.hpp>

#include "mock_backend.hpp"

using namespace beyond::graphics;

TEMPLATE_TEST_CASE("default constructed Mapping", "[beyond.graphics.backend]",
                   Mapping<int>, const Mapping<int>)
{
  TestType mapping;
  REQUIRE(!mapping);
  REQUIRE(mapping.data() == nullptr);
}

TEST_CASE("Mapping from a backend")
{
  MockContext context;

  GIVEN("A buffer created from context")
  {
    const auto buffer_size = 256;
    BufferCreateInfo info{.size = buffer_size * sizeof(int)};
    auto buffer = context.create_buffer(info);

    WHEN("Map memory from context")
    {
      auto mapping = context.map_memory<int>(buffer);
      REQUIRE(mapping);

      AND_THEN("Modifying a mapped value will persist between mapping")
      {
        *mapping = 1;
        mapping.release();
        REQUIRE(!mapping);
        mapping = context.map_memory<int>(buffer);
        REQUIRE(mapping);
        REQUIRE(*mapping == 1);
      }
    }
  }
}
