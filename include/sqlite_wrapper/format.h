#pragma once

#include <type_traits>
#include <utility>

#ifdef __has_include
#  if __has_include(<format>)
#    include <format>
#    define SQLITEWRAPPER_FORMAT_NAMESPACE ::std
#  elif __has_include(<fmt/format.h>)
#    include <fmt/format.h>
#    define SQLITEWRAPPER_FORMAT_NAMESPACE ::fmt
#  else
#     error "Did not find <format> nor <fmt/format.h>!"
#  endif
#endif

namespace sqlite_wrapper
{
  template<typename... Args>
  [[nodiscard]] std::string format(SQLITEWRAPPER_FORMAT_NAMESPACE::format_string<Args...> fmt, Args&&... args)
  {
    return SQLITEWRAPPER_FORMAT_NAMESPACE::format(fmt, std::forward<Args>(args)...);
  }

#ifdef __cpp_lib_to_underlying
  using std::to_underlying;
#else
  template <class Enum>
  constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept
  {
    return static_cast<std::underlying_type_t<Enum>>(e);
  }
#endif
}

#undef SQLITEWRAPPER_FORMAT_NAMESPACE
