set(SRC "sqlite_wrapper.cpp" 
        "../include/sqlite_wrapper/sqlite_wrapper.h" 
        "../include/sqlite_wrapper/raii.h" 
        "raii.cpp"
        "../include/sqlite_wrapper/sqlite_error.h"
        "sqlite_error.cpp"
        "../include/sqlite_wrapper/format.h" 
        "../include/sqlite_wrapper/with_location.h"
        "../include/sqlite_wrapper/create_table.h"
        "../include/sqlite_wrapper/config.h")

add_library(sqlite_wrapper SHARED ${SRC})

target_include_directories(sqlite_wrapper PUBLIC "../include")

# enable automatic export and import of symbols
target_compile_definitions(sqlite_wrapper PUBLIC SQLITE_WRAPPER_SHARED)

target_link_libraries(sqlite_wrapper PRIVATE common_target_settings unofficial::sqlite3::sqlite3)

# -------

add_library(sqlite_wrapper_static STATIC ${SRC})

target_include_directories(sqlite_wrapper_static PUBLIC "../include")

target_link_libraries(sqlite_wrapper_static PRIVATE common_target_settings)

target_include_directories(sqlite_wrapper_static PRIVATE ${SQLITE3_INCLUDES})

if (DEFINED MSVC)
  # this might prevent dynamic linking to SQLite on Windows due to name prefix of DLLImport symbols ...
  target_compile_definitions(sqlite_wrapper_static PRIVATE SQLITE_API=)
endif ()