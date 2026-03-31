#include "sqlite_wrapper/tuple_utils.h"

#include "assert_throws_with_msg.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <concepts>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

using namespace std::string_view_literals;

using ::testing::HasSubstr;

// test try_to_convert_to_array_type
static_assert(std::is_same_v<sqlite_wrapper::try_to_convert_to_array_type<std::tuple<int>>, std::array<int, 1>>);
static_assert(std::is_same_v<sqlite_wrapper::try_to_convert_to_array_type<std::tuple<bool, bool>>, std::array<bool, 2>>);
static_assert(std::is_same_v<sqlite_wrapper::try_to_convert_to_array_type<std::tuple<char, char, char>>, std::array<char, 3>>);
static_assert(std::is_same_v<sqlite_wrapper::try_to_convert_to_array_type<std::tuple<>>, std::tuple<>>);
static_assert(
    std::is_same_v<sqlite_wrapper::try_to_convert_to_array_type<std::tuple<int, char, int>>, std::tuple<int, char, int>>);
static_assert(
    std::is_same_v<sqlite_wrapper::try_to_convert_to_array_type<std::tuple<int, char, char>>, std::tuple<int, char, char>>);
static_assert(
    std::is_same_v<sqlite_wrapper::try_to_convert_to_array_type<std::tuple<char, char, int>>, std::tuple<char, char, int>>);

// test to_array_type
static_assert(std::is_same_v<sqlite_wrapper::convert_to_array_type<std::tuple<int>>, std::array<int, 1>>);
static_assert(std::is_same_v<sqlite_wrapper::convert_to_array_type<std::tuple<bool, bool>>, std::array<bool, 2>>);
static_assert(std::is_same_v<sqlite_wrapper::convert_to_array_type<std::tuple<char, char, char>>, std::array<char, 3>>);

// test add_type_front
static_assert(std::is_same_v<sqlite_wrapper::add_type_front<int, std::tuple<>>, std::tuple<int>>);
static_assert(std::is_same_v<sqlite_wrapper::add_type_front<int, std::tuple<char, float>>, std::tuple<int, char, float>>);
static_assert(std::is_same_v<sqlite_wrapper::add_type_front<int, std::pair<char, bool>>, std::tuple<int, char, bool>>);

// test add_type_back
static_assert(std::is_same_v<sqlite_wrapper::add_type_back<int, std::tuple<>>, std::tuple<int>>);
static_assert(std::is_same_v<sqlite_wrapper::add_type_back<int, std::tuple<char, float>>, std::tuple<char, float, int>>);
static_assert(std::is_same_v<sqlite_wrapper::add_type_back<int, std::pair<char, bool>>, std::tuple<char, bool, int>>);

// test remove_type_front
static_assert(std::is_same_v<sqlite_wrapper::remove_type_front<std::tuple<int>>, std::tuple<>>);
static_assert(std::is_same_v<sqlite_wrapper::remove_type_front<std::tuple<>>, std::tuple<>>);  // TODO: desired behavior?
static_assert(std::is_same_v<sqlite_wrapper::remove_type_front<std::tuple<char, float>>, std::tuple<float>>);
static_assert(std::is_same_v<sqlite_wrapper::remove_type_front<std::pair<int, unsigned>>, std::tuple<unsigned>>);
static_assert(std::is_same_v<sqlite_wrapper::remove_type_front<std::tuple<char, float, float, float>, std::false_type>,
                             std::tuple<float, float, float>>);

static_assert(
    std::is_same_v<sqlite_wrapper::remove_type_front<std::pair<int, unsigned>, std::true_type>, std::array<unsigned, 1>>);
static_assert(std::is_same_v<sqlite_wrapper::remove_type_front<std::tuple<char, float, float, float>, std::true_type>,
                             std::array<float, 3>>);

