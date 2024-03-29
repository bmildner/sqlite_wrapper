#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// TODO: maybe try to turn into a proper function?
// NOLINTNEXTLINE [cppcoreguidelines-macro-usage]
#define ASSERT_THROW_MSG(statement, expected_exception, matcher)                          \
  try                                                                                     \
  {                                                                                       \
    statement;                                                                            \
    FAIL() << "Expected: " #statement " throws an exception of type " #expected_exception \
              ".\n"                                                                       \
              "  Actual: it throws nothing.";                                             \
  }                                                                                       \
  catch (const expected_exception& e)                                                     \
  {                                                                                       \
    ASSERT_THAT(std::string{e.what()}, matcher);                                          \
  }                                                                                       \
  catch (const testing::AssertionException&)                                              \
  {                                                                                       \
    /* we need to ignore testing::AssertionException because FAIL() might throw them! */  \
  }                                                                                       \
  catch (...)                                                                             \
  {                                                                                       \
    FAIL() << "Expected: " #statement " throws an exception of type " #expected_exception \
              ".\n"                                                                       \
              "  Actual: it throws a different type.";                                    \
  }
