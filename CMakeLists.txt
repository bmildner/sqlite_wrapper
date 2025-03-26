cmake_minimum_required (VERSION 3.20)

project ("sqlite_wrapper" VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(unofficial-sqlite3 CONFIG REQUIRED)
# we do need to add the SQLite3 include directory to some targets but do NOT want a real link dependency!
get_target_property(SQLITE3_INCLUDES unofficial::sqlite3::sqlite3 INTERFACE_INCLUDE_DIRECTORIES)

find_package(GTest CONFIG REQUIRED)

find_package(fmt CONFIG REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile commands for clang tools." FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory where executables are dumped." FORCE)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib CACHE PATH "Directory where shared objects are dumped." FORCE)
set(CMAKE_BUILD_RPATH ../lib CACHE PATH "Path where executables should look for shared objects." FORCE)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON CACHE BOOL "Treat all warnings as errors." FORCE)

#enable ccache if found
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_FOUND} CACHE PATH "Path and name of ccache." FORCE)
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_FOUND} CACHE PATH "Path and name of ccache." FORCE)
endif()

# enable LTO
if (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
  include(CheckIPOSupported)
  check_ipo_supported(RESULT LTO_SUPPORTED)

  if (LTO_SUPPORTED)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON CACHE BOOL "Enable IPO/LTO" FORCE)
  endif()
endif()

include(static_analysis.cmake)
include(coverage.cmake)
include(common.cmake)

add_subdirectory ("src")
add_subdirectory ("test")
add_subdirectory ("docs")
