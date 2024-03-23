#pragma once

#include <memory>

#include "sqlite_wrapper/config.h"

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
    struct SQLITE_WRAPPER_EXPORT database_deleter
    {
      void operator()(::sqlite3* db) const noexcept;
    };

    struct SQLITE_WRAPPER_EXPORT statement_deleter
    {
      void operator()(::sqlite3_stmt* stmt) const noexcept;
    };
  }  // namespace details

  using database = std::unique_ptr<::sqlite3, details::database_deleter>;
  using statement = std::unique_ptr<::sqlite3_stmt, details::statement_deleter>;

}  // namespace sqlite_wrapper
