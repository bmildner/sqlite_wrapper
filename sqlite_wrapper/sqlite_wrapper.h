﻿#pragma once

#include <tuple>
#include <string>
#include <concepts>
#include <cstdint>
#include <vector>
#include <cstddef>
#include <optional>
#include <algorithm>
#include <span>
#include <limits>
#include <ranges>

#include "sqlite_wrapper_raii.h"
#include "sqlite_wrapper_location.h"
#include "sqlite_wrapper_error.h"
#include "sqlite_wrapper_format.h"

namespace sqlite_wrapper
{
  using byte_vector = std::vector<std::byte>;
  using const_byte_span = std::span<const std::byte>;


  template<typename... Args>
  struct type_list
  {
    using type = std::tuple<Args...>;

    static constexpr auto size = sizeof...(Args);

    template <std::size_t N>
    using nth_type = typename std::tuple_element<N, type>::type;
  };

  template <typename T>
  concept basic_database_type = std::same_as<std::int64_t, T> || std::same_as<double, T> || std::same_as<std::string, T> || std::same_as<byte_vector, T>;

  template <typename T>
  concept optional_database_type = std::same_as<T, std::optional<typename T::value_type>> && basic_database_type<typename T::value_type>;

  template <typename T>
  concept database_type = basic_database_type<T> || optional_database_type<T>;


  
  template <typename T>
  concept integral_binding_type = std::convertible_to<T, std::int64_t> && !std::same_as<bool, T>;

  template <typename T>
  concept real_binding_type = std::convertible_to<T, double>;

  template <typename T>
  concept string_binding_type = std::constructible_from<std::string, T> || std::constructible_from<std::string_view, T>;

  template <typename T>
  concept blob_binding_type = std::constructible_from<byte_vector, T> || std::constructible_from<const_byte_span, T>;

  template <typename T>
  concept null_binding_type = std::same_as<T, std::nullptr_t> || std::same_as<T, std::nullopt_t>;

  template <typename T>
  concept basic_binding_type = //basic_database_type<T> || std::convertible_to<T, std::int64_t> || std::convertible_to<T, double> || std::constructible_from<std::string_view, T> || 
                               //std::constructible_from<const_byte_span, T>;
                               integral_binding_type<T> || real_binding_type<T> || string_binding_type<T> || blob_binding_type<T> || null_binding_type<T>;

  template <typename T>
  concept optional_binding_type = std::same_as<T, std::optional<typename T::value_type>> && basic_binding_type<typename T::value_type>;

  template <typename T>
  concept single_binding_type = basic_binding_type<T> || null_binding_type<T> || optional_binding_type<T>;

  template <typename T>
  concept multi_binding_type = std::ranges::range<T> && single_binding_type<typename std::ranges::range_value_t<T>> && !basic_binding_type<T>;

  template <typename T>
  concept binding_type = single_binding_type<T> || multi_binding_type<T>;


  // see https://stackoverflow.com/questions/68443804/c20-concept-to-check-tuple-like-types
  template<class T, std::size_t N>
  concept has_tuple_element = requires(T t)
  {
    typename std::tuple_element_t<N, std::remove_const_t<T>>;
    { get<N>(t) } -> std::convertible_to<const std::tuple_element_t<N, T>&>;
  };

  template<class T>
  concept tuple_like = !std::is_reference_v<T> && requires(T t)
  {
    typename std::tuple_size<T>::type;
    requires std::derived_from<std::tuple_size<T>, std::integral_constant<std::size_t, std::tuple_size_v<T>>>;
  } 
  && []<std::size_t... N>(std::index_sequence<N...>)
  {
    return (has_tuple_element<T, N> && ...);
  } (std::make_index_sequence<std::tuple_size_v<T>>());


  template<class T, std::size_t N>
  concept database_type_tuple_element = database_type<typename std::tuple_element_t<N, T>>;

  template <typename T>
  concept row_type = tuple_like<T> && []<std::size_t... N>(std::index_sequence<N...>)
  {
    return (database_type_tuple_element<T, N> && ...);
  } (std::make_index_sequence<std::tuple_size_v<T>>());;



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
    column(std::string_view name) requires !details::is_foreign_key<T>
      :name(name)
    {}

    template <column_type... columns>
    column(std::string_view name, table<columns...> f_table) requires details::is_foreign_key<T>
    : name(name)
    {}

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
    {}

    std::string_view name;

    using column_types = type_list<Column_types...>;
    using row_types = type_list<typename details::to_database_type<Column_types>::type ...>;
    using full_row = typename row_types::type;

