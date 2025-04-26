#pragma once

#include "sqlite_wrapper/config.h"
#include "sqlite_wrapper/format.h"
#include "sqlite_wrapper/raii.h"
#include "sqlite_wrapper/sqlite_error.h"
#include "sqlite_wrapper/with_location.h"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace sqlite_wrapper
{
  using byte_vector = std::vector<std::byte>;
  using const_byte_span = std::span<const std::byte>;

  /**
   * Basic types that con be queried from the database.
   */
  template <typename T>
  concept basic_database_type = std::same_as<std::int64_t, T> || std::same_as<double, T> || std::same_as<std::string, T> || std::same_as<byte_vector, T>;

  /**
   * Optional versions of basic types that con be queried from the database.
   */
  template <typename T>
  concept optional_database_type = std::same_as<T, std::optional<typename T::value_type>> && basic_database_type<typename T::value_type>;

  /**
   * Full set of basic and optional types that can be queried from the database.
   */
  template <typename T>
  concept database_type = basic_database_type<T> || optional_database_type<T>;

  /**
   * Integral types that can be bound to a parameter in a database query.
   */
  template <typename T>
  concept integral_binding_type = std::integral<T> && std::convertible_to<T, std::int64_t>;

  /**
   * Floating point types that can be bound to a parameter in a database query.
   */
  template <typename T>
  concept floation_point_binding_type = std::floating_point<T> && std::convertible_to<T, double>;

  /**
   * String like types that can be bound to a parameter in a database query.
   */
  template <typename T>
  concept string_binding_type = std::constructible_from<std::string, T> || std::constructible_from<std::string_view, T>;

  /**
   * Byte vector or span like types that can be bound to a parameter in a database query.
   */
  template <typename T>
  concept blob_binding_type = std::constructible_from<byte_vector, T> || std::constructible_from<const_byte_span, T>;

  /**
   * Null like types that can be bound to a parameter in a database query.
   */
  template <typename T>
  concept null_binding_type = std::same_as<T, std::nullptr_t> || std::same_as<T, std::nullopt_t>;

  /**
   * Basic types that can be bound to a parameter in a database query.
   */
  template <typename T>
  concept basic_binding_type = integral_binding_type<T> || floation_point_binding_type<T> || string_binding_type<T> || blob_binding_type<T> || null_binding_type<T>;

  /**
   * Optional basic types that can be bound to a parameter in a database query.
   */
  template <typename T>
  concept optional_binding_type = std::same_as<T, std::optional<typename T::value_type>> && basic_binding_type<typename T::value_type>;

  /**
   * Binding types that contain a single value (column) that can be bound to a parameter in a database query.
   */
  template <typename T>
  concept single_binding_type = basic_binding_type<T> || optional_binding_type<T>;

  /**
   * Ranges of binding types that contain a multiple values (column) that can be bound to a parameter in a database query.
   * Ranges that aee also a valid basic_binding_type are excluded!
   */
  template <typename T>
  concept multi_binding_type = std::ranges::range<T> && single_binding_type<typename std::ranges::range_value_t<T>> && !basic_binding_type<T>;

  /**
   * Full set of types that can be bound to a parameter in a database query.
   */
  template <typename T>
  concept binding_type = single_binding_type<T> || multi_binding_type<T>;

  namespace details
  {
    /**
     * Check that a given type T is a tuple element type at a given index N.
     */
    // see https://stackoverflow.com/questions/68443804/c20-concept-to-check-tuple-like-types
    template<typename T, std::size_t N>
    concept has_tuple_element = requires(T tuple)
    {
      typename std::tuple_element_t<N, std::remove_const_t<T>>;
      { get<N>(tuple) } -> std::convertible_to<const std::tuple_element_t<N, T>&>;
    };

    /**
     * Checks that a given type is a "tuple_like" type.
     */
    template<typename T>
    concept tuple_like = !std::is_reference_v<T> && requires
    {
      typename std::tuple_size<T>::type;
      requires std::derived_from<std::tuple_size<T>, std::integral_constant<std::size_t, std::tuple_size_v<T>>>;
    }
    && []<std::size_t... N>(std::index_sequence<N...>)
    {
      return (has_tuple_element<T, N> && ...);
    } (std::make_index_sequence<std::tuple_size_v<T>>());

    template<typename T, std::size_t N>
    concept has_database_type_tuple_element = database_type<typename std::tuple_element_t<N, T>>;
  }

  template <typename T>
  concept row_type = details::tuple_like<T> && []<std::size_t... N>(std::index_sequence<N...>)
  {
    return (details::has_database_type_tuple_element<T, N> && ...);
  } (std::make_index_sequence<std::tuple_size_v<T>>());

  namespace details
  {
    [[nodiscard]] SQLITE_WRAPPER_EXPORT auto create_prepared_statement(const db_with_location& database, std::string_view sql) -> statement;

    SQLITE_WRAPPER_EXPORT void bind_value(const stmt_with_location& stmt, int index);  // null-value
    SQLITE_WRAPPER_EXPORT void bind_value(const stmt_with_location& stmt, int index, std::int64_t value);
    SQLITE_WRAPPER_EXPORT void bind_value(const stmt_with_location& stmt, int index, double value);
    SQLITE_WRAPPER_EXPORT void bind_value(const stmt_with_location& stmt, int index, std::string_view value);
    SQLITE_WRAPPER_EXPORT void bind_value(const stmt_with_location& stmt, int index, const_byte_span value);

    void bind_value(const stmt_with_location& stmt, int index, const integral_binding_type auto& param)
    {
      bind_value(stmt, index, static_cast<std::int64_t>(param));
    }

    void bind_value(const stmt_with_location& stmt, int index, const null_binding_type auto& /*unused*/)
    {
      bind_value(stmt, index);
    }

    void bind_value(const stmt_with_location& stmt, int index, const optional_binding_type auto& param)
    {
      if (param.has_value())
      {
        bind_value(stmt, index, param.value());
      }
      else
      {
        bind_value(stmt, index, std::nullopt);
      }
    }

    void bind_value_and_increment_index(const stmt_with_location& stmt, int& index, const multi_binding_type auto& param_list)
    {
      for (const auto& param : param_list)
      {
        bind_value(stmt, index, param);
        index++;
      }
    }

    void bind_value_and_increment_index(const stmt_with_location& stmt, int& index, const single_binding_type auto& param)
    {
      bind_value(stmt, index, param);
      index++;
    }

    SQLITE_WRAPPER_EXPORT auto get_column(const stmt_with_location& stmt, int index, std::int64_t& value, bool maybe_null) -> bool;
    SQLITE_WRAPPER_EXPORT auto get_column(const stmt_with_location& stmt, int index, double& value, bool maybe_null) -> bool;
    SQLITE_WRAPPER_EXPORT auto get_column(const stmt_with_location& stmt, int index, std::string& value, bool maybe_null) -> bool;
    SQLITE_WRAPPER_EXPORT auto get_column(const stmt_with_location& stmt, int index, byte_vector& value, bool maybe_null) -> bool;

    void get_column(const stmt_with_location& stmt, int index, basic_database_type auto& value)
    {
      get_column(stmt, index, value, false);
    }

    void get_column(const stmt_with_location& stmt, int index, optional_database_type auto& value)
    {
      if (!get_column(stmt, index, value.emplace(), true))
      {
        value.reset();
      }
    }

    template <database_type... Columns>
      requires(sizeof...(Columns) >= 1)
    void get_row(const stmt_with_location& stmt, Columns&... columns)
    {
      int index{0};

      (get_column(stmt, index++, columns), ...);
    }
  }  // namespace details

  /**
   * Controls if a database file may be created when trying to open it
   */
  enum class open_flags : unsigned
  {
    open_or_create = 1, ///< Open or create database file
    open_only           ///< Only open already existing database file
  };

  /**
   * Opens or creates a database file.
   *
   * @param file_name name of the database file to open or create incl. an absolute or relative path
   * @param flags one of the value define in ::open_flags
   * @param loc caller location
   * @returns a database handle in a RAII guard
   * @throws sqlite_error in case an invalid flag is used or SQLite returns an error or an invalid handle
   */
  [[nodiscard]] SQLITE_WRAPPER_EXPORT auto open(const std::string& file_name, open_flags flags, const std::source_location& loc = std::source_location::current()) -> database;

  /**
   * Opens or creates a database file.
   *
   * @param file_name name of the database file to open or create incl. an absolute or relative path
   * @param loc caller location
   * @returns a database handle in a RAII guard
   * @throws sqlite_error in case SQLite returns an error or an invalid handle
   */
  [[nodiscard]] inline auto open(const std::string& file_name, const std::source_location& loc = std::source_location::current()) -> database
  {
    return open(file_name, open_flags::open_or_create, loc);
  }

  [[nodiscard]] auto create_prepared_statement(const db_with_location& database, std::string_view sql, const binding_type auto&... params) -> statement
  {
    auto stmt{details::create_prepared_statement(database, sql)};

    // "false-positive" triggered by empty parameter pack NOLINTNEXTLINE(misc-const-correctness)
    [[maybe_unused]] int index{1};

    (details::bind_value_and_increment_index({stmt.get(), database.location}, index, params), ...);

    return stmt;
  }

  [[nodiscard]] SQLITE_WRAPPER_EXPORT auto step(const stmt_with_location& stmt) -> bool;


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

  void execute_no_data(const db_with_location& database, std::string_view sql, const binding_type auto&... params)
  {
    const auto stmt{create_prepared_statement(database, sql, params...)};

    if (step({stmt.get(), database.location}))
    {
      throw sqlite_error("unexpected data row in execute_no_data()", {stmt.get(), database.location});
    }
  }

  template <row_type Row>
  [[nodiscard]] auto execute_one_row(const db_with_location& database, std::string_view sql, const binding_type auto&... params) -> Row
  {
    const auto stmt{create_prepared_statement(database, sql, params...)};

    const auto result{get_rows<Row>(stmt, 2)};

    if (const auto size{result.size()}; size != 1)
    {
      throw sqlite_error(sqlite_wrapper::format("expected exactly one row but found {}", (size == 0) ? "none" : "more"), stmt);
    }

    return std::move(result[0]);
  }

  template <row_type Row>
  [[nodiscard]] auto execute(const db_with_location& database, std::string_view sql, const binding_type auto&... params) -> std::vector<Row>
  {
    const auto stmt{create_prepared_statement(database, sql, params...)};

    return get_rows<Row>({stmt.get(), database.location});
  }

}  // namespace sqlite_wrapper

namespace SQLITEWRAPPER_FORMAT_NAMESPACE_NAME
{
  template<>
  // NOLINTNEXTLINE(cert-dcl58-cpp) modification of 'std' namespace can result in undefined behavior
  struct formatter<sqlite_wrapper::open_flags> : sqlite_wrapper::empty_format_spec
  {
    template<typename FmtContext>
    static auto format(sqlite_wrapper::open_flags flags, FmtContext& ctx)
    {
      using namespace std::string_view_literals;
      std::string_view flagStr{};

      switch (flags)
      {
        case sqlite_wrapper::open_flags::open_or_create:
          flagStr = "open_or_create"sv;
        break;
        case sqlite_wrapper::open_flags::open_only:
          flagStr = "open_only"sv;
        break;
        default:
          return SQLITEWRAPPER_FORMAT_NAMESPACE::format_to(ctx.out(), "<unknown ({})>", sqlite_wrapper::to_underlying(flags));
      }
      return SQLITEWRAPPER_FORMAT_NAMESPACE::format_to(ctx.out(), "{}", flagStr);
    }
  };
}
