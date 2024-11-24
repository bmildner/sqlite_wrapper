#include "sqlite_wrapper/error.h"

#include <string>

#include <sqlite3.h>

#include "sqlite_wrapper/format.h"

namespace sqlite_wrapper
{
  namespace
  {
    auto error_code_to_string(int error) -> std::string
    {
      if (error == SQLITE_OK)
      {
        return "";
      }

      const auto* err_str{::sqlite3_errstr(error)};

      return (err_str != nullptr) ? err_str : sqlite_wrapper::format("<unknown value: {}>", error);
    }

    auto error_to_string(sqlite3* raw_db_handle, int error) -> std::string
    {
      const auto* err_msg{(raw_db_handle != nullptr) ? ::sqlite3_errmsg(raw_db_handle) : nullptr};

      if (err_msg != nullptr)
      {
        return sqlite_wrapper::format("{} {}", err_msg, error_code_to_string(error));
      }

      return sqlite_wrapper::format("{}", error_code_to_string(error));
    }

    auto error_to_string(sqlite3_stmt* stmt, int error) -> std::string
    {
      const auto* sql_str{::sqlite3_sql(stmt)};

      return sqlite_wrapper::format("{} for SQL \"{}\"", error_to_string(::sqlite3_db_handle(stmt), error), (sql_str != nullptr) ? sql_str : "<nullptr>");
    }
  }  // unnamed namespace

  sqlite_error::sqlite_error(const std::string& what, const db_with_location& database, int error)
    : std::runtime_error(sqlite_wrapper::format("{}, failed with {}", what, error_to_string(database.value, error))), m_location(database.location)
  {}

  sqlite_error::sqlite_error(const std::string& what, const stmt_with_location& stmt, int error)
    : std::runtime_error(sqlite_wrapper::format("{}, failed with {}", what, error_to_string(stmt.value, error))), m_location(stmt.location)
  {}

  auto sqlite_error::where() const -> const std::source_location&
  {
    return m_location;
  }

}  // namespace sqlite_qrapper
