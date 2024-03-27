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
  set_target_properties(common_target_settings PROPERTIES MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>")
else()
  target_compile_options(common_target_settings INTERFACE -Wall -Wextra -Wpedantic -Wformat -Wconversion -Wsign-conversion -Wtrampolines -Wimplicit-fallthrough -Wno-strict-overflow -fno-strict-aliasing)
  target_compile_options(common_target_settings INTERFACE -fstack-clash-protection -fstack-protector-strong -fcf-protection=full -fno-delete-null-pointer-checks -fno-strict-aliasing)
  target_compile_options(common_target_settings INTERFACE -D_FORTIFY_SOURCE=1 -Wl,-z,nodlopen -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now)
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
