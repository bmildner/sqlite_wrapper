# turn off locally for faster builds, do NOT commit when disabled!
set(ENABLE_STATIC_ANALYSIS ON CACHE BOOL "Enables static analysis with tools like clang-tidy" FORCE)

if ((ENABLE_STATIC_ANALYSIS) AND (NOT DEFINED MSVC))
  # search for clang-tidy
  if (DEFINED ENV{CLANG_TIDY})
    set(CLANG_TIDY $ENV{CLANG_TIDY})
  else()
    find_program(CLANG_TIDY NAMES clang-tidy clang-tidy-* REQUIRED)
  endif()

  # enable clang-tidy
  set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY}; -extra-arg=-Wno-unknown-warning-option; -extra-arg=-Wno-ignored-optimization-argument)
endif()