  private:
    // check that there is only one
//    template <typename... T>
//    static constexpr bool only_one_primary_key()
  };



  namespace details
  {
    [[nodiscard]] auto create_prepared_statement(const db_with_location& db, std::string_view sql) -> statement;

    void bind(const stmt_with_location& stmt, int index);
    void bind(const stmt_with_location& stmt, int index, std::int64_t value);
    void bind(const stmt_with_location& stmt, int index, double value);
    void bind(const stmt_with_location& stmt, int index, std::string_view value);
    void bind(const stmt_with_location& stmt, int index, const_byte_span value);

    void bind(const stmt_with_location& stmt, int index, const integral_binding_type auto& param)
    {
      bind(stmt, index, static_cast<std::int64_t>(param));
    }

    void bind(const stmt_with_location& stmt, int index, const null_binding_type auto&)
    {
      bind(stmt, index);
    }

    void bind(const stmt_with_location& stmt, int index, const optional_binding_type auto& param)
    {
      if (param.has_value())
      {
        bind(stmt, index, param.value());
      }
      else
      {
        bind(stmt, index, std::nullopt);
      }
    }

    void bind_and_increment_index(const stmt_with_location& stmt, int& index, const multi_binding_type auto& param_list)
    {
      for (const auto& param : param_list)
      {
        bind(stmt, index, param);
        index++;
      }
    }

    void bind_and_increment_index(const stmt_with_location& stmt, int& index, const single_binding_type auto& param)
    {
      bind(stmt, index, param);
      index++;
    }

    bool get_column(const stmt_with_location& stmt, int index, std::int64_t& value, bool maybe_null);
    bool get_column(const stmt_with_location& stmt, int index, double& value, bool maybe_null);
    bool get_column(const stmt_with_location& stmt, int index, std::string& value, bool maybe_null);
    bool get_column(const stmt_with_location& stmt, int index, byte_vector& value, bool maybe_null);
    
    void get_column(const stmt_with_location& stmt, int index, basic_database_type auto& value)
    {
      get_column(stmt, index, value, false);
    }

    void get_column(const stmt_with_location& stmt, int index, optional_database_type auto& value)
    {
      typename std::remove_reference_t<decltype(value)>::value_type base_value;

      if (get_column(stmt, index, base_value, true))
      {
        value = base_value;
      }
      else
      {
        value.reset();
      }
    }

    template <database_type... Columns>
      requires(sizeof...(Columns) > 1)
    void get_row(const stmt_with_location& stmt, Columns&... columns)
    {
      int index{0};

      (get_column(stmt, index++, columns), ...);
    }

  }  // namespace details

  [[nodiscard]] auto open(const std::string& file_name, const std::source_location& loc = std::source_location::current()) -> database;

  [[nodiscard]] auto create_prepared_statement(const db_with_location& db, std::string_view sql, const binding_type auto&... params) -> statement
  {
    auto stmt{details::create_prepared_statement(db, sql)};

    int index{1};

    (details::bind_and_increment_index({stmt.get(), db.location}, index, params), ...);

    return stmt;
  }

  [[nodiscard]] auto step(const stmt_with_location& stmt) -> bool;

  
  template <row_type Row>
  [[nodiscard]] auto get_row(const stmt_with_location& stmt) -> Row
  {
    Row row;

    std::apply([&stmt] (database_type auto&... columns) { details::get_row(stmt, columns...); }, row);

    return row;
  }

  template <row_type Row>
  [[nodiscard]] auto get_rows(const stmt_with_location& stmt, std::size_t limit) -> std::vector<Row>
  {
    std::vector<Row> rows;

    while ((rows.size() < limit) && step(stmt))
    {
      rows.emplace_back(get_row<Row>(stmt));
    }

    return rows;
  }

  template <row_type Row>
  [[nodiscard]] auto get_rows(const stmt_with_location& stmt) -> std::vector<Row>
  {
    return get_rows<Row>(stmt, std::numeric_limits<std::size_t>::max());
  }

  void execute_no_data(const db_with_location& db, std::string_view sql, const binding_type auto&... params)
  {
    const auto stmt{create_prepared_statement(db, sql, params...)};

    if (step({stmt.get(), db.location}))
    {
      throw sqlite_error("unexpected data row in execute_no_data()", {stmt.get(), db.location});
    }
  }

  template <row_type Row>
  [[nodiscard]] auto execute_one_row(const db_with_location& db, std::string_view sql, const binding_type auto&... params) -> Row
  {
    const auto stmt{create_prepared_statement(db, sql, params...)};

    const auto result{get_rows<Row>(stmt, 2)};

    if (const auto size{result.size()}; size != 1)
    {
      throw sqlite_error(sqlite_wrapper::format("expected exactly one row but found {}", (size == 0) ? "none" : "more"), stmt);
    }

    return std::move(result[0]);
  }

  template <row_type Row>
  [[nodiscard]] auto execute(const db_with_location& db, std::string_view sql, const binding_type auto&... params) -> std::vector<Row>
  {
    const auto stmt{create_prepared_statement(db, sql, params...)};

    return get_rows<Row>({stmt.get(), db.location});
  }

}  // namespace sqlite_wrapper
