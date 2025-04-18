#include "sqlite_wrapper/sqlite_wrapper.h"

#include "sqlite_wrapper/format.h"
#include "sqlite_wrapper/raii.h"

#include <sqlite3.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <source_location>
#include <string>
#include <string_view>

namespace sqlite_wrapper
{
  namespace details
  {
    auto create_prepared_statement(const db_with_location& database, std::string_view sql) -> statement
    {
      sqlite3_stmt* stmt{ nullptr };

      if (const auto result{ ::sqlite3_prepare_v2(database.value, sql.data(), static_cast<int>(sql.size()), &stmt, nullptr) }; (result != SQLITE_OK) || (stmt == nullptr))
      {
        assert(stmt == nullptr);
        throw sqlite_error(sqlite_wrapper::format("failed to create prepared statement \"{}\"", sql), database, result);
      }

      return statement{stmt};
    }

    void bind_value(const stmt_with_location& stmt, int index)
    {
      if (const auto result{ ::sqlite3_bind_null(stmt.value, index) }; result != SQLITE_OK)
      {
        throw sqlite_error(sqlite_wrapper::format("failed to bind null to index {}", index), stmt, result);
      }
    }

    void bind_value(const stmt_with_location& stmt, int index, std::int64_t value)
    {
      if (const auto result{ ::sqlite3_bind_int64(stmt.value, index, value) }; result != SQLITE_OK)
      {
        throw sqlite_error(sqlite_wrapper::format("failed to bind int64 to index {}", index), stmt, result);
      }
    }

    void bind_value(const stmt_with_location& stmt, int index, double value)
    {
      if (const auto result{ ::sqlite3_bind_double(stmt.value, index, value) }; result != SQLITE_OK)
      {
        throw sqlite_error(sqlite_wrapper::format("failed to bind double to index {}", index), stmt, result);
      }
    }

    void bind_value(const stmt_with_location& stmt, int index, std::string_view value)
    {
      if (const auto result{::sqlite3_bind_text64(stmt.value, index, value.data(), value.size(), nullptr, SQLITE_UTF8)}; result != SQLITE_OK)
      {
        throw sqlite_error(sqlite_wrapper::format("failed to bind string to index {}", index), stmt, result);
      }
    }

    void bind_value(const stmt_with_location& stmt, int index, const_byte_span value)
    {
      if (const auto result{::sqlite3_bind_blob64(stmt.value, index, value.data(), value.size(), nullptr)}; result != SQLITE_OK)
      {
        throw sqlite_error(sqlite_wrapper::format("failed to bind BLOB to index {}", index), stmt, result);
      }
    }

    namespace
    {
      /**
       * Checks a column in a "ready to be returned row", in a prepared statement, to have the expected type.
       *
       * @param stmt statement
       * @param index index of column to check (0 based)
       * @param expected_type expected column type
       * @param maybe_null indicates if NULL is allowed.
       *
       * @throws sqlite_error exception if expected_type does not match the actual columns type.
       * @returns true if the type matches, false if, and only if, it is NULL AND maybe_null is true.
       */
      auto check_null_and_column_type(const stmt_with_location& stmt, int index, int expected_type, bool maybe_null) -> bool
      {
        const auto type{sqlite3_column_type(stmt.value, index)};

        if (type == SQLITE_NULL)
        {
          if (maybe_null)
          {
            return false;
          }

          throw sqlite_error(sqlite_wrapper::format("column at index {} must not be NULL", index), stmt, SQLITE_MISMATCH);
        }

        if (type != expected_type)
        {
          throw sqlite_error(sqlite_wrapper::format("column at index {} has type {}, expected {}", index, type, expected_type), stmt, SQLITE_MISMATCH);
        }

        return true;
      }
    }

    auto get_column(const stmt_with_location& stmt, int index, std::int64_t& value, bool maybe_null) -> bool
    {
      if (!details::check_null_and_column_type(stmt, index, SQLITE_INTEGER, maybe_null))
      {
        return false;
      }

      value = sqlite3_column_int64(stmt.value, index);

      return true;
    }

    auto get_column(const stmt_with_location& stmt, int index, double& value, bool maybe_null) -> bool
    {
      if (!details::check_null_and_column_type(stmt, index, SQLITE_FLOAT, maybe_null))
      {
        return false;
      }

      value = sqlite3_column_double(stmt.value, index);

      return true;
    }

    auto get_column(const stmt_with_location& stmt, int index, std::string& value, bool maybe_null) -> bool
    {
      if (!details::check_null_and_column_type(stmt, index, SQLITE_TEXT, maybe_null))
      {
        return false;
      }

      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      const auto* str{reinterpret_cast<const char*>(sqlite3_column_text(stmt.value, index))};
      const auto length{static_cast<std::size_t>(sqlite3_column_bytes(stmt.value, index))};

      if (str == nullptr)
      {
        throw sqlite_error(sqlite_wrapper::format("sqlite3_column_text() for index {} returned nullptr", index), stmt, SQLITE_NOMEM);
      }

      value = std::string{str, length};

      return true;
    }

    auto get_column(const stmt_with_location& stmt, int index, byte_vector& value, bool maybe_null) -> bool
    {
      if (!details::check_null_and_column_type(stmt, index, SQLITE_BLOB, maybe_null))
      {
        return false;
      }

      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      const auto* data{reinterpret_cast<const std::byte*>(sqlite3_column_blob(stmt.value, index))};
      const auto length{static_cast<std::size_t>(sqlite3_column_bytes(stmt.value, index))};

      if (data == nullptr)
      {
        throw sqlite_error(sqlite_wrapper::format("sqlite3_column_blob() for index {} returned nullptr", index), stmt, SQLITE_NOMEM);
      }

      value.clear();
      value.reserve(length);

      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      value.insert(value.cbegin(), data, data + length);

      return true;
    }
  }  // namespace details

  auto open(const std::string& file_name, open_flags flags, const std::source_location& loc) -> database
  {
    sqlite3* raw_db_handle{nullptr};

    int sqlite_flags{};

    switch (flags)
    {
      case open_flags::open_or_create:
        sqlite_flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
        break;
      case open_flags::open_only:
        sqlite_flags = SQLITE_OPEN_READWRITE;
        break;
      default:
        throw sqlite_error(sqlite_wrapper::format("invalid open_flags value \"{}\"", flags), SQLITE_ERROR, loc);
    }

    if (const auto result{::sqlite3_open_v2(file_name.c_str(), &raw_db_handle, sqlite_flags, nullptr)}; result != SQLITE_OK)
    {
      if (raw_db_handle != nullptr)
      {
        // SQLite3 does return a handle we need to close in some error cases, see documentation
        ::sqlite3_close(raw_db_handle);
      }

      throw sqlite_error(sqlite_wrapper::format("sqlite3_open() failed to open database \"{}\"", file_name), result, loc);
    }

    if (raw_db_handle == nullptr)
    {
      // make sure we do not return a nullptr
      throw sqlite_error(sqlite_wrapper::format("sqlite3_open() returned nullptr for database \"{}\"", file_name), SQLITE_ERROR, loc);
    }

    return database{raw_db_handle};
  }

  auto step(const stmt_with_location& stmt) -> bool
  {
    const auto result{::sqlite3_step(stmt.value)};

    if ((result != SQLITE_DONE) && (result != SQLITE_ROW))
    {
      throw sqlite_error("failed to step", stmt, result);
    }

    return (result == SQLITE_ROW);
  }

}  // namespace sqlite_wrapper
