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
#include <mockturtle/utils/progress_bar.hpp>
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
            old2new{ntk_topo},
            available_fanouts{ntk_topo},
            ps{p}
    {}

    NtkDest run()
    {
        // initialize a network copy
        auto init = mockturtle::initialize_copy_network<NtkDest>(ntk_topo);

        substituted = init.first;
        old2new     = init.second;

        ntk_topo.foreach_pi([this](const auto& pi) { generate_fanout_tree(pi); });

#if (PROGRESS_BARS)
        // initialize a progress bar
        mockturtle::progress_bar bar{static_cast<uint32_t>(ntk_topo.num_gates()), "[i] fanout substitution: |{0}|"};
#endif

        ntk_topo.foreach_gate(
            [&, this](const auto& n, [[maybe_unused]] auto i)
            {
                // gather children, but substitute fanouts where applicable
                std::vector<mockturtle::signal<mockturtle::topo_view<NtkDest>>> children{};

                ntk_topo.foreach_fanin(n,
                                       [this, &children](const auto& f)
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

#if (PROGRESS_BARS)
                // update progress
                bar(i);
#endif
            });

        // add primary outputs to finalize the network
        ntk_topo.foreach_po(
            [this](const auto& po)
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

    NtkDest substituted;

    mockturtle::node_map<mockturtle::signal<NtkDest>, mockturtle::topo_view<NtkDest>> old2new;

    mockturtle::node_map<std::queue<mockturtle::signal<NtkDest>>, mockturtle::topo_view<NtkDest>> available_fanouts;

    const fanout_substitution_params ps;

    void generate_fanout_tree(const mockturtle::node<NtkSrc>& n)
    {
        // skip fanout tree generation if n is a proper fanout node
        if constexpr (has_is_fanout_v<NtkDest>)
        {
            if (ntk_topo.is_fanout(n) && ntk_topo.fanout_size(n) <= ps.degree)
                return;
        }

        auto num_fanouts = static_cast<uint32_t>(
            std::ceil(static_cast<double>(std::max(
                          static_cast<int32_t>(ntk_topo.fanout_size(n)) - static_cast<int32_t>(ps.threshold), 0)) /
                      static_cast<double>(std::max(static_cast<int32_t>(ps.degree) - 1, 1))));

        auto child = old2new[n];

        if (num_fanouts == 0)
            return;

        if (ps.strategy == fanout_substitution_params::substitution_strategy::DEPTH)
        {
            std::queue<mockturtle::signal<NtkDest>> q{};
            for (auto i = 0u; i < num_fanouts; ++i)
            {
                child = substituted.create_buf(child);
                q.push(child);
            }
            available_fanouts[n] = q;
        }
        else if (ps.strategy == fanout_substitution_params::substitution_strategy::BREADTH)
        {
            std::queue<mockturtle::signal<NtkDest>> q{{child}};

            for (auto f = 0ul; f < num_fanouts; ++f)
            {
                child = q.front();
                q.pop();
                child = substituted.create_buf(child);
                for (auto i = 0u; i < ps.degree; ++i) q.push(child);
            }
            available_fanouts[n] = q;
        }
    }

    mockturtle::signal<NtkDest> get_fanout(const mockturtle::node<NtkSrc>& n, mockturtle::signal<NtkDest>& child)
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
    }
};

template <typename Ntk>
class is_fanout_substituted_impl
{
  public:
    is_fanout_substituted_impl(const Ntk& src, fanout_substitution_params p) : ntk{src}, ps{p} {}

    bool run()
    {
        ntk.foreach_node(
            [this](const auto& n)
            {
                // skip constants
                if (ntk.is_constant(n))
                    return substituted;

                // check degree of fanout nodes
                if constexpr (fiction::has_is_fanout_v<Ntk>)
                {
                    if (ntk.is_fanout(n))
                    {
                        if (ntk.fanout_size(n) > ps.degree)
                        {
                            substituted = false;
                        }

                        return substituted;
                    }
                }
                // check threshold of non-fanout nodes
                if (ntk.fanout_size(n) > ps.threshold)
                {
                    substituted = false;
                }

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

    static_assert(mockturtle::has_is_constant_v<NtkDest>, "NtkSrc does not implement the is_constant function");
    static_assert(mockturtle::has_create_pi_v<NtkDest>, "NtkDest does not implement the create_pi function");
    static_assert(mockturtle::has_create_not_v<NtkDest>, "NtkDest does not implement the create_not function");
    static_assert(mockturtle::has_create_po_v<NtkDest>, "NtkDest does not implement the create_po function");
    static_assert(mockturtle::has_create_buf_v<NtkDest>, "NtkDest does not implement the create_buf function");
    static_assert(mockturtle::has_clone_node_v<NtkDest>, "NtkDest does not implement the clone_node function");
    static_assert(mockturtle::has_fanout_size_v<NtkDest>, "NtkDest does not implement the fanout_size function");
    static_assert(mockturtle::has_foreach_gate_v<NtkDest>, "NtkDest does not implement the foreach_gate function");
    static_assert(mockturtle::has_foreach_fanin_v<NtkDest>, "NtkDest does not implement the foreach_fanin function");
    static_assert(mockturtle::has_foreach_po_v<NtkDest>, "NtkDest does not implement the foreach_po function");

    detail::fanout_substitution_impl<NtkDest, NtkSrc> p{ntk_src, ps};

    auto result = p.run();

    return result;
}

template <typename Ntk>
bool is_fanout_substituted(const Ntk& ntk, fanout_substitution_params ps = {}) noexcept
{
    static_assert(mockturtle::is_network_type_v<Ntk>, "NtkSrc is not a network type");
    static_assert(mockturtle::has_foreach_gate_v<Ntk>, "Ntk does not implement the foreach_gate function");
    static_assert(mockturtle::has_fanout_size_v<Ntk>, "Ntk does not implement the fanout_size function");

    detail::is_fanout_substituted_impl<Ntk> p{ntk, ps};

    auto result = p.run();

    return result;
}

}  // namespace fiction

#endif  // FICTION_FANOUT_SUBSTITUTION_HPP
