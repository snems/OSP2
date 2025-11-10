#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include "../cgame/cg_superhud_private.h"
#include "../qcommon/qcommon.h"

namespace
{
  bool initialized = false;
  void shud_test_init()
  {
    if (initialized) return;
    Com_InitZoneMemory();
    CG_SHUDParserInit();
    initialized = true;
  }

}


TEST_CASE("Test SuperHUD: test split config to lines", "[cgame][CG_SHUDConfigFileSplitToLines]")
{
  const char *config0 = "";
  const char *config1 = "line1\n line22\n line333";
  const char *config2 = "line1\n line22\n line333\n";
  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  result = CG_SHUDFileInfoInit(&info, config0);
  CHECK(result == qfalse);


  CG_SHUDFileInfoTeardown(&info);
  result = CG_SHUDFileInfoInit(&info, config1);
  REQUIRE(result == qtrue);
  CHECK(info.root);

  REQUIRE(info.last_line);
  CHECK(info.last_line->line_number == 1);
  CHECK(info.last_line->size == 5);
  CHECK(strcmp(info.last_line->line, "line1") == 0);

  info.last_line = info.last_line->next;
  REQUIRE(info.last_line);
  CHECK(info.last_line->line_number == 2);
  CHECK(info.last_line->size == 7);
  CHECK(strcmp(info.last_line->line, " line22") == 0);

  info.last_line = info.last_line->next;
  REQUIRE(info.last_line);
  CHECK(info.last_line->line_number == 3);
  CHECK(info.last_line->size == 8);
  CHECK(strcmp(info.last_line->line, " line333") == 0);

  CG_SHUDFileInfoTeardown(&info);

  result = CG_SHUDFileInfoInit(&info, config2);
  CHECK(result == qtrue);
  CHECK(info.root);

  REQUIRE(info.last_line);
  CHECK(info.last_line->line_number == 1);
  CHECK(info.last_line->size == 5);
  CHECK(strcmp(info.last_line->line, "line1") == 0);

  info.last_line = info.last_line->next;
  REQUIRE(info.last_line);
  CHECK(info.last_line->line_number == 2);
  CHECK(info.last_line->size == 7);
  CHECK(strcmp(info.last_line->line, " line22") == 0);

  info.last_line = info.last_line->next;
  REQUIRE(info.last_line);
  CHECK(info.last_line->line_number == 3);
  CHECK(info.last_line->size == 8);
  CHECK(strcmp(info.last_line->line, " line333") == 0);
  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: find element in dictonary", "[cgame][cg_superhud_configparser]")
{
  shud_test_init();
  auto result = CG_SHUDFindConfigElementItem("fragmessage");

  REQUIRE(result);
  CHECK(std::strcmp(result->name, "fragmessage") == 0);

}

TEST_CASE("Test SuperHUD: get element from config", "[cgame][cg_superhud_configparser]")
{
  const char *config0 = "   \n    \n    ";
  const char *config1 = " !default\n {}\n fps {}";
  const char *config2 = " !777\n {}\n fps {}";
  const char *config3 = " !default\n ";
  const char *config4 = " !default d {}\n ";
  const char *config5 = "\x0d\x0a!default\n {}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  // no elements
  result = CG_SHUDFileInfoInit(&info, config0);
  CHECK(result == qtrue);

  auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
  CHECK(elemResult.status == SUPERHUD_CONFIG_END_OF_FILE);

  CG_SHUDFileInfoTeardown(&info);

  // default element
  result = CG_SHUDFileInfoInit(&info, config1);
  CHECK(result == qtrue);

  elemResult = CG_SHUDFileInfoGetElementItem(&info);
  CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

  REQUIRE(elemResult.item);

  CHECK(std::strcmp(elemResult.item->name, "!default") == 0);

  CG_SHUDFileInfoTeardown(&info);

  // unknown element
  result = CG_SHUDFileInfoInit(&info, config2);
  CHECK(result == qtrue);

  elemResult = CG_SHUDFileInfoGetElementItem(&info);
  CHECK(elemResult.status == SUPERHUD_CONFIG_WRONG_ELEMENT_NAME);

  CG_SHUDFileInfoTeardown(&info);

  // unknown element
  result = CG_SHUDFileInfoInit(&info, config3);
  CHECK(result == qtrue);

  elemResult = CG_SHUDFileInfoGetElementItem(&info);
  CHECK(elemResult.status == SUPERHUD_CONFIG_END_OF_FILE);

  CG_SHUDFileInfoTeardown(&info);

  // unknown element
  result = CG_SHUDFileInfoInit(&info, config4);
  CHECK(result == qtrue);

  elemResult = CG_SHUDFileInfoGetElementItem(&info);
  CHECK(elemResult.status == SUPERHUD_CONFIG_UNEXPECTED_CHARACTER);

  CG_SHUDFileInfoTeardown(&info);

  // newline
  result = CG_SHUDFileInfoInit(&info, config5);
  CHECK(result == qtrue);

  elemResult = CG_SHUDFileInfoGetElementItem(&info);
  CHECK(elemResult.status == SUPERHUD_CONFIG_OK);
  REQUIRE(elemResult.item);

  CHECK(std::strcmp(elemResult.item->name, "!default") == 0);

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: get command from config", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {color 1 1 1 1}\n fps {}";
  const char *config2 = " !default\n {}\n fps {}";
  const char *config3 = " !default\n {";
  const char *config4 = " !default\n {blabla}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  // default element
  result = CG_SHUDFileInfoInit(&info, config1);
  CHECK(result == qtrue);

  auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
  CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

  auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
  CHECK(comResult.status == SUPERHUD_CONFIG_OK);

  REQUIRE(comResult.item);

  CHECK(std::strcmp(comResult.item->name, "color") == 0);

  CG_SHUDFileInfoTeardown(&info);

  // End of element
  result = CG_SHUDFileInfoInit(&info, config2);
  CHECK(result == qtrue);

  elemResult = CG_SHUDFileInfoGetElementItem(&info);
  CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

  comResult = CG_SHUDFileInfoGetCommandItem(&info);
  CHECK(comResult.status == SUPERHUD_CONFIG_END_OF_ELEMENT);

  CG_SHUDFileInfoTeardown(&info);

  // eof
  result = CG_SHUDFileInfoInit(&info, config3);
  CHECK(result == qtrue);

  elemResult = CG_SHUDFileInfoGetElementItem(&info);
  CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

  comResult = CG_SHUDFileInfoGetCommandItem(&info);
  CHECK(comResult.status == SUPERHUD_CONFIG_END_OF_FILE);

  CG_SHUDFileInfoTeardown(&info);

  // unknown element
  result = CG_SHUDFileInfoInit(&info, config4);
  CHECK(result == qtrue);

  elemResult = CG_SHUDFileInfoGetElementItem(&info);
  CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

  comResult = CG_SHUDFileInfoGetCommandItem(&info);
  CHECK(comResult.status == SUPERHUD_CONFIG_WRONG_COMMAND_NAME);

  CG_SHUDFileInfoTeardown(&info);

}

TEST_CASE("Test SuperHUD: parse COLOR command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {COLOR T; color E\n color I}";
  const char *config2 = " !default\n {COLOR 0.1 0.2 0.3 0.4}\n fps {}";
  const char *config3 = " !default\n {COLOR 0.1 0.2 0.3}\n fps {}";
  const char *config4 = " !default\n {COLOR 0.1 0.2}\n fps {}";
  const char *config5 = " !default\n {COLOR 0.1}\n fps {}";
  const char *config6 = " !default\n {COLOR }\n fps {}";
  superhudConfig_t config{}; 

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  SECTION("TEI")
  {
    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    // color T
    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "color") == 0);

    REQUIRE(comResult.item->parse);

    config.color.isSet = qfalse;
    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.color.isSet == qtrue);

    CHECK(config.color.value.type == SUPERHUD_COLOR_T);

    // color E
    comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "color") == 0);

    REQUIRE(comResult.item->parse);

    config.color.isSet = qfalse;
    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.color.isSet == qtrue);

    CHECK(config.color.value.type == SUPERHUD_COLOR_E);

    // color I
    comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "color") == 0);

    REQUIRE(comResult.item->parse);

    config.color.isSet = qfalse;
    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.color.isSet == qtrue);

    CHECK(config.color.value.type == SUPERHUD_COLOR_I);

    // End of element
    comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_END_OF_ELEMENT);

    // End of file
    elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_END_OF_FILE);
  }

  SECTION("color rgba command")
  {
    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "color") == 0);

    REQUIRE(comResult.item->parse);

    config.color.isSet = qfalse;
    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.color.isSet == qtrue);

    CHECK(config.color.value.type == SUPERHUD_COLOR_RGBA);
    CHECK(config.color.value.rgba[0] == 0.1f);
    CHECK(config.color.value.rgba[1] == 0.2f);
    CHECK(config.color.value.rgba[2] == 0.3f);
    CHECK(config.color.value.rgba[3] == 0.4f);
  }

  SECTION("color rgba command, wrong args")
  {
    SECTION("variant 1")
    {
      result = CG_SHUDFileInfoInit(&info, config3);
    }
    SECTION("variant 2")
    {
      result = CG_SHUDFileInfoInit(&info, config4);
    }
    SECTION("variant 3")
    {
      result = CG_SHUDFileInfoInit(&info, config5);
    }
    SECTION("variant 4")
    {
      result = CG_SHUDFileInfoInit(&info, config6);
    }
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "color") == 0);

    REQUIRE(comResult.item->parse);

    config.color.isSet = qtrue;
    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_UNEXPECTED_CHARACTER);
    CHECK(config.color.isSet == qfalse);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse ALIGNH command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {ALIGNH L}";
  const char *config2 = " !default\n {ALIGNH C}";
  const char *config3 = " !default\n {ALIGNH R}";
  const char *config4 = " !default\n {ALIGNH B}";


  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "alignh") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.alignH.isSet == qtrue);

    CHECK(config.alignH.value == SUPERHUD_ALIGNH_LEFT);
  }
  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "alignh") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.alignH.isSet == qtrue);

    CHECK(config.alignH.value == SUPERHUD_ALIGNH_CENTER);
  }
  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config3);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "alignh") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.alignH.isSet == qtrue);

    CHECK(config.alignH.value == SUPERHUD_ALIGNH_RIGHT);
  }
  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config4);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "alignh") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_UNEXPECTED_CHARACTER);
    CHECK(config.alignH.isSet == qfalse);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse ALIGNV command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {ALIGNV T}";
  const char *config2 = " !default\n {ALIGNV C}";
  const char *config3 = " !default\n {ALIGNV B}";
  const char *config4 = " !default\n {ALIGNV Z}";


  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "alignv") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.alignV.isSet == qtrue);

    CHECK(config.alignV.value == SUPERHUD_ALIGNV_TOP);
  }
  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "alignv") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.alignV.isSet == qtrue);

    CHECK(config.alignV.value == SUPERHUD_ALIGNV_CENTER);
  }
  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config3);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "alignv") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.alignV.isSet == qtrue);

    CHECK(config.alignV.value == SUPERHUD_ALIGNV_BOTTOM);
  }
  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config4);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "alignv") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_UNEXPECTED_CHARACTER);
    CHECK(config.alignV.isSet == qfalse);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse ANGLES command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {ANGLES 0.1 0.2 0.3 -0.4}\n fps {}";
  const char *config2 = " !default\n {ANGLES 0.1 0.2 0.3}\n fps {}";
  const char *config3 = " !default\n {ANGLES 0.1 0.2}\n fps {}";
  const char *config4 = " !default\n {ANGLES}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "angles") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.angles.isSet == qtrue);

    CHECK(config.angles.value[0] == 0.1f);
    CHECK(config.angles.value[1] == 0.2f);
    CHECK(config.angles.value[2] == 0.3f);
    CHECK(config.angles.value[3] == -0.4f);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "angles") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.angles.isSet == qtrue);

    CHECK(config.angles.value[0] == 0.1f);
    CHECK(config.angles.value[1] == 0.2f);
    CHECK(config.angles.value[2] == 0.3f);
    CHECK(config.angles.value[3] == 0.0f);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config3);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "angles") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_UNEXPECTED_CHARACTER);
    CHECK(config.angles.isSet == qfalse);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config4);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "angles") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_UNEXPECTED_CHARACTER);
    CHECK(config.angles.isSet == qfalse);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse BGCOLOR command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {BGCOLOR 0.1 -0.2 0.3 -0.4}\n fps {}";
  const char *config2 = " !default\n {BGCOLOR 0.1 0.2 0.3}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "bgcolor") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.bgcolor.isSet == qtrue);

    CHECK(config.bgcolor.value[0] == 0.1f);
    CHECK(config.bgcolor.value[1] == -0.2f);
    CHECK(config.bgcolor.value[2] == 0.3f);
    CHECK(config.bgcolor.value[3] == -0.4f);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "bgcolor") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) != SUPERHUD_CONFIG_OK);
    CHECK(config.bgcolor.isSet == qfalse);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse DIRECTION command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {DIRECTION L}\n fps {}";
  const char *config2 = " !default\n {DIRECTION R}\n fps {}";
  const char *config3 = " !default\n {DIRECTION T}\n fps {}";
  const char *config4 = " !default\n {DIRECTION B}\n fps {}";
  const char *config5 = " !default\n {DIRECTION X}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "direction") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.direction.isSet == qtrue);
    CHECK(config.direction.value == SUPERHUD_DIR_LEFT_TO_RIGHT);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "direction") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.direction.isSet == qtrue);
    CHECK(config.direction.value == SUPERHUD_DIR_RIGHT_TO_LEFT);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config3);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "direction") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.direction.isSet == qtrue);
    CHECK(config.direction.value == SUPERHUD_DIR_TOP_TO_BOTTOM);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config4);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "direction") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.direction.isSet == qtrue);
    CHECK(config.direction.value == SUPERHUD_DIR_BOTTOM_TO_TOP);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config5);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "direction") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) != SUPERHUD_CONFIG_OK);
    CHECK(config.direction.isSet == false);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse DOUBLEBAR command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {DOUBLEBAR}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  superhudConfig_t config{}; 

  result = CG_SHUDFileInfoInit(&info, config1);
  CHECK(result == qtrue);

  auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
  CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

  auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
  CHECK(comResult.status == SUPERHUD_CONFIG_OK);

  REQUIRE(comResult.item);

  CHECK(std::strcmp(comResult.item->name, "doublebar") == 0);

  REQUIRE(comResult.item->parse);

  CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
  CHECK(config.doublebar.isSet == qtrue);

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse FADE command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {FADE}\n fps {}";
  const char *config2 = " !default\n {FADE 0.1 0.2 0.3 0.4}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "fade") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.fade.isSet == qtrue);
    CHECK(config.fade.value[0] == 0.0f);
    CHECK(config.fade.value[1] == 0.0f);
    CHECK(config.fade.value[2] == 0.0f);
    CHECK(config.fade.value[3] == 0.0f);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "fade") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.fade.isSet == qtrue);
    CHECK(config.fade.value[0] == 0.1f);
    CHECK(config.fade.value[1] == 0.2f);
    CHECK(config.fade.value[2] == 0.3f);
    CHECK(config.fade.value[3] == 0.4f);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse FADEDELAY command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {FADEDELAY 123}\n fps {}";
  const char *config2 = " !default\n {FADEDELAY}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "fadedelay") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.fadedelay.isSet == qtrue);
    CHECK(config.fadedelay.value == 123);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "fadedelay") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) != SUPERHUD_CONFIG_OK);
    CHECK(config.fadedelay.isSet == qfalse);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse FILL command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {FILL}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  superhudConfig_t config{}; 

  result = CG_SHUDFileInfoInit(&info, config1);
  CHECK(result == qtrue);

  auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
  CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

  auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
  CHECK(comResult.status == SUPERHUD_CONFIG_OK);

  REQUIRE(comResult.item);

  CHECK(std::strcmp(comResult.item->name, "fill") == 0);

  REQUIRE(comResult.item->parse);

  CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
  CHECK(config.fill.isSet == qtrue);

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse FONT command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {FONT \"arial black\"}\n fps {}";
  const char *config2 = " !default\n {FONT arial black}\n fps {}";
  const char *config3 = " !default\n {FONT}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "font") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.font.isSet == qtrue);
    CHECK(strcmp(config.font.value, "arial black") == 0);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "font") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.font.isSet == qtrue);
    CHECK(strcmp(config.font.value, "arial") == 0);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config3);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "font") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) != SUPERHUD_CONFIG_OK);
    CHECK(config.font.isSet == false);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse FONTSIZE command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {FONTSIZE 123 456}\n fps {}";
  const char *config2 = " !default\n {FONTSIZE 123}\n fps {}";
  const char *config3 = " !default\n {FONTSIZE}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "fontsize") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.fontsize.isSet == qtrue);
    CHECK(config.fontsize.value[0] == 123.0f);
    CHECK(config.fontsize.value[1] == 456.0f);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "fontsize") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.fontsize.isSet == qtrue);
    CHECK(config.fontsize.value[0] == 123.0f);
    CHECK(config.fontsize.value[1] == 123.0f);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config3);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "fontsize") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) != SUPERHUD_CONFIG_OK);
    CHECK(config.fontsize.isSet == false);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse IMAGE command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {IMAGE \"/dev/null\"}\n fps {}";
  const char *config2 = " !default\n {IMAGE /dev/null}\n fps {}";
  const char *config3 = " !default\n {IMAGE}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "image") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.image.isSet == qtrue);
    CHECK(strcmp(config.image.value, "/dev/null") == 0);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "image") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.image.isSet == qtrue);
    CHECK(strcmp(config.image.value, "/dev/null") == 0);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config3);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "image") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) != SUPERHUD_CONFIG_OK);
    CHECK(config.image.isSet == false);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse IMAGETC command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {IMAGETC}\n fps {}";
  const char *config2 = " !default\n {IMAGETC 0.1 0.2 0.3 0.4}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "imagetc") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) != SUPERHUD_CONFIG_OK);
    CHECK(config.imagetc.isSet == qfalse);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "imagetc") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.imagetc.isSet == qtrue);
    CHECK(config.imagetc.value[0] == 0.1f);
    CHECK(config.imagetc.value[1] == 0.2f);
    CHECK(config.imagetc.value[2] == 0.3f);
    CHECK(config.imagetc.value[3] == 0.4f);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse ITTEAM command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {ITTEAM B}\n fps {}";
  const char *config2 = " !default\n {ITTEAM R}\n fps {}";
  const char *config3 = " !default\n {ITTEAM N}\n fps {}";
  const char *config4 = " !default\n {ITTEAM O}\n fps {}";
  const char *config5 = " !default\n {ITTEAM E}\n fps {}";
  const char *config6 = " !default\n {ITTEAM X}\n fps {}";
  const char *config7 = " !default\n {ITTEAM}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "itteam") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.itTeam.isSet == qtrue);
    CHECK(config.itTeam.value == SUPERHUD_ITTEAM_BLUE);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "itteam") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.itTeam.isSet == qtrue);
    CHECK(config.itTeam.value == SUPERHUD_ITTEAM_RED);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config3);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "itteam") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.itTeam.isSet == qtrue);
    CHECK(config.itTeam.value == SUPERHUD_ITTEAM_NEUTRAL);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config4);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "itteam") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.itTeam.isSet == qtrue);
    CHECK(config.itTeam.value == SUPERHUD_ITTEAM_OWN);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config5);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "itteam") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.itTeam.isSet == qtrue);
    CHECK(config.itTeam.value == SUPERHUD_ITTEAM_ENEMY);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config6);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "itteam") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) != SUPERHUD_CONFIG_OK);
    CHECK(config.itTeam.isSet == qfalse);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config7);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "itteam") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) != SUPERHUD_CONFIG_OK);
    CHECK(config.itTeam.isSet == qfalse);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse MARGINS command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {MARGINS}\n fps {}";
  const char *config2 = " !default\n {MARGINS 0.1 0.2 0.3 0.4}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "margins") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) != SUPERHUD_CONFIG_OK);
    CHECK(config.margins.isSet == qfalse);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "margins") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.margins.isSet == qtrue);
    CHECK(config.margins.value[0] == 0.1f);
    CHECK(config.margins.value[1] == 0.2f);
    CHECK(config.margins.value[2] == 0.3f);
    CHECK(config.margins.value[3] == 0.4f);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse MODEL command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {MODEL \"/dev/null\"}\n fps {}";
  const char *config2 = " !default\n {MODEL /dev/null}\n fps {}";
  const char *config3 = " !default\n {MODEL}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "model") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.model.isSet == qtrue);
    CHECK(strcmp(config.model.value, "/dev/null") == 0);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "model") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.model.isSet == qtrue);
    CHECK(strcmp(config.model.value, "/dev/null") == 0);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config3);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "model") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) != SUPERHUD_CONFIG_OK);
    CHECK(config.model.isSet == false);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse MONOSPACE command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {MONOSPACE}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  superhudConfig_t config{}; 

  result = CG_SHUDFileInfoInit(&info, config1);
  CHECK(result == qtrue);

  auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
  CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

  auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
  CHECK(comResult.status == SUPERHUD_CONFIG_OK);

  REQUIRE(comResult.item);

  CHECK(std::strcmp(comResult.item->name, "monospace") == 0);

  REQUIRE(comResult.item->parse);

  CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
  CHECK(config.monospace.isSet == qtrue);

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse OFFSET command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {OFFSET}\n fps {}";
  const char *config2 = " !default\n {OFFSET 0.1 0.2 0.3}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "offset") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) != SUPERHUD_CONFIG_OK);
    CHECK(config.offset.isSet == qfalse);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "offset") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.offset.isSet == qtrue);
    CHECK(config.offset.value[0] == 0.1f);
    CHECK(config.offset.value[1] == 0.2f);
    CHECK(config.offset.value[2] == 0.3f);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse TEXT command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {TEXT \"/dev/null\"}\n fps {}";
  const char *config2 = " !default\n {TEXT /dev/null}\n fps {}";
  const char *config3 = " !default\n {TEXT}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "text") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.text.isSet == qtrue);
    CHECK(strcmp(config.text.value, "/dev/null") == 0);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "text") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.text.isSet == qtrue);
    CHECK(strcmp(config.text.value, "/dev/null") == 0);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config3);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "text") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) != SUPERHUD_CONFIG_OK);
    CHECK(config.text.isSet == false);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse TEXTALIGN command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {TEXTALIGN L}";
  const char *config2 = " !default\n {TEXTALIGN C}";
  const char *config3 = " !default\n {TEXTALIGN R}";
  const char *config4 = " !default\n {TEXTALIGN B}";


  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "textalign") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.textAlign.isSet == qtrue);

    CHECK(config.textAlign.value == SUPERHUD_ALIGNH_LEFT);
  }
  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "textalign") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.textAlign.isSet == qtrue);

    CHECK(config.textAlign.value == SUPERHUD_ALIGNH_CENTER);
  }
  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config3);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "textalign") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.textAlign.isSet == qtrue);

    CHECK(config.textAlign.value == SUPERHUD_ALIGNH_RIGHT);
  }
  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config4);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "textalign") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_UNEXPECTED_CHARACTER);
    CHECK(config.textAlign.isSet == qfalse);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse TEXTOFFSET command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {TEXTOFFSET}\n fps {}";
  const char *config2 = " !default\n {TEXTOFFSET 0.1 0.2}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "textoffset") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) != SUPERHUD_CONFIG_OK);
    CHECK(config.textOffset.isSet == qfalse);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "textoffset") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.textOffset.isSet == qtrue);
    CHECK(config.textOffset.value[0] == 0.1f);
    CHECK(config.textOffset.value[1] == 0.2f);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse TEXTSTYLE command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {TEXTSTYLE 123}\n fps {}";
  const char *config2 = " !default\n {TEXTSTYLE}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "textstyle") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.textStyle.isSet == qtrue);
    CHECK(config.textStyle.value == 123);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "textstyle") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) != SUPERHUD_CONFIG_OK);
    CHECK(config.textStyle.isSet == qfalse);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse TIME command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {TIME 123}\n fps {}";
  const char *config2 = " !default\n {TIME}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "time") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.time.isSet == qtrue);
    CHECK(config.time.value == 123);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "time") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) != SUPERHUD_CONFIG_OK);
    CHECK(config.time.isSet == qfalse);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse VISFLAGS command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {VISFLAGS 123}\n fps {}";
  const char *config2 = " !default\n {VISFLAGS}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  // Tests disbled as I don't know what visflags is
  //

  if (0){ 
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "visflags") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.visflags.isSet == qtrue);
    CHECK(config.visflags.value == 123);
  }

  if (0){
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "visflags") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) != SUPERHUD_CONFIG_OK);
    CHECK(config.visflags.isSet == qfalse);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse HLSIZE command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {HLSIZE 123}\n fps {}";
  const char *config2 = " !default\n {HLSIZE}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "hlsize") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.hlsize.isSet == qtrue);
    CHECK(config.hlsize.value == 123);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "hlsize") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) != SUPERHUD_CONFIG_OK);
    CHECK(config.hlsize.isSet == qfalse);
  }

  CG_SHUDFileInfoTeardown(&info);
}

