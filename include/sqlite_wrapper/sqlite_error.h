#pragma once

#include "sqlite_wrapper/config.h"
#include "sqlite_wrapper/raii.h"

#include <source_location>
#include <stdexcept>
#include <string_view>

namespace sqlite_wrapper
{
  class sqlite_error : public std::runtime_error
  {
   public:
    SQLITE_WRAPPER_EXPORT sqlite_error(std::string_view what, const db_with_location& database, int error);

    SQLITE_WRAPPER_EXPORT sqlite_error(std::string_view what, const stmt_with_location& stmt, int error);

    sqlite_error(std::string_view what, const stmt_with_location& stmt) : sqlite_error(what, stmt, 0) {}

    sqlite_error(std::string_view what, int error, const std::source_location& loc = std::source_location::current())
        : sqlite_error(what, db_with_location{static_cast<sqlite3*>(nullptr), loc}, error)
    {
    }

    SQLITE_WRAPPER_EXPORT ~sqlite_error() override = default;

    [[nodiscard]] SQLITE_WRAPPER_EXPORT auto where() const -> const std::source_location&;

    SQLITE_WRAPPER_EXPORT sqlite_error(const sqlite_error& other) = default;
    SQLITE_WRAPPER_EXPORT sqlite_error(sqlite_error&& other) = default;

    SQLITE_WRAPPER_EXPORT auto operator=(const sqlite_error& other) -> sqlite_error& = default;
    SQLITE_WRAPPER_EXPORT auto operator=(sqlite_error&& other) -> sqlite_error& = default;

   private:
    std::source_location m_location;
  };

}  // namespace sqlite_wrapper
