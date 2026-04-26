#include "assert_throws_with_msg.h"

#include "sqlite_wrapper/format.h"
#include "sqlite_wrapper/raii.h"
#include "sqlite_wrapper/sqlite_error.h"
#include "sqlite_wrapper/sqlite_wrapper.h"
#include "sqlite_wrapper/tuple_utils.h"

#include <sqlite3.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <optional>
#include <random>
#include <source_location>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

using namespace std::string_view_literals;

using ::testing::AllOf;
using ::testing::HasSubstr;
using ::testing::StartsWith;
using ::testing::StrEq;
using ::testing::Test;

namespace
{
  class random_data_generator
  {
   public:
    static constexpr std::size_t default_string_and_blob_size_limit{50};

    random_data_generator() = default;

    template <sqlite_wrapper::database_type... Columns>
      requires(sizeof...(Columns) >= 1)
    void generate_random_row(std::size_t string_and_blob_size_limit, Columns&... columns);

    template <sqlite_wrapper::row_type Row>
    [[nodiscard]] auto generate_random_rows(std::size_t row_count, std::size_t string_and_blob_size_limit) -> std::vector<Row>;

    template <sqlite_wrapper::row_type Row>
    [[nodiscard]] auto generate_random_rows(std::size_t row_count) -> std::vector<Row>;

   private:
    void generate_random_column(std::size_t string_and_blob_size_limit, std::int64_t& column);
    void generate_random_column(std::size_t string_and_blob_size_limit, double& column);
    void generate_random_column(std::size_t string_and_blob_size_limit, sqlite_wrapper::byte_vector& column);
    void generate_random_column(std::size_t string_and_blob_size_limit, std::string& column);

    template <sqlite_wrapper::optional_database_type OptionalColumn>
    void generate_random_column(std::size_t string_and_blob_size_limit, OptionalColumn& column);

    static constexpr auto get_and_print_seed{[]()
                                             {
                                               std::random_device random_device;
                                               const auto seed = random_device();
                                               std::cout << sqlite_wrapper::format("Using random seed: {}\n", seed);
                                               return seed;
                                             }};

    std::mt19937 m_rng{get_and_print_seed()};
  };

  void random_data_generator::generate_random_column([[maybe_unused]] std::size_t string_and_blob_size_limit,
                                                     std::int64_t& column)
  {
    column = std::uniform_int_distribution<std::int64_t>(std::numeric_limits<std::int64_t>::min(),
                                                         std::numeric_limits<std::int64_t>::max())(m_rng);
  }

  void random_data_generator::generate_random_column([[maybe_unused]] std::size_t string_and_blob_size_limit, double& column)
  {
    column = std::uniform_real_distribution(std::numeric_limits<double>::min(), std::numeric_limits<double>::max())(m_rng);
  }

  void random_data_generator::generate_random_column(std::size_t string_and_blob_size_limit, sqlite_wrapper::byte_vector& column)
  {
    const auto size = std::uniform_int_distribution<std::size_t>(1, string_and_blob_size_limit)(m_rng);

    column.resize(size);
    std::ranges::generate(column,
                          [&]()
                          {
                            // apparently std::uniform_int_distribution can not be used with "char, signed char, unsigned char,
                            // char8_t, int8_t, and uint8_t"
                            return static_cast<sqlite_wrapper::byte_vector::value_type>(
                                std::uniform_int_distribution<unsigned short>(0x00, 0xff)(m_rng));
                          });
  }

  void random_data_generator::generate_random_column(std::size_t string_and_blob_size_limit, std::string& column)
  {
    const auto size = std::uniform_int_distribution<std::size_t>(1, string_and_blob_size_limit)(m_rng);

    column.resize(size);
    // apparently std::uniform_int_distribution can not be used with "char, signed char, unsigned char, char8_t, int8_t, and
    // uint8_t"
    std::ranges::generate(column,
                          [&]() { return static_cast<char>(std::uniform_int_distribution<unsigned short>('0', 'z')(m_rng)); });
  }

  template <sqlite_wrapper::optional_database_type OptionalColumn>
  void random_data_generator::generate_random_column(std::size_t string_and_blob_size_limit, OptionalColumn& column)
  {
    if (std::uniform_int_distribution(0, 1)(m_rng))
    {
      column.emplace();
      generate_random_column(string_and_blob_size_limit, *column);
    }
    else
    {
      column.reset();
    }
  }

  template <sqlite_wrapper::database_type... Columns>
    requires(sizeof...(Columns) >= 1)
  void random_data_generator::generate_random_row(std::size_t string_and_blob_size_limit, Columns&... columns)
  {
    (generate_random_column(string_and_blob_size_limit, columns), ...);
  }

