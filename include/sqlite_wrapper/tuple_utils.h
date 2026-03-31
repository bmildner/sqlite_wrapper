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
    template <typename T, typename Tuple>
    struct add_type_front;

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
    struct add_type_back;

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
    struct remove_type_front;

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
    struct remove_type_back;

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

  template <typename T, tuple_like Tuple>
  using add_type_front = details::add_type_front<T, std::remove_cvref_t<Tuple>>::type;

  template <typename T, tuple_like Tuple>
  using add_type_back = details::add_type_back<T, std::remove_cvref_t<Tuple>>::type;

  template <tuple_like Tuple, boolean_constant convert = std::false_type>
  using remove_type_front = details::try_to_convert_to_array_type<typename details::remove_type_front<Tuple>::type, convert>;

  template <tuple_like Tuple, boolean_constant convert = std::false_type>
  using remove_type_back = details::try_to_convert_to_array_type<typename details::remove_type_back<Tuple>::type, convert>;

  template <tuple_like Tuple>
    requires(std::tuple_size_v<std::remove_cvref_t<Tuple>> >= 1)
  [[nodiscard]] constexpr auto pop_front(Tuple&& tuple) -> auto
  {
    return std::make_pair(std::get<0>(std::forward<Tuple>(tuple)),
                          std::apply([]<typename... T>(auto&&, T&&... rest) -> auto
                                     { return std::make_tuple(std::forward<T>(rest)...); }, std::forward<Tuple>(tuple)));
  }

  template <tuple_like Tuple>
    requires(std::tuple_size_v<std::remove_cvref_t<Tuple>> >= 1)
  [[nodiscard]] constexpr auto pop_back(Tuple&& tuple) -> auto
  {
    return std::make_pair(std::get<std::tuple_size_v<std::remove_cvref_t<Tuple>> - 1>(std::forward<Tuple>(tuple)),
                          [&]<std::size_t... I>(std::index_sequence<I...>) -> auto
                          {
                            return std::make_tuple(std::get<I>(std::forward<Tuple>(tuple))...);
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
      return std::make_tuple(std::get<I>(std::forward<Tuple>(tuple))..., std::forward<T>(element));
    }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>>());
  }

  template <array_like Tuple>
  [[nodiscard]] constexpr auto to_array(Tuple&& tuple) -> auto
  {
    using array_type =
        std::array<std::tuple_element_t<0, std::remove_cvref_t<Tuple>>, std::tuple_size_v<std::remove_cvref_t<Tuple>>>;
    return std::apply([]<typename... T>(T&&... elements) -> auto { return array_type{std::forward<T>(elements)...}; },
                      std::forward<Tuple>(tuple));
  }

}  // namespace sqlite_wrapper
