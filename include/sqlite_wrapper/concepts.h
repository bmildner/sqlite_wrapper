#pragma once

#include <concepts>
#include <ranges>
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
  }  // namespace details

  /**
   * Checks that a given type is a "tuple_like" type.
   */
  // see https://stackoverflow.com/questions/68443804/c20-concept-to-check-tuple-like-types
  template <typename T>
  concept tuple_like =
      requires {
        typename std::tuple_size<T>::type;
        requires std::derived_from<std::tuple_size<T>, std::integral_constant<std::size_t, std::tuple_size_v<T>>>;
      } && []<std::size_t... N>(std::index_sequence<N...>) -> auto
  { return (details::has_tuple_element<T, N> && ...); }(std::make_index_sequence<std::tuple_size_v<T>>());

  template <typename T>
  concept array_like =
      tuple_like<T> &&
      (std::ranges::range<T> || ((std::tuple_size_v<T> >= 1) && []<std::size_t... N>(std::index_sequence<N...>) -> auto
                                 {
                                   return (std::same_as<std::tuple_element_t<0, T>, std::tuple_element_t<N, T>> && ...);
                                 }(std::make_index_sequence<std::tuple_size_v<T>>())));

  template <typename T, typename U, typename... V>
  concept same_as_either = std::same_as<T, U> || (std::same_as<T, V> || ...);

  template <typename T, typename U, typename... V>
  concept same_as_all = std::same_as<T, U> && (std::same_as<T, V> && ...);

  template <typename T>
  concept bool_integral_constant = same_as_either<T, std::true_type, std::false_type>;;
}  // namespace sqlite_wrapper
