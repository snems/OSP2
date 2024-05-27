#include <catch2/catch.hpp>

#include "../cgame/cg_local.h"


TEST_CASE("Test sructures: player colors set, color1 is one character", "[API][CG_PlayerColorsFromCS]")
{
  playerColors_t colors{};
  playerColorsOverride_t override{};

  CG_PlayerColorsFromCS(&colors, &override, "1", "");

  CHECK(override.isRailColorSet == qtrue);
}

TEST_CASE("Test sructures: player colors set, color1 four characters", "[API][CG_PlayerColorsFromCS]")
{
  playerColors_t colors{};
  playerColorsOverride_t override{};

  CG_PlayerColorsFromCS(&colors, &override, "1234", "");

  CHECK(override.isRailColorSet == qtrue);
}

TEST_CASE("Test sructures: player colors set, color1 four characters, first zero", "[API][CG_PlayerColorsFromCS]")
{
  playerColors_t colors{};
  playerColorsOverride_t override{};

  CG_PlayerColorsFromCS(&colors, &override, "0234", "");

  CHECK(override.isRailColorSet == qfalse);
}
