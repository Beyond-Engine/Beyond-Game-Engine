#include <catch2/catch.hpp>

#include <sstream>

#include "beyond/core/math/angle.hpp"
#include "beyond/core/math/constants.hpp"

using namespace beyond;

using beyond::float_constants::pi;

TEST_CASE("Radian", "[beyond.core.math.angle]")
{
  SECTION("Default construction")
  {
    Radian<float> rad;
    REQUIRE(rad.value() == Approx(0));
  }

  GIVEN("A Radian r1 = pi")
  {
    Radian<float> r1{pi};
    REQUIRE(r1.value() == Approx(pi));

    THEN("-r1 = -pi")
    {
      REQUIRE((-r1).value() == Approx(-pi));
    }

    AND_GIVEN("Another Radian r2 = pi / 2")
    {
      const Radian<float> r2{pi / 2};

      WHEN("r1 += r2")
      {
        r1 += r2;
        THEN("r1 = pi * 3 /2")
        {
          REQUIRE(r1.value() == Approx(pi * 3 / 2));
        }
      }
      WHEN("r1 -= r2")
      {
        r1 -= r2;
        THEN("r1 = pi * 1 /2")
        {
          REQUIRE(r1.value() == Approx(pi * 1 / 2));
        }
      }

      THEN("r1 == r1")
      {
        REQUIRE(r1 == r1);
      }

      THEN("r1 != r2")
      {
        REQUIRE(r1 != r2);
      }

      THEN("r2 < r1")
      {
        REQUIRE(r2 < r1);
      }

      THEN("r2 <= r1")
      {
        REQUIRE(r2 <= r1);
      }

      THEN("r1 > r2")
      {
        REQUIRE(r1 > r2);
      }

      THEN("r1 >= r2")
      {
        REQUIRE(r1 >= r2);
      }

      THEN("r1 + r2 = pi * 3 / 2")
      {
        const auto result = r1 + r2;
        REQUIRE(result.value() == Approx(pi * 3 / 2));
      }

      THEN("r1 - r2 = pi * 1 / 2")
      {
        const auto result = r1 - r2;
        REQUIRE(result.value() == Approx(pi * 1 / 2));
      }

      THEN("r1 / r2 = 2")
      {
        REQUIRE((r1 / r2) == Approx(2));
      }
    }

    AND_GIVEN("A scalar s = 2")
    {
      const float s = 2;
      WHEN("r1 *= s")
      {
        r1 *= s;
        THEN("r1 = pi * s")
        {
          REQUIRE(r1.value() == Approx(pi * s));
        }
      }
      WHEN("r1 /= s")
      {
        r1 /= s;
        THEN("r1 = pi / s")
        {
          REQUIRE(r1.value() == Approx(pi / s));
        }
      }

      THEN("r1 * s = s * pi")
      {
        REQUIRE((r1 * s).value() == Approx(pi * s));
        REQUIRE((s * r1).value() == Approx(pi * s));
      }

      THEN("r1 / s = pi / s")
      {
        REQUIRE((r1 / s).value() == Approx(pi / s));
      }
    }
  }

  SECTION("Create Radian by a literal")
  {
    REQUIRE((1._rad).value() == Approx(1));
  }

  SECTION("Converts from radian of another type")
  {
    const Radian<double> r{static_cast<double>(pi)};
    REQUIRE(Radian<float>{r}.value() == Approx(pi));
  }
}

TEST_CASE("Degree", "[beyond.core.math.angle]")
{

  SECTION("Default construction")
  {
    Degree<float> deg;
    REQUIRE(deg.value() == Approx(0));
  }

  GIVEN("A Degree d1 = 90_deg")
  {
    Degree<float> d1{90};
    REQUIRE(d1.value() == Approx(90));

    THEN("-d1 = -90")
    {
      REQUIRE((-d1).value() == Approx(-90));
    }

    AND_GIVEN("Another Degree d2 = 45_deg")
    {
      const Degree<float> d2{45};

      WHEN("d1 += d2")
      {
        d1 += d2;
        THEN("d1 = 135_deg")
        {
          REQUIRE(d1.value() == Approx(135));
        }
      }
      WHEN("d1 -= d2")
      {
        d1 -= d2;
        THEN("d1 = 45")
        {
          REQUIRE(d1.value() == Approx(45));
        }
      }

      THEN("d1 == d1")
      {
        REQUIRE(d1 == d1);
      }

      THEN("d1 != d2")
      {
        REQUIRE(d1 != d2);
      }

      THEN("d2 < d1")
      {
        REQUIRE(d2 < d1);
      }

      THEN("d2 <= d1")
      {
        REQUIRE(d2 <= d1);
      }

      THEN("d1 > d2")
      {
        REQUIRE(d1 > d2);
      }

      THEN("d1 >= d2")
      {
        REQUIRE(d1 >= d2);
      }

      THEN("d1 + d2 = 135_deg")
      {
        const auto result = d1 + d2;
        REQUIRE(result.value() == Approx(135));
      }

      THEN("d1 - d2 = 45_deg")
      {
        const auto result = d1 - d2;
        REQUIRE(result.value() == Approx(45));
      }

      THEN("d1 / d2 = 2")
      {
        REQUIRE((d1 / d2) == Approx(2));
      }
    }

    AND_GIVEN("A scalar s = 1.5")
    {
      const float s = 1.5f;
      WHEN("d1 *= s")
      {
        d1 *= s;
        THEN("d1 = s * 90")
        {
          REQUIRE(d1.value() == Approx(s * 90));
        }
      }
      WHEN("d1 /= s")
      {
        d1 /= s;
        THEN("d1 = 90 / s")
        {
          REQUIRE(d1.value() == Approx(90 / s));
        }
      }

      THEN("d1 * s = 90 * s")
      {
        REQUIRE((d1 * s).value() == Approx(90 * s));
        REQUIRE((s * d1).value() == Approx(90 * s));
      }

      THEN("d1 / s = 0.5 * s")
      {
        REQUIRE((d1 / s).value() == Approx(90 / s));
      }
    }
  }

  SECTION("Create Degree by a literal")
  {
    REQUIRE((45._deg).value() == Approx(45));
  }

  SECTION("Converts from Degree of another type")
  {
    const Degree<double> r{60};
    REQUIRE(Degree<float>{r}.value() == Approx(60));
  }
}

TEST_CASE("Conversion between Radian and Degree", "[beyond.core.math.angle]")
{
  GIVEN("A Radian pi")
  {
    const Radian<float> r{pi};
    THEN("Create a Degree 180_deg from that radian")
    {
      REQUIRE(Degree<float>{r}.value() == Approx(180));
    }
  }

  GIVEN("A Degree 90")
  {
    const Degree<float> d{90};
    THEN("Create a Radian pi/2 from that degree")
    {
      REQUIRE(Radian<float>{d}.value() == Approx(pi / 2));
    }
  }
}

TEST_CASE("Streaming test", "[beyond.core.math.angle]")
{
  std::stringstream ss;
  SECTION("Output Radian to a stream")
  {
    const Radian<float> r{1};
    ss << r;
    REQUIRE(ss.str() == "1_radian");
  }

  SECTION("Output Degree to a stream")
  {
    const Degree<float> r{90};
    ss << r;
    REQUIRE(ss.str() == "90_degree");
  }
}