TEST_CASE("Test SuperHUD: parse HLCOLOR command", "[cgame][cg_superhud_configparser]")
{
  const char *config1 = " !default\n {HLCOLOR 0.1 -0.2 0.3 -0.4}\n fps {}";
  const char *config2 = " !default\n {HLCOLOR 0.1 0.2 0.3}\n fps {}";

  qboolean result;
  configFileInfo_t info{};

  shud_test_init();

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config1);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "hlcolor") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) == SUPERHUD_CONFIG_OK);
    CHECK(config.hlcolor.isSet == qtrue);
    CHECK(config.hlcolor.value[0] == 0.1f);
    CHECK(config.hlcolor.value[1] == -0.2f);
    CHECK(config.hlcolor.value[2] == 0.3f);
    CHECK(config.hlcolor.value[3] == -0.4f);
  }

  {
    superhudConfig_t config{}; 

    result = CG_SHUDFileInfoInit(&info, config2);
    CHECK(result == qtrue);

    auto elemResult = CG_SHUDFileInfoGetElementItem(&info);
    CHECK(elemResult.status == SUPERHUD_CONFIG_OK);

    auto comResult = CG_SHUDFileInfoGetCommandItem(&info);
    CHECK(comResult.status == SUPERHUD_CONFIG_OK);

    REQUIRE(comResult.item);

    CHECK(std::strcmp(comResult.item->name, "hlcolor") == 0);

    REQUIRE(comResult.item->parse);

    CHECK(comResult.item->parse(&info, &config) != SUPERHUD_CONFIG_OK);
    CHECK(config.hlcolor.isSet == qfalse);
  }

  CG_SHUDFileInfoTeardown(&info);
}

