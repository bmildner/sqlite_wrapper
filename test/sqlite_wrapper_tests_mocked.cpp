#include <string>

#include <gtest/gtest.h>

#include "sqlite_wrapper/sqlite_wrapper.h"

#include "sqlite_mock.h"
#include "free_function_mock.h"

using sqlite_wrapper::mocks::sqlite3_mock;
using sqlite3_mock_ptr = sqlite_wrapper::mocks::mock_ptr<sqlite3_mock>;

using testing::Mock;
using testing::Test;
using testing::StrEq;
using testing::NotNull;
using testing::Return;

namespace
{
  class sqlite_wrapper_mocked_tests : public Test
  {
  public:
    sqlite_wrapper_mocked_tests() = default;
    ~sqlite_wrapper_mocked_tests() override = default;

    void SetUp() override;

    void TearDown() override;
 
    sqlite3_mock_ptr m_mock{};
  };

  void sqlite_wrapper_mocked_tests::SetUp()
  {
    m_mock = std::make_shared<sqlite3_mock>();
  }

  void sqlite_wrapper_mocked_tests::TearDown()
  {
    EXPECT_TRUE(Mock::VerifyAndClearExpectations(m_mock.get()));
    m_mock.reset();
  }

  constexpr auto* db_file_name{"db file name"};
}  // unnamed namespace

TEST_F(sqlite_wrapper_mocked_tests, open)
{
  EXPECT_CALL(*m_mock, sqlite3_open(StrEq(db_file_name), NotNull())).WillOnce(Return(SQLITE_OK));

  ASSERT_NE(sqlite_wrapper::open(db_file_name), nullptr);
}
