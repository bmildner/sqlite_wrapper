#pragma once

#include <stdexcept>
#include <source_location>

#include "sqlite_wrapper/config.h"
#include "sqlite_wrapper/location.h"

namespace sqlite_wrapper
{
  class sqlite_error : public std::runtime_error
  {
  public:
    SQLITE_WRAPPER_EXPORT sqlite_error(const std::string& what, const db_with_location& db, int error);

    SQLITE_WRAPPER_EXPORT sqlite_error(const std::string& what, const stmt_with_location& stmt, int error);

    inline sqlite_error(const std::string& what, const stmt_with_location& stmt)
      : sqlite_error(what, stmt, 0)
    {}

    inline sqlite_error(const std::string& what, int error, const std::source_location& loc = std::source_location::current())
      : sqlite_error(what, {static_cast<sqlite3*>(nullptr), loc}, error)
    {}

    SQLITE_WRAPPER_EXPORT ~sqlite_error() override = default;

    SQLITE_WRAPPER_EXPORT auto where() const -> const std::source_location&;

  private:
    std::source_location m_location;
  };

}  // namespace sqlite_qrapper
