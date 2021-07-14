//
// Created by marcel on 25.05.21.
//

#ifndef FICTION_NETWORK_BALANCING_HPP
#define FICTION_NETWORK_BALANCING_HPP

#include "../traits.hpp"
#include "network_conversion.hpp"

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/traits.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <mockturtle/views/depth_view.hpp>
#include <mockturtle/views/topo_view.hpp>

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

namespace fiction
{

struct network_balancing_params
{
    bool unify_outputs = false;
};

namespace detail
{

template <typename Ntk>
std::vector<uint32_t> get_po_levels(const Ntk& ntk)
{
    std::vector<uint32_t> po_levels{};
    ntk.foreach_po([&ntk, &po_levels](const auto& po) { po_levels.emplace_back(ntk.level(ntk.get_node(po))); });

    return po_levels;
}

template <typename NtkDest, typename NtkSrc>
class network_balancing_impl
{
  public:
    network_balancing_impl(const NtkSrc& src, const network_balancing_params p) :
            ntk{convert_network<NtkDest>(src)},
            ntk_topo{ntk},
            ntk_depth{ntk},
            ps{p}
    {}

    NtkDest run()
    {
        // insert num buffers after child in balance_ntk
        const auto insert_buf_chain = [](auto& balance_ntk, const auto& num, auto& child)
        {
            for (auto i = 0u; i < num; ++i) { child = balance_ntk.create_buf(child); }
        };

        // initialize a network copy
        auto init = mockturtle::initialize_copy_network<NtkDest>(ntk_topo);

        auto& balanced = init.first;
        auto& old2new  = init.second;

        ntk_topo.foreach_gate(
            [this, &balanced, &old2new, &insert_buf_chain](const auto& n)
            {
                // gather children, but substitute fanins by buf where applicable
                std::vector<typename mockturtle::topo_view<NtkDest>::signal> children{};

                ntk_topo.foreach_fanin(n,
                                       [this, &n, &balanced, &old2new, &children, &insert_buf_chain](const auto& f)
                                       {
                                           const auto fn = ntk_topo.get_node(f);

                                           auto child = old2new[fn];

                                           // do not balance constants
                                           if (!ntk_topo.is_constant(fn))
                                           {
                                               const auto level_diff = ntk_depth.level(n) - ntk_depth.level(fn) - 1;
                                               insert_buf_chain(balanced, level_diff, child);
                                           }

                                           children.push_back(child);
                                       });

                // clone the node with new children according to its depth
                old2new[n] = balanced.clone_node(ntk_topo, n, children);
            });

        // gather PO levels
        const auto po_levels    = get_po_levels(ntk_depth);
        const auto max_po_level = *std::max_element(po_levels.cbegin(), po_levels.cend());

        // add primary outputs to finalize the network
        ntk_topo.foreach_po(
            [this, &old2new, &balanced, &po_levels, &max_po_level, &insert_buf_chain](const auto& po, auto i)
            {
                const auto tgt_signal = old2new[ntk_topo.get_node(po)];

                auto tgt_po = ntk_topo.is_complemented(po) ? balanced.create_not(tgt_signal) : tgt_signal;

                if (ps.unify_outputs)
                {
                    insert_buf_chain(balanced, max_po_level - po_levels[i], tgt_po);
                }

                balanced.create_po(tgt_po);
            });

        // restore signal names if applicable
        fiction::restore_names(ntk_topo, balanced, old2new);

        return balanced;
    }

  private:
    NtkDest ntk;

    mockturtle::topo_view<NtkDest>  ntk_topo;
    mockturtle::depth_view<NtkDest> ntk_depth;

    const network_balancing_params ps;
};

template <typename Ntk>
class is_balanced_impl
{
  public:
    is_balanced_impl(const Ntk& src, network_balancing_params p) : ntk{src}, ntk_depth{src}, ps{p} {}

    bool run()
    {
        ntk.foreach_gate(
            [this](const auto& n)
            {
                ntk.foreach_fanin(n,
                                  [this, &n](const auto& f)
                                  {
                                      const auto fn = ntk.get_node(f);

                                      // skip constants
                                      if (!ntk.is_constant(fn))
                                      {
                                          if (const auto level_diff = ntk_depth.level(n) - ntk_depth.level(fn) - 1;
                                              level_diff > 0)
                                          {
                                              balanced = false;
                                          }
                                      }

                                      return balanced;
                                  });

                return balanced;
            });

        if (ps.unify_outputs)
        {
            const auto po_levels = get_po_levels(ntk_depth);
            // check whether POs with different depth levels exist
            if (std::adjacent_find(po_levels.begin(), po_levels.end(), std::not_equal_to<>()) != po_levels.end())
            {
                balanced = false;
            }
        }

        return balanced;
    }

  private:
    const Ntk& ntk;

    const mockturtle::depth_view<Ntk> ntk_depth;

    const network_balancing_params ps;

    bool balanced = true;
};

}  // namespace detail

template <typename NtkDest, typename NtkSrc>
NtkDest network_balancing(const NtkSrc& ntk_src, network_balancing_params ps = {})
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

    detail::network_balancing_impl<NtkDest, NtkSrc> p{ntk_src, ps};

    auto result = p.run();

    return result;
}

template <typename Ntk>
bool is_balanced(const Ntk& ntk, network_balancing_params ps = {}) noexcept
{
    static_assert(mockturtle::is_network_type_v<Ntk>, "NtkSrc is not a network type");
    static_assert(mockturtle::has_foreach_gate_v<Ntk>, "Ntk does not implement the foreach_gate function");
    static_assert(mockturtle::has_foreach_fanin_v<Ntk>, "Ntk does not implement the foreach_fanin function");
    static_assert(mockturtle::has_foreach_po_v<Ntk>, "Ntk does not implement the foreach_po function");
    static_assert(mockturtle::has_get_node_v<Ntk>, "Ntk does not implement the get_node function");

    detail::is_balanced_impl<Ntk> p{ntk, ps};

    auto result = p.run();

    return result;
}

}  // namespace fiction

#endif  // FICTION_NETWORK_BALANCING_HPP
