enable_testing()

add_executable (tests_runner "sqlite_wrapper_tests.cpp" "test_runner.cpp")

target_link_libraries(tests_runner PRIVATE common_target_settings sqlite_wrapper GTest::gtest GTest::gmock)

#add_test(AllTestsInMain tests)

# enable_testing()

# find_package(GTest CONFIG REQUIRED)
# target_link_libraries(main PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

# add_test(AllTestsInMain main)
