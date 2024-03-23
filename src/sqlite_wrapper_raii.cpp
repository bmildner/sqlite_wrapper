#include "sqlite_wrapper/raii.h"

#include <sqlite3.h>

namespace sqlite_wrapper
{
  namespace details
  {
    void database_deleter::operator()(::sqlite3* db) const noexcept
    {
      // TODO: log error
      ::sqlite3_close(db);
    }

    void statement_deleter::operator()(::sqlite3_stmt* stmt) const noexcept
    {
      // TODO: log error
      ::sqlite3_finalize(stmt);
    }
  }  // namespace details
}  // namespace sqlite_wrapper
