add_library(sqlite_mock STATIC "sqlite_mock.h" "sqlite_mock.cpp" "free_function_mock.h")

target_link_libraries(sqlite_mock PRIVATE common_target_settings GTest::gmock)

target_include_directories(sqlite_mock PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(sqlite_mock PRIVATE "../../include")

target_include_directories(sqlite_mock INTERFACE ${SQLITE3_INCLUDES})

if (DEFINED MSVC)
  target_compile_definitions(sqlite_mock PUBLIC SQLITE_API=)
endif ()