// test remove_type_back
static_assert(std::is_same_v<sqlite_wrapper::remove_type_back<std::tuple<int>>, std::tuple<>>);
static_assert(std::is_same_v<sqlite_wrapper::remove_type_back<std::tuple<>>, std::tuple<>>);  // TODO: desired behavior?
static_assert(std::is_same_v<sqlite_wrapper::remove_type_back<std::tuple<char, float>>, std::tuple<char>>);
static_assert(std::is_same_v<sqlite_wrapper::remove_type_back<std::tuple<int, char, float>>, std::tuple<int, char>>);
static_assert(std::is_same_v<sqlite_wrapper::remove_type_back<std::pair<int, unsigned>>, std::tuple<int>>);
static_assert(std::is_same_v<sqlite_wrapper::remove_type_back<std::tuple<float, float, float, long>, std::false_type>,
                             std::tuple<float, float, float>>);

static_assert(std::is_same_v<sqlite_wrapper::remove_type_back<std::pair<int, unsigned>, std::true_type>, std::array<int, 1>>);
static_assert(std::is_same_v<sqlite_wrapper::remove_type_back<std::tuple<float, float, float, long>, std::true_type>,
                             std::array<float, 3>>);

// test pop_front
static_assert(std::is_same_v<decltype(sqlite_wrapper::pop_front(std::make_tuple("lol"))), std::pair<const char*, std::tuple<>>>);
static_assert(
    std::is_same_v<decltype(sqlite_wrapper::pop_front(std::make_tuple('a', -1, 42))), std::pair<char, std::tuple<int, int>>>);

// test pop_back
static_assert(std::is_same_v<decltype(sqlite_wrapper::pop_back(std::make_tuple("lol"))), std::pair<const char*, std::tuple<>>>);
static_assert(
    std::is_same_v<decltype(sqlite_wrapper::pop_back(std::make_tuple('a', -1, 42))), std::pair<int, std::tuple<char, int>>>);

// test push_front
static_assert(std::is_same_v<decltype(sqlite_wrapper::push_front(std::make_tuple("lol"), -1)), std::tuple<int, const char*>>);
static_assert(
    std::is_same_v<decltype(sqlite_wrapper::push_front(std::make_tuple('a', 4711U), 1.1)), std::tuple<double, char, unsigned>>);

// test push_back
static_assert(std::is_same_v<decltype(sqlite_wrapper::push_back(std::make_tuple("lol"), -1)), std::tuple<const char*, int>>);
static_assert(
    std::is_same_v<decltype(sqlite_wrapper::push_back(std::make_tuple('a', 4711U), 1.1)), std::tuple<char, unsigned, double>>);

namespace
{
  struct throws_on_copy
  {
    throws_on_copy() = default;
    ~throws_on_copy() = default;

    throws_on_copy(throws_on_copy&&) noexcept = default;
    auto operator=(throws_on_copy&&) noexcept -> throws_on_copy& = default;

    throws_on_copy([[maybe_unused]] const throws_on_copy& other)
    {
      throw std::runtime_error("copy constructor called");
    }

    // NOLINTNEXTLINE(cert-oop54-cpp) "does not handle self-assignment properly"
    auto operator=([[maybe_unused]] const throws_on_copy& other) -> throws_on_copy&
    {
      throw std::runtime_error("copy assignment operator called");
    }
  };

  struct counts_copy_and_move
  {
    counts_copy_and_move() = default;
    ~counts_copy_and_move() = default;

    counts_copy_and_move(counts_copy_and_move&& other) noexcept : copy_count(other.copy_count), move_count(other.move_count + 1)
    {
    }

    counts_copy_and_move(const counts_copy_and_move& other) : copy_count(other.copy_count + 1), move_count(other.move_count) {}

    auto operator=(counts_copy_and_move&& other) noexcept -> counts_copy_and_move&
    {
      copy_count = other.copy_count;
      move_count = other.move_count + 1;
      return *this;
    }

    auto operator=(const counts_copy_and_move& other) -> counts_copy_and_move&
    {
      if (this == &other)
      {
        return *this;
      }
      copy_count = other.copy_count + 1;
      move_count = other.move_count;
      return *this;
    }

    std::size_t copy_count{0};
    std::size_t move_count{0};
  };
}  // namespace

