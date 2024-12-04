#include <catch2/catch_test_macros.hpp>
#include "../cgame/cg_local.h"


TEST_CASE("Test cg_customloc: parse tinfo", "[cgame][cg_customloc]")
{
  vec3_t loc;
  CG_CustomLocationsSetLocation ("23_-527_-541_198", loc);
  CHECK(loc[0] == -527.0);
  CHECK(loc[1] == -541.0);
  CHECK(loc[2] == 198.0);

  CG_CustomLocationsSetLocation ("23", loc);
  CHECK(loc[0] == 0.0);
  CHECK(loc[1] == 0.0);
  CHECK(loc[2] == 0.0);
}

