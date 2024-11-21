#pragma once

#include <source_location>
#include <type_traits>

namespace sqlite_wrapper
{
  template <typename T>
  struct with_location
  {
    using value_type = std::remove_reference_t<T>;

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
