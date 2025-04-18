#pragma once

#include <source_location>
#include <type_traits>
#include <utility>

namespace sqlite_wrapper
{
  /**
   * Wrapper that adds a std::source_location to any type.
   * Useful in cases where one can not have a defaulted std::source_location as the last parameter due to perfect forwarding.
   *
   * Like:
   * @code
   * auto create_prepared_statement(const db_with_location& database, std::string_view sql, const binding_type auto&... params) -> statement
   * @endcode
   *
   * @tparam T the type to be wrapped
   */
  template <typename T>
  struct with_location
  {
    using value_type = std::decay_t<T>;

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    with_location(const value_type& value, const std::source_location& loc = std::source_location::current())
      : value(value), location(loc)
    {}

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    with_location(value_type&& value, const std::source_location& loc = std::source_location::current())
      : value(std::move(value)), location(loc)
    {}

    value_type value;
    std::source_location location;
  };
}  // namespace sqlite_wrapper
