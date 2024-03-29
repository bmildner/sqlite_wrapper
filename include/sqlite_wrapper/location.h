#pragma once

#include <source_location>

namespace sqlite_wrapper
{
  template <typename T>
  struct with_location
  {
    // NOLINTNEXTLINE [hicpp-explicit-conversions]
    with_location(const T& value, const std::source_location& loc = std::source_location::current())
      : value(value), location(loc)
    {}

    // NOLINTNEXTLINE [hicpp-explicit-conversions]
    with_location(T&& value, const std::source_location& loc = std::source_location::current())
      : value(std::move(value)), location(loc)
    {}

    T value;
    std::source_location location;
  };
}  // namespace sqlite_wrapper
