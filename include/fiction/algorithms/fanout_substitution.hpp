//
// Created by marcel on 31.05.21.
//

#ifndef FICTION_FANOUT_SUBSTITUTION_HPP
#define FICTION_FANOUT_SUBSTITUTION_HPP

#include "../traits.hpp"
#include "../utils/debug/network_writer.hpp"
#include "network_conversion.hpp"

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/traits.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <mockturtle/views/topo_view.hpp>

#include <algorithm>
#include <cmath>
#include <deque>
#include <functional>
#include <queue>
#include <utility>
#include <vector>

namespace fiction
{

struct fanout_substitution_params
{
    enum substitution_strategy
    {
        BREADTH,
        DEPTH
    };

    /**
     * Decomposition strategy (DEPTH vs. BREADTH).
     */
    substitution_strategy strategy = BREADTH;
    /**
     * Maximum output degree of each fan-out node.
     */
    uint32_t degree = 2ul;
    /**
     * Maximum number of outputs any gate is allowed to have before substitution applies.
     */
    uint32_t threshold = 1ul;
};

namespace detail
{

template <typename NtkDest, typename NtkSrc>
class fanout_substitution_impl
{
  public:
    fanout_substitution_impl(const NtkSrc& src, const fanout_substitution_params p) :
            ntk_topo{convert_network<NtkDest>(src)},
            available_fanouts{ntk_topo},
            ps{p}
    {}

    NtkDest run()
    {
        // initialize a network copy
        auto init = mockturtle::initialize_copy_network<NtkDest>(ntk_topo);

        auto& substituted = init.first;
        auto& old2new     = init.second;

        const auto generate_fanout_tree = [this, &substituted, &old2new](const auto& n)
        {
            auto num_fanouts = static_cast<uint32_t>(
                std::ceil(static_cast<double>(std::max(
                              static_cast<int32_t>(ntk_topo.fanout_size(n)) - static_cast<int32_t>(ps.threshold), 0)) /
                          static_cast<double>(std::max(static_cast<int32_t>(ps.degree) - 1, 1))));

            auto child = old2new[n];

            if (num_fanouts == 0)
                return;

            if (ps.strategy == fanout_substitution_params::substitution_strategy::DEPTH)
            {
                std::queue<typename NtkDest::signal> q{};
                for (auto i = 0u; i < num_fanouts; ++i)
                {
                    child = substituted.create_buf(child);
                    q.push(child);
                }
                available_fanouts[n] = q;
            }
            else if (ps.strategy == fanout_substitution_params::substitution_strategy::BREADTH)
            {
                std::queue<typename NtkDest::signal> q{{child}};

                for (auto f = 0ul; f < num_fanouts; ++f)
                {
                    child = q.front();
                    q.pop();
                    child = substituted.create_buf(child);
                    for (auto i = 0u; i < ps.degree; ++i) q.push(child);
                }
                available_fanouts[n] = q;
            }
        };

        const auto get_fanout = [this, &substituted](const auto& n, auto& child)
        {
            if (substituted.fanout_size(child) >= ps.threshold)
            {
                if (auto fanouts = available_fanouts[n]; !fanouts.empty())
                {
                    // find non-overfull fanout node
                    do {
                        child = fanouts.front();
                        if (substituted.fanout_size(child) >= ps.degree)
                            fanouts.pop();
                        else
                            break;
                    } while (true);
                }
            }

            return child;
        };

        ntk_topo.foreach_pi([&generate_fanout_tree](const auto& pi) { generate_fanout_tree(pi); });

        ntk_topo.foreach_gate(
            [this, &substituted, &old2new, &generate_fanout_tree, &get_fanout](const auto& n)
            {
                // gather children, but substitute fanouts where applicable
                std::vector<typename mockturtle::topo_view<NtkDest>::signal> children{};

                ntk_topo.foreach_fanin(n,
                                       [this, &old2new, &get_fanout, &children](const auto& f)
                                       {
                                           const auto fn = ntk_topo.get_node(f);

                                           auto child = old2new[fn];

                                           // constants do not need fanout trees
                                           if (!ntk_topo.is_constant(fn))
                                           {
                                               child = get_fanout(fn, child);
                                           }

                                           children.push_back(child);
                                       });

                // clone the node with new children according to its depth
                old2new[n] = substituted.clone_node(ntk_topo, n, children);

                // generate the fanout tree for n
                generate_fanout_tree(n);
            });

        // add primary outputs to finalize the network
        ntk_topo.foreach_po(
            [this, &old2new, &substituted, &get_fanout](const auto& po)
            {
                const auto po_node    = ntk_topo.get_node(po);
                auto       tgt_signal = old2new[po_node];
                auto       tgt_po     = get_fanout(po_node, tgt_signal);

                tgt_po = ntk_topo.is_complemented(po) ? substituted.create_not(tgt_signal) : tgt_po;

                substituted.create_po(tgt_po);
            });

        // restore signal names if applicable
        fiction::restore_names(ntk_topo, substituted, old2new);

        return substituted;
    }

