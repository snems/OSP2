cmake_minimum_required(VERSION 3.22)

project(osp2_tests)

include(basegame_sources)
include(libraries/all)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_BUILD_TYPE Debug)

include_directories(${SOURCE_DIR})

# This is for MinGW. Tell him we don't want to dynamically link common libraries,
# so CI tests can run peacefully.
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND WIN32)
  add_link_options(-static-libgcc -static-libstdc++ -lwinpthread -dynamic)
endif()

set(SOURCES_TESTS
    ${SOURCE_DIR}/tests/test_cg_customloc.cxx
    ${SOURCE_DIR}/tests/test_cg_osputil_colorparser.cxx
    ${SOURCE_DIR}/tests/test_cg_playerscolors.cxx
    ${SOURCE_DIR}/tests/test_cg_superhud_configparser.cxx
    ${SOURCE_DIR}/tests/test_main.cxx
    ${SOURCE_DIR}/tests/test_osp_drawstring.cxx
    ${SOURCE_DIR}/tests/test_Q_sscanf.cxx
    ${SOURCE_DIR}/tests/test_qcommon_zone_allocator.cxx
)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        devel
)

FetchContent_MakeAvailable(Catch2)

add_executable(osp2_tests ${CGAME_SOURCES_BASEGAME} ${CGAME_BINARY_SOURCES} ${SOURCES_TESTS})

add_custom_command(
    TARGET osp2_tests
    COMMENT "Run tests"
    POST_BUILD
    COMMAND osp2_tests
)

target_link_libraries(osp2_tests PRIVATE Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(osp2_tests)

