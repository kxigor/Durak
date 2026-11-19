cmake_minimum_required(VERSION 3.25)

project(durak LANGUAGES CXX)

# =========================================================
# 0. STANDALONE vs SUBPROJECT
# =========================================================
# PROJECT_IS_TOP_LEVEL is set by project() since CMake 3.21.
# When this project is consumed via add_subdirectory / FetchContent, dev-time
# machinery (app, tests, qa checks, sanitizers, coverage) stays disabled by
# default.

if(PROJECT_IS_TOP_LEVEL)
  set(DURAK_DEV_DEFAULT ON)
else()
  set(DURAK_DEV_DEFAULT OFF)
endif()

# =========================================================
# 1. OPTIONS (prefix with DURAK_ — rename to keep the cache name unique)
# =========================================================
option(DURAK_BUILD_WEB "Build the HTTP game server" OFF)
option(DURAK_BUILD_APP           "Build the example app"             ${DURAK_DEV_DEFAULT})
option(DURAK_BUILD_TESTS         "Build the GTest test suite"        ${DURAK_DEV_DEFAULT})
option(DURAK_ENABLE_FORMAT_CHECK "Register clang-format as CTest"    ${DURAK_DEV_DEFAULT})
option(DURAK_ENABLE_TIDY_CHECK   "Register clang-tidy as CTest"      ${DURAK_DEV_DEFAULT})
option(DURAK_ENABLE_COVERAGE     "Enable gcov/lcov coverage targets" OFF)

# Format/tidy/coverage only make sense alongside tests.
if(NOT DURAK_BUILD_TESTS)
  set(DURAK_ENABLE_FORMAT_CHECK OFF CACHE BOOL "Disabled: requires DURAK_BUILD_TESTS=ON" FORCE)
  set(DURAK_ENABLE_TIDY_CHECK   OFF CACHE BOOL "Disabled: requires DURAK_BUILD_TESTS=ON" FORCE)
  set(DURAK_ENABLE_COVERAGE     OFF CACHE BOOL "Disabled: requires DURAK_BUILD_TESTS=ON" FORCE)
endif()

# =========================================================
# 2. TOP-LEVEL-ONLY GLOBAL SETTINGS
# =========================================================
# These touch *global* CMake state. When this project is a subproject, the
# parent decides build type, exports and the C++ standard.

if(PROJECT_IS_TOP_LEVEL)
  if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
  endif()
  set(CMAKE_CXX_STANDARD 23)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# =========================================================
# 3. SUBDIRECTORIES
# =========================================================
add_subdirectory(src)

# =========================================================
# 4. DEV-ONLY MACHINERY (skipped for subproject consumers)
# =========================================================
if(DURAK_BUILD_APP OR DURAK_BUILD_WEB OR DURAK_BUILD_TESTS OR DURAK_ENABLE_COVERAGE)
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
  include(ProjectOptions)
  include(ProjectSanitizers)
  include(ProjectCoverage)
endif()

if(DURAK_BUILD_WEB)
  add_subdirectory(app/server)
endif()

if(DURAK_BUILD_APP)
  add_subdirectory(app/demo)
endif()

if(DURAK_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
