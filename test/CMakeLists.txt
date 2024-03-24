enable_testing()

add_subdirectory(mocks)

add_executable(test_runner "test_runner.cpp" "sqlite_wrapper_tests.cpp" "assert_throw_msg.h")

target_link_libraries(test_runner PRIVATE common_target_settings sqlite_wrapper GTest::gtest)


add_executable(test_runner_mocked "test_runner.cpp" "sqlite_wrapper_tests_mocked.cpp" "assert_throw_msg.h")

target_link_libraries(test_runner_mocked PRIVATE common_target_settings sqlite_wrapper_static sqlite_mock GTest::gtest)


#add_test(AllTestsInMain tests)

# enable_testing()

# find_package(GTest CONFIG REQUIRED)
# target_link_libraries(main PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

# add_test(AllTestsInMain main)
