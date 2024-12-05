#include <catch2/catch_test_macros.hpp>
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
  CHECK(CG_CompileText(NULL) == NULL);
  CHECK(CG_CompileText("") == NULL);

  {
    text_command_t *commands = CG_CompileText("text");
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
    text_command_t *commands = CG_CompileText("te^fxt");
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
    text_command_t *commands = CG_CompileText("te^Fxt");
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
    text_command_t *commands = CG_CompileText("te^bxt");
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
    text_command_t *commands = CG_CompileText("te^Bxt");
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
    text_command_t *commands = CG_CompileText("te^Bxt^N");
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

    // reset color skipped because color was not set

    CHECK(commands[6].type == OSP_TEXT_CMD_STOP);

    CG_CompiledTextDestroy(commands);
  }
  {
    text_command_t *commands = CG_CompileText("^13");
    REQUIRE(commands);
    
    CHECK(commands[0].type == OSP_TEXT_CMD_TEXT_COLOR);

    CHECK(commands[1].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[1].value.character == '3');

    CHECK(commands[2].type == OSP_TEXT_CMD_STOP);

    CG_CompiledTextDestroy(commands);
  }

  {
    text_command_t *commands = CG_CompileText("te^X00FF00x^Nt");
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
    text_command_t *commands = CG_CompileText("^xfebd36^n^x800000Kerrazzy");
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
    text_command_t *commands = CG_CompileText("^x990000^bx0ry");
    REQUIRE(commands);
    
    CHECK(commands[0].type == OSP_TEXT_CMD_SHADOW_COLOR);
    CHECK(commands[1].type == OSP_TEXT_CMD_TEXT_COLOR);
    CHECK(commands[2].type == OSP_TEXT_CMD_FADE);

    CHECK(commands[3].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[3].value.character == 'x');

    CHECK(commands[4].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[4].value.character == '0');

    CG_CompiledTextDestroy(commands);
  }
  {
    text_command_t *commands = CG_CompileText("^0test^7");
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
    text_command_t *commands = CG_CompileText("^b ^1x^2A^1");
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

  {
    text_command_t *commands = CG_CompileText("Xaero");
    REQUIRE(commands);

    CHECK(commands[0].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[0].value.character == 'X');
    
    CHECK(commands[1].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[1].value.character == 'a');
    
    CHECK(commands[2].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[2].value.character == 'e');
    
    CHECK(commands[3].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[3].value.character == 'r');

    CHECK(commands[4].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[4].value.character == 'o');

    CG_CompiledTextDestroy(commands);
  }
}

TEST_CASE("Text compiler: bug ^^0 ", "[API][cg_drawtools.c]")
{
  Com_InitZoneMemory();
  CHECK(CG_CompileText(NULL) == NULL);
  CHECK(CG_CompileText("") == NULL);

  {
    text_command_t *commands = CG_CompileText("te^^0xt");
    REQUIRE(commands);
    
    CHECK(commands[0].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[0].value.character == 't');

    CHECK(commands[1].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[1].value.character == 'e');

    CHECK(commands[2].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[2].value.character == '^');

    CHECK(commands[3].type == OSP_TEXT_CMD_TEXT_COLOR);

    CHECK(commands[4].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[4].value.character == 'x');

    CHECK(commands[5].type == OSP_TEXT_CMD_CHAR);
    CHECK(commands[5].value.character == 't');

    CHECK(commands[6].type == OSP_TEXT_CMD_STOP);

    CG_CompiledTextDestroy(commands);
  }

}

