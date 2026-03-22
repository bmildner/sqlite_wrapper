#include "sqlite_wrapper/tuple_utils.h"

#include "assert_throws_with_msg.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

using testing::HasSubstr;

// test is_tuple_like concept
static_assert(sqlite_wrapper::is_tuple_like<std::tuple<>>);
static_assert(sqlite_wrapper::is_tuple_like<std::tuple<int, char, float>>);
static_assert(sqlite_wrapper::is_tuple_like<decltype(std::make_tuple(1, "a", 3.14))>);
static_assert(sqlite_wrapper::is_tuple_like<std::pair<int, long>>);
static_assert(sqlite_wrapper::is_tuple_like<decltype(std::make_pair("hello", 42))>);

static_assert(!sqlite_wrapper::is_tuple_like<std::tuple<>&>);
static_assert(!sqlite_wrapper::is_tuple_like<std::tuple<>&&>);

static_assert(!sqlite_wrapper::is_tuple_like<void>);
static_assert(!sqlite_wrapper::is_tuple_like<int>);
static_assert(!sqlite_wrapper::is_tuple_like<std::string>);
static_assert(!sqlite_wrapper::is_tuple_like<std::vector<int>>);

// test add_type_front
static_assert(std::is_same_v<sqlite_wrapper::add_type_front<int, std::tuple<>>, std::tuple<int>>);
static_assert(std::is_same_v<sqlite_wrapper::add_type_front<int, std::tuple<char, float>>, std::tuple<int, char, float>>);

// test add_type_back
static_assert(std::is_same_v<sqlite_wrapper::add_type_back<int, std::tuple<>>, std::tuple<int>>);
static_assert(std::is_same_v<sqlite_wrapper::add_type_back<int, std::tuple<char, float>>, std::tuple<char, float, int>>);

// test pop_front
static_assert(std::is_same_v<decltype(sqlite_wrapper::pop_front(std::make_tuple("lol"))), std::tuple<>>);
static_assert(std::is_same_v<decltype(sqlite_wrapper::pop_front(std::make_tuple('a', -1, 42))), std::tuple<int, int>>);

// test pop_back
static_assert(std::is_same_v<decltype(sqlite_wrapper::pop_back(std::make_tuple("lol"))), std::tuple<>>);
static_assert(std::is_same_v<decltype(sqlite_wrapper::pop_back(std::make_tuple('a', -1, 42))), std::tuple<char, int>>);

namespace
{
  struct throw_on_copy
  {
    throw_on_copy() = default;
    ~throw_on_copy() = default;

    throw_on_copy(throw_on_copy&&) noexcept = default;
    auto operator=(throw_on_copy&&) noexcept -> throw_on_copy& = default;

    throw_on_copy([[maybe_unused]] const throw_on_copy& other)
    {
      throw std::runtime_error("copy constructor called");
    }

    // NOLINTNEXTLINE(cert-oop54-cpp) "does not handle self-assignment properly"
    auto operator=([[maybe_unused]] const throw_on_copy& other) -> throw_on_copy&
    {
      throw std::runtime_error("copy assignment operator called");
    }
  };
}  // namespace

TEST(tuple_utils_tests, test_pop_front)
{
  const auto tuple{std::make_tuple(42, "hello", 3.14)};
  const auto result{sqlite_wrapper::pop_front(tuple)};

  static_assert(std::is_same_v<decltype(result), const std::tuple<const char*, double>>);
  ASSERT_EQ(result, std::make_tuple("hello", 3.14));

  ASSERT_EQ(sqlite_wrapper::pop_front(std::make_tuple('a', "hello", 4711)), std::make_tuple("hello", 4711));
}

TEST(tuple_utils_tests, test_pop_back)
{
  const auto tuple{std::make_tuple(42, "hello", 3.14)};
  const auto result{sqlite_wrapper::pop_back(tuple)};

  static_assert(std::is_same_v<decltype(result), const std::tuple<int, const char*>>);
  ASSERT_EQ(result, std::make_tuple(42, "hello"));

  ASSERT_EQ(sqlite_wrapper::pop_back(std::make_tuple('a', "hello", 4711)), std::make_tuple('a', "hello"));
}

TEST(tuple_utils_tests, test_pop_front_perfect_forwarding)
{
  auto tuple{std::make_tuple(42, throw_on_copy{})};

  ASSERT_THROWS_WITH_MSG([&] { (void)sqlite_wrapper::pop_front(tuple); }, std::runtime_error,
                         HasSubstr("copy constructor called"));
  (void)sqlite_wrapper::pop_front(std::move(tuple));
}

TEST(tuple_utils_tests, test_pop_back_perfect_forwarding)
{
  auto tuple{std::make_tuple(throw_on_copy{}, 42)};

  ASSERT_THROWS_WITH_MSG([&] { (void)sqlite_wrapper::pop_back(tuple); }, std::runtime_error,
                         HasSubstr("copy constructor called"));
  (void)sqlite_wrapper::pop_back(std::move(tuple));
}