  template <sqlite_wrapper::row_type Row>
  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  auto random_data_generator::generate_random_rows(std::size_t row_count, std::size_t string_and_blob_size_limit)
      -> std::vector<Row>
  {
    std::vector<Row> rows;
    rows.reserve(row_count);

    for (size_t i = 0; i < row_count; ++i)
    {
      Row row;
      std::apply([this, string_and_blob_size_limit](sqlite_wrapper::database_type auto&... columns)
                 { this->generate_random_row(string_and_blob_size_limit, columns...); }, row);
      rows.emplace_back(std::move(row));
    }
    return rows;
  }

  template <sqlite_wrapper::row_type Row>
  auto random_data_generator::generate_random_rows(std::size_t row_count) -> std::vector<Row>
  {
    return generate_random_rows<Row>(row_count, default_string_and_blob_size_limit);
  }

  class sqlite_wrapper_tests : public Test
  {
   public:
    static constexpr auto select_all_from_test_table{"SELECT * FROM Test"sv};
    static const std::filesystem::path temp_db_file_name;

    sqlite_wrapper_tests() = default;
    ~sqlite_wrapper_tests() override = default;

    sqlite_wrapper_tests(const sqlite_wrapper_tests& other) = delete;
    sqlite_wrapper_tests(sqlite_wrapper_tests&& other) noexcept = delete;
    auto operator=(const sqlite_wrapper_tests& other) -> sqlite_wrapper_tests& = delete;
    auto operator=(sqlite_wrapper_tests&& other) noexcept -> sqlite_wrapper_tests& = delete;

   protected:
    void SetUp() override;

    void TearDown() override;

    using row_type = std::tuple<std::int64_t, std::string, double, sqlite_wrapper::byte_vector, std::optional<std::int64_t>,
                                std::optional<std::string>, std::optional<double>, std::optional<sqlite_wrapper::byte_vector>>;

    [[nodiscard]] static auto set_up_test_database() -> sqlite_wrapper::database;
    [[nodiscard]] static auto fill_test_database(sqlite_wrapper::db_with_location database, std::size_t row_count)
        -> std::vector<row_type>;
    [[nodiscard]] static auto fill_test_database(sqlite_wrapper::db_with_location database) -> std::vector<row_type>
    {
      return fill_test_database(database, random_data_generator::default_string_and_blob_size_limit);
    }
  };

  const std::filesystem::path sqlite_wrapper_tests::temp_db_file_name{std::filesystem::temp_directory_path() /
                                                                      "sqlite_wrapper_test.db"};

  void sqlite_wrapper_tests::SetUp()
  {
    std::filesystem::remove(temp_db_file_name);
  }

  void sqlite_wrapper_tests::TearDown()
  {
    std::filesystem::remove(temp_db_file_name);
  }

  auto sqlite_wrapper_tests::set_up_test_database() -> ::sqlite_wrapper::database
  {
    auto database{sqlite_wrapper::open(temp_db_file_name.string())};

    EXPECT_NE(database.get(), nullptr) << "Expected non-null database handle!";

    constexpr auto create_test_table_sql{
        R"(CREATE TABLE "Test" (
		"Id"		INTEGER NOT NULL UNIQUE,
		"Int"		INTEGER NOT NULL,
		"String"	TEXT NOT NULL,
		"Double"	REAL NOT NULL,
		"Blob"		BLOB NOT NULL,
		"OptInt"	INTEGER,
		"OptString"	TEXT,
		"OptDouble"	REAL,
		"OptBlob"	BLOB,
		PRIMARY KEY("Id" AUTOINCREMENT)))"sv};

    sqlite_wrapper::execute_no_data(database.get(), create_test_table_sql);

    return database;
  }

  auto sqlite_wrapper_tests::fill_test_database(sqlite_wrapper::db_with_location database, std::size_t row_count)
      -> std::vector<row_type>
  {
    constexpr auto insert_into_test_table{
        R"(INSERT INTO "Test" ("Int", "String", "Double", "Blob", "OptInt", "OptString", "OptDouble", "OptBlob") VALUES (?, ?, ?, ?, ?, ?, ?, ?))"};

    random_data_generator generator;
    const auto rows{generator.generate_random_rows<row_type>(row_count)};

    sqlite_wrapper::statement stmt{sqlite_wrapper::create_prepared_statement(database, insert_into_test_table)};

    for (const auto& row : rows)
    {
      std::apply([&](const sqlite_wrapper::database_type auto&... columns)
                 { sqlite_wrapper::reset_and_rebind_prepared_statement(stmt.get(), columns...); }, row);
      EXPECT_FALSE(sqlite_wrapper::step(stmt.get()));
    }

    return rows;
  }
}  // unnamed namespace

