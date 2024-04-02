#include <string>

#include <gtest/gtest.h>

#include "sqlite_wrapper/format.h"
#include "sqlite_wrapper/sqlite_wrapper.h"

#include "sqlite_mock.h"
#include "free_function_mock.h"
#include "assert_throw_msg.h"

using sqlite_wrapper::mocks::sqlite3_mock;
using sqlite3_mock_ptr = sqlite_wrapper::mocks::mock_ptr<sqlite3_mock>;

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

    template <typename... Params>
    static auto expect_and_get_statement(const sqlite_wrapper::db_with_location& database, sqlite3_stmt& statement, Params&&... params) -> sqlite_wrapper::statement;
    template <typename... Params>
    static auto expect_and_get_statement(const sqlite_wrapper::db_with_location& database, Params&&... params) -> sqlite_wrapper::statement;
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
  auto sqlite_wrapper_mocked_tests::expect_and_get_statement(const sqlite_wrapper::db_with_location& database, sqlite3_stmt& statement, Params&&... params) -> sqlite_wrapper::statement
  {
    EXPECT_CALL(*get_mock(), sqlite3_prepare_v2(database.value, StrEq(dummy_sql), dummy_sql.size(), NotNull(), IsNull()))
        .WillOnce(DoAll(SetArgPointee<3>(&statement), Return(SQLITE_OK)));
    EXPECT_CALL(*get_mock(), sqlite3_finalize(&statement)).WillOnce(Return(SQLITE_OK));

    auto stmt{sqlite_wrapper::create_prepared_statement(database, dummy_sql, std::forward<Params>(params)...)};

    return stmt;
  }

  template <typename... Params>
  auto sqlite_wrapper_mocked_tests::expect_and_get_statement(const sqlite_wrapper::db_with_location& database, Params&&... params) -> sqlite_wrapper::statement
  {
    static sqlite3_stmt statement{};

    return expect_and_get_statement(database, statement, std::forward<Params>(params)...);
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

  // NOLINTNEXTLINE [hicpp-signed-bitwise]
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
    sqlite3_stmt statement{};

    const auto stmt{expect_and_get_statement(database.get(), statement)};

    EXPECT_EQ(stmt.get(), &statement);
  }

  // std::int64_t parameter
  {
    // TODO: add callback(?) for expected bindings!
    const auto stmt{expect_and_get_statement(database.get(), std::int64_t{4711})};

    EXPECT_NE(stmt.get(), nullptr);
  }
}
