#pragma once

#include <source_location>

#include "sqlite_wrapper/raii.h"

namespace sqlite_wrapper
{
  template <typename T>
  struct with_location
  {
    with_location(const T& value, const std::source_location& loc = std::source_location::current())
      : value(value), location(loc)
    {}

    with_location(T&& value, const std::source_location& loc = std::source_location::current())
      : value(std::move(value)), location(loc)
    {}

    T value;
    std::source_location location;
  };

  using db_with_location = with_location<sqlite3*>;
  using stmt_with_location = with_location<sqlite3_stmt*>;
}  // namespace sqlite_wrapper
