#include "assert_throws_with_msg.h"

#include "sqlite_wrapper/format.h"
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

  ASSERT_THROWS_WITH_MSG(
      [&]
      {
        location = std::source_location::current();
        (void)sqlite_wrapper::open(temp_db_file_name.string(), sqlite_wrapper::open_flags::open_only);
      },
      sqlite_wrapper::sqlite_error,
      AllOf(StartsWith("sqlite3_open() failed to open database"), HasSubstr("failed with: unable to open database file"),
            HasSubstr(temp_db_file_name.string()), HasSubstr(location.file_name()), HasSubstr(location.function_name())));
}

TEST_F(sqlite_wrapper_tests, open_failes_get_location)
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

    ASSERT_EQ(location.file_name(), error_location.file_name());
    ASSERT_EQ(location.function_name(), error_location.function_name());
    ASSERT_EQ(location.line() + 1, error_location.line());

    return;
  }
  FAIL() << "Did not catch sqlite_wrapper::sqlite_error as expected";
}

TEST_F(sqlite_wrapper_tests, open_flags_formating)
{
  ASSERT_EQ(sqlite_wrapper::format("{}", sqlite_wrapper::open_flags::open_only), "open_only");
  ASSERT_EQ(sqlite_wrapper::format("{}", sqlite_wrapper::open_flags::open_or_create), "open_or_create");
  // NOLINTNEXTLINE(*.EnumCastOutOfRange)
  ASSERT_EQ(sqlite_wrapper::format("{}", static_cast<sqlite_wrapper::open_flags>(999)), "<unknown (999)>");
}