TEST_F(sqlite_wrapper_tests, test_sqlite_type_to_string)
{
  ASSERT_THAT(sqlite_wrapper::details::sqlite_type_to_string(SQLITE_INTEGER),
              StrEq(sqlite_wrapper::format("INTEGER ({})", SQLITE_INTEGER)));
  ASSERT_THAT(sqlite_wrapper::details::sqlite_type_to_string(SQLITE_FLOAT),
              StrEq(sqlite_wrapper::format("FLOAT ({})", SQLITE_FLOAT)));
  ASSERT_THAT(sqlite_wrapper::details::sqlite_type_to_string(SQLITE_BLOB),
              StrEq(sqlite_wrapper::format("BLOB ({})", SQLITE_BLOB)));
  ASSERT_THAT(sqlite_wrapper::details::sqlite_type_to_string(SQLITE_NULL),
              StrEq(sqlite_wrapper::format("NULL ({})", SQLITE_NULL)));
  ASSERT_THAT(sqlite_wrapper::details::sqlite_type_to_string(SQLITE3_TEXT),
              StrEq(sqlite_wrapper::format("TEXT ({})", SQLITE3_TEXT)));
  ASSERT_THAT(sqlite_wrapper::details::sqlite_type_to_string(-1), StrEq(sqlite_wrapper::format("<unknown type> ({})", -1)));
}

TEST_F(sqlite_wrapper_tests, test_open)
{
  {
    auto database{sqlite_wrapper::open(temp_db_file_name.string())};

    ASSERT_NE(database.get(), nullptr);
    ASSERT_TRUE(std::filesystem::exists(temp_db_file_name));
  }
  {
    auto database{sqlite_wrapper::open(temp_db_file_name.string(), sqlite_wrapper::open_flags::open_only)};
    ASSERT_NE(database.get(), nullptr);
  }
}

TEST_F(sqlite_wrapper_tests, test_open_fails)
{
  std::source_location location{};

  ASSERT_THROWS_WITH_MSG_AND_STACK(
      [&]
      {
        location = std::source_location::current();
        (void)sqlite_wrapper::open(temp_db_file_name.string(), sqlite_wrapper::open_flags::open_only);
      },
      sqlite_wrapper::sqlite_error,
      AllOf(StartsWith("sqlite3_open() failed to open database"), HasSubstr("failed with: unable to open database file"),
            HasSubstr(temp_db_file_name.string()), HasSubstr(location.file_name()), HasSubstr(location.function_name())),
      AllOf(sqlite_wrapper::stack_trace_contains_function("sqlite_wrapper::open"),
            sqlite_wrapper::stack_trace_contains_function_in("test_open_fails", std::source_location::current().file_name())));
}

TEST_F(sqlite_wrapper_tests, test_open_failes_get_location)
{
  std::source_location location{};

  try
  {
    location = std::source_location::current();
    (void)sqlite_wrapper::open(temp_db_file_name.string(), sqlite_wrapper::open_flags::open_only);
  }
  catch (const sqlite_wrapper::sqlite_error& error)
  {
    const auto error_location{error.where()};

    ASSERT_STREQ(location.file_name(), error_location.file_name());
    ASSERT_STREQ(location.function_name(), error_location.function_name());
    ASSERT_EQ(location.line() + 1, error_location.line());

    return;
  }
  FAIL() << "Did not catch sqlite_wrapper::sqlite_error as expected";
}

TEST_F(sqlite_wrapper_tests, test_open_flags_formating)
{
  ASSERT_EQ(sqlite_wrapper::format("{}", sqlite_wrapper::open_flags::open_only), "open_only");
  ASSERT_EQ(sqlite_wrapper::format("{}", sqlite_wrapper::open_flags::open_or_create), "open_or_create");
  // NOLINTNEXTLINE(*.EnumCastOutOfRange)
  ASSERT_EQ(sqlite_wrapper::format("{}", static_cast<sqlite_wrapper::open_flags>(999)), "<unknown (999)>");
}

TEST_F(sqlite_wrapper_tests, test_simple_select_query)
{
  const auto database{set_up_test_database()};
  const auto rows{fill_test_database(database.get())};

  const auto stmt{sqlite_wrapper::create_prepared_statement(database.get(), select_all_from_test_table)};

  auto row_iter{rows.cbegin()};
  std::int64_t row_id_iter{1};
  while (sqlite_wrapper::step(stmt.get()))
  {
    using read_row_type = sqlite_wrapper::add_type_front<std::int64_t, row_type>;
    const auto [row_id, row] = sqlite_wrapper::pop_front(sqlite_wrapper::get_row<read_row_type>(stmt.get()));

    ASSERT_NE(row_iter, rows.cend()) << "More rows returned from database than inserted!";
    ASSERT_EQ(row_id_iter++, row_id) << "Generated row id does not match expected value!";
    ASSERT_EQ(*(row_iter++), row) << "Returned row data does not match inserted row!";
  }
}
