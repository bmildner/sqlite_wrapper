add_custom_target(sqlite_wrapper.run_all_tests
        COMMAND echo "--------------------------------------------------------------------------------"
        COMMAND echo "test_runner"
        COMMAND ./test_runner
        COMMAND echo "--------------------------------------------------------------------------------"
        COMMAND echo "test_runner_mocked"
        COMMAND ./test_runner_mocked
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        USES_TERMINAL)

add_dependencies(sqlite_wrapper.run_all_tests sqlite_wrapper::test_runner sqlite_wrapper::test_runner_mocked)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  find_program(GCOVR NAMES gcovr)

  if (NOT GCOVR)
    message(WARNING "No coverage analysis, gcovr not found!")
  else()
    if (NOT GCOV)  # use GCOV to set non-std gcov like "gcov-14"
      set(GCOV "gcov")
    endif ()

    # Get version number from gcovr, e.g. "gcovr 8.4" -> "8.4"
    execute_process(
        COMMAND ${GCOVR} --version
        OUTPUT_VARIABLE GCOVR_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REGEX MATCH "[0-9]+\.[0-9]+" GCOVR_VERSION "${GCOVR_VERSION}")

    message(STATUS "Found gcovr ${GCOVR_VERSION}")

    # Only add --merge-lines option on gcovr >= 8.4 based on version
    if (GCOVR_VERSION VERSION_GREATER_EQUAL "8.4")
      set(GCOVR_EXTRA_ARGUMENTS "--merge-lines")
    else()
      set(GCOVR_EXTRA_ARGUMENTS "")
    endif()

    add_custom_target(sqlite_wrapper.coverage_report
            COMMAND ${GCOVR} --gcov-executable ${GCOV} -r ${CMAKE_CURRENT_SOURCE_DIR} -e ${CMAKE_CURRENT_SOURCE_DIR}/test --sort=uncovered-percent --txt-metric=branch --exclude-unreachable-branches --exclude-throw-branches ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/..
            COMMAND ${GCOVR} --gcov-executable ${GCOV} -r ${CMAKE_CURRENT_SOURCE_DIR} -e ${CMAKE_CURRENT_SOURCE_DIR}/test --sort=uncovered-percent --txt-metric=line --exclude-function-lines --exclude-noncode-lines ${GCOVR_EXTRA_ARGUMENTS} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/..
            WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            USES_TERMINAL)

    add_dependencies(sqlite_wrapper.coverage_report sqlite_wrapper.run_all_tests)
    endif()
endif()

# avoid warning about unused GCOV variable
set(ignoreMe "${GCOV}")
