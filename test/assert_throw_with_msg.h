#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <concepts>
#include <exception>
#include <functional>
#include <optional>
#include <source_location>
#include <string>
#include <string_view>

namespace sqlite_wrapper::details
{
  [[nodiscard]] auto demangle(const char* name) -> std::string;

  template <typename ExpectedException, std::invocable Statement>
  [[nodiscard]] auto assert_throw_with_msg(Statement statement, std::string_view statement_as_text,
                                           std::optional<std::string>& what,
                                           const std::source_location& location = std::source_location::current())
  {
    const auto line_number{static_cast<int>(location.line())};
    what.reset();

    try
    {
      std::invoke(statement);
      FAIL_AT(location.file_name(), line_number) << "Expected: \"" << statement_as_text << "\" to throw an exception of type "
                                                 << demangle(typeid(ExpectedException).name()) << ".\n"
                                                 << "  Actual: it throws nothing.";
    }
    catch (const testing::AssertionException&)
    {
      // we need to ignore testing::AssertionException because FAIL() might throw them!
      throw;
    }
    catch (const ExpectedException& e)
    {
      if (typeid(ExpectedException) != typeid(e))
      {
        FAIL_AT(location.file_name(), line_number) << "Expected: \"" << statement_as_text << "\" to throw an exception of type "
                                                   << demangle(typeid(ExpectedException).name()) << ".\n"
                                                   << "  Actual: it throws type " << demangle(typeid(e).name());
      }
      what = e.what();
    }
    catch (const std::exception& e)
    {
      FAIL_AT(location.file_name(), line_number)
          << "Expected: \"" << statement_as_text << "\" to throw an exception of type "
          << demangle(typeid(ExpectedException).name()) << ".\n"
          << "  Actual: it throws type " << sqlite_wrapper::details::demangle(typeid(e).name());
    }
    catch (...)
    {
      FAIL_AT(location.file_name(), line_number) << "Expected: \"" << statement_as_text << "\" to throw an exception of type "
                                                 << demangle(typeid(ExpectedException).name()) << ".\n"
                                                 << "  Actual: it throws an unknown type.";
    }
  }
}  // namespace sqlite_wrapper::details

// NOLINTNEXTLINE(*-macro-usage)
#define ASSERT_THROW_WITH_MSG2(statement, expected_exception, matcher)                                         \
  {                                                                                                            \
    std::optional<std::string> what;                                                                           \
    if (sqlite_wrapper::details::assert_throw_with_msg<expected_exception>(statement, #statement, what); what) \
    {                                                                                                          \
      ASSERT_THAT(what.value(), matcher);                                                                      \
    }                                                                                                          \
  }

// NOLINTNEXTLINE(*-macro-usage)
#define ASSERT_THROW_WITH_MSG(statement, expected_exception, matcher)                             \
  try                                                                                             \
  {                                                                                               \
    statement;                                                                                    \
    FAIL() << "Expected: \"" #statement "\" to throw an exception of type " #expected_exception   \
              ".\n"                                                                               \
              "  Actual: it throws nothing.";                                                     \
  }                                                                                               \
  catch (const testing::AssertionException&)                                                      \
  {                                                                                               \
    /* we need to ignore testing::AssertionException because FAIL() might throw them! */          \
    throw;                                                                                        \
  }                                                                                               \
  catch (const expected_exception& e)                                                             \
  {                                                                                               \
    if (typeid(expected_exception) != typeid(e))                                                  \
    {                                                                                             \
      FAIL() << "Expected: \"" #statement "\" to throw an exception of type " #expected_exception \
                ".\n"                                                                             \
                "  Actual: it throws type "                                                       \
             << sqlite_wrapper::details::demangle(typeid(e).name());                              \
    }                                                                                             \
    ASSERT_THAT(std::string_view{e.what()}, matcher);                                             \
  }                                                                                               \
  catch (const std::exception& e)                                                                 \
  {                                                                                               \
    FAIL() << "Expected: \"" #statement "\" to throw an exception of type " #expected_exception   \
              ".\n"                                                                               \
              "  Actual: it throws type "                                                         \
           << sqlite_wrapper::details::demangle(typeid(e).name());                                \
  }                                                                                               \
  catch (...)                                                                                     \
  {                                                                                               \
    FAIL() << "Expected: \"" #statement "\" to throw an exception of type " #expected_exception   \
              ".\n"                                                                               \
              "  Actual: it throws an unknown type.";                                             \
  }
