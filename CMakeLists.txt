cmake_minimum_required(VERSION 3.25)

if(POLICY CMP0177)
    # Policy CMP0177: install() DESTINATION paths are normalized
    # Introduced in CMake 3.31
    cmake_policy(SET CMP0177 NEW)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(mod)

project(${MOD_NAME} VERSION ${MOD_VERSION} LANGUAGES C)

option(BUILD_MOD_LIBRARIES "Build OSP2 gamecode as native libraries" ON)
option(BUILD_MOD_QVMS "Build OSP2 gamecode as qvms" ON)
option(BUILD_MOD_PK3 "Make .pk3 with assets and (if enabled) qvms" ON)
option(BUILD_TESTS "Build tests" OFF)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

# Release build by default, set externally if you want something else
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Try to enable LTO by default
include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED)

if(IPO_SUPPORTED)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
endif()

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/code)

include(compilers/all)

include(basegame)
include(tests)

include(post_configure)
include(installer)
