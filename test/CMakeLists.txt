enable_testing()

add_subdirectory(mocks)

set(COMMON_SRCS "test_runner.cpp" "assert_throws_with_msg.h" "assert_throws_with_msg.cpp")

add_executable(sqlite_wrapper.test_runner ${COMMON_SRCS}
    "sqlite_wrapper_tests.cpp"
    "format_tests.cpp"
    "tuple_utils_test.cpp"
    "concepts_test.cpp")
add_executable(sqlite_wrapper::test_runner ALIAS sqlite_wrapper.test_runner)

set_target_properties(sqlite_wrapper.test_runner PROPERTIES OUTPUT_NAME "test_runner")

target_link_libraries(sqlite_wrapper.test_runner PRIVATE
    common_target_settings
    sqlite_wrapper::sqlite_wrapper
    GTest::gtest)

add_executable(sqlite_wrapper.test_runner_mocked ${COMMON_SRCS} "sqlite_wrapper_tests_mocked.cpp")
add_executable(sqlite_wrapper::test_runner_mocked ALIAS sqlite_wrapper.test_runner_mocked)

set_target_properties(sqlite_wrapper.test_runner_mocked PROPERTIES OUTPUT_NAME "test_runner_mocked")

target_link_libraries(sqlite_wrapper.test_runner_mocked PRIVATE
    common_target_settings
    sqlite_wrapper::sqlite_wrapper_static
    sqlite_wrapper::sqlite_mock
    GTest::gtest)
