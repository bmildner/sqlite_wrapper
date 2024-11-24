#pragma once

#include <type_traits>
#include <utility>
#include <source_location>

#ifdef __has_include
#  if __has_include(<format>)
#    include <format>
#    define SQLITEWRAPPER_FORMAT_NAMESPACE_NAME std
#    define SQLITEWRAPPER_FORMAT_NAMESPACE ::std
#  elif __has_include(<fmt/format.h>)
#    include <fmt/format.h>
#    define SQLITEWRAPPER_FORMAT_NAMESPACE_NAME fmt
#    define SQLITEWRAPPER_FORMAT_NAMESPACE ::fmt
#  else
#     error "Did not find <format> nor <fmt/format.h>!"
#  endif
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

namespace SQLITEWRAPPER_FORMAT_NAMESPACE_NAME  // doxygen can't handle namespace qualified class names ...
{
  template<>
  // NOLINTNEXTLINE(cert-dcl58-cpp) modification of 'std' namespace can result in undefined behavior
  struct formatter<std::source_location>
  {
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static) non-static mandated by standard
    constexpr auto parse(SQLITEWRAPPER_FORMAT_NAMESPACE::format_parse_context& parse_ctx)
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
        throw SQLITEWRAPPER_FORMAT_NAMESPACE::format_error("only an empty format-spec is supported");
      }
      return iter;
    }

    template<typename FmtContext>
    auto format(std::source_location location, FmtContext& ctx) const
    {
      return SQLITEWRAPPER_FORMAT_NAMESPACE::format_to(ctx.out(), "{}:{} '{}'", location.file_name(), location.line(), location.function_name());
    }
  };
}

#undef SQLITEWRAPPER_FORMAT_NAMESPACE_NAME
#undef SQLITEWRAPPER_FORMAT_NAMESPACE
