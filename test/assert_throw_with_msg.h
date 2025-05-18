#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <exception>
#include <string>
#include <string_view>

namespace sqlite_wrapper::details
{
  [[nodiscard]] auto demangle(const char* name) -> std::string;
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_THROW_WITH_MSG(statement, expected_exception, matcher)                                 \
  try                                                                                                 \
  {                                                                                                   \
    statement;                                                                                        \
    FAIL() << "Expected: \"" #statement "\" to throw an exception of type " #expected_exception ".\n" \
              "  Actual: it throws nothing.";                                                         \
  }                                                                                                   \
  catch (const testing::AssertionException&)                                                          \
  {                                                                                                   \
    /* we need to ignore testing::AssertionException because FAIL() might throw them! */              \
  }                                                                                                   \
  catch (const expected_exception& e)                                                                 \
  {                                                                                                   \
    ASSERT_THAT(std::string_view{e.what()}, matcher);                                                 \
  }                                                                                                   \
  catch (const std::exception& e)                                                                     \
  {                                                                                                   \
    FAIL() << "Expected: \"" #statement "\" to throw an exception of type " #expected_exception ".\n" \
              "  Actual: it throws type " << sqlite_wrapper::details::demangle(typeid(e).name());     \
  }                                                                                                   \
  catch (...)                                                                                         \
  {                                                                                                   \
    FAIL() << "Expected: \"" #statement "\" to throw an exception of type " #expected_exception ".\n" \
              "  Actual: it throws an unknown type.";                                                 \
  }
