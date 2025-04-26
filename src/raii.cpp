#include "sqlite_wrapper/raii.h"

#include <sqlite3.h>

#include <cassert>

namespace sqlite_wrapper::details
{
  void database_deleter::operator()(::sqlite3* raw_db_handle) const noexcept
  {
    [[maybe_unused]] const auto err{::sqlite3_close(raw_db_handle)};
    assert(err == SQLITE_OK);
  }

  void statement_deleter::operator()(::sqlite3_stmt* stmt) const noexcept
  {
    [[maybe_unused]] const auto err{::sqlite3_finalize(stmt)};
    assert(err == SQLITE_OK);
  }
}  // namespace sqlite_wrapper::details
