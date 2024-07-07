#include <catch2/catch.hpp>
#include <cstring>

#include "../cgame/cg_local.h"
#include "../qcommon/qcommon.h"


TEST_CASE("Prepare string", "[cgame][cg_drawtools.c]")
{
  char out[1024];
  Com_InitZoneMemory();

  CG_OSPDrawStringPrepare("text", out, 1024);
  CHECK(strncmp("text", out, 4) == 0);

  CG_OSPDrawStringPrepare("text", out, 3);
  CHECK(strncmp("te", out, 4) == 0);

  cg.time = 1;
  CG_OSPDrawStringPrepare("text^Ftext2", out, 1024);
  CHECK(strncmp("text", out, 4) == 0);

  cg.time = 1023;
  CG_OSPDrawStringPrepare("text^Ftext2", out, 1024);
  CHECK(strncmp("texttext2", out, 1024) == 0);

  cg.time = 1023;
  CG_OSPDrawStringPrepare("text^ftext2", out, 1024);
  CHECK(strncmp("text", out, 4) == 0);

  cg.time = 1;
  CG_OSPDrawStringPrepare("text^ftext2", out, 1024);
  CHECK(strncmp("texttext2", out, 1024) == 0);

  cg.time = 1;
  CG_OSPDrawStringPrepare("text^Ftext2^Ntext^13", out, 1024);
  CHECK(strncmp("text^Ntext^13", out, 4) == 0);

  cg.time = 1023;
  CG_OSPDrawStringPrepare("text^Ftext2^Ntext^13", out, 1024);
  CHECK(strncmp("texttext2^Ntext^13", out, 1024) == 0);

  cg.time = 1023;
  CG_OSPDrawStringPrepare("text^ftext2^Ntext^13", out, 1024);
  CHECK(strncmp("text^Ntext^13", out, 4) == 0);

  cg.time = 1;
  CG_OSPDrawStringPrepare("text^ftext2^Ntext^13", out, 1024);
  CHECK(strncmp("texttext2^Ntext^13", out, 1024) == 0);
}

