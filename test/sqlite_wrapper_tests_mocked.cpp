#include <string_view>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "sqlite_wrapper/format.h"
#include "sqlite_wrapper/sqlite_wrapper.h"

#include "sqlite_mock.h"
#include "free_function_mock.h"
#include "assert_throw_msg.h"

using sqlite_wrapper::mocks::sqlite3_mock;

using sqlite_wrapper::mocks::reset_global_mock;
constexpr auto get_mock{[] { return sqlite_wrapper::mocks::get_global_mock<sqlite3_mock>(); }};
constexpr auto create_and_set_global_mock{[] { return sqlite_wrapper::mocks::create_and_set_global_mock<sqlite3_mock>(); }};

using testing::Mock;
using testing::Test;
using testing::StrEq;
using testing::NotNull;
using testing::IsNull;
using testing::Return;
using testing::SetArgPointee;
using testing::DoAll;
using testing::Eq;
using testing::StartsWith;
using testing::HasSubstr;
using testing::AllOf;
using testing::Invoke;
using testing::ElementsAreArray;

extern "C"
{
  struct sqlite3
  {
    int in_t{};
  };

  struct sqlite3_stmt
  {
    double dub{};
  };
}

namespace
{
  constexpr auto* db_file_name{"db file name"};
  constexpr std::string_view dummy_sql{"SELECT * FROM table WHERE x == ? AND y != ?"};

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

    static auto expect_and_get_database() -> sqlite_wrapper::database;

    using expect_bind_function = std::function<void(::sqlite3_stmt* stmt, int index)>;
    using expect_bind_list = std::vector<expect_bind_function>;

    static auto expect_null_bind() -> expect_bind_function
    {
      return [] (::sqlite3_stmt* stmt, int index)
      {
        EXPECT_CALL(*get_mock(), sqlite3_bind_null(stmt, index)).WillOnce(Return(SQLITE_OK)).RetiresOnSaturation();
      };
    }

    static auto expect_int64_bind(std::int64_t value) -> expect_bind_function
    {
      return [value] (::sqlite3_stmt* stmt, int index)
      {
        EXPECT_CALL(*get_mock(), sqlite3_bind_int64(stmt, index, value)).WillOnce(Return(SQLITE_OK)).RetiresOnSaturation();
      };
    }

    static auto expect_double_bind(double value) -> expect_bind_function
    {
      return [value] (::sqlite3_stmt* stmt, int index)
      {
        EXPECT_CALL(*get_mock(), sqlite3_bind_double(stmt, index, value)).WillOnce(Return(SQLITE_OK)).RetiresOnSaturation();
      };
    }

    static auto expect_text_bind(const std::string& value) -> expect_bind_function
    {
      return [value] (::sqlite3_stmt* stmt, int index)
      {
        EXPECT_CALL(*get_mock(), sqlite3_bind_text64(stmt, index, StrEq(value), value.size(), IsNull(), SQLITE_UTF8))
            .WillOnce(Return(SQLITE_OK)).RetiresOnSaturation();
      };
    }

    static auto expect_text_blob(const sqlite_wrapper::byte_vector& value) -> expect_bind_function
    {
      return [value] (::sqlite3_stmt* stmt, int index)
      {
        EXPECT_CALL(*get_mock(), sqlite3_bind_blob64(stmt, index, NotNull(), value.size(), IsNull()))
            .WillOnce(DoAll(Invoke([value] (sqlite3_stmt*, int, const void* param_value, sqlite3_uint64 byteSize, void(*)(void*))
                                   {
                                     // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                                     const auto data{sqlite_wrapper::const_byte_span{reinterpret_cast<const std::byte*>(param_value), byteSize}};

                                     ASSERT_THAT(data, ElementsAreArray(value));
                                   }), Return(SQLITE_OK)))
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

      return expect_and_get_statement(database, statement, binders, std::forward<Params>(params)...);
    }

    template <typename... Params>
    static auto expect_and_get_statement(const sqlite_wrapper::db_with_location& database,
                                         Params&&... params) -> sqlite_wrapper::statement
    {
      return expect_and_get_statement(database, {}, std::forward<Params>(params)...);
    }
  };

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
    EXPECT_CALL(*get_mock(), sqlite3_open_v2(StrEq(file_name), NotNull(), flags, nullptr))
        .WillOnce(DoAll(SetArgPointee<1>(&database), Return(SQLITE_OK)))
        .RetiresOnSaturation();
    EXPECT_CALL(*get_mock(), sqlite3_close(Eq(&database)))
        .Times(1)
        .RetiresOnSaturation();
  }

  auto sqlite_wrapper_mocked_tests::expect_and_get_database() -> sqlite_wrapper::database
  {
    static sqlite3 database{};

    expect_open_and_close(db_file_name, database);

    return sqlite_wrapper::open(db_file_name);
  }

  template <typename... Params>
  auto sqlite_wrapper_mocked_tests::expect_and_get_statement(const sqlite_wrapper::db_with_location& database,
                                                             ::sqlite3_stmt& statement, const expect_bind_list& binders,
                                                             Params&&... params) -> sqlite_wrapper::statement
  {
    EXPECT_CALL(*get_mock(), sqlite3_prepare_v2(database.value, StrEq(dummy_sql), static_cast<int>(dummy_sql.size()), NotNull(), IsNull()))
        .WillOnce(DoAll(SetArgPointee<3>(&statement), Return(SQLITE_OK)));
    EXPECT_CALL(*get_mock(), sqlite3_finalize(&statement)).WillOnce(Return(SQLITE_OK));

    int index{1};

    for (const auto& binder : binders)
    {
      binder(&statement, index++);
    }

    auto stmt{sqlite_wrapper::create_prepared_statement(database, dummy_sql, std::forward<Params>(params)...)};

    return stmt;
  }

  auto to_byte_vector(const std::string_view& str) -> sqlite_wrapper::byte_vector
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<const std::byte*>(str.data()), reinterpret_cast<const std::byte*>(str.data() + str.size())};
  }

  auto to_const_byte_span(const std::string_view& str) -> sqlite_wrapper::const_byte_span
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<const std::byte*>(str.data()), str.size()};
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

  ASSERT_THROW_MSG((void) sqlite_wrapper::open(db_file_name), sqlite_wrapper::sqlite_error,
    AllOf(StartsWith(sqlite_wrapper::format("sqlite3_open() failed to open database \"{}\"", db_file_name)), HasSubstr("SQLITE_INTERNAL")));

  ASSERT_THROW_MSG((void)sqlite_wrapper::open(db_file_name), sqlite_wrapper::sqlite_error,
    AllOf(StartsWith(sqlite_wrapper::format("sqlite3_open() returned nullptr for database \"{}\"", db_file_name)), HasSubstr("SQLITE_ERROR")));

  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  const sqlite_wrapper::open_flags bad_open_flags{sqlite_wrapper::to_underlying(sqlite_wrapper::open_flags::open_only) |
                                                  sqlite_wrapper::to_underlying(sqlite_wrapper::open_flags::open_or_create)};

  ASSERT_THROW_MSG((void)sqlite_wrapper::open(db_file_name, bad_open_flags), sqlite_wrapper::sqlite_error,
                   StartsWith(sqlite_wrapper::format("invalid open_flags value  \"{}\"", to_underlying(bad_open_flags))));
}

