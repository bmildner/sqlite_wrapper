#include "free_function_mock.h"
#include "sqlite_mock.h"

#include "sqlite_wrapper/format.h"
#include "sqlite_wrapper/raii.h"
#include "sqlite_wrapper/sqlite_error.h"
#include "sqlite_wrapper/sqlite_wrapper.h"

#include <sqlite3.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using sqlite_wrapper::mocks::reset_global_mock;
using sqlite_wrapper::mocks::sqlite3_mock;

using ::testing::AllOf;
using ::testing::DoAll;
using ::testing::ElementsAreArray;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::IsNull;
using ::testing::Mock;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgPointee;
using ::testing::StartsWith;
using ::testing::StrEq;
using ::testing::Test;
using ::testing::ThrowsMessage;

extern "C"
{
  struct sqlite3
  {
    int value{};
  };

  struct sqlite3_stmt
  {
    double value{};
  };
}

namespace
{
  constexpr auto* db_file_name{"db_file_name"};
  constexpr std::string_view dummy_sql{"SELECT * FROM table WHERE x == ? AND y != ?"};
  const std::string dummy_sql_str{dummy_sql};
  constexpr auto* sqlite_error_message{"error message from sqlite3_errmsg()"};
  constexpr auto* sqlite_errstr{"error string from sqlite3_errstr(21)"};

  constexpr auto get_mock{[] { return sqlite_wrapper::mocks::get_global_mock<sqlite3_mock>(); }};
  constexpr auto create_and_set_global_mock{[] { return sqlite_wrapper::mocks::create_and_set_global_mock<sqlite3_mock>(); }};

  class sqlite_wrapper_mocked_tests : public Test
  {
   public:
    sqlite_wrapper_mocked_tests() = default;
    ~sqlite_wrapper_mocked_tests() override = default;

    sqlite_wrapper_mocked_tests(const sqlite_wrapper_mocked_tests& other) = delete;
    sqlite_wrapper_mocked_tests(sqlite_wrapper_mocked_tests&& other) noexcept = delete;
    auto operator=(const sqlite_wrapper_mocked_tests& other) -> sqlite_wrapper_mocked_tests& = delete;
    auto operator=(sqlite_wrapper_mocked_tests&& other) noexcept -> sqlite_wrapper_mocked_tests& = delete;

    void SetUp() override;
    void TearDown() override;

