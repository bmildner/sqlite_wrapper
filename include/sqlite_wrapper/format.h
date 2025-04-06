#pragma once

#include <type_traits>
#include <utility>

#ifdef __has_include
#  if __has_include(<format>)
#    include <format>
#    include <source_location>
#    define SQLITEWRAPPER_FORMAT_NAMESPACE_NAME std
#    define SQLITEWRAPPER_FORMAT_NAMESPACE ::std
#  elif __has_include(<fmt/format.h>)
#    include <fmt/format.h>
#    include <fmt/std.h>
#    define SQLITEWRAPPER_FORMAT_NAMESPACE_NAME fmt
#    define SQLITEWRAPPER_FORMAT_NAMESPACE ::fmt
#    define SQLITEWRAPPER_FORMAT_USE_FMT
#  else
#    error "Did not find <format> nor <fmt/format.h>!"
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
  template <typename... Args>
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
      // NOLINTNEXTLINE(readability-qualified-auto) "const auto*" is more an implementation detail here!
      const auto iter{parse_ctx.begin()};
      if ((iter != parse_ctx.end()) && (*iter != '}'))
      {
        throw sqlite_wrapper::format_error("unknown format specifier");
      }
      return iter;
    }
  };

#if defined(SQLITEWRAPPER_FORMAT_USE_FMT) || defined(DOXYGEN)
  /**
   * Creates a runtime format string for either fmt::format or std::format in C++26 or higher.
   * @note This function is only provided if fmt::format is used or in >= C++26!
   *
   * @param fmt_string format string
   * @returns a runtime format string containing the given format string
   */
  inline auto runtime_format(std::string_view fmt_string) noexcept(noexcept(fmt::runtime(fmt_string)))
  {
    return fmt::runtime(fmt_string);
  }
#elif __cpp_lib_format >= 202311L  // >= C++26
  inline auto runtime_format(std::string_view fmt_string) noexcept
  {
    return std::runtime_format(fmt_string);
  }
  #endif
}  // namespace sqlite_wrapper

#ifndef SQLITEWRAPPER_FORMAT_USE_FMT
namespace SQLITEWRAPPER_FORMAT_NAMESPACE_NAME
{
  template <>
  // NOLINTNEXTLINE(cert-dcl58-cpp) modification of 'std' namespace can result in undefined behavior
  struct formatter<std::source_location> : sqlite_wrapper::empty_format_spec
  {
    template <typename FmtContext>
    static auto format(std::source_location location, FmtContext& ctx)
    {
      return SQLITEWRAPPER_FORMAT_NAMESPACE::format_to(ctx.out(), "{}:{}:{}: {}", location.file_name(), location.line(),
                                                       location.column(), location.function_name());
    }
  };
}  // namespace SQLITEWRAPPER_FORMAT_NAMESPACE_NAME
#endif
