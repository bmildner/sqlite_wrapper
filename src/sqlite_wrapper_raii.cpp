#include "sqlite_wrapper/raii.h"

#include <sqlite3.h>

namespace sqlite_wrapper::details
{
  void database_deleter::operator()(::sqlite3* raw_db_handle) const noexcept
  {
    // TODO: log error
    ::sqlite3_close(raw_db_handle);
  }

  void statement_deleter::operator()(::sqlite3_stmt* stmt) const noexcept
  {
    // TODO: log error
    ::sqlite3_finalize(stmt);
  }
}  // namespace sqlite_wrapper::details
