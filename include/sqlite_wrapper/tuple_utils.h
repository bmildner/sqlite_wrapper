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
  }  // namespace details

  template <typename T, typename Tuple>
  using add_type_front = details::add_type_front<T, Tuple>::type;

  template <typename T, typename Tuple>
  using add_type_back = details::add_type_back<T, Tuple>::type;

  template <typename Tuple>
  using remove_type_front = details::remove_type_front<Tuple>::type;

  template <typename Tuple>
  using remove_type_back = details::remove_type_back<Tuple>::type;

  template <typename Tuple>
    requires is_tuple_like<std::decay_t<Tuple>> && (std::tuple_size_v<std::decay_t<Tuple>> >= 1)
  [[nodiscard]] constexpr auto pop_front(Tuple&& tuple)
  {
    return std::make_pair(
        std::get<0>(std::forward<Tuple>(tuple)),
        std::apply([](auto&&, auto&&... rest) -> auto { return std::make_tuple(std::forward<decltype(rest)>(rest)...); },
                   std::forward<Tuple>(tuple)));
  }

  template <typename Tuple>
    requires is_tuple_like<std::decay_t<Tuple>> && (std::tuple_size_v<std::decay_t<Tuple>> >= 1)
  [[nodiscard]] constexpr auto pop_back(Tuple&& tuple)
  {
    return std::make_pair(std::get<std::tuple_size_v<std::decay_t<Tuple>> - 1>(std::forward<Tuple>(tuple)),
                          [&]<std::size_t... I>(std::index_sequence<I...>) -> auto
                          {
                            return std::make_tuple(std::get<I>(std::forward<Tuple>(tuple))...);
                          }(std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>> - 1>()));
  }

  template <typename Tuple>
    requires is_array_like<std::decay_t<Tuple>>
  [[nodiscard]] constexpr auto to_array(Tuple&& tuple)
  {
    using array_type = std::array<std::tuple_element_t<0, std::decay_t<Tuple>>, std::tuple_size_v<std::decay_t<Tuple>>>;
    return std::apply([]<typename... T>(T&&... elements) -> auto { return array_type{std::forward<T>(elements)...}; },
                      std::forward<Tuple>(tuple));
  }

}  // namespace sqlite_wrapper
