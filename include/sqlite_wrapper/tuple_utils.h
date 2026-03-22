#pragma once

#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace sqlite_wrapper
{
  namespace details
  {
    /**
     * Check that a given type T is a tuple element type at a given index N.
     */
    // see https://stackoverflow.com/questions/68443804/c20-concept-to-check-tuple-like-types
    template <typename T, std::size_t N>
    concept has_tuple_element = requires(T tuple) {
      typename std::tuple_element_t<N, std::remove_const_t<T>>;
      { std::get<N>(tuple) } -> std::convertible_to<const std::tuple_element_t<N, T>&>;
    };

    template <typename T, typename Tuple>
    struct add_type_front;

    template <typename T, typename... Args>
    struct add_type_front<T, std::tuple<Args...>>
    {
      using type = std::tuple<T, Args...>;
    };

    template <typename T, typename Tuple>
    struct add_type_back;

    template <typename T, typename... Args>
    struct add_type_back<T, std::tuple<Args...>>
    {
      using type = std::tuple<Args..., T>;
    };
  }  // namespace details

  /**
   * Checks that a given type is a "tuple_like" type.
   */
  // see https://stackoverflow.com/questions/68443804/c20-concept-to-check-tuple-like-types
  template <typename T>
  concept is_tuple_like =
      requires {
        typename std::tuple_size<T>::type;
        requires std::derived_from<std::tuple_size<T>, std::integral_constant<std::size_t, std::tuple_size_v<T>>>;
      } && []<std::size_t... N>(std::index_sequence<N...>) -> auto
  { return (details::has_tuple_element<T, N> && ...); }(std::make_index_sequence<std::tuple_size_v<T>>());

  template <typename T, typename... Args>
  using add_type_front = details::add_type_front<T, Args...>::type;

  template <typename T, typename... Args>
  using add_type_back = details::add_type_back<T, Args...>::type;

  template <typename Tuple>
    requires is_tuple_like<std::decay_t<Tuple>> && (std::tuple_size_v<std::decay_t<Tuple>> >= 1)
  [[nodiscard]] constexpr auto pop_front(Tuple&& tuple)
  {
    return std::apply([](auto&&, auto&&... rest) -> auto { return std::make_tuple(std::forward<decltype(rest)>(rest)...); },
                      std::forward<Tuple>(tuple));
  }

  template <typename Tuple>
    requires is_tuple_like<std::decay_t<Tuple>> && (std::tuple_size_v<std::decay_t<Tuple>> >= 1)
  [[nodiscard]] constexpr auto pop_back(Tuple&& tuple)
  {
    return [&]<std::size_t... I>(std::index_sequence<I...>) -> auto
    {
      return std::make_tuple(std::get<I>(std::forward<Tuple>(tuple))...);
    }(std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>> - 1>());
  }

}  // namespace sqlite_wrapper
