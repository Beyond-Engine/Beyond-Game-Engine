#include <catch2/catch.hpp>

#include <beyond/graphics/backend.hpp>

#include "mock_gpu_device.hpp"

TEST_CASE("Mapping from a the mock")
{
  using namespace beyond::graphics;

  MockGPUDevice context;

  GIVEN("A buffer created from context")
  {
    const auto buffer_size = 256;
    BufferCreateInfo info{.size = buffer_size * sizeof(int)};
    auto buffer = context.create_buffer(info);

    WHEN("Map memory from context")
    {
      auto* mapping = static_cast<int*>(context.map(buffer));
      REQUIRE(mapping);

      AND_THEN("Modifying a mapped value will persist between mapping")
      {
        *mapping = 1;
        context.unmap(buffer);
        mapping = nullptr;
        REQUIRE(!mapping);

        mapping = static_cast<int*>(context.map(buffer));
        REQUIRE(mapping);
        REQUIRE(*mapping == 1);
      }
    }
  }
}
