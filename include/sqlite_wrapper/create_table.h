#pragma once

#include "sqlite_wrapper.h"

// TODO: cleanup or remove all togther!
namespace sqlite_wrapper
{
  template<typename... Args>
  struct type_list
  {
    using type = std::tuple<Args...>;

    static constexpr auto size = sizeof...(Args);

    template <std::size_t N>
    using nth_type = typename std::tuple_element<N, type>::type;
  };


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

  template<column_type T>
  struct column
  {
    column(std::string_view name) requires (!details::is_foreign_key<T>)
      :name(name)
    {}

    template <column_type... columns>
    column(std::string_view name, table<columns...> f_table) requires details::is_foreign_key<T>
      : name(name)
    {
      (void) f_table;
    }

    using type = typename details::to_database_type<T>;

    static constexpr auto is_primary_key{details::is_primary_key<T>};

    std::string_view name;

    using foreign_key_info = std::optional<std::pair<std::string_view, std::string_view>>;
    foreign_key_info foreign_key{std::nullopt};
  };

  template <column_type... Column_types>
  struct table
  {
    static_assert(sizeof...(Column_types) > 0);

    table(std::string_view name, const column<Column_types>&... col)
      :name(name)
    {
      ((void) col, ...);
    }

    std::string_view name;

    using column_types = type_list<Column_types...>;
    using row_types = type_list<typename details::to_database_type<Column_types>::type ...>;
    using full_row = typename row_types::type;

  private:
    // check that there is only one
//    template <typename... T>
//    static constexpr bool only_one_primary_key()
  };

}  // namespace sqlite_wrapper
