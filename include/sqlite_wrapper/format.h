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
  [[nodiscard]] auto format(SQLITEWRAPPER_FORMAT_NAMESPACE::format_string<Args...> fmt, Args&&... args) -> std::string
  {
    return SQLITEWRAPPER_FORMAT_NAMESPACE::format(fmt, std::forward<Args>(args)...);
  }

#ifdef __cpp_lib_to_underlying
  using std::to_underlying;
#else
  template <typename Enum>
  constexpr auto to_underlying(Enum value) noexcept -> std::underlying_type_t<Enum>
  {
    return static_cast<std::underlying_type_t<Enum>>(value);
  }
#endif
}

#undef SQLITEWRAPPER_FORMAT_NAMESPACE
