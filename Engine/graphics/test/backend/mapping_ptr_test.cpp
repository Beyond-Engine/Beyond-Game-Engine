#include <catch2/catch.hpp>

#include <beyond/graphics/backend.hpp>

#include "mock_backend.hpp"

using namespace beyond::graphics;

TEMPLATE_TEST_CASE("default constructed MappingPtr",
                   "[beyond.graphics.backend]", MappingPtr<int>,
                   const MappingPtr<int>)
{
  TestType ptr;
  REQUIRE(!ptr);
  REQUIRE(ptr.get() == nullptr);
}

TEST_CASE("MappingPtr from a backend")
{
  MockContext context;

  GIVEN("A buffer created from context")
  {
    const auto buffer_size = 256;
    BufferCreateInfo info{.size = buffer_size * sizeof(int)};

    auto buffer = context.create_buffer(info);

    WHEN("Map memory from context")
    {
      auto ptr = context.map_memory<int>(buffer);
      REQUIRE(ptr);

      AND_THEN("Modifying a mapped value will persist between mapping")
      {
        *ptr = 1;
        ptr.release();
        REQUIRE(!ptr);
        ptr = context.map_memory<int>(buffer);
        REQUIRE(ptr);
        REQUIRE(*ptr == 1);
      }
    }
  }
}
