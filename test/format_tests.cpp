#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "sqlite_wrapper/format.h"

TEST(sqlite_wrapper_format_tests, format_source_location_success)
{
  constexpr auto location{std::source_location::current()};

  const auto result{sqlite_wrapper::format("{}", location)};

  ASSERT_EQ(result, sqlite_wrapper::format("{}:{}:{}: {}", location.file_name(), location.line(), location.column(), location.function_name()));

  ASSERT_EQ(sqlite_wrapper::format("{}", location), sqlite_wrapper::format("{:}", location));
}

// only enabled for fmt::format or std::format if std::runtime_format is available (C++26)
#if defined(SQLITEWRAPPER_FORMAT_USE_FMT) || (__cpp_lib_format >= 202311L)

using ::testing::HasSubstr;
using ::testing::ThrowsMessage;

TEST(sqlite_wrapper_format_tests, format_source_location_fails)
{
  ASSERT_THAT([&]()
              { (void)sqlite_wrapper::format(sqlite_wrapper::runtime_format("{:6}"), std::source_location::current()); },
              ThrowsMessage<sqlite_wrapper::format_error>(HasSubstr("unknown format specifier")));
}

struct my_struct
{
  int i{4711};
};

template <>
struct SQLITEWRAPPER_FORMAT_NAMESPACE_NAME::formatter<my_struct> : sqlite_wrapper::empty_format_spec
{
  template <typename FmtContext>
  static auto format(my_struct mys, FmtContext& ctx)
  {
    return SQLITEWRAPPER_FORMAT_NAMESPACE::format_to(ctx.out(), "{}", mys.i);
  }
};

TEST(sqlite_wrapper_format_tests, formatting_fails_in_empty_format_spec)
{
  const my_struct mys;

  EXPECT_EQ(sqlite_wrapper::format("{}", mys), sqlite_wrapper::format("{}", mys.i));

  ASSERT_THAT([&]() { (void)sqlite_wrapper::format(sqlite_wrapper::runtime_format("{:6}"), mys); },
              ThrowsMessage<sqlite_wrapper::format_error>(HasSubstr("unknown format specifier")));

  ASSERT_THAT([&]() { (void)sqlite_wrapper::format(sqlite_wrapper::runtime_format("{: }"), mys); },
              ThrowsMessage<sqlite_wrapper::format_error>(HasSubstr("unknown format specifier")));

  ASSERT_THAT([&]() { (void)sqlite_wrapper::format(sqlite_wrapper::runtime_format("{:\t}"), mys); },
              ThrowsMessage<sqlite_wrapper::format_error>(HasSubstr("unknown format specifier")));
}
#endif

/*
 * TODO: implement ASSERT_DOES_NOT_COMPILE()!
TEST(sqlite_wrapper_format_tests, format_source_location_fails_to_compile)
{
  using namespace std::literals;

  constexpr auto fmt_str{"{:6}"sv};

  ASSERT_DOES_NOT_COMPILE((void)sqlite_wrapper::format({:6}, std::source_location::current()), sqlite_wrapper::format_error);
}
*/