    static void expect_open_and_close(std::string_view file_name, sqlite3& database, int flags);
    static void expect_open_and_close(std::string_view file_name, sqlite3& database)
    {
      expect_open_and_close(file_name, database, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    }

    using expect_bind_function = std::function<void(::sqlite3_stmt* stmt, int index, const Sequence& sequence, int sqlite_error)>;
    using expect_bind_list = std::vector<expect_bind_function>;

    static auto expect_null_bind() -> expect_bind_function
    {
      return [](::sqlite3_stmt* stmt, int index, const Sequence& sequence, int sqlite_error)
      {
        EXPECT_CALL(*get_mock(), sqlite3_bind_null(stmt, index))
            .InSequence(sequence)
            .WillOnce(Return(sqlite_error))
            .RetiresOnSaturation();
      };
    }

    static auto expect_int64_bind(std::int64_t value) -> expect_bind_function
    {
      return [value](::sqlite3_stmt* stmt, int index, const Sequence& sequence, int sqlite_error)
      {
        EXPECT_CALL(*get_mock(), sqlite3_bind_int64(stmt, index, value))
            .InSequence(sequence)
            .WillOnce(Return(sqlite_error))
            .RetiresOnSaturation();
      };
    }

    static auto expect_double_bind(double value) -> expect_bind_function
    {
      return [value](::sqlite3_stmt* stmt, int index, const Sequence& sequence, int sqlite_error)
      {
        EXPECT_CALL(*get_mock(), sqlite3_bind_double(stmt, index, value))
            .InSequence(sequence)
            .WillOnce(Return(sqlite_error))
            .RetiresOnSaturation();
      };
    }

    static auto expect_text_bind(const std::string& value) -> expect_bind_function
    {
      return [value](::sqlite3_stmt* stmt, int index, const Sequence& sequence, int sqlite_error)
      {
        EXPECT_CALL(*get_mock(), sqlite3_bind_text64(stmt, index, StrEq(value), value.size(), IsNull(), SQLITE_UTF8))
            .InSequence(sequence)
            .WillOnce(Return(sqlite_error))
            .RetiresOnSaturation();
      };
    }

    static auto expect_blob_bind(const sqlite_wrapper::byte_vector& value) -> expect_bind_function
    {
      return [value](::sqlite3_stmt* stmt, int index, const Sequence& sequence, int sqlite_error)
      {
        EXPECT_CALL(*get_mock(), sqlite3_bind_blob64(stmt, index, NotNull(), value.size(), IsNull()))
            .InSequence(sequence)
            .WillOnce(DoAll(Invoke(
                                [value](sqlite3_stmt*, int, const void* param_value, sqlite3_uint64 byteSize, void (*)(void*))
                                {
                                  const auto data{sqlite_wrapper::const_byte_span{static_cast<const std::byte*>(param_value),
                                                                                  static_cast<std::size_t>(byteSize)}};

                                  ASSERT_THAT(data, ElementsAreArray(value));
                                }),
                            Return(sqlite_error)))
            .RetiresOnSaturation();
      };
    }

    template <typename... Params>
    static auto expect_and_get_statement(const sqlite_wrapper::db_with_location& database, ::sqlite3_stmt& statement,
                                         const expect_bind_list& binders, Params&&... params) -> sqlite_wrapper::statement;

    template <typename... Params>
    static auto expect_and_get_statement(const sqlite_wrapper::db_with_location& database, ::sqlite3_stmt& statement,
                                         Params&&... params) -> sqlite_wrapper::statement
    {
      return expect_and_get_statement(database, statement, {}, std::forward<Params>(params)...);
    }

    template <typename... Params>
    static auto expect_and_get_statement(const sqlite_wrapper::db_with_location& database, const expect_bind_list& binders,
                                         Params&&... params) -> sqlite_wrapper::statement
    {
      static ::sqlite3_stmt statement{};

      auto stmt{expect_and_get_statement(database, statement, binders, std::forward<Params>(params)...)};

      EXPECT_EQ(stmt.get(), &statement);

      return stmt;
    }

    template <typename... Params>
    void expect_and_create_statement_with_failed_binding(const sqlite_wrapper::db_with_location& database,
                                                         const expect_bind_function& binder, std::string_view type,
                                                         Params&&... params);

    static void expect_sqlite_error_with_statement(const sqlite_wrapper::db_with_location& database,
                                                   const sqlite_wrapper::stmt_with_location& statement, const Sequence& sequence,
                                                   const char* error_message);
  };  // class sqlite_wrapper_mocked_tests

  void sqlite_wrapper_mocked_tests::SetUp()
  {
    create_and_set_global_mock();
  }

  void sqlite_wrapper_mocked_tests::TearDown()
  {
    EXPECT_TRUE(Mock::VerifyAndClearExpectations(get_mock().get()));
    reset_global_mock<sqlite3_mock>();
  }

  void sqlite_wrapper_mocked_tests::expect_open_and_close(std::string_view file_name, sqlite3& database, int flags)
  {
    // make sure mocks are called in the correct order!
    const InSequence sequence_guard;

    EXPECT_CALL(*get_mock(), sqlite3_open_v2(StrEq(file_name), NotNull(), flags, nullptr))
        .WillOnce(DoAll(SetArgPointee<1>(&database), Return(SQLITE_OK)))
        .RetiresOnSaturation();
    EXPECT_CALL(*get_mock(), sqlite3_close(Eq(&database))).WillOnce(Return(SQLITE_OK)).RetiresOnSaturation();
  }

