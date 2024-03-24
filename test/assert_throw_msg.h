#pragma once

#include <gmock/gmock.h>

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
  catch (...)                                                                             \
  {                                                                                       \
    FAIL() << "Expected: " #statement " throws an exception of type " #expected_exception \
              ".\n"                                                                       \
              "  Actual: it throws a different type.";                                    \
  }
