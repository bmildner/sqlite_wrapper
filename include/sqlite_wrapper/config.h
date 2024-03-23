#pragma once

#ifdef sqlite_wrapper_EXPORTS
#  ifdef _WIN32
#    define SQLITE_WRAPPER_EXPORT __declspec(dllexport)
#  else
#    define SQLITE_WRAPPER_EXPORT __attribute__((visibility("default")))
#  endif
#else
#  ifdef _WIN32
#    define SQLITE_WRAPPER_EXPORT __declspec(dllimport)
#  else
#    define SQLITE_WRAPPER_EXPORT
#  endif
#endif