  template <typename... Params>
  auto sqlite_wrapper_mocked_tests::expect_and_get_statement(const sqlite_wrapper::db_with_location& database,
                                                             ::sqlite3_stmt& statement, const expect_bind_list& binders,
                                                             Params&&... params) -> sqlite_wrapper::statement
  {
    // make sure mocks are called in the correct order!
    const Sequence sequence{};

    EXPECT_CALL(*get_mock(),
                sqlite3_prepare_v2(database.value, StrEq(dummy_sql), static_cast<int>(dummy_sql.size()), NotNull(), IsNull()))
        .InSequence(sequence)
        .WillOnce(DoAll(SetArgPointee<3>(&statement), Return(SQLITE_OK)))
        .RetiresOnSaturation();

    int index{1};

    for (const auto& binder : binders)
    {
      binder(&statement, index++, sequence, SQLITE_OK);
    }

    EXPECT_CALL(*get_mock(), sqlite3_finalize(&statement)).InSequence(sequence).WillOnce(Return(SQLITE_OK)).RetiresOnSaturation();

    auto stmt{sqlite_wrapper::create_prepared_statement(database, dummy_sql, std::forward<Params>(params)...)};

    return stmt;
  }

  template <typename... Params>
  void sqlite_wrapper_mocked_tests::expect_and_create_statement_with_failed_binding(
      const sqlite_wrapper::db_with_location& database, const expect_bind_function& binder, std::string_view type,
      Params&&... params)
  {
    static ::sqlite3_stmt statement;

    // make sure mocks are called in the correct order!
    const Sequence sequence{};

    constexpr auto* error_message{"bind failed"};

    EXPECT_CALL(*get_mock(),
                sqlite3_prepare_v2(database.value, StrEq(dummy_sql), static_cast<int>(dummy_sql.size()), NotNull(), IsNull()))
        .InSequence(sequence)
        .WillOnce(DoAll(SetArgPointee<3>(&statement), Return(SQLITE_OK)))
        .RetiresOnSaturation();

    binder(&statement, 1, sequence, SQLITE_MISUSE);

    expect_sqlite_error_with_statement(database.value, &statement, sequence, error_message);

    EXPECT_CALL(*get_mock(), sqlite3_finalize(&statement)).InSequence(sequence).WillOnce(Return(SQLITE_OK)).RetiresOnSaturation();

    ASSERT_THAT([&]()
                { (void)sqlite_wrapper::create_prepared_statement(database.value, dummy_sql, std::forward<Params>(params)...); },
                ThrowsMessage<sqlite_wrapper::sqlite_error>(
                    AllOf(StartsWith(sqlite_wrapper::format("failed to bind {} to index 1, failed with:", type)),
                          HasSubstr(dummy_sql), HasSubstr(sqlite_errstr), HasSubstr(error_message))));
  }

  void sqlite_wrapper_mocked_tests::expect_sqlite_error_with_statement(const sqlite_wrapper::db_with_location& database,
                                                                       const sqlite_wrapper::stmt_with_location& statement,
                                                                       const Sequence& sequence, const char* error_message)
  {
    EXPECT_CALL(*get_mock(), sqlite3_sql(statement.value))
        .InSequence(sequence)
        .WillOnce(Return(dummy_sql_str.c_str()))
        .RetiresOnSaturation();

    EXPECT_CALL(*get_mock(), sqlite3_db_handle(statement.value))
        .InSequence(sequence)
        .WillOnce(Return(database.value))
        .RetiresOnSaturation();

    ASSERT_NE(error_message, nullptr);

    EXPECT_CALL(*get_mock(), sqlite3_errmsg(database.value))
        .InSequence(sequence)
        .WillOnce(Return(error_message))
        .RetiresOnSaturation();

    EXPECT_CALL(*get_mock(), sqlite3_errstr(SQLITE_MISUSE))
        .InSequence(sequence)
        .WillOnce(Return(sqlite_errstr))
        .RetiresOnSaturation();
  }

