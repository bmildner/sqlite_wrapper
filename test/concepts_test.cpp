#include "sqlite_wrapper/concepts.h"

#include <array>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

// test is_tuple_like concept
static_assert(sqlite_wrapper::is_tuple_like<std::tuple<>>);
static_assert(sqlite_wrapper::is_tuple_like<std::tuple<int, char, float>>);
static_assert(sqlite_wrapper::is_tuple_like<decltype(std::make_tuple(1, "a", 3.14))>);
static_assert(sqlite_wrapper::is_tuple_like<std::pair<int, long>>);
static_assert(sqlite_wrapper::is_tuple_like<decltype(std::make_pair("hello", 42))>);
static_assert(sqlite_wrapper::is_tuple_like<std::array<int, 3>>);

static_assert(!sqlite_wrapper::is_tuple_like<std::tuple<>&>);
static_assert(!sqlite_wrapper::is_tuple_like<std::tuple<>&&>);

static_assert(!sqlite_wrapper::is_tuple_like<void>);
static_assert(!sqlite_wrapper::is_tuple_like<int>);
static_assert(!sqlite_wrapper::is_tuple_like<std::string>);
static_assert(!sqlite_wrapper::is_tuple_like<std::vector<int>>);

// test is_array_like concept
static_assert(sqlite_wrapper::is_array_like<std::array<bool, 0>>);
static_assert(sqlite_wrapper::is_array_like<std::array<char, 1>>);
static_assert(sqlite_wrapper::is_array_like<std::array<std::string, 10>>);
static_assert(sqlite_wrapper::is_array_like<std::tuple<int>>);
static_assert(sqlite_wrapper::is_array_like<std::tuple<int, int>>);
static_assert(sqlite_wrapper::is_array_like<std::pair<bool, bool>>);

static_assert(!sqlite_wrapper::is_array_like<std::tuple<>>);
static_assert(!sqlite_wrapper::is_array_like<std::tuple<int, int, bool>>);
static_assert(!sqlite_wrapper::is_array_like<std::tuple<unsigned, int, int>>);

// test smake_as_either concept
static_assert(sqlite_wrapper::same_as_either<int, int>);
static_assert(sqlite_wrapper::same_as_either<int, char, int, float>);

static_assert(!sqlite_wrapper::same_as_either<char, int>);
static_assert(!sqlite_wrapper::same_as_either<int, const int>);
static_assert(!sqlite_wrapper::same_as_either<int, char, float>);
static_assert(!sqlite_wrapper::same_as_either<int, char, bool, float>);

// test same_as_all concept
static_assert(sqlite_wrapper::same_as_all<int, int>);
static_assert(sqlite_wrapper::same_as_all<int, int, int, int>);

static_assert(!sqlite_wrapper::same_as_all<bool, int>);
static_assert(!sqlite_wrapper::same_as_all<bool, const bool>);
static_assert(!sqlite_wrapper::same_as_all<int, int, int, int, unsigned, int>);