TEST(tuple_utils_tests, test_pop_front)
{
  const auto tuple{std::make_tuple(42, "hello"sv, 3.14)};
  const auto result{sqlite_wrapper::pop_front(tuple)};

  static_assert(std::is_same_v<decltype(result), const std::pair<int, std::tuple<std::string_view, double>>>);
  EXPECT_EQ(result.first, 42);
  EXPECT_EQ(result.second, std::make_tuple("hello"sv, 3.14));

  EXPECT_EQ(sqlite_wrapper::pop_front(std::make_tuple('a', "hello"sv, 4711)),
            std::make_pair('a', std::make_tuple("hello"sv, 4711)));
}

TEST(tuple_utils_tests, test_pop_back)
{
  const auto tuple{std::make_tuple(42, "hello"sv, 3.14)};
  const auto result{sqlite_wrapper::pop_back(tuple)};

  static_assert(std::is_same_v<decltype(result), const std::pair<double, std::tuple<int, std::string_view>>>);
  EXPECT_EQ(result.first, 3.14);
  EXPECT_EQ(result.second, std::make_tuple(42, "hello"sv));

  EXPECT_EQ(sqlite_wrapper::pop_back(std::make_tuple('a', "hello"sv, 4711)),
            std::make_pair(4711, std::make_tuple('a', "hello"sv)));
}

TEST(tuple_utils_tests, test_pop_front_perfect_forwarding)
{
  {
    auto tuple{std::make_tuple(42, throws_on_copy{})};

    ASSERT_THROWS_WITH_MSG([&] { (void)sqlite_wrapper::pop_front(tuple); }, std::runtime_error,
                           HasSubstr("copy constructor called"));
    (void)sqlite_wrapper::pop_front(std::move(tuple));
  }
  {
    auto tuple{std::make_tuple('a', counts_copy_and_move{})};
    EXPECT_EQ(std::get<1>(tuple).copy_count, 0);
    EXPECT_EQ(std::get<1>(tuple).move_count, 1);

    const auto result1{sqlite_wrapper::pop_front(tuple)};
    EXPECT_EQ(result1.first, 'a');
    EXPECT_EQ(std::get<0>(result1.second).copy_count, 1);  // make_tuple
    EXPECT_EQ(std::get<0>(result1.second).move_count, 2);  // make_pair

    const auto result2{sqlite_wrapper::pop_front(std::move(tuple))};
    EXPECT_EQ(result2.first, 'a');
    EXPECT_EQ(std::get<0>(result2.second).copy_count, 0);
    EXPECT_EQ(std::get<0>(result2.second).move_count, 3);  // make_pair + make_tuple!
  }
  {
    auto tuple{std::make_tuple(counts_copy_and_move{}, true)};
    EXPECT_EQ(std::get<0>(tuple).copy_count, 0);
    EXPECT_EQ(std::get<0>(tuple).move_count, 1);

    const auto result1{sqlite_wrapper::pop_front(tuple)};
    EXPECT_EQ(result1.first.copy_count, 1);
    EXPECT_EQ(result1.first.move_count, 1);

    const auto result2{sqlite_wrapper::pop_front(std::move(tuple))};
    EXPECT_EQ(result2.first.copy_count, 0);
    EXPECT_EQ(result2.first.move_count, 2);
  }
}