  auto to_byte_vector(std::string_view str) -> sqlite_wrapper::byte_vector
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<const std::byte*>(str.data()), reinterpret_cast<const std::byte*>(str.data() + str.size())};
  }

  auto to_const_byte_span(std::string_view str) -> sqlite_wrapper::const_byte_span
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<const std::byte*>(str.data()), str.size()};
  }

  auto to_byte_span(std::string& str) -> std::span<std::byte>
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<std::byte*>(str.data()), str.size()};
  }
}  // unnamed namespace

TEST_F(sqlite_wrapper_mocked_tests, open_success)
{
  sqlite3 database;

  expect_open_and_close(db_file_name, database);
  expect_open_and_close(db_file_name, database, SQLITE_OPEN_READWRITE);

  ASSERT_EQ(sqlite_wrapper::open(db_file_name).get(), &database);
  ASSERT_EQ(sqlite_wrapper::open(db_file_name, sqlite_wrapper::open_flags::open_only).get(), &database);
}

TEST_F(sqlite_wrapper_mocked_tests, open_fails)
{
  EXPECT_CALL(*get_mock(), sqlite3_open_v2(StrEq(db_file_name), NotNull(), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr))
      .WillOnce(Return(SQLITE_INTERNAL))
      .WillOnce(DoAll(SetArgPointee<1>(nullptr), Return(SQLITE_OK)));
  EXPECT_CALL(*get_mock(), sqlite3_errstr(SQLITE_INTERNAL)).WillOnce(Return("SQLITE_INTERNAL"));
  EXPECT_CALL(*get_mock(), sqlite3_errstr(SQLITE_ERROR)).Times(2).WillRepeatedly(Return("SQLITE_ERROR"));

  ASSERT_THAT([] { (void)sqlite_wrapper::open(db_file_name); },
              testing::ThrowsMessage<sqlite_wrapper::sqlite_error>(
                  AllOf(StartsWith(sqlite_wrapper::format("sqlite3_open() failed to open database \"{}\"", db_file_name)),
                        HasSubstr("SQLITE_INTERNAL"))));

  ASSERT_THAT([] { (void)sqlite_wrapper::open(db_file_name); },
              testing::ThrowsMessage<sqlite_wrapper::sqlite_error>(
                  AllOf(StartsWith(sqlite_wrapper::format("sqlite3_open() returned nullptr for database \"{}\"", db_file_name)),
                        HasSubstr("SQLITE_ERROR"))));

  constexpr sqlite_wrapper::open_flags bad_open_flags{to_underlying(sqlite_wrapper::open_flags::open_only) |
                                                      to_underlying(sqlite_wrapper::open_flags::open_or_create)};

  ASSERT_THAT([] { (void)sqlite_wrapper::open(db_file_name, bad_open_flags); },
              testing::ThrowsMessage<sqlite_wrapper::sqlite_error>(
                  StartsWith(sqlite_wrapper::format("invalid open_flags value \"{}\"", bad_open_flags))));
}

// we can only thest this in release builds because there is an assert!
#ifdef NDEBUG
TEST_F(sqlite_wrapper_mocked_tests, close_fails_silently)
{
  sqlite3 database;

  EXPECT_CALL(*get_mock(), sqlite3_close(Eq(&database))).WillOnce(Return(SQLITE_INTERNAL));

  {
    const sqlite_wrapper::database db_handle{&database};
  }
}
#endif

TEST_F(sqlite_wrapper_mocked_tests, create_prepared_statement_basic_binding_no_param_success)
{
  ::sqlite3 database{};
  ::sqlite3_stmt statement{};

  const auto stmt{expect_and_get_statement(&database, statement)};

  EXPECT_EQ(stmt.get(), &statement);
}

