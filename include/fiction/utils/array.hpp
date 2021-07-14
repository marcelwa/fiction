//
// Created by marcel on 28.06.21.
//

#ifndef FICTION_ARRAY_HPP
#define FICTION_ARRAY_HPP

#include <array>
#include <utility>

namespace fiction
{

namespace detail
{
/**
 * From https://stackoverflow.com/questions/57756557/initializing-a-stdarray-with-a-constant-value
 * @tparam T
 * @tparam Is
 * @param value
 * @return
 */
template <typename T, std::size_t... Is>
constexpr std::array<T, sizeof...(Is)> create_array(T value, [[maybe_unused]] std::index_sequence<Is...>)
{
    return {{(static_cast<void>(Is), value)...}};
}

/**
 * Based on https://stackoverflow.com/questions/57756557/initializing-a-stdarray-with-a-constant-value
 * @tparam ElementType
 * @tparam ArrayType
 * @tparam Is
 * @param a
 * @return
 */
template <typename ElementType, typename ArrayType, std::size_t... Is>
constexpr auto convert_array(const ArrayType& a, [[maybe_unused]] std::index_sequence<Is...>)
    -> std::array<ElementType, sizeof...(Is)>
{
    return {{static_cast<ElementType>(a[Is])...}};
}

/**
 * Based on https://stackoverflow.com/questions/57756557/initializing-a-stdarray-with-a-constant-value
 * @tparam ElementType
 * @tparam T
 * @tparam N
 * @tparam Is
 * @param a
 * @return
 */
template <typename ElementType, typename T, std::size_t N, std::size_t... Is>
constexpr auto convert_array_of_arrays(const std::array<T, N>& a, [[maybe_unused]] std::index_sequence<Is...>)
    -> std::array<std::array<ElementType, sizeof...(Is)>, N>
{
    return {{convert_array<ElementType>(a[Is], std::make_index_sequence<N>())...}};
}

}  // namespace detail

template <std::size_t N, typename T>
constexpr auto create_array(const T& value)
{
    return detail::create_array(value, std::make_index_sequence<N>());
}

template <typename ElementType, typename T, std::size_t N>
constexpr auto convert_array(const std::array<T, N>& a)
{
    return detail::convert_array<ElementType>(a, std::make_index_sequence<N>());
}

template <typename ElementType, typename T, std::size_t N>
constexpr auto convert_array_of_arrays(const std::array<T, N>& a)
{
    return detail::convert_array_of_arrays<ElementType>(a, std::make_index_sequence<N>());
}

}  // namespace fiction

#endif  // FICTION_ARRAY_HPP
