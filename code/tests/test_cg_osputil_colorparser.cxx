#include <catch2/catch_test_macros.hpp>
#include "../cgame/cg_local.h"

TEST_CASE("Test Util: color parser, fail cases", "[cgame][CG_ParseColorStr]")
{
  vec4_t result;

  CHECK(CG_ParseColorStr("a;sdlkfj", result) == qfalse);
  CHECK(CG_ParseColorStr("0xalsdkfj", result) == qfalse);
  CHECK(CG_ParseColorStr("xalsdkfj", result) == qfalse);
  CHECK(CG_ParseColorStr("#alsdkfj", result) == qfalse);
}

TEST_CASE("Test Util: color parser, one letter numbers", "[cgame][CG_ParseColorStr]")
{
  vec4_t result = {};
  //black
  CHECK(CG_ParseColorStr("0", result) == qtrue);
  CHECK(VectorCompare(colorBlack, result));

  //red
  CHECK(CG_ParseColorStr("1", result) == qtrue);
  CHECK(VectorCompare(colorRed, result));

  //green
  CHECK(CG_ParseColorStr("2", result) == qtrue);
  CHECK(VectorCompare(colorGreen, result));

  //white
  CHECK(CG_ParseColorStr("7", result) == qtrue);
  CHECK(VectorCompare(colorWhite, result));
}

TEST_CASE("Test Util: color parser, words", "[cgame][CG_ParseColorStr]")
{
  vec4_t result = {};
  //black
  CHECK(CG_ParseColorStr("black", result) == qtrue);
  CHECK(VectorCompare(colorBlack, result));
  CHECK(CG_ParseColorStr("BLACK", result) == qtrue);
  CHECK(VectorCompare(colorBlack, result));

  //red
  CHECK(CG_ParseColorStr("red", result) == qtrue);
  CHECK(VectorCompare(colorRed, result));
  CHECK(CG_ParseColorStr("RED", result) == qtrue);
  CHECK(VectorCompare(colorRed, result));

  //green
  CHECK(CG_ParseColorStr("green", result) == qtrue);
  CHECK(VectorCompare(colorGreen, result));
  CHECK(CG_ParseColorStr("GREEN", result) == qtrue);
  CHECK(VectorCompare(colorGreen, result));

  //white
  CHECK(CG_ParseColorStr("white", result) == qtrue);
  CHECK(VectorCompare(colorWhite, result));
  CHECK(CG_ParseColorStr("WHITE", result) == qtrue);
  CHECK(VectorCompare(colorWhite, result));

}

TEST_CASE("Test Util: color parser, 12 bit ", "[cgame][CG_ParseColorStr]")
{
  vec4_t result = {};

  //black
  CHECK(CG_ParseColorStr("000", result) == qtrue);
  CHECK(VectorCompare(colorBlack, result));
  CHECK(CG_ParseColorStr("0x000", result) == qtrue);
  CHECK(VectorCompare(colorBlack, result));
  CHECK(CG_ParseColorStr("0X000", result) == qtrue);
  CHECK(VectorCompare(colorBlack, result));
  CHECK(CG_ParseColorStr("x000", result) == qtrue);
  CHECK(VectorCompare(colorBlack, result));
  CHECK(CG_ParseColorStr("X000", result) == qtrue);
  CHECK(VectorCompare(colorBlack, result));
  CHECK(CG_ParseColorStr("#000", result) == qtrue);
  CHECK(VectorCompare(colorBlack, result));

  //one char, blue
  CHECK(CG_ParseColorStr("0xF", result) == qtrue);
  CHECK(VectorCompare((vec4_t){0, 0, 1}, result));
  CHECK(CG_ParseColorStr("0XF", result) == qtrue);
  CHECK(VectorCompare(colorBlue, result));
  CHECK(CG_ParseColorStr("xF", result) == qtrue);
  CHECK(VectorCompare(colorBlue, result));
  CHECK(CG_ParseColorStr("XF", result) == qtrue);
  CHECK(VectorCompare(colorBlue, result));
  CHECK(CG_ParseColorStr("#F", result) == qtrue);
  CHECK(VectorCompare(colorBlue, result));

  //two chars
  vec4_t twoCharsColor = {0, 1, 0.533333361, 0};
  CHECK(CG_ParseColorStr("0xF8", result) == qtrue);
  CHECK(VectorCompare(twoCharsColor, result));
  CHECK(CG_ParseColorStr("0XF8", result) == qtrue);
  CHECK(VectorCompare(twoCharsColor, result));
  CHECK(CG_ParseColorStr("xF8", result) == qtrue);
  CHECK(VectorCompare(twoCharsColor, result));
  CHECK(CG_ParseColorStr("XF8", result) == qtrue);
  CHECK(VectorCompare(twoCharsColor, result));
  CHECK(CG_ParseColorStr("#F8", result) == qtrue);
  CHECK(VectorCompare(twoCharsColor, result));

  //three chars
  vec4_t threeCharsColor = {1, 0.533333361, 0.266666681, 0};
  CHECK(CG_ParseColorStr("0xF84", result) == qtrue);
  CHECK(VectorCompare(threeCharsColor, result));
  CHECK(CG_ParseColorStr("0XF84", result) == qtrue);
  CHECK(VectorCompare(threeCharsColor, result));
  CHECK(CG_ParseColorStr("xF84", result) == qtrue);
  CHECK(VectorCompare(threeCharsColor, result));
  CHECK(CG_ParseColorStr("XF84", result) == qtrue);
  CHECK(VectorCompare(threeCharsColor, result));
  CHECK(CG_ParseColorStr("#F84", result) == qtrue);
  CHECK(VectorCompare(threeCharsColor, result));
}

