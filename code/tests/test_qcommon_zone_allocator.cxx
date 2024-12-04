#include <catch2/catch_test_macros.hpp>

#include "../qcommon/qcommon.h"


TEST_CASE("Test memory allocator", "[qcommon][memory][zone_allocator]")
{
  Com_InitZoneMemory();

  void *ptr = Z_Malloc(100);

  REQUIRE(ptr);

  Z_Free(ptr);
}

TEST_CASE("Test memory allocator, overflow", "[qcommon][memory][zone_allocator]")
{
  Com_InitZoneMemory();

  // too big
  void *ptr1 = Z_Malloc(MAINZONE_STATIC_SIZE);
  CHECK(ptr1 == NULL);

  // not too big
  ptr1 = Z_Malloc(MAINZONE_STATIC_SIZE/2);
  REQUIRE(ptr1);

  // but one more will too big
  void *ptr2 = Z_Malloc(MAINZONE_STATIC_SIZE/2);
  CHECK(ptr2 == NULL);

  // if free first, there are enought memory
  Z_Free(ptr1);
  ptr2 = Z_Malloc(MAINZONE_STATIC_SIZE/2);
  REQUIRE(ptr2);

  Z_Free(ptr2);
}
