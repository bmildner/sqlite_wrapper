if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  find_program(GCOVR NAMES gcovr)

  if (NOT GCOVR)
    message(WARNING "No coverage analysis, gcovr not found!")
  else()
    add_custom_target(coverage_report
            COMMAND test_runner
            COMMAND test_runner_mocked
            COMMAND ${GCOVR} -r ${PROJECT_SOURCE_DIR} -e ../vcpkg_installed/ -e ${PROJECT_SOURCE_DIR}/test --sort-percentage --txt-metric branch -exclude-unreachable-branches --exclude-throw-branches
            COMMAND ${GCOVR} -r ${PROJECT_SOURCE_DIR} -e ../vcpkg_installed/ -e ${PROJECT_SOURCE_DIR}/test --sort-percentage --txt-metric line -exclude-function-lines --exclude-noncode-lines
            WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            USES_TERMINAL)

    add_dependencies(coverage_report test_runner test_runner_mocked)
    endif()
endif()