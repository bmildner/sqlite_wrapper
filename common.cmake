add_library(common_target_settings INTERFACE)

set_target_properties(common_target_settings PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(common_target_settings PROPERTIES VISIBILITY_INLINES_HIDDEN ON)
set_target_properties(common_target_settings PROPERTIES C_VISIBILITY_PRESET hidden)
set_target_properties(common_target_settings PROPERTIES CXX_VISIBILITY_PRESET hidden)

if (DEFINED MSVC)
  # remove all /W options to avoid warnings due to multiple /W ...
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-9]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  endif()

  target_compile_options(common_target_settings INTERFACE /W4)

  if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
  endif()

  set_target_properties(common_target_settings PROPERTIES MSVC_DEBUG_INFORMATION_FORMAT ProgramDatabase)
else()
  target_compile_options(common_target_settings INTERFACE -Wall -Wextra -Wpedantic -Wformat -Wformat=2 -Wconversion -Wsign-conversion -Wtrampolines -Wimplicit-fallthrough -Wno-strict-overflow -fno-strict-aliasing)
  target_compile_options(common_target_settings INTERFACE -fstack-clash-protection -fstack-protector-strong -fcf-protection=full -fno-delete-null-pointer-checks -fno-strict-aliasing)
  target_compile_options(common_target_settings INTERFACE -Wl,-z,nodlopen -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now)

  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(common_target_settings INTERFACE -D_FORTIFY_SOURCE=1)

    if (GCOVR)
      target_compile_options(common_target_settings INTERFACE --coverage -g -O0)
      target_link_libraries(common_target_settings INTERFACE gcov)
    endif()
  else()
    # we always want do generate debug infos and use max optimization in non-debug builds
    target_compile_options(common_target_settings INTERFACE -g -O3)

    # we can't use _FORTIFY_SOURCE=3 with GCC 12 on Ubuntu, causes redefinition warning
    #target_compile_options(common_target_settings INTERFACE -D_FORTIFY_SOURCE=3)

    # GCC 12 causes a lot of false-positive warnings in GTest ...
    target_compile_options(common_target_settings INTERFACE -Wno-restrict)
  endif()

  target_link_libraries(common_target_settings INTERFACE fmt::fmt)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  if (NOT DEFINED MSVC)
    target_compile_options(common_target_settings INTERFACE -fsanitize=address,undefined)
    target_link_options(common_target_settings INTERFACE -fsanitize=address,undefined)
  else()
    target_compile_options(common_target_settings INTERFACE /fsanitize=address)
  endif()
endif()

# TODO: extract debug infos on non-windows/ELF platforms
# objcopy --only-keep-debug executable_file executable_file.debug
# objcopy --strip-unneeded executable_file
# objcopy --add-gnu-debuglink=executable_file.debug executable_file
