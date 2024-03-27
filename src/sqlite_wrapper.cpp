// sqlite_wrapper.cpp : Defines the entry point for the application.
//

#include "sqlite_wrapper/sqlite_wrapper.h"

#include <iostream>
#include <filesystem>
#include <cassert>

#include <sqlite3.h>

#include "sqlite_wrapper/create_table.h"

namespace sqlite_wrapper
{
  namespace details
  {
    auto create_prepared_statement(const db_with_location& database, std::string_view sql) -> statement
    {
      sqlite3_stmt* stmt{ nullptr };

      if (const auto result{ ::sqlite3_prepare_v2(database.value, sql.data(), static_cast<int>(sql.size()), &stmt, nullptr) }; (result != SQLITE_OK) || (stmt == nullptr))
      {
        throw sqlite_error(sqlite_wrapper::format("failed to create prepated statement \"{}\"", sql), database, result);
      }

      return statement{ stmt };
    }

    void bind(const stmt_with_location& stmt, int index)
    {
      if (const auto result{ ::sqlite3_bind_null(stmt.value, index) }; result != SQLITE_OK)
      {
        throw sqlite_error(sqlite_wrapper::format("failed to bind null to index {}", index), stmt, result);
      }
    }

    void bind(const stmt_with_location& stmt, int index, std::int64_t value)
    {
      if (const auto result{ ::sqlite3_bind_int64(stmt.value, index, value) }; result != SQLITE_OK)
      {
        throw sqlite_error(sqlite_wrapper::format("failed to bind int64 to index {}", index), stmt, result);
      }
    }

    void bind(const stmt_with_location& stmt, int index, double value)
    {
      if (const auto result{ ::sqlite3_bind_double(stmt.value, index, value) }; result != SQLITE_OK)
      {
        throw sqlite_error(sqlite_wrapper::format("failed to bind double to index {}", index), stmt, result);
      }
    }

    void bind(const stmt_with_location& stmt, int index, std::string_view value)
    {
      if (const auto result{::sqlite3_bind_text64(stmt.value, index, value.data(), value.size(), nullptr, SQLITE_UTF8)}; result != SQLITE_OK)
      {
        throw sqlite_error(sqlite_wrapper::format("failed to bind string to index {}", index), stmt, result);
      }
    }

    void bind(const stmt_with_location& stmt, int index, const_byte_span value)
    {
      if (const auto result{::sqlite3_bind_blob64(stmt.value, index, value.data(), value.size(), nullptr)}; result != SQLITE_OK)
      {
        throw sqlite_error(sqlite_wrapper::format("failed to bind BLOB to index {}", index), stmt, result);
      }
    }

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

      // NOLINTNEXTLINE [cppcoreguidelines-pro-type-reinterpret-cast]
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

      // NOLINTNEXTLINE [cppcoreguidelines-pro-type-reinterpret-cast]
      const auto* data{reinterpret_cast<const std::byte*>(sqlite3_column_blob(stmt.value, index))};
      const auto length{static_cast<std::size_t>(sqlite3_column_bytes(stmt.value, index))};

      if (data == nullptr)
      {
        throw sqlite_error(sqlite_wrapper::format("sqlite3_column_blob() for index {} returned nullptr", index), stmt, SQLITE_NOMEM);
      }

      value.clear();
      value.reserve(length);

      // NOLINTNEXTLINE [cppcoreguidelines-pro-bounds-pointer-arithmetic]
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
        throw sqlite_error(sqlite_wrapper::format("invalid open_flags value  \"{}\"", to_underlying(flags)), SQLITE_ERROR, loc);
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


// TODO: remove or turn into unit-tests
// NOLINTBEGIN

auto to_byte_vector(const std::string_view& str) -> sqlite_wrapper::byte_vector
{
  return {reinterpret_cast<const std::byte*>(str.data()), reinterpret_cast<const std::byte*>(str.data() + str.size())};
}

int XXXmain()
{
  try
  {
    auto test_table{ sqlite_wrapper::table("test_table",
                                           sqlite_wrapper::column<sqlite_wrapper::primary_key>("pk_test_table"),
                                           sqlite_wrapper::column<std::string>("text"),
                                           sqlite_wrapper::column<double>("double"),
                                           sqlite_wrapper::column<std::optional<sqlite_wrapper::byte_vector>>("blob")) };

    decltype(test_table)::full_row row;

    auto other_table{ sqlite_wrapper::table("other_table",
                        sqlite_wrapper::column<sqlite_wrapper::primary_key>("pk_other_table"),
                        sqlite_wrapper::column <sqlite_wrapper::foreign_key>("fk_test_table", test_table)) };


    std::filesystem::remove("Test.database");

    const auto database{sqlite_wrapper::open("Test.database") };

    sqlite_wrapper::execute_no_data(database.get(), "CREATE TABLE IF NOT EXISTS testtable (num INTEGER, double REAL, txt TEXT, blob BLOB)");

    sqlite_wrapper::execute_no_data(database.get(), "INSERT INTO testtable (num, double, txt, blob) VALUES (4711, 1.23, \"Hallo\", X'000102030405060708090a0b0c0d0e0f')");
    sqlite_wrapper::execute_no_data(database.get(), "INSERT INTO testtable (num, double, txt, blob) VALUES (4712, 67.89, \"world\", X'fffefcfbfaf9f8f7f6f5f4f3f2f1f0')");
    sqlite_wrapper::execute_no_data(database.get(), "INSERT INTO testtable (num, double, txt) VALUES (1, 67.89, \"lala\")");
    sqlite_wrapper::execute_no_data(database.get(), "INSERT INTO testtable (double, txt, blob) VALUES (67.89, \"lulu\", X'fffefcfbfaf9f8f7f6f5f4f3f2f1f0')");

    constexpr auto* sql{ "SELECT * from testtable WHERE num > ?" };

    auto stmt{ sqlite_wrapper::create_prepared_statement(database.get(), sql) };
    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, std::int64_t{1});
    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, 1);
    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, nullptr);
    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, std::nullopt);
    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, 1.23);
    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, float{1.23});
    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, "fgsdfg");
    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, std::string("fgsdfg"));

    const auto vector{to_byte_vector("Hello")};

    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, vector);
    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, sqlite_wrapper::const_byte_span{vector});


    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, std::optional<int>{});

    using row_type = std::tuple<std::optional<std::int64_t>, double, std::string, std::optional<sqlite_wrapper::byte_vector>>;

    const auto rows = sqlite_wrapper::execute<row_type>(database.get(), sql, 0);

    assert(rows.size() == 3);
    for (const auto& row : rows)
    {
      std::cout << sqlite_wrapper::format("{} {}\n", std::get<0>(row).value_or(0), std::get<1>(row));
    }
  }
  catch (const std::exception& exception)
  {
    std::cout << "Caught execption, what: " << exception.what() << std::endl;
  }

  return 0;
}
// NOLINTEND