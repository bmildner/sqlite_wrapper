add_custom_target(run_all_tests
        COMMAND test_runner
        COMMAND echo ""
        COMMAND echo "--------------------------------------------------------------------------------"
        COMMAND echo ""
        COMMAND test_runner_mocked
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        USES_TERMINAL)

add_dependencies(run_all_tests test_runner test_runner_mocked)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  find_program(GCOVR NAMES gcovr)

  if (NOT GCOVR)
    message(WARNING "No coverage analysis, gcovr not found!")
  else()
    if (NOT GCOV)  # use GCOV to set non-std gcov like "gcov-14"
      set(GCOV "gcov")
    endif ()

    add_custom_target(coverage_report
            COMMAND ${GCOVR} --gcov-executable ${GCOV} -r ${CMAKE_CURRENT_SOURCE_DIR} -e ../vcpkg_installed/ -e ${CMAKE_CURRENT_SOURCE_DIR}/test --sort=uncovered-percent --txt-metric=branch --exclude-unreachable-branches --exclude-throw-branches
            COMMAND ${GCOVR} --gcov-executable ${GCOV} -r ${CMAKE_CURRENT_SOURCE_DIR} -e ../vcpkg_installed/ -e ${CMAKE_CURRENT_SOURCE_DIR}/test --sort=uncovered-percent --txt-metric=line --exclude-function-lines --exclude-noncode-lines
            WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            USES_TERMINAL)

    add_dependencies(coverage_report run_all_tests)
    endif()
endif()
