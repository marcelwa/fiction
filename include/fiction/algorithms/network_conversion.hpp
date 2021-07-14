//
// Created by marcel on 20.05.21.
//

#ifndef FICTION_NETWORK_CONVERSION_HPP
#define FICTION_NETWORK_CONVERSION_HPP

#include "../traits.hpp"
#include "../types.hpp"
#include "name_restoration.hpp"

#include <mockturtle/traits.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <mockturtle/utils/progress_bar.hpp>
#include <mockturtle/views/topo_view.hpp>

#include <type_traits>

namespace fiction
{

namespace detail
{

template <typename NtkDest, typename NtkSrc, bool is_same_ntk_type = std::is_same_v<NtkDest, NtkSrc>>
class convert_network_impl
{};

template <typename NtkDest, typename NtkSrc>
class convert_network_impl<NtkDest, NtkSrc, true>
{
  public:
    explicit convert_network_impl(const NtkSrc& ntk_src) : ntk{ntk_src} {}

    NtkDest run()
    {
        return ntk;
    }

  private:
    NtkDest ntk;
};

template <typename NtkDest, typename NtkSrc>
class convert_network_impl<NtkDest, NtkSrc, false>
{
  public:
    explicit convert_network_impl(const NtkSrc& ntk_src) : ntk{ntk_src} {}

    NtkDest run()
    {
        auto  init     = mockturtle::initialize_copy_network<NtkDest>(ntk);
        auto& ntk_dest = init.first;
        auto& old2new  = init.second;

        const auto gather_fanin_signals = [this, &ntk_dest, &old2new](const auto& n)
        {
            std::vector<typename NtkDest::signal> children{};

            ntk.foreach_fanin(n,
                              [this, &ntk_dest, &old2new, &children](const auto& f)
                              {
                                  const auto fn         = ntk.get_node(f);
                                  const auto tgt_signal = old2new[fn];

                                  children.emplace_back(ntk.is_complemented(f) ? ntk_dest.create_not(tgt_signal) :
                                                                                 tgt_signal);
                              });

            return children;
        };

#if (PROGRESS_BARS)
        // initialize a progress bar
        mockturtle::progress_bar bar{static_cast<uint32_t>(ntk.num_gates()), "[i] network conversion: |{0}|"};
#endif

        ntk.foreach_gate(
            [&, this](const auto& g, [[maybe_unused]] auto i)
            {
                auto children = gather_fanin_signals(g);

#if (PROGRESS_BARS)
                // update progress
                bar(i);
#endif

                if constexpr (mockturtle::has_is_and_v<TopoNtkSrc> && mockturtle::has_create_and_v<NtkDest>)
                {
                    if (ntk.is_and(g))
                    {
                        old2new[g] = ntk_dest.create_and(children[0], children[1]);
                        return true;
                    }
                }
                if constexpr (mockturtle::has_is_or_v<TopoNtkSrc> && mockturtle::has_create_or_v<NtkDest>)
                {
                    if (ntk.is_or(g))
                    {
                        old2new[g] = ntk_dest.create_or(children[0], children[1]);
                        return true;
                    }
                }
                if constexpr (mockturtle::has_is_xor_v<TopoNtkSrc> && mockturtle::has_create_xor_v<NtkDest>)
                {
                    if (ntk.is_xor(g))
                    {
                        old2new[g] = ntk_dest.create_xor(children[0], children[1]);
                        return true;
                    }
                }
                if constexpr (mockturtle::has_is_maj_v<TopoNtkSrc> && mockturtle::has_create_maj_v<NtkDest>)
                {
                    if (ntk.is_maj(g))
                    {
                        old2new[g] = ntk_dest.create_maj(children[0], children[1], children[2]);
                        return true;
                    }
                }
                if constexpr (mockturtle::has_is_nary_and_v<TopoNtkSrc> && mockturtle::has_create_nary_and_v<NtkDest>)
                {
                    if (ntk.is_nary_and(g))
                    {
                        old2new[g] = ntk_dest.create_nary_and(children);
                        return true;
                    }
                }
                if constexpr (mockturtle::has_is_nary_or_v<TopoNtkSrc> && mockturtle::has_create_nary_or_v<NtkDest>)
                {
                    if (ntk.is_nary_or(g))
                    {
                        old2new[g] = ntk_dest.create_nary_or(children);
                        return true;
                    }
                }
                if constexpr (mockturtle::has_is_nary_xor_v<TopoNtkSrc> && mockturtle::has_create_nary_xor_v<NtkDest>)
                {
                    if (ntk.is_nary_xor(g))
                    {
                        old2new[g] = ntk_dest.create_nary_xor(children);
                        return true;
                    }
                }
                if constexpr (fiction::has_is_buf_v<TopoNtkSrc> && mockturtle::has_create_buf_v<NtkDest>)
                {
                    if (ntk.is_buf(g))
                    {
                        old2new[g] = ntk_dest.create_buf(children[0]);
                        return true;
                    }
                }
                if constexpr (mockturtle::has_node_function_v<TopoNtkSrc> && mockturtle::has_create_node_v<NtkDest>)
                {
                    old2new[g] = ntk_dest.create_node(children, ntk.node_function(g));
                    return true;
                }

                return true;
            });

        ntk.foreach_po(
            [this, &ntk_dest, &old2new](const auto& po)
            {
                const auto tgt_signal = old2new[ntk.get_node(po)];
                const auto tgt_po     = ntk.is_complemented(po) ? ntk_dest.create_not(tgt_signal) : tgt_signal;

                ntk_dest.create_po(tgt_po);
            });

        // restore signal names if applicable
        fiction::restore_names(ntk, ntk_dest, old2new);

        return ntk_dest;
    }

