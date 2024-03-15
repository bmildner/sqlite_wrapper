# CMakeList.txt : CMake project for sqlite_wrapper, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (sqlite_wrapper "sqlite_wrapper.cpp" "sqlite_wrapper.h" "sqlite_wrapper_raii.h" "sqlite_wrapper_raii.cpp" "sqlite_wrapper_error.h" "sqlite_wrapper_error.cpp" "sqlite_wrapper_format.h" "sqlite_wrapper_location.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET sqlite_wrapper PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(sqlite_wrapper PRIVATE unofficial::sqlite3::sqlite3 fmt::fmt)

# TODO: Add tests and install targets if needed.
