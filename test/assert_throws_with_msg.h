#pragma once

#include "sqlite_wrapper/concepts.h"
#include "sqlite_wrapper/sqlite_error.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <concepts>
#include <exception>
#include <functional>
#include <optional>
#include <source_location>
#include <stacktrace>
#include <string>
#include <string_view>

namespace sqlite_wrapper
{
  namespace details
  {
    /**
     * Demangle typenames returned from typeid(...).name()/std::type_info.name() if needed.
     * Simply returns \p name if compiler returns already demangled name.
     *
     * @param name (mangled) name returned by std::type_info.name()
     * @return demangled name, returns \p name if demangle failed or {} if name is a nullptr
     */
    [[nodiscard]] auto demangle(const char* name) -> std::string;

    template <typename ExpectedException, std::invocable Statement>
    void assert_throws_with_msg(Statement statement, std::string_view statement_as_text, std::optional<std::string>& what,
                                std::optional<std::stacktrace>& stack_trace,
                                const std::source_location& location = std::source_location::current())
    {
      const auto line_number{static_cast<int>(location.line())};
      what.reset();
      stack_trace.reset();

      try
      {
        std::invoke(statement);
        FAIL_AT(location.file_name(), line_number) << "Expected: \"" << statement_as_text << "\" to throw an exception of type "
                                                   << demangle(typeid(ExpectedException).name()) << ".\n"
                                                   << "  Actual: it throws nothing.";
      }
      // we need to ignore testing::AssertionException because FAIL() might throw them!
      catch (const testing::AssertionException&)
      {
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
        if constexpr (are_stack_traces_supported() && exception_with_stack_trace<ExpectedException>)
        {
          stack_trace = e.stack_trace();
        }
      }
      catch (const std::exception& e)
      {
        FAIL_AT(location.file_name(), line_number) << "Expected: \"" << statement_as_text << "\" to throw an exception of type "
                                                   << demangle(typeid(ExpectedException).name()) << ".\n"
                                                   << "  Actual: it throws type " << demangle(typeid(e).name());
      }
      catch (...)
      {
        FAIL_AT(location.file_name(), line_number) << "Expected: \"" << statement_as_text << "\" to throw an exception of type "
                                                   << demangle(typeid(ExpectedException).name()) << ".\n"
                                                   << "  Actual: it throws an unknown type.";
      }
    }
  }  // namespace details

  /**
   * Matcher for std::stacktrace that checks if any frame in the stack trace contains a given function name.
   */
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
  MATCHER_P(stack_trace_contains_function, function_name, "")
  {
    return std::ranges::any_of(arg, [this](const auto& frame) { return frame.description().contains(function_name); });
  }

  /**
   * Matcher for std::stacktrace that checks if any frame in the stack trace contains a given function name where the file name
   * ends with the given file name.
   */
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
  MATCHER_P2(stack_trace_contains_function_in, function_name, file_name, "")
  {
    return std::ranges::any_of(
        arg, [this](const auto& frame)
        { return (frame.description().contains(function_name)) && (frame.source_file().ends_with(file_name)); });
  }
}  // namespace sqlite_wrapper

/**
 *  Replacement for ASSERT_THAT() with ThrowsMessage<>.
 *  Advantages:
 *    Does not execute \p statement a second time in case something fails (like ThrowsMessage<> does) and cause numerous
 *    unexpected calls to mocked functions. The given \p expected_exception must match exactly, can not be something derived from
 *    it! Has only a thin macro wrapper, mostly implemented as a template function.
 *
 * @param statement something that fulfills the std::invocable concept
 * @param expected_exception exact type of exception that is expected to be thrown by std::invoke(statement)
 * @param msg_matcher any gtest matcher that can be used with an std::string to validate the .what() of \p expected_exception
 */
#define ASSERT_THROWS_WITH_MSG(statement, expected_exception, msg_matcher) /* NOLINT(*-macro-usage) */             \
  {                                                                                                                \
    std::optional<std::string> what;                                                                               \
    [[maybe_unused]] std::optional<std::stacktrace> stack_trace;                                                   \
    sqlite_wrapper::details::assert_throws_with_msg<expected_exception>(statement, #statement, what, stack_trace); \
    ASSERT_TRUE(what.has_value());                                                                                 \
    ASSERT_THAT(*what, msg_matcher);                                             /* NOLINT(bugprone-unchecked-optional-access) */                                  \
  }

/**
 *  Replacement for ASSERT_THAT() with ThrowsMessage<> with support for exceptions that collect an stacktrace.
 *  Advantages:
 *    Does not execute \p statement a second time in case something fails (like ThrowsMessage<> does) and cause numerous
 *    unexpected calls to mocked functions. The given \p expected_exception must match exactly, can not be something derived from
 *    it! Has only a thin macro wrapper, mostly implemented as a template function.
 *    If collecting stacktraces is disabled, the matchers given in \p stack_trace_matcher are ignored.
 *
 * @param statement something that fulfills the std::invocable concept
 * @param expected_exception exact type of exception that is expected to be thrown by std::invoke(statement)
 * @param msg_matcher any gtest matcher that can be used with an std::string to validate the .what() of \p expected_exception
 * @param stack_trace_matcher any gtest matcher for std::stacktrace, like sqlite_wrapper::stack_trace_contains_function or
 *                            sqlite_wrapper::stack_trace_contains_function_in, to validate the stack trace of \p
 *                            expected_exception
 */
// NOLINTNEXTLINE(*-macro-usage)
#define ASSERT_THROWS_WITH_MSG_AND_STACK(statement, expected_exception, msg_matcher, stack_trace_matcher)          \
  {                                                                                                                \
    std::optional<std::string> what;                                                                               \
    std::optional<std::stacktrace> stack_trace;                                                                    \
    sqlite_wrapper::details::assert_throws_with_msg<expected_exception>(statement, #statement, what, stack_trace); \
    ASSERT_TRUE(what.has_value());                                                                                 \
    ASSERT_THAT(*what, msg_matcher);                                                 /* NOLINT(bugprone-unchecked-optional-access) */                              \
    if constexpr (sqlite_wrapper::are_stack_traces_supported())                                                    \
    {                                                                                                              \
      ASSERT_TRUE(stack_trace.has_value()) << "Expected a stack trace to be captured!";                            \
      EXPECT_THAT(*stack_trace, stack_trace_matcher);                                                              \
    }                                                                                                              \
    else                                                                                                           \
    {                                                                                                              \
      ASSERT_FALSE(stack_trace.has_value()) << "Expected no stack trace to be captured!";                          \
    }                                                                                                              \
  }
