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
    /**
     * Custom deleter for database handle RAII-guard.
     */
    struct database_deleter
    {
      SQLITE_WRAPPER_EXPORT void operator()(::sqlite3* raw_db_handle) const noexcept;
    };

    /**
     * Custom deleter for prepared statement handle RAII-guard.
     */
    struct statement_deleter
    {
      SQLITE_WRAPPER_EXPORT void operator()(::sqlite3_stmt* stmt) const noexcept;
    };
  }  // namespace details

  /**
   * RAII-guard for database handles.
   */
  using database = std::unique_ptr<::sqlite3, details::database_deleter>;
  /**
   * RAII-guard for prepared statement handles.
   */
  using statement = std::unique_ptr<::sqlite3_stmt, details::statement_deleter>;

  /**
   * *Non-owning* database handle with an std::source_location .
   */
  using db_with_location = with_location<sqlite3*>;
  /**
   * *Non-owning* prepared statement handle with an std::source_location .
   */
  using stmt_with_location = with_location<sqlite3_stmt*>;
}  // namespace sqlite_wrapper
