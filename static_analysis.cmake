# turn off locally for faster builds, do NOT commit when disabled!
set(ENABLE_STATIC_ANALYSIS ON CACHE BOOL "Enables static analysis with tools like clang-tidy" FORCE)

# In MSVC one has to install the clang toolset to clang-tidy to be found,
# but currently the /EHsc option seems to be ignored and clang-tidy complains about exceptions ...
if ((ENABLE_STATIC_ANALYSIS) AND (NOT DEFINED MSVC))
  # search for clang-tidy
  if (NOT CLANG_TIDY)
    find_program(CLANG_TIDY NAMES clang-tidy REQUIRED)
  endif ()

  execute_process(COMMAND ${CLANG_TIDY} --version OUTPUT_VARIABLE CLANG_TIDY_VERSION_OUTPUT)
  string(STRIP "${CLANG_TIDY_VERSION_OUTPUT}" CLANG_TIDY_VERSION_OUTPUT)
  message(STATUS "Using clang-tidy: ${CLANG_TIDY} (${CLANG_TIDY_VERSION_OUTPUT})")

  # enable clang-tidy
  set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY};
      -extra-arg=-Wno-unknown-warning-option;
      -extra-arg=-Wno-ignored-optimization-argument;
      -extra-arg=-fno-builtin-std-forward_like)  # avoid "deduced return type cannot be used before it is defined" error
endif()
