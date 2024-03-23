# CMakeList.txt : CMake project for sqlite_wrapper, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_library(sqlite_wrapper "sqlite_wrapper.cpp" 
                           "../include/sqlite_wrapper/sqlite_wrapper.h" 
                           "../include/sqlite_wrapper/raii.h" 
                           "sqlite_wrapper_raii.cpp" 
                           "../include/sqlite_wrapper/error.h" 
                           "sqlite_wrapper_error.cpp" 
                           "../include/sqlite_wrapper/format.h" 
                           "../include/sqlite_wrapper/location.h" 
                           "../include/sqlite_wrapper/create_table.h"
                           "../include/sqlite_wrapper/config.h")

target_include_directories(sqlite_wrapper PUBLIC "../include")

target_link_libraries(sqlite_wrapper PRIVATE common_target_settings unofficial::sqlite3::sqlite3)

if (!MSVC)
  target_link_libraries(sqlite_wrapper PRIVATE fmt::fmt)
endif()
