//
// Created by marcel on 24.05.21.
//

#ifndef FICTION_TRAITS_HPP
#define FICTION_TRAITS_HPP

#include <mockturtle/traits.hpp>

namespace fiction
{

#pragma region has_is_po
template <class Ntk, class = void>
struct has_is_po : std::false_type
{};

template <class Ntk>
struct has_is_po<Ntk, std::void_t<decltype(std::declval<Ntk>().is_po(std::declval<mockturtle::node<Ntk>>()))>>
        : std::true_type
{};

template <class Ntk>
inline constexpr bool has_is_po_v = has_is_po<Ntk>::value;
#pragma endregion

#pragma region has_is_buf
template <class Ntk, class = void>
struct has_is_buf : std::false_type
{};

template <class Ntk>
struct has_is_buf<Ntk, std::void_t<decltype(std::declval<Ntk>().is_buf(std::declval<mockturtle::node<Ntk>>()))>>
        : std::true_type
{};

template <class Ntk>
inline constexpr bool has_is_buf_v = has_is_buf<Ntk>::value;
#pragma endregion

#pragma region has_is_inv
template <class Ntk, class = void>
struct has_is_inv : std::false_type
{};

template <class Ntk>
struct has_is_inv<Ntk, std::void_t<decltype(std::declval<Ntk>().is_inv(std::declval<mockturtle::node<Ntk>>()))>>
        : std::true_type
{};

template <class Ntk>
inline constexpr bool has_is_inv_v = has_is_inv<Ntk>::value;
#pragma endregion

#pragma region has_is_fanout
template <class Ntk, class = void>
struct has_is_fanout : std::false_type
{};

template <class Ntk>
struct has_is_fanout<Ntk, std::void_t<decltype(std::declval<Ntk>().is_fanout(std::declval<mockturtle::node<Ntk>>()))>>
        : std::true_type
{};

template <class Ntk>
inline constexpr bool has_is_fanout_v = has_is_fanout<Ntk>::value;
#pragma endregion

#pragma region has_is_nand
template <class Ntk, class = void>
struct has_is_nand : std::false_type
{};

template <class Ntk>
struct has_is_nand<Ntk, std::void_t<decltype(std::declval<Ntk>().is_nand(std::declval<mockturtle::node<Ntk>>()))>>
        : std::true_type
{};

template <class Ntk>
inline constexpr bool has_is_nand_v = has_is_nand<Ntk>::value;
#pragma endregion

#pragma region has_is_nor
template <class Ntk, class = void>
struct has_is_nor : std::false_type
{};

template <class Ntk>
struct has_is_nor<Ntk, std::void_t<decltype(std::declval<Ntk>().is_nor(std::declval<mockturtle::node<Ntk>>()))>>
        : std::true_type
{};

template <class Ntk>
inline constexpr bool has_is_nor_v = has_is_nor<Ntk>::value;
#pragma endregion

#pragma region has_is_xnor
template <class Ntk, class = void>
struct has_is_xnor : std::false_type
{};

template <class Ntk>
struct has_is_xnor<Ntk, std::void_t<decltype(std::declval<Ntk>().is_xnor(std::declval<mockturtle::node<Ntk>>()))>>
        : std::true_type
{};

template <class Ntk>
inline constexpr bool has_is_xnor_v = has_is_xnor<Ntk>::value;
#pragma endregion

#pragma region has_is_and3
template <class Ntk, class = void>
struct has_is_and3 : std::false_type
{};

template <class Ntk>
struct has_is_and3<Ntk, std::void_t<decltype(std::declval<Ntk>().is_and3(std::declval<mockturtle::node<Ntk>>()))>>
        : std::true_type
{};

template <class Ntk>
inline constexpr bool has_is_and3_v = has_is_and3<Ntk>::value;
#pragma endregion

#pragma region has_is_xor_and
template <class Ntk, class = void>
struct has_is_xor_and : std::false_type
{};

template <class Ntk>
struct has_is_xor_and<Ntk, std::void_t<decltype(std::declval<Ntk>().is_xor_and(std::declval<mockturtle::node<Ntk>>()))>>
        : std::true_type
{};

template <class Ntk>
inline constexpr bool has_is_xor_and_v = has_is_xor_and<Ntk>::value;
#pragma endregion

#pragma region has_is_or_and
template <class Ntk, class = void>
struct has_is_or_and : std::false_type
{};

template <class Ntk>
struct has_is_or_and<Ntk, std::void_t<decltype(std::declval<Ntk>().is_or_and(std::declval<mockturtle::node<Ntk>>()))>>
        : std::true_type
{};

template <class Ntk>
inline constexpr bool has_is_or_and_v = has_is_or_and<Ntk>::value;
#pragma endregion

#pragma region has_is_onehot
template <class Ntk, class = void>
struct has_is_onehot : std::false_type
{};

template <class Ntk>
struct has_is_onehot<Ntk, std::void_t<decltype(std::declval<Ntk>().is_onehot(std::declval<mockturtle::node<Ntk>>()))>>
        : std::true_type
{};

template <class Ntk>
inline constexpr bool has_is_onehot_v = has_is_onehot<Ntk>::value;
#pragma endregion

#pragma region has_is_gamble
template <class Ntk, class = void>
struct has_is_gamble : std::false_type
{};

template <class Ntk>
struct has_is_gamble<Ntk, std::void_t<decltype(std::declval<Ntk>().is_gamble(std::declval<mockturtle::node<Ntk>>()))>>
        : std::true_type
{};

template <class Ntk>
inline constexpr bool has_is_gamble_v = has_is_gamble<Ntk>::value;
#pragma endregion

#pragma region has_create_dot
template <class Ntk, class = void>
struct has_create_dot : std::false_type
{};

template <class Ntk>
struct has_create_dot<Ntk, std::void_t<decltype(std::declval<Ntk>().create_dot(
                               std::declval<mockturtle::signal<Ntk>>(), std::declval<mockturtle::signal<Ntk>>(),
                               std::declval<mockturtle::signal<Ntk>>()))>> : std::true_type
{};

template <class Ntk>
inline constexpr bool has_create_dot_v = has_create_dot<Ntk>::value;
#pragma endregion

#pragma region has_is_dot
template <class Ntk, class = void>
struct has_is_dot : std::false_type
{};

template <class Ntk>
struct has_is_dot<Ntk, std::void_t<decltype(std::declval<Ntk>().is_dot(std::declval<mockturtle::node<Ntk>>()))>>
        : std::true_type
{};

template <class Ntk>
inline constexpr bool has_is_dot_v = has_is_dot<Ntk>::value;
#pragma endregion

#pragma region has_is_mux
template <class Ntk, class = void>
struct has_is_mux : std::false_type
{};

template <class Ntk>
struct has_is_mux<Ntk, std::void_t<decltype(std::declval<Ntk>().is_mux(std::declval<mockturtle::node<Ntk>>()))>>
        : std::true_type
{};

template <class Ntk>
inline constexpr bool has_is_mux_v = has_is_mux<Ntk>::value;
#pragma endregion

#pragma region has_is_and_xor
template <class Ntk, class = void>
struct has_is_and_xor : std::false_type
{};

template <class Ntk>
struct has_is_and_xor<Ntk, std::void_t<decltype(std::declval<Ntk>().is_and_xor(std::declval<mockturtle::node<Ntk>>()))>>
        : std::true_type
{};

template <class Ntk>
inline constexpr bool has_is_and_xor_v = has_is_and_xor<Ntk>::value;
#pragma endregion

#pragma region has_get_layout_name
template <class Ntk, class = void>
struct has_get_layout_name : std::false_type
{};

template <class Ntk>
struct has_get_layout_name<Ntk, std::void_t<decltype(std::declval<Ntk>().get_layout_name())>> : std::true_type
{};

template <class Ntk>
inline constexpr bool has_get_layout_name_v = has_get_layout_name<Ntk>::value;
#pragma endregion

#pragma region has_set_layout_name
template <class Ntk, class = void>
struct has_set_layout_name : std::false_type
{};

template <class Ntk>
struct has_set_layout_name<Ntk, std::void_t<decltype(std::declval<Ntk>().set_layout_name(std::string()))>>
        : std::true_type
{};

template <class Ntk>
inline constexpr bool has_set_layout_name_v = has_set_layout_name<Ntk>::value;
#pragma endregion

}  // namespace fiction

#endif  // FICTION_TRAITS_HPP
