//
// Created by marcel on 28.06.21.
//

#ifndef FICTION_APPLY_GATE_LIBRARY_HPP
#define FICTION_APPLY_GATE_LIBRARY_HPP

#include "../technology/qca_one_library.hpp"
#include "../traits.hpp"

#include <mockturtle/traits.hpp>
#include <mockturtle/utils/progress_bar.hpp>

#include <type_traits>

namespace fiction
{

namespace detail
{

template <typename CellLyt, typename GateLibrary, typename GateLyt>
class apply_gate_library_impl
{
  public:
    explicit apply_gate_library_impl(const GateLyt& lyt) :
            gate_lyt{lyt},
            cell_lyt{typename CellLyt::aspect_ratio{((gate_lyt.x() + 1) * GateLibrary::gate_x_size()) - 1,
                                                    ((gate_lyt.y() + 1) * GateLibrary::gate_y_size()) - 1,
                                                    gate_lyt.z()},
                     gate_lyt.get_clocking_scheme(), "", GateLibrary::gate_x_size(), GateLibrary::gate_y_size()}
    {}

    CellLyt run()
    {
#if (PROGRESS_BARS)
        // initialize a progress bar
        mockturtle::progress_bar bar{static_cast<uint32_t>(gate_lyt.size()), "[i] applying gate library: |{0}|"};
#endif
        gate_lyt.foreach_node(
            [&, this](const auto& n, [[maybe_unused]] auto i)
            {
                if (!gate_lyt.is_constant(n))
                {
                    const auto t = gate_lyt.get_tile(n);
                    assign_gate({t.x * GateLibrary::gate_x_size(), t.y * GateLibrary::gate_y_size(), t.z},
                                GateLibrary::set_up_gate(gate_lyt, t));
                }
#if (PROGRESS_BARS)
                // update progress
                bar(i);
#endif
            });

        if constexpr (std::is_same_v<GateLibrary, qca_one_library>)
        {
            GateLibrary::assign_via_cells(cell_lyt);
        }

        if constexpr (mockturtle::has_get_network_name_v<GateLyt> && fiction::has_set_layout_name_v<CellLyt>)
        {
            cell_lyt.set_layout_name(gate_lyt.get_network_name());
        }

        return cell_lyt;
    }

  private:
    GateLyt gate_lyt;
    CellLyt cell_lyt;

    void assign_gate(const typename CellLyt::cell& c, const typename GateLibrary::fcn_gate& g)
    {
        auto start_x = c.x;
        auto start_y = c.y;
        auto layer   = c.z;

        for (auto y = 0ul; y < g.size(); ++y)
        {
            for (auto x = 0ul; x < g[y].size(); ++x)
            {
                const typename CellLyt::cell      pos{start_x + x, start_y + y, layer};
                const typename CellLyt::cell_type type{g[y][x]};

                cell_lyt.assign_cell_type(pos, type);
            }
        }
    }
};

}  // namespace detail

/**
 * May pass through, and thereby throw, an 'unsupported_gate_type_exception' or an
 * 'unsupported_gate_orientation_exception'.
 *
 * @tparam CellLyt
 * @tparam GateLibrary
 * @tparam GateLyt
 * @param lyt
 * @return
 */
template <typename CellLyt, typename GateLibrary, typename GateLyt>
CellLyt apply_gate_library(const GateLyt& lyt)
{
    static_assert(mockturtle::is_network_type_v<GateLyt>, "GateLyt is not a network type");
    static_assert(mockturtle::has_is_constant_v<GateLyt>, "GateLyt does not implement the is_constant function");
    static_assert(mockturtle::has_foreach_node_v<GateLyt>, "GateLyt does not implement the foreach_node function");

    static_assert(std::is_same_v<typename CellLyt::technology, typename GateLibrary::technology>,
                  "CellLyt and GateLibrary must implement the same technology");

    detail::apply_gate_library_impl<CellLyt, GateLibrary, GateLyt> p{lyt};

    auto result = p.run();

    return result;
}

}  // namespace fiction

#endif  // FICTION_APPLY_GATE_LIBRARY_HPP