TEST_F(sqlite_wrapper_mocked_tests, create_prepared_statement_success)
{
  const auto database{expect_and_get_database()};

  // no parameter
  {
    ::sqlite3_stmt statement{};

    const auto stmt{expect_and_get_statement(database.get(), statement)};

    EXPECT_EQ(stmt.get(), &statement);
  }

  // null parameter
  {
    ::sqlite3_stmt statement{};

    const auto stmt{expect_and_get_statement(database.get(), statement,
                                             {expect_null_bind(), expect_null_bind()},
                                             nullptr, std::nullopt)};

    EXPECT_EQ(stmt.get(), &statement);
  }

  // std::int64_t parameter
  {
    constexpr auto int64_value{std::int64_t{4711}};
    constexpr auto int32_value{std::int32_t{4712}};
    constexpr auto uint32_value{std::uint32_t{4713}};
    constexpr auto int16_value{std::int16_t{4714}};
    constexpr auto uint16_value{std::uint16_t{4715}};
    constexpr auto int8_value{std::int8_t{16}};
    constexpr auto uint8_value{std::uint8_t{17}};

    ::sqlite3_stmt statement{};

    const auto stmt{expect_and_get_statement(database.get(), statement,
                                             {expect_int64_bind(int64_value),
                                                 expect_int64_bind(int32_value), expect_int64_bind(uint32_value),
                                                 expect_int64_bind(int16_value), expect_int64_bind(uint16_value),
                                                 expect_int64_bind(int8_value), expect_int64_bind(uint8_value)},
                                             int64_value,
                                             int32_value, uint32_value,
                                             int16_value, uint16_value,
                                             int8_value, uint8_value)};

    EXPECT_EQ(stmt.get(), &statement);
  }

  // double parameter
  {
    constexpr auto double_value{double{1.23}};
    constexpr auto float_value{float{4.56F}};

    ::sqlite3_stmt statement{};

    const auto stmt{expect_and_get_statement(database.get(), statement,
                                             {expect_double_bind(double_value), expect_double_bind(float_value)},
                                             double_value, float_value)};

    EXPECT_EQ(stmt.get(), &statement);
  }

  // text parameter
  {
    constexpr auto* char_ptr_value{"char* value"};
    constexpr char char_array_value[]{"char[] value"};  // NOLINT(*-avoid-c-arrays)
    const std::string string_value{"std::string value"};
    constexpr auto string_view_value{std::string_view{"std::string_view value"}};

    ::sqlite3_stmt statement{};

    const auto stmt{expect_and_get_statement(database.get(), statement,
                                             {expect_text_bind(char_ptr_value),
                                              expect_text_bind({static_cast<const char*>(char_array_value)}),
                                              expect_text_bind(string_value),
                                              expect_text_bind(std::string{string_view_value})},
                                             char_ptr_value, char_array_value, string_value, string_view_value)};

    EXPECT_EQ(stmt.get(), &statement);
  }

  // blob parameter
  {
    const auto byte_vector{to_byte_vector("byte vector blob")};
    const auto const_byte_span{to_const_byte_span("const byte span blob")};

    ::sqlite3_stmt statement{};

    const auto stmt{expect_and_get_statement(database.get(), statement,
                                             {expect_text_blob(byte_vector),
                                              expect_text_blob({const_byte_span.begin(), const_byte_span.end()})},
                                             byte_vector, const_byte_span)};

    EXPECT_EQ(stmt.get(), &statement);
  }
}
