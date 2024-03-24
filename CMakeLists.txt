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
find_package(GTest CONFIG REQUIRED)

get_target_property(SQLITE3_INCLUDES unofficial::sqlite3::sqlite3 INTERFACE_INCLUDE_DIRECTORIES)

if (!MSVC)
  find_package(fmt CONFIG REQUIRED)
endif()

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "Directory where executeables and shared objects are dumped." FORCE)

include(common.cmake)

# Include sub-projects.
add_subdirectory ("src")
add_subdirectory ("test")
