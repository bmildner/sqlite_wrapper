add_library(common_target_settings INTERFACE)

set_target_properties(common_target_settings PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(common_target_settings PROPERTIES VISIBILITY_INLINES_HIDDEN ON)
set_target_properties(common_target_settings PROPERTIES C_VISIBILITY_PRESET hidden)
set_target_properties(common_target_settings PROPERTIES CXX_VISIBILITY_PRESET hidden)


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  if (!MSVC)
    target_compile_options(common_target_settings INTERFACE -fsanitize=address,undefined)
    target_link_options(common_target_settings INTERFACE -fsanitize=address,undefined)
  else()
    target_compile_options(common_target_settings INTERFACE /fsanitize=address)
    target_link_options(common_target_settings INTERFACE /fsanitize=address)
  endif()
endif()