TEST_CASE("Test Util: color parser, 24 bit ", "[cgame][CG_ParseColorStr]")
{
  vec4_t result = {};

  //black
  CHECK(CG_ParseColorStr("000000", result) == qtrue);
  CHECK(VectorCompare(colorBlack, result));
  CHECK(CG_ParseColorStr("0x000000", result) == qtrue);
  CHECK(VectorCompare(colorBlack, result));
  CHECK(CG_ParseColorStr("0X000000", result) == qtrue);
  CHECK(VectorCompare(colorBlack, result));
  CHECK(CG_ParseColorStr("x000000", result) == qtrue);
  CHECK(VectorCompare(colorBlack, result));
  CHECK(CG_ParseColorStr("X000000", result) == qtrue);
  CHECK(VectorCompare(colorBlack, result));
  CHECK(CG_ParseColorStr("#000000", result) == qtrue);
  CHECK(VectorCompare(colorBlack, result));

  //four chars
  CHECK(CG_ParseColorStr("0xFF00", result) == qtrue);
  CHECK(VectorCompare(colorGreen, result));
  CHECK(CG_ParseColorStr("0XFF00", result) == qtrue);
  CHECK(VectorCompare(colorGreen, result));
  CHECK(CG_ParseColorStr("xFF00", result) == qtrue);
  CHECK(VectorCompare(colorGreen, result));
  CHECK(CG_ParseColorStr("XFF00", result) == qtrue);
  CHECK(VectorCompare(colorGreen, result));
  CHECK(CG_ParseColorStr("#FF00", result) == qtrue);
  CHECK(VectorCompare(colorGreen, result));

  //five chars
  vec4_t fiveCharsColor = {0.0588235334, 1.0, 0, 0};
  CHECK(CG_ParseColorStr("0xFFF00", result) == qtrue);
  CHECK(VectorCompare(fiveCharsColor, result));
  CHECK(CG_ParseColorStr("0XFFF00", result) == qtrue);
  CHECK(VectorCompare(fiveCharsColor, result));
  CHECK(CG_ParseColorStr("xFFF00", result) == qtrue);
  CHECK(VectorCompare(fiveCharsColor, result));
  CHECK(CG_ParseColorStr("XFFF00", result) == qtrue);
  CHECK(VectorCompare(fiveCharsColor, result));
  CHECK(CG_ParseColorStr("#FFF00", result) == qtrue);
  CHECK(VectorCompare(fiveCharsColor, result));

  //six chars
  CHECK(CG_ParseColorStr("0xFF0000", result) == qtrue);
  CHECK(VectorCompare(colorRed, result));
  CHECK(CG_ParseColorStr("0XFF0000", result) == qtrue);
  CHECK(VectorCompare(colorRed, result));
  CHECK(CG_ParseColorStr("xFF0000", result) == qtrue);
  CHECK(VectorCompare(colorRed, result));
  CHECK(CG_ParseColorStr("XFF0000", result) == qtrue);
  CHECK(VectorCompare(colorRed, result));
  CHECK(CG_ParseColorStr("#FF0000", result) == qtrue);
  CHECK(VectorCompare(colorRed, result));

}







