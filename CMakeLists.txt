# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  #set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

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
set(CMAKE_BUILD_RPATH ${PROJECT_BINARY_DIR}/lib CACHE PATH "Directory where exeexecutables and shared objects are dumped." FORCE)

if (DEFINED ENV{CLANG_TIDY})
  set(CLANG_TIDY $ENV{CLANG_TIDY})
else()
  find_program(CLANG_TIDY NAMES clang-tidy clang-tidy-*)
endif()

if (NOT DEFINED MSVC)
  set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY})
endif()

set(CMAKE_COMPILE_WARNING_AS_ERROR ON CACHE BOOL "Treat all warnings as errors." FORCE)

include(common.cmake)

add_subdirectory ("src")
add_subdirectory ("test")
