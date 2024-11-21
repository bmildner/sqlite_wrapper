#pragma once

#include <memory>

#include "sqlite_wrapper/config.h"
#include "sqlite_wrapper/with_location.h"

// forward declarations of sqlite3 (pimpl) types
extern "C"
{
  struct sqlite3;
  struct sqlite3_stmt;
}

namespace sqlite_wrapper
{
  namespace details
  {
    struct database_deleter
    {
      SQLITE_WRAPPER_EXPORT void operator()(::sqlite3* raw_db_handle) const noexcept;
    };

    struct statement_deleter
    {
      SQLITE_WRAPPER_EXPORT void operator()(::sqlite3_stmt* stmt) const noexcept;
    };
  }  // namespace details

  using database = std::unique_ptr<::sqlite3, details::database_deleter>;
  using statement = std::unique_ptr<::sqlite3_stmt, details::statement_deleter>;

  using db_with_location = with_location<sqlite3*>;
  using stmt_with_location = with_location<sqlite3_stmt*>;
}  // namespace sqlite_wrapper
