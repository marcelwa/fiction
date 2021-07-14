//
// Created by marcel on 26.05.21.
//

#ifndef FICTION_NAME_RESTORATION_HPP
#define FICTION_NAME_RESTORATION_HPP

#include <mockturtle/traits.hpp>
#include <mockturtle/utils/node_map.hpp>

namespace fiction
{

template <typename NtkSrc, typename NtkDest>
void restore_network_name(const NtkSrc& ntk_src, NtkDest& ntk_dest) noexcept
{
    static_assert(mockturtle::is_network_type_v<NtkSrc>, "NtkSrc is not a network type");
    static_assert(mockturtle::is_network_type_v<NtkDest>, "NtkDest is not a network type");

    if constexpr (mockturtle::has_get_network_name_v<NtkSrc> && mockturtle::has_set_network_name_v<NtkDest>)
    {
        ntk_dest.set_network_name(ntk_src.get_network_name());
    }
}

template <typename NtkSrc, typename NtkDest>
void restore_names(const NtkSrc& ntk_src, NtkDest& ntk_dest,
                   mockturtle::node_map<mockturtle::signal<NtkDest>, NtkSrc>& old2new) noexcept
{
    restore_network_name(ntk_src, ntk_dest);

    if constexpr (mockturtle::has_has_name_v<NtkSrc> && mockturtle::has_get_name_v<NtkSrc> &&
                  mockturtle::has_set_name_v<NtkDest>)
    {
        static_assert(mockturtle::has_foreach_node_v<NtkSrc>, "NtkSrc does not implement the foreach_node function");
        static_assert(mockturtle::has_foreach_fanin_v<NtkSrc>, "NtkSrc does not implement the foreach_fanin function");
        static_assert(mockturtle::has_foreach_po_v<NtkSrc>, "NtkSrc does not implement the foreach_po function");
        static_assert(mockturtle::has_get_node_v<NtkSrc>, "NtkSrc does not implement the get_node function");

        const auto restore_signal_name = [&ntk_src, &ntk_dest, &old2new](const auto& f)
        {
            if (ntk_src.has_name(f))
            {
                auto name = ntk_src.get_name(f);

                ntk_dest.set_name(old2new[ntk_src.get_node(f)], name);
            }
        };

        ntk_src.foreach_node([&ntk_src, &restore_signal_name](const auto& n)
                             { ntk_src.foreach_fanin(n, restore_signal_name); });

        ntk_src.foreach_po(restore_signal_name);
    }
}

}  // namespace fiction

#endif  // FICTION_NAME_RESTORATION_HPP
