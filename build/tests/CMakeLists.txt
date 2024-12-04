cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER clang)

project(osp2_tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_BUILD_TYPE Debug)

include_directories(.)
include_directories(/home/snems/src/cmmdlib/include)

file(GLOB SOURCES_COMMON
        ../../code/cgame/*.c
        ../../code/cgame/*.h
        ../../code/game/*.c
        ../../code/game/*.h
        ../../code/ui/*.c
        ../../code/ui/*.h
        ../../code/qcommon/*.c
        ../../code/qcommon/*.h
        )

file(GLOB SOURCES_TESTS 
        ../../code/tests/*.cxx
        ../../code/tests/*.hxx
        )

find_package(Catch2 3.4.0 REQUIRED)
add_executable(osp2_tests ${SOURCES_COMMON} ${SOURCES_TESTS})

target_link_libraries(osp2_tests PRIVATE Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(osp2_tests)

