#pragma once

#include <source_location>
#include <type_traits>
#include <utility>

#ifdef __has_include
#  if __has_include(<format>)
#    include <format>
#    define SQLITEWRAPPER_FORMAT_NAMESPACE_NAME std
#    define SQLITEWRAPPER_FORMAT_NAMESPACE ::std
#  elif __has_include(<fmt/format.h>)
#    include <fmt/format.h>
#    define SQLITEWRAPPER_FORMAT_NAMESPACE_NAME fmt
#    define SQLITEWRAPPER_FORMAT_NAMESPACE ::fmt
#    define SQLITEWRAPPER_FORMAT_USE_FMT
#  else
#     error "Did not find <format> nor <fmt/format.h>!"
#  endif
#else
#  error has_include is needed to detect presents of <format> header!
#endif

#ifdef DOXYGEN
#  define SQLITEWRAPPER_FORMAT_NAMESPACE_NAME std_or_fmt
#  define SQLITEWRAPPER_FORMAT_NAMESPACE ::std_or_fmt
#endif

namespace sqlite_wrapper
{
  template<typename... Args>
  [[nodiscard]] auto format(SQLITEWRAPPER_FORMAT_NAMESPACE::format_string<Args...> fmt, Args&&... args) -> std::string
  {
    return SQLITEWRAPPER_FORMAT_NAMESPACE::format(fmt, std::forward<Args>(args)...);
  }

  using format_error = SQLITEWRAPPER_FORMAT_NAMESPACE::format_error;

#ifdef __cpp_lib_to_underlying
  using std::to_underlying;
#else
  template <typename Enum>
  constexpr auto to_underlying(Enum value) noexcept -> std::underlying_type_t<Enum>
  {
    return static_cast<std::underlying_type_t<Enum>>(value);
  }
#endif

  struct empty_format_spec
  {
    static constexpr auto parse(SQLITEWRAPPER_FORMAT_NAMESPACE::format_parse_context& parse_ctx)
    {
      // NOLINTNEXTLINE(readability-qualified-auto) false positive!
      auto iter{parse_ctx.begin()};
      // skip spaces and tabs
      while ((iter != parse_ctx.end()) && (*iter != '}') && ((*iter == ' ') || (*iter == '\t')))
      {
        iter++;
      }
      if ((iter != parse_ctx.end()) && (*iter != '}'))
      {
        throw sqlite_wrapper::format_error("only an empty format-spec is supported");
      }
      return iter;
    }
  };
}

namespace SQLITEWRAPPER_FORMAT_NAMESPACE_NAME
{
  template<>
  // NOLINTNEXTLINE(cert-dcl58-cpp) modification of 'std' namespace can result in undefined behavior
  struct formatter<std::source_location> : sqlite_wrapper::empty_format_spec
  {
    template<typename FmtContext>
    static auto format(std::source_location location, FmtContext& ctx)
    {
      return SQLITEWRAPPER_FORMAT_NAMESPACE::format_to(ctx.out(), "{}:{} '{}'", location.file_name(), location.line(), location.function_name());
    }
  };
}
