#pragma once

#ifdef SQLITE_WRAPPER_SHARED  // set as PUBLIC for sqlite_wrapper target
#  ifdef sqlite_wrapper_EXPORTS  // automatically set by cmake when building sqlite_wrapper target
#    ifdef _WIN32
#       define SQLITE_WRAPPER_EXPORT __declspec(dllexport)
#    else
#      define SQLITE_WRAPPER_EXPORT __attribute__((visibility("default")))
#    endif
#  else
#    ifdef _WIN32
#      define SQLITE_WRAPPER_EXPORT __declspec(dllimport)
#    else
#      define SQLITE_WRAPPER_EXPORT
#    endif
#  endif
#else  // ignore SQLITE_WRAPPER_EXPORT when building or using sqlite_wrapper_static
#  define SQLITE_WRAPPER_EXPORT
#endif