TEST_F(sqlite_wrapper_mocked_tests, create_prepared_statement_basic_binding_null_param_success)
{
  ::sqlite3 database{};

  constexpr std::optional<std::int64_t> optional_int{};
  constexpr std::optional<double> optional_double{};
  constexpr std::optional<std::string> optional_string{};
  constexpr std::optional<sqlite_wrapper::byte_vector> optional_blob{};

  expect_and_get_statement(
      &database,
      {expect_null_bind(), expect_null_bind(), expect_null_bind(), expect_null_bind(), expect_null_bind(), expect_null_bind()},
      nullptr, std::nullopt, optional_int, optional_double, optional_string, optional_blob);
}

TEST_F(sqlite_wrapper_mocked_tests, create_prepared_statement_basic_binding_int64_param_success)
{
  ::sqlite3 database{};

  constexpr auto int64_value{std::int64_t{4711}};
  constexpr auto int32_value{std::int32_t{4712}};
  constexpr auto uint32_value{std::uint32_t{4713}};
  constexpr auto int16_value{std::int16_t{4714}};
  constexpr auto uint16_value{std::uint16_t{4715}};
  constexpr auto int8_value{std::int8_t{16}};
  constexpr auto uint8_value{std::uint8_t{17}};
  constexpr std::optional<std::int64_t> optional_int64{4718};
  constexpr std::optional<std::int16_t> optional_int16{4719};

  expect_and_get_statement(
      &database,
      {expect_int64_bind(int64_value), expect_int64_bind(int32_value), expect_int64_bind(uint32_value),
       expect_int64_bind(int16_value), expect_int64_bind(uint16_value), expect_int64_bind(int8_value),
       expect_int64_bind(uint8_value), expect_int64_bind(optional_int64.value()), expect_int64_bind(optional_int16.value())},
      int64_value, int32_value, uint32_value, int16_value, uint16_value, int8_value, uint8_value, optional_int64, optional_int16);
}

TEST_F(sqlite_wrapper_mocked_tests, create_prepared_statement_basic_binding_double_param_success)
{
  ::sqlite3 database{};

  constexpr auto double_value{1.23};
  constexpr auto float_value{4.56F};
  constexpr std::optional optional_double{9.87};
  constexpr std::optional optional_float{10.3F};

  expect_and_get_statement(&database,
                           {expect_double_bind(double_value), expect_double_bind(float_value),
                            expect_double_bind(optional_double.value()), expect_double_bind(optional_float.value())},
                           double_value, float_value, optional_double, optional_float);
}

TEST_F(sqlite_wrapper_mocked_tests, create_prepared_statement_basic_binding_text_param_success)
{
  ::sqlite3 database{};

  constexpr auto* char_ptr_value{"char* value"};
  constexpr char char_array_value[]{"char[] value"};  // NOLINT(*-avoid-c-arrays)
  const std::string string_value{"std::string value"};
  constexpr auto string_view_value{std::string_view{"std::string_view value"}};
  constexpr std::optional optional_string_view{std::string_view{"optional std::string_view value"}};
  const std::optional optional_string{std::string{"optional std::string value"}};

  expect_and_get_statement(
      &database,
      {expect_text_bind(char_ptr_value), expect_text_bind({static_cast<const char*>(char_array_value)}),
       expect_text_bind(string_value), expect_text_bind(std::string{string_view_value}),
       expect_text_bind(std::string{optional_string_view.value()}), expect_text_bind(optional_string.value())},
      char_ptr_value, char_array_value, string_value, string_view_value, optional_string_view, optional_string);
}