TEST(tuple_utils_tests, test_pop_back_perfect_forwarding)
{
  {
    auto tuple{std::make_tuple(throws_on_copy{}, 42)};

    ASSERT_THROWS_WITH_MSG([&] { (void)sqlite_wrapper::pop_back(tuple); }, std::runtime_error,
                           HasSubstr("copy constructor called"));
    (void)sqlite_wrapper::pop_back(std::move(tuple));
  }
  {
    auto tuple{std::make_tuple(counts_copy_and_move{}, 'a')};
    EXPECT_EQ(std::get<0>(tuple).copy_count, 0);
    EXPECT_EQ(std::get<0>(tuple).move_count, 1);

    const auto result1{sqlite_wrapper::pop_back(tuple)};
    EXPECT_EQ(result1.first, 'a');
    EXPECT_EQ(std::get<0>(result1.second).copy_count, 1);  // make_tuple
    EXPECT_EQ(std::get<0>(result1.second).move_count, 2);  // make_pair

    const auto result2{sqlite_wrapper::pop_back(std::move(tuple))};
    EXPECT_EQ(result2.first, 'a');
    EXPECT_EQ(std::get<0>(result2.second).copy_count, 0);
    EXPECT_EQ(std::get<0>(result2.second).move_count, 3);  // make_pair + make_tuple!
  }
  {
    auto tuple{std::make_tuple(true, counts_copy_and_move{})};
    EXPECT_EQ(std::get<1>(tuple).copy_count, 0);
    EXPECT_EQ(std::get<1>(tuple).move_count, 1);

    const auto result1{sqlite_wrapper::pop_back(tuple)};
    EXPECT_EQ(result1.first.copy_count, 1);
    EXPECT_EQ(result1.first.move_count, 1);

    const auto result2{sqlite_wrapper::pop_back(std::move(tuple))};
    EXPECT_EQ(result2.first.copy_count, 0);
    EXPECT_EQ(result2.first.move_count, 2);
  }
}

TEST(tuple_utils_tests, test_to_array)
{
  {
    const auto tuple{std::make_tuple(3.4, 24.0, 4711.0)};
    const auto result{sqlite_wrapper::to_array(tuple)};

    ASSERT_EQ(result.size(), std::tuple_size_v<decltype(tuple)>);
    EXPECT_EQ(result[0], std::get<0>(tuple));
    EXPECT_EQ(result[1], std::get<1>(tuple));
    EXPECT_EQ(result[2], std::get<2>(tuple));
  }
  {
    const auto array{std::to_array({42, 4711, -1})};
    EXPECT_EQ(sqlite_wrapper::to_array(array), array);
  }
}

TEST(tuple_utils_tests, test_to_array_perfect_forwarding)
{
  {
    auto tuple{std::make_tuple(throws_on_copy{}, throws_on_copy{}, throws_on_copy{})};

    ASSERT_THROWS_WITH_MSG([&] { (void)sqlite_wrapper::to_array(tuple); }, std::runtime_error,
                           HasSubstr("copy constructor called"));
    (void)sqlite_wrapper::to_array(std::move(tuple));
  }
  {
    auto tuple{std::make_tuple(counts_copy_and_move{})};
    EXPECT_EQ(std::get<0>(tuple).copy_count, 0);
    EXPECT_EQ(std::get<0>(tuple).move_count, 1);

    const auto result1{sqlite_wrapper::to_array(tuple)};
    ASSERT_EQ(result1.size(), 1U);
    EXPECT_EQ(result1[0].copy_count, 1);
    EXPECT_EQ(result1[0].move_count, 1);

    const auto result2{sqlite_wrapper::to_array(std::move(tuple))};
    ASSERT_EQ(result2.size(), 1U);
    EXPECT_EQ(result2[0].copy_count, 0);
    EXPECT_EQ(result2[0].move_count, 2);
  }
}

TEST(tuple_utils_tests, test_push_front)
{
  {
    const auto tuple{std::make_tuple(3.4, 24.0, 4711.0)};
    const auto result{sqlite_wrapper::push_front(tuple, 1.1F)};

    static_assert(std::same_as<std::remove_cvref_t<decltype(result)>, sqlite_wrapper::add_type_front<float, decltype(tuple)>>);
    EXPECT_EQ(result, std::make_tuple(1.1F, 3.4, 24.0, 4711.0));
  }
  {
    int addme = -4711;
    const auto result{sqlite_wrapper::push_front(std::make_tuple(true, 'x'), std::ref(addme))};

    static_assert(std::same_as<std::remove_cvref_t<decltype(result)>, std::tuple<int&, bool, char>>);
    EXPECT_EQ(result, std::make_tuple(std::ref(addme), true, 'x'));
  }
}

