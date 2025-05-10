#include "sqlite_wrapper/create_table.h"

#include "sqlite_wrapper/format.h"

#include <cassert>
#include <filesystem>
#include <iostream>

// TODO: remove or turn into unit-tests
// NOLINTBEGIN GCOVR_EXCL_START

auto to_byte_vector(const std::string_view& str) -> sqlite_wrapper::byte_vector
{
  return {reinterpret_cast<const std::byte*>(str.data()), reinterpret_cast<const std::byte*>(str.data() + str.size())};
}

int XXXmain()
{
  try
  {
    auto test_table{sqlite_wrapper::table("test_table", sqlite_wrapper::column<sqlite_wrapper::primary_key>("pk_test_table"),
                                          sqlite_wrapper::column<std::string>("text"), sqlite_wrapper::column<double>("double"),
                                          sqlite_wrapper::column<std::optional<sqlite_wrapper::byte_vector>>("blob"))};

    decltype(test_table)::full_row the_row;

    auto other_table{sqlite_wrapper::table("other_table", sqlite_wrapper::column<sqlite_wrapper::primary_key>("pk_other_table"),
                                           sqlite_wrapper::column<sqlite_wrapper::foreign_key>("fk_test_table", test_table))};

    std::filesystem::remove("Test.database");

    const auto database{sqlite_wrapper::open("Test.database")};

    sqlite_wrapper::execute_no_data(database.get(),
                                    "CREATE TABLE IF NOT EXISTS testtable (num INTEGER, double REAL, txt TEXT, blob BLOB)");

    sqlite_wrapper::execute_no_data(
        database.get(),
        "INSERT INTO testtable (num, double, txt, blob) VALUES (4711, 1.23, \"Hallo\", X'000102030405060708090a0b0c0d0e0f')");
    sqlite_wrapper::execute_no_data(
        database.get(),
        "INSERT INTO testtable (num, double, txt, blob) VALUES (4712, 67.89, \"world\", X'fffefcfbfaf9f8f7f6f5f4f3f2f1f0')");
    sqlite_wrapper::execute_no_data(database.get(), "INSERT INTO testtable (num, double, txt) VALUES (1, 67.89, \"lala\")");
    sqlite_wrapper::execute_no_data(
        database.get(), "INSERT INTO testtable (double, txt, blob) VALUES (67.89, \"lulu\", X'fffefcfbfaf9f8f7f6f5f4f3f2f1f0')");

    constexpr auto* sql{"SELECT * from testtable WHERE num > ?"};

    auto stmt{sqlite_wrapper::create_prepared_statement(database.get(), sql)};
    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, std::int64_t{1});
    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, 1);
    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, nullptr);
    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, std::nullopt);
    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, 1.23);
    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, 1.23f);
    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, "fgsdfg");
    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, std::string("fgsdfg"));

    const auto vector{to_byte_vector("Hello")};

    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, vector);
    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, sqlite_wrapper::const_byte_span{vector});

    stmt = sqlite_wrapper::create_prepared_statement(database.get(), sql, std::optional<int>{});

    using row_type = std::tuple<std::optional<std::int64_t>, double, std::string, std::optional<sqlite_wrapper::byte_vector>>;

    const auto rows = sqlite_wrapper::execute<row_type>(database.get(), sql);

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
// NOLINTEND GCOVR_EXCL_STOP