  private:
    mockturtle::topo_view<NtkDest> ntk_topo;

    mockturtle::node_map<std::queue<typename NtkDest::signal>, mockturtle::topo_view<NtkDest>> available_fanouts;

    const fanout_substitution_params ps;
};

template <typename Ntk>
class is_fanout_substituted_impl
{
  public:
    is_fanout_substituted_impl(const Ntk& ntk, fanout_substitution_params p) : ntk{ntk}, ps{p} {}

    bool run()
    {
        ntk.foreach_gate(
            [this](const auto& n)
            {
                if (ntk.is_fanout(n))
                {
                    if (ntk.fanout_size(n) > ps.degree)
                        substituted = false;
                }
                else if (ntk.fanout_size(n) > ps.threshold)
                    substituted = false;

                return substituted;
            });

        return substituted;
    }

  private:
    const Ntk& ntk;

    const fanout_substitution_params ps;

    bool substituted = true;
};

}  // namespace detail

template <typename NtkDest, typename NtkSrc>
NtkDest fanout_substitution(const NtkSrc& ntk_src, fanout_substitution_params ps = {})
{
    static_assert(mockturtle::is_network_type_v<NtkSrc>, "NtkSrc is not a network type");
    static_assert(mockturtle::is_network_type_v<NtkDest>, "NtkDest is not a network type");

    static_assert(mockturtle::has_get_node_v<NtkSrc>, "NtkSrc does not implement the get_node function");
    static_assert(mockturtle::has_is_complemented_v<NtkSrc>, "NtkSrc does not implement the is_complemented function");
    static_assert(mockturtle::has_foreach_gate_v<NtkSrc>, "NtkSrc does not implement the foreach_gate function");
    static_assert(mockturtle::has_foreach_fanin_v<NtkSrc>, "NtkSrc does not implement the foreach_fanin function");
    static_assert(mockturtle::has_foreach_po_v<NtkSrc>, "NtkSrc does not implement the foreach_po function");

    static_assert(mockturtle::has_create_pi_v<NtkDest>, "NtkDest does not implement the create_pi function");
    static_assert(mockturtle::has_create_not_v<NtkDest>, "NtkDest does not implement the create_not function");
    static_assert(mockturtle::has_create_po_v<NtkDest>, "NtkDest does not implement the create_po function");
    static_assert(mockturtle::has_create_buf_v<NtkDest>, "NtkDest does not implement the create_buf function");
    static_assert(mockturtle::has_clone_node_v<NtkDest>, "NtkDest does not implement the clone_node function");

    detail::fanout_substitution_impl<NtkDest, NtkSrc> p{ntk_src, ps};

    auto result = p.run();

    return result;
}

template <typename Ntk>
bool is_fanout_substituted(const Ntk& ntk, fanout_substitution_params ps = {}) noexcept
{
    static_assert(mockturtle::is_network_type_v<Ntk>, "NtkSrc is not a network type");
    static_assert(mockturtle::has_foreach_gate_v<Ntk>, "Ntk does not implement the foreach_gate function");
    static_assert(mockturtle::has_foreach_fanin_v<Ntk>, "Ntk does not implement the foreach_fanin function");
    static_assert(mockturtle::has_foreach_po_v<Ntk>, "Ntk does not implement the foreach_po function");
    static_assert(mockturtle::has_get_node_v<Ntk>, "Ntk does not implement the get_node function");

    detail::is_fanout_substituted_impl<Ntk> p{ntk, ps};

    auto result = p.run();

    return result;
}

}  // namespace fiction

#endif  // FICTION_FANOUT_SUBSTITUTION_HPP
