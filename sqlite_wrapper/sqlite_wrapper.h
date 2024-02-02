#pragma once

#include <tuple>
#include <string>
#include <concepts>
#include <cstdint>
#include <vector>
#include <cstddef>
#include <optional>
#include <algorithm>

namespace sqlite_wrapper
{
  using byte_vector = std::vector<std::byte>;

  template<size_t N>
  struct string_literal
  {
    constexpr string_literal(const char(&str)[N])
    {
      std::copy_n(str, N, value);
    }

    char value[N];
  };


  template <typename T>
  concept basic_database_type = std::same_as<std::int64_t, T> || std::same_as<double, T> || std::same_as<std::string, T> || std::same_as<byte_vector, T>;

  template <typename T>
  concept optional_database_type = std::same_as<T, std::optional<typename T::value_type>> && basic_database_type<typename T::value_type>;

  template <typename T>
  concept database_type = basic_database_type<T> || optional_database_type<T>;

  struct primary_key
  {
    using type = std::int64_t;
  };

  struct foreign_key
  {
    using type = std::int64_t;
  };

  template <typename T>
  concept column_type = database_type<T> || std::same_as<primary_key, T> || std::same_as<foreign_key, T>;

  namespace details
  {
    template <column_type T>
    struct to_database_type
    {
      using type = T;
    };

    template <>
    struct to_database_type<primary_key>
    {
      using type = primary_key::type;
    };

    template <>
    struct to_database_type<foreign_key>
    {
      using type = primary_key::type;
    };

    template <typename T>
    constexpr bool is_primary_key = std::is_same_v<primary_key, T>;

    template <typename T>
    constexpr bool is_foreign_key = std::is_same_v<foreign_key, T>;
  }  // namespace details

  template <column_type... column_types>
  struct table;

  template<string_literal col_name, column_type T>
  struct column
  {    
    column(std::string_view name) requires !details::is_foreign_key<T>
      :name(name)
    {}

    template <column_type... columns>
    column(std::string_view name, table<columns...> f_table) requires details::is_foreign_key<T>
      : name(name)
    {}

    using type = typename details::to_database_type<T>;

    static constexpr auto is_primary_key{details::is_primary_key<T>};

    static constexpr auto name{col_name}

    using foreign_key_info = std::optional<std::pair<std::string_view, std::string_view>>;
    foreign_key_info foreign_key{std::nullopt};
  };

  template <column_type... column_types>
  struct table
  {
    static_assert(sizeof...(column_types) > 0);

    table(std::string_view name, column<column_types>... col)
      :name(name)
    {}

    std::string_view name;

    using row_type = std::tuple<typename details::to_database_type<column_types>::type ...>;
  };
}  // namespace sqlite_wrapper
