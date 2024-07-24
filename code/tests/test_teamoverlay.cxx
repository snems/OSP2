
#include <catch2/catch.hpp>
#include <iostream>
#include "../cgame/cg_superhud_private.h"

TEST_CASE("Test ch_teamOverlay parser", "[qcommon][memory][TeamOverlay]")
{
  shudTeamOverlay_t config{};
  const char *errmsg;

  auto result = shudElementCompileTeamOverlayConfig("p12n h/aw l", 5, 10, &config, &errmsg);
  CHECK(result);
  CHECK(config.powerupOffsetChar == 0);
  CHECK(config.powerupLenChar == 1);

  CHECK(config.nameOffsetChar == 1);
  CHECK(config.nameLenChar == 12);

  CHECK(config.healthOffsetChar == 14);
  CHECK(config.healthLenChar == 3);

  CHECK(config.armorOffsetChar == 18);
  CHECK(config.armorLenChar == 3);

  CHECK(config.ammoOffsetChar == 21);
  CHECK(config.ammoLenChar == 1);

  CHECK(config.locationOffsetChar == 23);
  CHECK(config.locationLenChar == 10);

  CHECK(config.overlayWidthChar == 33);
  CHECK(config.strX == 0);
}

