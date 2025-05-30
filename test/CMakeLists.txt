enable_testing()

add_subdirectory(mocks)

set(COMMON_SRCS "test_runner.cpp" "assert_throws_with_msg.h" "assert_throws_with_msg.cpp")

add_executable(test_runner ${COMMON_SRCS} "sqlite_wrapper_tests.cpp" "format_tests.cpp")

target_link_libraries(test_runner PRIVATE
    common_target_settings
    sqlite_wrapper
    GTest::gtest)

add_executable(test_runner_mocked ${COMMON_SRCS} "sqlite_wrapper_tests_mocked.cpp")

target_link_libraries(test_runner_mocked PRIVATE
    common_target_settings
    sqlite_wrapper_static
    sqlite_mock
    GTest::gtest)