TEST_F(sqlite_wrapper_mocked_tests, create_prepared_statement_basic_binding_blob_param_success)
{
  ::sqlite3 database{};

  const auto byte_vector{to_byte_vector("byte vector blob")};
  const auto const_byte_span{to_const_byte_span("const byte span blob")};
  const std::optional optional_byte_vector{to_byte_vector("optional byte vector blob")};
  const std::optional optional_const_byte_span{to_const_byte_span("optional const byte span blob")};
  std::string non_const_string{"non-const byte span"};
  const std::span<std::byte> non_const_byte_span{to_byte_span(non_const_string)};

  expect_and_get_statement(&database,
                           {expect_blob_bind(byte_vector), expect_blob_bind({const_byte_span.begin(), const_byte_span.end()}),
                            expect_blob_bind(optional_byte_vector.value()),
                            expect_blob_bind({optional_const_byte_span.value().begin(), optional_const_byte_span.value().end()}),
                            expect_blob_bind({non_const_byte_span.begin(), non_const_byte_span.end()})},
                           byte_vector, const_byte_span, optional_byte_vector, optional_const_byte_span, non_const_byte_span);
}

TEST_F(sqlite_wrapper_mocked_tests, create_prepared_statement_range_binding_null_params_success)
{
  ::sqlite3 database{};

  const std::vector vector_nullptr{nullptr, nullptr, nullptr, nullptr};
  constexpr std::array array_nullopt{std::nullopt, std::nullopt, std::nullopt};

  expect_bind_list expected_binder;

  for (size_t i = 0; i < vector_nullptr.size(); i++)
  {
    expected_binder.push_back(expect_null_bind());
  }

  for (size_t i = 0; i < array_nullopt.size(); i++)
  {
    expected_binder.push_back(expect_null_bind());
  }

  expect_and_get_statement(&database, expected_binder, vector_nullptr, array_nullopt);
}

TEST_F(sqlite_wrapper_mocked_tests, create_prepared_statement_range_binding_int64_params_success)
{
  ::sqlite3 database{};

  const std::list<std::int64_t> int64_list{4711, 4712, 4713, 4714, 4715, 4716};
  constexpr std::array<std::int64_t, 4> int64_array{4717, 4718, 4719, 4720};

  expect_bind_list expected_binder;

  for (const auto value : int64_list)
  {
    expected_binder.push_back(expect_int64_bind(value));
  }

  for (const auto value : int64_array)
  {
    expected_binder.push_back(expect_int64_bind(value));
  }

  expect_and_get_statement(&database, expected_binder, int64_list, int64_array);
}

TEST_F(sqlite_wrapper_mocked_tests, create_prepared_statement_range_binding_double_params_success)
{
  ::sqlite3 database{};

  const std::set double_set{0.12345, 1.23456, 2.34567, 3.45678, 4.56789};
  const auto double_view{std::ranges::take_view{
      std::ranges::transform_view{std::ranges::reverse_view{double_set}, [](double value) { return value * 2.0; }}, 3}};

  expect_bind_list expected_binder;

  for (const auto value : double_set)
  {
    expected_binder.push_back(expect_double_bind(value));
  }

  for (const auto value : double_view)
  {
    expected_binder.push_back(expect_double_bind(value));
  }

  expect_and_get_statement(&database, expected_binder, double_set, double_view);
}

TEST_F(sqlite_wrapper_mocked_tests, create_prepared_statement_range_binding_text_params_success)
{
  ::sqlite3 database{};

  using namespace std::literals;
  const std::vector string_vector{"abc"s, "defg"s, "hijklm"s, "nopqrst"s};  // NOLINT(*-include-cleaner)
  const std::list string_view_list{"0.12345"sv, "1.23456"sv, "2.34567"sv, "3.45678"sv, "4.56789"sv};

  expect_bind_list expected_binder;

  for (const auto& value : string_vector)
  {
    expected_binder.push_back(expect_text_bind(value));
  }

  for (const auto value : string_view_list)
  {
    expected_binder.push_back(expect_text_bind(std::string{value}));
  }

  expect_and_get_statement(&database, expected_binder, string_vector, string_view_list);
}

