#pragma once

#include "sqlite_wrapper/concepts.h"

#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace sqlite_wrapper
{
  namespace details
  {
    // Dependent "false" for use in static_assert inside templates, so the assert only fires on
    // instantiation instead of unconditionally.
    template <typename...>
    inline constexpr bool always_false = false;

    template <typename T, typename Tuple>
    struct add_type_front
    {
      static_assert(always_false<Tuple>,
                    "add_type_front: unsupported tuple-like type; only std::tuple and std::pair are supported");
      using type = Tuple;  // suppress follow-on "no member type" errors
    };

    // std::array or another homogeneous, range-backed tuple-like container: its element type is fixed, so adding a
    // differently-typed element is not representable.
    template <typename T, array_like Array>
    struct add_type_front<T, Array>
    {
      static_assert(always_false<Array>,
                    "add_type_front: cannot add a type to std::array or another homogeneous, range-backed tuple-like "
                    "container; its element type is fixed. Convert to std::tuple first.");
      using type = Array;
    };

    template <typename T, typename... Args>
    struct add_type_front<T, std::tuple<Args...>>
    {
      using type = std::tuple<T, Args...>;
    };

    template <typename T, typename U, typename V>
    struct add_type_front<T, std::pair<U, V>>
    {
      using type = std::tuple<T, U, V>;
    };

    template <typename T, typename Tuple>
    struct add_type_back
    {
      static_assert(always_false<Tuple>,
                    "add_type_back: unsupported tuple-like type; only std::tuple and std::pair are supported");
      using type = Tuple;  // suppress follow-on "no member type" errors
    };

    // std::array or another homogeneous, range-backed tuple-like container: its element type is
    // fixed, so adding a differently-typed element is not representable.
    template <typename T, array_like Array>
    struct add_type_back<T, Array>
    {
      static_assert(always_false<Array>,
                    "add_type_back: cannot add a type to std::array or another homogeneous, range-backed tuple-like "
                    "container; its element type is fixed. Convert to std::tuple first.");
      using type = Array;
    };

    template <typename T, typename... Args>
    struct add_type_back<T, std::tuple<Args...>>
    {
      using type = std::tuple<Args..., T>;
    };

    template <typename T, typename U, typename V>
    struct add_type_back<T, std::pair<U, V>>
    {
      using type = std::tuple<U, V, T>;
    };

    template <typename Tuple>
    struct remove_type_front
    {
      static_assert(always_false<Tuple>,
                    "remove_type_front: unsupported tuple-like type; only std::tuple and std::pair are supported");
      using type = Tuple;  // suppress follow-on "no member type" errors
    };

    // std::array or another homogeneous, range-backed tuple-like container: its element type/size
    // is fixed, so removing an element is not representable.
    template <array_like Array>
    struct remove_type_front<Array>
    {
      static_assert(always_false<Array>,
                    "remove_type_front: cannot remove a type from std::array or another homogeneous, range-backed "
                    "tuple-like container; its size/element type is fixed. Convert to std::tuple first.");
      using type = Array;
    };

    // For empty tuple
    template <>
    struct remove_type_front<std::tuple<>>
    {
      using type = std::tuple<>;
    };

    template <typename T, typename... Args>
    struct remove_type_front<std::tuple<T, Args...>>
    {
      using type = std::tuple<Args...>;
    };

    template <typename T, typename U>
    struct remove_type_front<std::pair<T, U>>
    {
      using type = std::tuple<U>;
    };

    template <typename Tuple>
    struct remove_type_back
    {
      static_assert(always_false<Tuple>,
                    "remove_type_back: unsupported tuple-like type; only std::tuple and std::pair are supported");
      using type = Tuple;  // suppress follow-on "no member type" errors
    };

    // std::array or another homogeneous, range-backed tuple-like container: its element type/size is fixed, so removing an
    // element is not representable.
    template <array_like Array>
    struct remove_type_back<Array>
    {
      static_assert(always_false<Array>,
                    "remove_type_back: cannot remove a type from std::array or another homogeneous, range-backed "
                    "tuple-like container; its size/element type is fixed. Convert to std::tuple first.");
      using type = Array;
    };

    // For empty tuple
    template <>
    struct remove_type_back<std::tuple<>>
    {
      using type = std::tuple<>;
    };

    template <typename T>
    struct remove_type_back<std::tuple<T>>
    {
      using type = std::tuple<>;
    };

    template <typename First, typename Second>
    struct remove_type_back<std::tuple<First, Second>>
    {
      using type = std::tuple<First>;
    };

    template <typename First, typename Second, typename... Rest>
    struct remove_type_back<std::tuple<First, Second, Rest...>>
    {
      using type = add_type_front<First, typename remove_type_back<std::tuple<Second, Rest...>>::type>::type;
    };

    template <typename T, typename U>
    struct remove_type_back<std::pair<T, U>>
    {
      using type = std::tuple<T>;
    };

    template <boolean_constant convert, typename T>
    struct try_to_convert_to_array_type_impl;

    template <boolean_constant convert, typename T>
      requires(!convert::value) || (!array_like<T>)
    struct try_to_convert_to_array_type_impl<convert, T>
    {
      using type = T;
    };

    template <boolean_constant convert, array_like T>
      requires convert::value
    struct try_to_convert_to_array_type_impl<convert, T>
    {
      using type = std::array<std::tuple_element_t<0, T>, std::tuple_size_v<T>>;
    };

    template <tuple_like Tuple, boolean_constant convert = std::true_type>
    using try_to_convert_to_array_type = try_to_convert_to_array_type_impl<convert, Tuple>::type;
  }  // namespace details

  template <tuple_like T>
  using try_to_convert_to_array_type = details::try_to_convert_to_array_type<T>;

  template <array_like T>
  using convert_to_array_type = try_to_convert_to_array_type<T>;

  template <typename T, heterogeneous_tuple_like Tuple>
  using add_type_front = details::add_type_front<T, std::remove_cvref_t<Tuple>>::type;

  template <typename T, heterogeneous_tuple_like Tuple>
  using add_type_back = details::add_type_back<T, std::remove_cvref_t<Tuple>>::type;

  template <heterogeneous_tuple_like Tuple, boolean_constant convert = std::false_type>
  using remove_type_front =
      details::try_to_convert_to_array_type<typename details::remove_type_front<std::remove_cvref_t<Tuple>>::type, convert>;

  template <heterogeneous_tuple_like Tuple, boolean_constant convert = std::false_type>
  using remove_type_back =
      details::try_to_convert_to_array_type<typename details::remove_type_back<std::remove_cvref_t<Tuple>>::type, convert>;

  template <tuple_like Tuple>
    requires(std::tuple_size_v<std::remove_cvref_t<Tuple>> >= 1)
  [[nodiscard]] constexpr auto pop_front(Tuple&& tuple) -> auto
  {
    auto& first_element = std::get<0>(tuple);
    return std::make_pair(std::forward_like<Tuple>(first_element),
                          std::apply([]<typename... T>(auto&&, T&&... rest) -> auto
                                     { return std::make_tuple(std::forward<T>(rest)...); }, std::forward<Tuple>(tuple)));
  }

  template <tuple_like Tuple>
    requires(std::tuple_size_v<std::remove_cvref_t<Tuple>> >= 1)
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward) we forward each element in the tuple by its own here!
  [[nodiscard]] constexpr auto pop_back(Tuple&& tuple) -> auto
  {
    auto& last_element = std::get<std::tuple_size_v<std::remove_cvref_t<Tuple>> - 1>(tuple);
    return std::make_pair(std::forward_like<Tuple>(last_element),
                          [&]<std::size_t... I>(std::index_sequence<I...>) -> auto
                          {
                            return std::make_tuple(std::forward_like<Tuple>(std::get<I>(tuple))...);
                          }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>> - 1>()));
  }

  // use std::ref/std::reference_wrapper to add reference types
  template <tuple_like Tuple, typename T>
  [[nodiscard]] constexpr auto push_front(Tuple&& tuple, T&& element) -> auto
  {
    return std::apply([&]<typename... U>(U&&... rest) -> auto
                      { return std::make_tuple(std::forward<T>(element), std::forward<U>(rest)...); },
                      std::forward<Tuple>(tuple));
  }

  // use std::ref/std::reference_wrapper to add reference types
  template <tuple_like Tuple, typename T>
  [[nodiscard]] constexpr auto push_back(Tuple&& tuple, T&& element) -> auto
  {
    return [&]<std::size_t... I>(std::index_sequence<I...>) -> auto
    {
      return std::make_tuple(std::forward_like<Tuple>(std::get<I>(tuple))..., std::forward<T>(element));
    }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>>());
  }

  template <array_like Tuple>
  [[nodiscard]] constexpr auto to_array(Tuple&& tuple) -> auto
  {
    using array_type =
        std::array<std::tuple_element_t<0, std::remove_cvref_t<Tuple>>, std::tuple_size_v<std::remove_cvref_t<Tuple>>>;
#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(push)
#  pragma warning(disable : 4702)  // unreachable code warning, false positive?
#endif
    return std::apply([]<typename... T>(T&&... elements) -> auto { return array_type{std::forward<T>(elements)...}; },
                      std::forward<Tuple>(tuple));
#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(pop)
#endif
  }

  /**
   * Checks whether add_type_front<T, Tuple> is well-formed, i.e. whether a type T could be added to
   * the front of Tuple. Useful for SFINAE-checking rejection of unsupported inputs (e.g. std::array)
   * without triggering the hard compile error that instantiating add_type_front directly would.
   *
   * @tparam Tuple the tuple-like type a type would be added to
   * @tparam T the type that would be added; defaults to int since it is usually irrelevant to whether
   *           the operation is supported at all
   */
  template <typename Tuple, typename T = int>
  concept can_add_type_front = requires { typename add_type_front<T, Tuple>; };

  /**
   * Checks whether add_type_back<T, Tuple> is well-formed, i.e. whether a type T could be added to
   * the back of Tuple. Useful for SFINAE-checking rejection of unsupported inputs (e.g. std::array)
   * without triggering the hard compile error that instantiating add_type_back directly would.
   *
   * @tparam Tuple the tuple-like type a type would be added to
   * @tparam T the type that would be added; defaults to int since it is usually irrelevant to whether
   *           the operation is supported at all
   */
  template <typename Tuple, typename T = int>
  concept can_add_type_back = requires { typename add_type_back<T, Tuple>; };

  /**
   * Checks whether remove_type_front<Tuple> is well-formed, i.e. whether the front element of Tuple
   * could be removed. Useful for SFINAE-checking rejection of unsupported inputs (e.g. std::array)
   * without triggering the hard compile error that instantiating remove_type_front directly would.
   *
   * @tparam Tuple the tuple-like type an element would be removed from
   */
  template <typename Tuple>
  concept can_remove_type_front = requires { typename remove_type_front<Tuple>; };

  /**
   * Checks whether remove_type_back<Tuple> is well-formed, i.e. whether the back element of Tuple
   * could be removed. Useful for SFINAE-checking rejection of unsupported inputs (e.g. std::array)
   * without triggering the hard compile error that instantiating remove_type_back directly would.
   *
   * @tparam Tuple the tuple-like type an element would be removed from
   */
  template <typename Tuple>
  concept can_remove_type_back = requires { typename remove_type_back<Tuple>; };

}  // namespace sqlite_wrapper
