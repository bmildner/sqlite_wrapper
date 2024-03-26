#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <filesystem>

#include "sqlite_wrapper/sqlite_wrapper.h"

#include "assert_throw_msg.h"

using ::testing::Test;
using ::testing::StartsWith;
using ::testing::EndsWith;
using ::testing::AllOf;
using ::testing::HasSubstr;

namespace
{
  const std::filesystem::path temp_db_file_name{std::filesystem::temp_directory_path() / "sqlite_wrapper_test.db"};

  class sqlite_wrapper_tests : public Test
  {
  protected:
    sqlite_wrapper_tests();
    ~sqlite_wrapper_tests() override;

    void SetUp() override;

    void TearDown() override;
  };

  sqlite_wrapper_tests::sqlite_wrapper_tests() = default;
  sqlite_wrapper_tests::~sqlite_wrapper_tests() = default;

  void sqlite_wrapper_tests::SetUp()
  {
    std::filesystem::remove(temp_db_file_name);
  }

  void sqlite_wrapper_tests::TearDown()
  {
    std::filesystem::remove(temp_db_file_name);
  }

}  // unnamed namespace

TEST_F(sqlite_wrapper_tests, open)
{
  auto db{sqlite_wrapper::open(temp_db_file_name.string())};
  
  ASSERT_NE(db.get(), nullptr);
  ASSERT_TRUE(std::filesystem::exists(temp_db_file_name));
  ASSERT_THROW(std::filesystem::remove(temp_db_file_name), std::filesystem::filesystem_error);
}

TEST_F(sqlite_wrapper_tests, open_failes)
{
  const auto bad_db_file_name{std::string("___") + temp_db_file_name.string()};

  ASSERT_THROW_MSG(sqlite_wrapper::open(bad_db_file_name), sqlite_wrapper::sqlite_error,
    AllOf(StartsWith("failed to open database"), HasSubstr(bad_db_file_name), EndsWith("failed with unable to open database file")));
}