TEST_CASE("Text compiler", "[API][cg_drawtools.c]")
{
  Com_InitZoneMemory();
  CHECK(CG_CompiledTextCreate(NULL) == NULL);
  CHECK(CG_CompiledTextCreate("") == NULL);

  {
    text_command_t *commands = CG_CompiledTextCreate("text");
    REQUIRE(commands);
    
    CHECK(commands[0].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[0].value.character == 't');

    CHECK(commands[1].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[1].value.character == 'e');

    CHECK(commands[2].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[2].value.character == 'x');

    CHECK(commands[3].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[3].value.character == 't');

    CHECK(commands[4].type == OSP_TEXT_CMD_STOP);

    CG_CompiledTextDestroy(commands);
  }

  {
    text_command_t *commands = CG_CompiledTextCreate("te^fxt");
    REQUIRE(commands);
    
    CHECK(commands[0].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[0].value.character == 't');

    CHECK(commands[1].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[1].value.character == 'e');

    CHECK(commands[2].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[2].value.character == 'x');

    CHECK(commands[3].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[3].value.character == 't');

    CHECK(commands[4].type == OSP_TEXT_CMD_STOP);

    CG_CompiledTextDestroy(commands);
  }

  {
    cg.time = 1023;
    text_command_t *commands = CG_CompiledTextCreate("te^Fxt");
    REQUIRE(commands);
    
    CHECK(commands[0].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[0].value.character == 't');

    CHECK(commands[1].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[1].value.character == 'e');

    CHECK(commands[2].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[2].value.character == 'x');

    CHECK(commands[3].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[3].value.character == 't');

    CHECK(commands[4].type == OSP_TEXT_CMD_STOP);

    CG_CompiledTextDestroy(commands);
  }

  {
    text_command_t *commands = CG_CompiledTextCreate("te^bxt");
    REQUIRE(commands);
    
    CHECK(commands[0].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[0].value.character == 't');

    CHECK(commands[1].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[1].value.character == 'e');

    CHECK(commands[2].type == OSP_TEXT_CMD_FADE);

    CHECK(commands[3].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[3].value.character == 'x');

    CHECK(commands[4].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[4].value.character == 't');

    CHECK(commands[5].type == OSP_TEXT_CMD_STOP);

    CG_CompiledTextDestroy(commands);
  }

  {
    text_command_t *commands = CG_CompiledTextCreate("te^Bxt");
    REQUIRE(commands);
    
    CHECK(commands[0].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[0].value.character == 't');

    CHECK(commands[1].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[1].value.character == 'e');

    CHECK(commands[2].type == OSP_TEXT_CMD_FADE);

    CHECK(commands[3].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[3].value.character == 'x');

    CHECK(commands[4].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[4].value.character == 't');

    CHECK(commands[5].type == OSP_TEXT_CMD_STOP);

    CG_CompiledTextDestroy(commands);
  }

  {
    text_command_t *commands = CG_CompiledTextCreate("te^Bxt^N");
    REQUIRE(commands);
    
    CHECK(commands[0].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[0].value.character == 't');

    CHECK(commands[1].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[1].value.character == 'e');

    CHECK(commands[2].type == OSP_TEXT_CMD_FADE);

    CHECK(commands[3].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[3].value.character == 'x');

    CHECK(commands[4].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[4].value.character == 't');

    CHECK(commands[5].type == OSP_TEXT_CMD_FADE);

    CHECK(commands[6].type == OSP_TEXT_CMD_TEXT_COLOR);

    CHECK(commands[7].type == OSP_TEXT_CMD_STOP);

    CG_CompiledTextDestroy(commands);
  }
  {
    text_command_t *commands = CG_CompiledTextCreate("^i3");
    REQUIRE(commands);
    
    CHECK(commands[0].type == OSP_TEXT_CMD_TEXT_COLOR);

    CHECK(commands[1].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[1].value.character == '3');

    CHECK(commands[2].type == OSP_TEXT_CMD_STOP);

    CG_CompiledTextDestroy(commands);
  }

  {
    text_command_t *commands = CG_CompiledTextCreate("te^X00FF00x^Nt");
    REQUIRE(commands);
    
    CHECK(commands[0].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[0].value.character == 't');

    CHECK(commands[1].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[1].value.character == 'e');

    CHECK(commands[2].type == OSP_TEXT_CMD_SHADOW_COLOR);

    CHECK(commands[3].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[3].value.character == 'x');

    CHECK(commands[4].type == OSP_TEXT_CMD_FADE);

    CHECK(commands[5].type == OSP_TEXT_CMD_TEXT_COLOR);

    CHECK(commands[6].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[6].value.character == 't');

    CHECK(commands[7].type == OSP_TEXT_CMD_STOP);

    CG_CompiledTextDestroy(commands);
  }

  {
    text_command_t *commands = CG_CompiledTextCreate("^xfebd36^n^x800000Kerrazzy");
    REQUIRE(commands);
    
    CHECK(commands[0].type == OSP_TEXT_CMD_SHADOW_COLOR);
    CHECK(commands[1].type == OSP_TEXT_CMD_FADE);
    CHECK(commands[2].type == OSP_TEXT_CMD_TEXT_COLOR);
    CHECK(commands[3].type == OSP_TEXT_CMD_SHADOW_COLOR);

    CHECK(commands[4].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[4].value.character == 'K');

    CHECK(commands[5].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[5].value.character == 'e');

    CG_CompiledTextDestroy(commands);
  }
  {
    text_command_t *commands = CG_CompiledTextCreate("^0test^7");
    REQUIRE(commands);
    
    CHECK(commands[0].type == OSP_TEXT_CMD_TEXT_COLOR);
    CHECK(commands[1].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[1].value.character == 't');
    CHECK(commands[2].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[2].value.character == 'e');
    CHECK(commands[3].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[3].value.character == 's');
    CHECK(commands[4].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[4].value.character == 't');
    CHECK(commands[5].type == OSP_TEXT_CMD_TEXT_COLOR);

    CG_CompiledTextDestroy(commands);
  }
  
  {
    text_command_t *commands = CG_CompiledTextCreate("^b ^1x^2A^1");
    REQUIRE(commands);
    
    CHECK(commands[0].type == OSP_TEXT_CMD_FADE);
    
    CHECK(commands[1].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[1].value.character == ' ');

    CHECK(commands[2].type == OSP_TEXT_CMD_TEXT_COLOR);

    CHECK(commands[3].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[3].value.character == 'x');

    CHECK(commands[4].type == OSP_TEXT_CMD_TEXT_COLOR);

    CHECK(commands[5].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[5].value.character == 'A');

    CHECK(commands[6].type == OSP_TEXT_CMD_TEXT_COLOR);

    CG_CompiledTextDestroy(commands);
  }
}

