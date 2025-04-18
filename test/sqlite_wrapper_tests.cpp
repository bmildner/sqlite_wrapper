#include "sqlite_wrapper/sqlite_error.h"
#include "sqlite_wrapper/sqlite_wrapper.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <source_location>

using ::testing::AllOf;
using ::testing::HasSubstr;
using ::testing::StartsWith;
using ::testing::Test;
using ::testing::ThrowsMessage;

namespace
{
  const std::filesystem::path temp_db_file_name{std::filesystem::temp_directory_path() / "sqlite_wrapper_test.db"};

  class sqlite_wrapper_tests : public Test
  {
   public:
    sqlite_wrapper_tests();
    ~sqlite_wrapper_tests() override;

    sqlite_wrapper_tests(const sqlite_wrapper_tests& other) = delete;
    sqlite_wrapper_tests(sqlite_wrapper_tests&& other) noexcept = delete;
    auto operator=(const sqlite_wrapper_tests& other) -> sqlite_wrapper_tests& = delete;
    auto operator=(sqlite_wrapper_tests&& other) noexcept -> sqlite_wrapper_tests& = delete;

   protected:
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

TEST_F(sqlite_wrapper_tests, open_failes)
{
  std::source_location location{};

  ASSERT_THAT(
      [&]
      {
        location = std::source_location::current();
        (void)sqlite_wrapper::open(temp_db_file_name.string(), sqlite_wrapper::open_flags::open_only);
      },
      ThrowsMessage<sqlite_wrapper::sqlite_error>(
          AllOf(StartsWith("sqlite3_open() failed to open database"), HasSubstr("failed with: unable to open database file"),
                HasSubstr(temp_db_file_name.string()), HasSubstr(location.file_name()), HasSubstr(location.function_name()))));
}
