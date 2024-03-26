#include <string>

#include <gtest/gtest.h>

#include "sqlite_wrapper/sqlite_wrapper.h"
#include "sqlite_wrapper/format.h"

#include "sqlite_mock.h"
#include "free_function_mock.h"
#include "assert_throw_msg.h"

using sqlite_wrapper::mocks::sqlite3_mock;
using sqlite3_mock_ptr = sqlite_wrapper::mocks::mock_ptr<sqlite3_mock>;

using sqlite_wrapper::mocks::set_global_mock;
using sqlite_wrapper::mocks::reset_global_mock;
constexpr auto get_mock{[] { return sqlite_wrapper::mocks::get_global_mock<sqlite3_mock>(); }};

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

    void SetUp() override;

    void TearDown() override;
  };

  void sqlite_wrapper_mocked_tests::SetUp()
  {   
    set_global_mock(std::make_shared<sqlite3_mock>());
  }

  void sqlite_wrapper_mocked_tests::TearDown()
  {
    EXPECT_TRUE(Mock::VerifyAndClearExpectations(get_mock().get()));
    reset_global_mock<sqlite3_mock>();
  }

  constexpr auto* db_file_name{"db file name"};
  sqlite3*const db_ptr{const_cast<sqlite3* const>(reinterpret_cast<const sqlite3*>(db_file_name))};
}  // unnamed namespace

TEST_F(sqlite_wrapper_mocked_tests, open_success)
{
  EXPECT_CALL(*get_mock(), sqlite3_open(StrEq(db_file_name), NotNull())).WillOnce(DoAll(SetArgPointee<1>(db_ptr), Return(SQLITE_OK)));
  EXPECT_CALL(*get_mock(), sqlite3_close(Eq(db_ptr))).Times(1);

  ASSERT_EQ(sqlite_wrapper::open(db_file_name).get(), db_ptr);
}

TEST_F(sqlite_wrapper_mocked_tests, open_fails)
{
  EXPECT_CALL(*get_mock(), sqlite3_open(StrEq(db_file_name), NotNull())).WillOnce(Return(SQLITE_ERROR));
  EXPECT_CALL(*get_mock(), sqlite3_errstr(SQLITE_ERROR)).WillOnce(Return("SQLITE_ERROR"));

  ASSERT_THROW_MSG((void) sqlite_wrapper::open(db_file_name), sqlite_wrapper::sqlite_error,
    AllOf(StartsWith(sqlite_wrapper::format("sqlite3_open() failed to open database \"{}\"", db_file_name)), HasSubstr("SQLITE_ERROR")));
}