TEST_F(sqlite_wrapper_mocked_tests, create_prepared_statement_fails)
{
  ::sqlite3 database{};

  EXPECT_CALL(*get_mock(),
              sqlite3_prepare_v2(&database, StrEq(dummy_sql), static_cast<int>(dummy_sql.size()), NotNull(), IsNull()))
      .WillOnce(Return(SQLITE_MISUSE));

  EXPECT_CALL(*get_mock(), sqlite3_errmsg(&database)).WillOnce(Return(sqlite_error_message));
  EXPECT_CALL(*get_mock(), sqlite3_errstr(SQLITE_MISUSE)).WillOnce(Return(sqlite_errstr));

  ASSERT_THAT([&]() { (void)sqlite_wrapper::create_prepared_statement(&database, dummy_sql); },
              ThrowsMessage<sqlite_wrapper::sqlite_error>(
                  AllOf(StartsWith("failed to create prepared statement"), HasSubstr(dummy_sql), HasSubstr(sqlite_errstr))));
}

TEST_F(sqlite_wrapper_mocked_tests, bind_value_null_fails)
{
  ::sqlite3 database{};

  expect_and_create_statement_with_failed_binding(&database, expect_null_bind(), "null", nullptr);
}

TEST_F(sqlite_wrapper_mocked_tests, bind_value_int64_fails)
{
  ::sqlite3 database{};
  constexpr auto value{4711ULL};

  expect_and_create_statement_with_failed_binding(&database, expect_int64_bind(value), "int64", value);
}

TEST_F(sqlite_wrapper_mocked_tests, bind_value_double_fails)
{
  ::sqlite3 database{};
  constexpr auto value{3.41};

  expect_and_create_statement_with_failed_binding(&database, expect_double_bind(value), "double", value);
}

TEST_F(sqlite_wrapper_mocked_tests, bind_value_string_fails)
{
  ::sqlite3 database{};
  constexpr auto value{"hello world"};

  expect_and_create_statement_with_failed_binding(&database, expect_text_bind(value), "string", value);
}

TEST_F(sqlite_wrapper_mocked_tests, bind_value_blob_fails)
{
  ::sqlite3 database{};
  const auto value{to_byte_vector("hello world")};

  expect_and_create_statement_with_failed_binding(&database, expect_blob_bind(value), "BLOB", value);
}

TEST_F(sqlite_wrapper_mocked_tests, step_success)
{
  ::sqlite3_stmt statement{};

  const Sequence sequence{};

  EXPECT_CALL(*get_mock(), sqlite3_step(&statement)).InSequence(sequence).WillOnce(Return(SQLITE_ROW)).RetiresOnSaturation();
  EXPECT_CALL(*get_mock(), sqlite3_step(&statement)).InSequence(sequence).WillOnce(Return(SQLITE_DONE)).RetiresOnSaturation();

  ASSERT_TRUE(sqlite_wrapper::step(&statement));
  ASSERT_FALSE(sqlite_wrapper::step(&statement));
}

TEST_F(sqlite_wrapper_mocked_tests, step_fails)
{
  ::sqlite3 database{};
  ::sqlite3_stmt statement{};
  constexpr auto* error_message{"step failure"};

  const Sequence sequence{};

  EXPECT_CALL(*get_mock(), sqlite3_step(&statement)).InSequence(sequence).WillOnce(Return(SQLITE_MISUSE));

  expect_sqlite_error_with_statement(&database, &statement, sequence, error_message);

  ASSERT_THAT([&]() { (void)sqlite_wrapper::step(&statement); },
              ThrowsMessage<sqlite_wrapper::sqlite_error>(AllOf(StartsWith("failed to step, failed with:"), HasSubstr(dummy_sql),
                                                                HasSubstr(sqlite_errstr), HasSubstr(error_message))));
}

// TODO: tests for other database base types and other containers/ranges
