#include <string>

#include <gtest/gtest.h>

#include "sqlite_wrapper/sqlite_wrapper.h"
#include "sqlite_wrapper/format.h"

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
using testing::Return;
using testing::SetArgPointee;
using testing::DoAll;
using testing::Eq;
using testing::StartsWith;
using testing::HasSubstr;
using testing::AllOf;

namespace
{
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

  constexpr auto* db_file_name{"db file name"};
}  // unnamed namespace

extern "C"
struct sqlite3
{
  int i{};
};

TEST_F(sqlite_wrapper_mocked_tests, open_success)
{
  sqlite3 database;

  EXPECT_CALL(*get_mock(), sqlite3_open_v2(StrEq(db_file_name), NotNull(), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr))
  .WillOnce(DoAll(SetArgPointee<1>(&database), Return(SQLITE_OK)));
  EXPECT_CALL(*get_mock(), sqlite3_close(Eq(&database))).Times(1);

  ASSERT_EQ(sqlite_wrapper::open(db_file_name).get(), &database);
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
