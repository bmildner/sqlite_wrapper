#include "sqlite_wrapper/format.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <source_location>

using ::testing::HasSubstr;
using ::testing::ThrowsMessage;

TEST(sqlite_wrapper_format_tests, format_source_location_success)
{
  constexpr auto location{std::source_location::current()};

  const auto result{sqlite_wrapper::format("{}", location)};

  ASSERT_EQ(result, sqlite_wrapper::format("{}:{}:{}: {}", location.file_name(), location.line(), location.column(), location.function_name()));

  ASSERT_EQ(sqlite_wrapper::format("{}", location), sqlite_wrapper::format("{:}", location));
}

#ifndef SQLITEWRAPPER_FORMAT_USE_FMT
TEST(sqlite_wrapper_format_tests, format_source_location_fails)
{
  const auto loc{std::source_location::current()};

  ASSERT_THAT(
      [&]()
      {
        // NOLINTNEXTLINE(*-include-cleaner)
        (void) SQLITEWRAPPER_FORMAT_NAMESPACE::vformat("{:6}", SQLITEWRAPPER_FORMAT_NAMESPACE::make_format_args(loc));
      },
      ThrowsMessage<sqlite_wrapper::format_error>(HasSubstr("unknown format specifier")));
}
#endif

namespace
{
  struct my_struct
  {
    int i{4711};
  };
}

template <>
// NOLINTNEXTLINE(*-include-cleaner)
struct SQLITEWRAPPER_FORMAT_NAMESPACE_NAME::formatter<my_struct> : sqlite_wrapper::empty_format_spec
{
  template <typename FmtContext>
  // NOLINTNEXTLINE(*-include-cleaner)
  static auto format(my_struct mys, FmtContext& ctx)
  {
    // NOLINTNEXTLINE(*-include-cleaner)
    return SQLITEWRAPPER_FORMAT_NAMESPACE::format_to(ctx.out(), "{}", mys.i);
  }
};


TEST(sqlite_wrapper_format_tests, empty_format_spec_failes_if_not_empty)
{
  constexpr my_struct mys;

  EXPECT_EQ(sqlite_wrapper::format("{}", mys), sqlite_wrapper::format("{}", mys.i));

  // NOLINTNEXTLINE(*-include-cleaner)
  ASSERT_THAT([&]() { (void) SQLITEWRAPPER_FORMAT_NAMESPACE::vformat("{:6}", SQLITEWRAPPER_FORMAT_NAMESPACE::make_format_args(mys)); },
              ThrowsMessage<sqlite_wrapper::format_error>(HasSubstr("unknown format specifier")));

  // NOLINTNEXTLINE(*-include-cleaner)
  ASSERT_THAT([&]() { (void) SQLITEWRAPPER_FORMAT_NAMESPACE::vformat("{: }", SQLITEWRAPPER_FORMAT_NAMESPACE::make_format_args(mys)); },
              ThrowsMessage<sqlite_wrapper::format_error>(HasSubstr("unknown format specifier")));

  // NOLINTNEXTLINE(*-include-cleaner)
  ASSERT_THAT([&]() { (void) SQLITEWRAPPER_FORMAT_NAMESPACE::vformat("{:\t}", SQLITEWRAPPER_FORMAT_NAMESPACE::make_format_args(mys)); },
              ThrowsMessage<sqlite_wrapper::format_error>(HasSubstr("unknown format specifier")));
}

/*
 * TODO: implement ASSERT_DOES_NOT_COMPILE()!
TEST(sqlite_wrapper_format_tests, format_source_location_fails_to_compile)
{
  using namespace std::literals;

  constexpr auto fmt_str{"{:6}"sv};

  ASSERT_DOES_NOT_COMPILE((void)sqlite_wrapper::format({:6}, std::source_location::current()), sqlite_wrapper::format_error);
}
*/
