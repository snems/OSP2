#include <catch2/catch_test_macros.hpp>
#include "../qcommon/qcommon.h"


TEST_CASE("Test Q_sscanf: return code", "[cgame][Q_sscanf]")
{
  int rc;
  int a, b;
  const char *str;
		//Q_sscanf(str, "%i %i", &cgs.scores1, &cgs.osp.osp_scores1);
  str = "10 20";
  rc = Q_sscanf(str, "%i %i", &a, &b);
  CHECK(rc == 2);
  CHECK(a == 10);
  CHECK(b == 20);

  str = "11";
  rc = Q_sscanf(str, "%i %i", &a, &b);
  CHECK(rc == 1);
  CHECK(a == 11);

}
