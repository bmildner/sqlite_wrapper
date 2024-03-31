cmake_minimum_required (VERSION 3.20)

project ("sqlite_wrapper" VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(unofficial-sqlite3 CONFIG REQUIRED)    
get_target_property(SQLITE3_INCLUDES unofficial::sqlite3::sqlite3 INTERFACE_INCLUDE_DIRECTORIES)

find_package(GTest CONFIG REQUIRED)

if (NOT DEFINED MSVC)
  find_package(fmt CONFIG REQUIRED)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory where exeexecutables are dumped." FORCE)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib CACHE PATH "Directory where shared objects are dumped." FORCE)
set(CMAKE_BUILD_RPATH ../lib CACHE PATH "Path where exeexecutables should look for shared objects." FORCE)

if (DEFINED ENV{CLANG_TIDY})
  set(CLANG_TIDY $ENV{CLANG_TIDY})
else()
  find_program(CLANG_TIDY NAMES clang-tidy clang-tidy-*)
endif()

if (NOT DEFINED MSVC)
  set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY}; -extra-arg=-Wno-unknown-warning-option -extra-arg=-Wno-ignored-optimization-argument)
endif()

set(CMAKE_COMPILE_WARNING_AS_ERROR ON CACHE BOOL "Treat all warnings as errors." FORCE)

if (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
  include(CheckIPOSupported)
  check_ipo_supported(RESULT LTO_SUPPORTED)

  if (LTO_SUPPORTED)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON CACHE BOOL "Enable IPO/LTO" FORCE)
  endif()
endif()

include(coverage.cmake)
include(common.cmake)

add_subdirectory ("src")
add_subdirectory ("test")
