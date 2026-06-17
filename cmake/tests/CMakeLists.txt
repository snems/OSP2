cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER clang)

project(osp2_tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_BUILD_TYPE Debug)

include_directories(${SOURCE_DIR})
# Yo wtf snems?
# include_directories(/home/snems/src/cmmdlib/include)

file(GLOB SOURCES_COMMON
    ${SOURCE_DIR}/cgame/*.c
    ${SOURCE_DIR}/cgame/*.h
    ${SOURCE_DIR}/game/*.c
    ${SOURCE_DIR}/game/*.h
    ${SOURCE_DIR}/ui/*.c
    ${SOURCE_DIR}/ui/*.h
    ${SOURCE_DIR}/qcommon/*.c
    ${SOURCE_DIR}/qcommon/*.h
)

file(GLOB SOURCES_TESTS
    ${SOURCE_DIR}/tests/*.cxx
    ${SOURCE_DIR}/tests/*.hxx
)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.15.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(osp2_tests ${SOURCES_COMMON} ${SOURCES_TESTS})
add_test(NAME osp2_tests COMMAND osp2_tests)

add_custom_command(
    TARGET osp2_tests
    COMMENT "Run tests"
    POST_BUILD
    COMMAND osp2_tests
)

target_link_libraries(osp2_tests PRIVATE Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(osp2_tests)