  private:
    using TopoNtkSrc = mockturtle::topo_view<NtkSrc>;
    TopoNtkSrc ntk;
};
}  // namespace detail

template <typename NtkDest, typename NtkSrc>
NtkDest convert_network(const NtkSrc& ntk)
{
    static_assert(mockturtle::is_network_type_v<NtkSrc>, "NtkSrc is not a network type");
    static_assert(mockturtle::is_network_type_v<NtkDest>, "NtkDest is not a network type");

    static_assert(mockturtle::has_get_node_v<NtkSrc>, "NtkSrc does not implement the get_node function");
    static_assert(mockturtle::has_is_complemented_v<NtkSrc>, "NtkSrc does not implement the is_complemented function");
    static_assert(mockturtle::has_foreach_pi_v<NtkSrc>, "NtkSrc does not implement the foreach_pi function");
    static_assert(mockturtle::has_foreach_gate_v<NtkSrc>, "NtkSrc does not implement the foreach_gate function");
    static_assert(mockturtle::has_foreach_po_v<NtkSrc>, "NtkSrc does not implement the foreach_po function");
    static_assert(mockturtle::has_foreach_fanin_v<NtkSrc>, "NtkSrc does not implement the foreach_fanin function");

    static_assert(mockturtle::has_get_constant_v<NtkSrc>, "NtkSrc does not implement the get_constant function");
    static_assert(mockturtle::has_get_constant_v<NtkDest>, "NtkDest does not implement the get_constant function");

    static_assert(mockturtle::has_create_pi_v<NtkDest>, "NtkDest does not implement the create_pi function");
    static_assert(mockturtle::has_create_po_v<NtkDest>, "NtkDest does not implement the create_po function");
    static_assert(mockturtle::has_create_not_v<NtkDest>, "NtkDest does not implement the create_not function");
    static_assert(mockturtle::has_create_and_v<NtkDest>, "NtkDest does not implement the create_and function");
    static_assert(mockturtle::has_create_or_v<NtkDest>, "NtkDest does not implement the create_or function");
    static_assert(mockturtle::has_create_xor_v<NtkDest>, "NtkDest does not implement the create_xor function");
    static_assert(mockturtle::has_create_maj_v<NtkDest>, "NtkDest does not implement the create_maj function");
    // TODO handle ci/ro/etc...

    assert(ntk.is_combinational() && "Network has to be combinational");

    detail::convert_network_impl<NtkDest, NtkSrc> p{ntk};

    auto result = p.run();

    return result;
}

}  // namespace fiction

#endif  // FICTION_NETWORK_CONVERSION_HPP