TEST(tuple_utils_tests, test_push_front_perfect_forwarding)
{
  {
    auto tuple{std::make_tuple(throws_on_copy{}, 42)};

    ASSERT_THROWS_WITH_MSG([&] { (void)sqlite_wrapper::push_front(tuple, sizeof(tuple)); }, std::runtime_error,
                           HasSubstr("copy constructor called"));
    (void)sqlite_wrapper::push_front(std::move(tuple), sizeof(tuple));
  }
  {
    auto tuple{std::make_tuple(counts_copy_and_move{})};
    EXPECT_EQ(std::get<0>(tuple).copy_count, 0);
    EXPECT_EQ(std::get<0>(tuple).move_count, 1);

    const auto result1{sqlite_wrapper::push_front(tuple, sizeof(tuple))};
    EXPECT_EQ(std::get<1>(result1).copy_count, 1);
    EXPECT_EQ(std::get<1>(result1).move_count, 1);

    const auto result2{sqlite_wrapper::push_front(std::move(tuple), sizeof(tuple))};
    EXPECT_EQ(std::get<1>(result2).copy_count, 0);
    EXPECT_EQ(std::get<1>(result2).move_count, 2);
  }
  {
    auto obj{counts_copy_and_move{}};

    const auto result1{sqlite_wrapper::push_front(std::tuple<bool>{}, obj)};
    EXPECT_EQ(std::get<0>(result1).copy_count, 1);
    EXPECT_EQ(std::get<0>(result1).move_count, 0);

    const auto result2{sqlite_wrapper::push_front(std::tuple<bool>{}, std::move(obj))};
    EXPECT_EQ(std::get<0>(result2).copy_count, 0);
    EXPECT_EQ(std::get<0>(result2).move_count, 1);
  }
}

TEST(tuple_utils_tests, test_push_back)
{
  {
    const auto tuple{std::make_tuple(3.4, 24.0, 4711.0)};
    const auto result{sqlite_wrapper::push_back(tuple, 1.1F)};

    static_assert(std::same_as<std::remove_cvref_t<decltype(result)>, sqlite_wrapper::add_type_back<float, decltype(tuple)>>);
    EXPECT_EQ(result, std::make_tuple(3.4, 24.0, 4711.0, 1.1F));
  }
  {
    int addme = -4711;
    const auto result{sqlite_wrapper::push_back(std::make_tuple(true, 'x'), std::ref(addme))};

    static_assert(std::same_as<std::remove_cvref_t<decltype(result)>, std::tuple<bool, char, int&>>);
    EXPECT_EQ(result, std::make_tuple(true, 'x', std::ref(addme)));
  }
}

TEST(tuple_utils_tests, test_push_back_perfect_forwarding)
{
  {
    auto tuple{std::make_tuple(throws_on_copy{}, 42)};

    ASSERT_THROWS_WITH_MSG([&] { (void)sqlite_wrapper::push_back(tuple, sizeof(tuple)); }, std::runtime_error,
                           HasSubstr("copy constructor called"));
    (void)sqlite_wrapper::push_back(std::move(tuple), sizeof(tuple));
  }
  {
    auto tuple{std::make_tuple(counts_copy_and_move{})};
    EXPECT_EQ(std::get<0>(tuple).copy_count, 0);
    EXPECT_EQ(std::get<0>(tuple).move_count, 1);

    const auto result1{sqlite_wrapper::push_back(tuple, sizeof(tuple))};
    EXPECT_EQ(std::get<0>(result1).copy_count, 1);
    EXPECT_EQ(std::get<0>(result1).move_count, 1);

    const auto result2{sqlite_wrapper::push_back(std::move(tuple), sizeof(tuple))};
    EXPECT_EQ(std::get<0>(result2).copy_count, 0);
    EXPECT_EQ(std::get<0>(result2).move_count, 2);
  }
  {
    auto obj{counts_copy_and_move{}};

    const auto result1{sqlite_wrapper::push_back(std::tuple<bool>{}, obj)};
    EXPECT_EQ(std::get<1>(result1).copy_count, 1);
    EXPECT_EQ(std::get<1>(result1).move_count, 0);

    const auto result2{sqlite_wrapper::push_back(std::tuple<bool>{}, std::move(obj))};
    EXPECT_EQ(std::get<1>(result2).copy_count, 0);
    EXPECT_EQ(std::get<1>(result2).move_count, 1);
  }
}
