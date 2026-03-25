#include "sqlite_wrapper/tuple_utils.h"

#include "assert_throws_with_msg.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

using namespace std::string_view_literals;

using ::testing::HasSubstr;

// test add_type_front
static_assert(std::is_same_v<sqlite_wrapper::add_type_front<int, std::tuple<>>, std::tuple<int>>);
static_assert(std::is_same_v<sqlite_wrapper::add_type_front<int, std::tuple<char, float>>, std::tuple<int, char, float>>);

// test add_type_back
static_assert(std::is_same_v<sqlite_wrapper::add_type_back<int, std::tuple<>>, std::tuple<int>>);
static_assert(std::is_same_v<sqlite_wrapper::add_type_back<int, std::tuple<char, float>>, std::tuple<char, float, int>>);

// test remove_type_front
static_assert(std::is_same_v<sqlite_wrapper::remove_type_front<std::tuple<int>>, std::tuple<>>);
static_assert(std::is_same_v<sqlite_wrapper::remove_type_front<std::tuple<char, float>>, std::tuple<float>>);

// test remove_type_back
static_assert(std::is_same_v<sqlite_wrapper::remove_type_back<std::tuple<int>>, std::tuple<>>);
static_assert(std::is_same_v<sqlite_wrapper::remove_type_back<std::tuple<char, float>>, std::tuple<char>>);
static_assert(std::is_same_v<sqlite_wrapper::remove_type_back<std::tuple<int, char, float>>, std::tuple<int, char>>);

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
  const auto tuple{std::make_tuple(42, "hello"sv, 3.14)};
  const auto result{sqlite_wrapper::pop_front(tuple)};

  static_assert(std::is_same_v<decltype(result), const std::tuple<std::string_view, double>>);
  ASSERT_EQ(result, std::make_tuple("hello"sv, 3.14));

  ASSERT_EQ(sqlite_wrapper::pop_front(std::make_tuple('a', "hello"sv, 4711)), std::make_tuple("hello"sv, 4711));
}

TEST(tuple_utils_tests, test_pop_back)
{
  const auto tuple{std::make_tuple(42, "hello"sv, 3.14)};
  const auto result{sqlite_wrapper::pop_back(tuple)};

  static_assert(std::is_same_v<decltype(result), const std::tuple<int, std::string_view>>);
  ASSERT_EQ(result, std::make_tuple(42, "hello"sv));

  ASSERT_EQ(sqlite_wrapper::pop_back(std::make_tuple('a', "hello"sv, 4711)), std::make_tuple('a', "hello"sv));
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

TEST(tuple_utils_tests, test_to_array)
{
  {
    const auto tuple{std::make_tuple(3.4, 24.0, 4711.0)};
    const auto result{sqlite_wrapper::to_array(tuple)};

    ASSERT_EQ(result.size(), std::tuple_size_v<decltype(tuple)>);
    ASSERT_EQ(result[0], std::get<0>(tuple));
    ASSERT_EQ(result[1], std::get<1>(tuple));
    ASSERT_EQ(result[2], std::get<2>(tuple));
  }
  {
    const auto array{std::to_array({42, 4711, -1})};
    ASSERT_EQ(sqlite_wrapper::to_array(array), array);
  }
}

TEST(tuple_utils_tests, test_to_array_perfect_forwarding)
{
  auto tuple{std::make_tuple(throw_on_copy{}, throw_on_copy{}, throw_on_copy{})};

  ASSERT_THROWS_WITH_MSG([&] { (void)sqlite_wrapper::to_array(tuple); }, std::runtime_error,
                         HasSubstr("copy constructor called"));
  (void)sqlite_wrapper::to_array(std::move(tuple));
}
