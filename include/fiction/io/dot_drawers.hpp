//
// Created by marcel on 26.05.21.
//

#ifndef FICTION_DOT_DRAWERS_HPP
#define FICTION_DOT_DRAWERS_HPP

#include "../traits.hpp"
#include "utils/version_info.hpp"

#include <fmt/format.h>
#include <kitty/print.hpp>
#include <mockturtle/io/write_dot.hpp>
#include <mockturtle/traits.hpp>

#include <algorithm>
#include <array>

namespace fiction
{

template <typename Ntk, bool DrawIndexes = false, bool DrawHexTT = false>
class topology_dot_drawer : public mockturtle::gate_dot_drawer<Ntk>
{
  public:
    std::string node_label(const Ntk& ntk, const mockturtle::node<Ntk>& n) const override
    {
        if constexpr (DrawIndexes)
        {
            if (!ntk.is_constant(n) && !ntk.is_pi(n))
                return fmt::format("{}: {}", ntk.node_to_index(n), node_label_callback(ntk, n));
        }

        return node_label_callback(ntk, n);
    }

    std::string node_fillcolor(const Ntk& ntk, const mockturtle::node<Ntk>& n) const override
    {
        if (ntk.is_pi(n))
        {
            return "snow2";
        }
        if constexpr (has_is_fanout_v<Ntk>)
        {
            if (ntk.is_fanout(n))
            {
                return "navajowhite2";
            }
        }
        if constexpr (has_is_buf_v<Ntk>)
        {
            if (ntk.is_buf(n))
            {
                return "palegoldenrod";
            }
        }
        if constexpr (has_is_inv_v<Ntk>)
        {
            if (ntk.is_inv(n))
            {
                return "paleturquoise";
            }
        }
        if constexpr (has_is_nand_v<Ntk>)
        {
            if (ntk.is_nand(n))
            {
                return "lightcoral";
            }
        }
        if constexpr (has_is_nor_v<Ntk>)
        {
            if (ntk.is_nor(n))
            {
                return "palegreen2";
            }
        }
        if constexpr (has_is_dot_v<Ntk>)
        {
            if (ntk.is_dot(n))
            {
                return "thistle";
            }
        }

        return mockturtle::gate_dot_drawer<Ntk>::node_fillcolor(ntk, n);
    }

  private:
    std::string node_label_callback(const Ntk& ntk, const mockturtle::node<Ntk>& n) const
    {
        if constexpr (has_is_fanout_v<Ntk>)
        {
            if (ntk.is_fanout(n))
            {
                return "F";
            }
        }
        if constexpr (has_is_buf_v<Ntk>)
        {
            if (ntk.is_buf(n))
            {
                return "BUF";
            }
        }
        if constexpr (has_is_inv_v<Ntk>)
        {
            if (ntk.is_inv(n))
            {
                return "INV";
            }
        }
        if constexpr (has_is_nand_v<Ntk>)
        {
            if (ntk.is_nand(n))
            {
                return "NAND";
            }
        }
        if constexpr (has_is_nor_v<Ntk>)
        {
            if (ntk.is_nor(n))
            {
                return "NOR";
            }
        }
        if constexpr (has_is_dot_v<Ntk>)
        {
            if (ntk.is_dot(n))
            {
                return "DOT";
            }
        }

        const auto label = mockturtle::gate_dot_drawer<Ntk>::node_label(ntk, n);

        // check if base drawer could not identify the gate either
        if (is_node_number(label) && !ntk.is_pi(n) && !ntk.is_constant(n))
        {
            // try to fetch the node's truth table
            if constexpr (mockturtle::has_node_function_v<Ntk>)
            {
                const auto node_tt = ntk.node_function(n);

                // print truth tables with up to 4 variables exclusively
                if (node_tt.num_vars() <= 4)
                {
                    if constexpr (DrawHexTT)
                    {
                        return kitty::to_hex(node_tt);
                    }
                    else
                    {
                        return kitty::to_binary(node_tt);
                    }
                }
            }
        }

        return label;
    }

    [[nodiscard]] bool is_node_number(const std::string& s) const noexcept
    {
        return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
    }
};

template <typename Ntk, bool DrawIndexes = false>
class color_view_drawer : public mockturtle::default_dot_drawer<Ntk>
{
  public:
    std::string node_label(const Ntk& ntk, const mockturtle::node<Ntk>& n) const override
    {
        if constexpr (DrawIndexes)
        {
            return fmt::format("n: {}, c: {}", ntk.node_to_index(n), ntk.color(n));
        }
        else
        {
            return fmt::format("c: {}", ntk.color(n));
        }
    }

    std::string node_fillcolor(const Ntk& ntk, const mockturtle::node<Ntk>& n) const override
    {
        auto c = ntk.color(n);
        return c >= colors.size() ? "black" : colors[ntk.color(n)];
    }

  private:
    const std::array<std::string, 8> colors{{"ghostwhite", "deepskyblue1", "darkseagreen2", "crimson", "goldenrod1",
                                             "darkorchid2", "chocolate1", "gray28"}};
};

template <typename Lyt, bool DrawIndexes = false>
class gate_layout_tile_drawer : public topology_dot_drawer<Lyt, DrawIndexes>
{
  public:
    std::string tile_id(const typename Lyt::tile& t) const noexcept
    {
        return fmt::format("x{}y{}", t.x, t.y);
    }

    [[nodiscard]] std::vector<std::string> additional_node_attributes() const noexcept
    {
        if constexpr (DrawIndexes)
            return {"shape=square", "fixedsize=true", "width=1"};
        else
            return {"shape=square", "fixedsize=true", "width=0.5"};
    }

    std::string tile_label(const Lyt& lyt, const typename Lyt::tile& t) const noexcept
    {
        if (lyt.is_empty_tile(t))
            return "";

        if (lyt.is_pi_tile(t))
            return "PI";

        if (lyt.is_po_tile(t))
            return "PO";

        if constexpr (has_is_buf_v<Lyt>)
        {
            // crossing case
            if (lyt.is_buf(lyt.get_node(t)) && lyt.is_buf(lyt.get_node(lyt.above(t))))
            {
                return "+";
            }
        }

        return topology_dot_drawer<Lyt, DrawIndexes>::node_label(lyt, lyt.get_node(t));
    }

    std::string tile_fillcolor(const Lyt& lyt, const typename Lyt::tile& t) const noexcept
    {
        if (lyt.is_empty_tile(t))
            return "white";

        if (lyt.is_pi_tile(t) || lyt.is_po_tile(t))
            return "snow2";

        return topology_dot_drawer<Lyt, DrawIndexes>::node_fillcolor(lyt, lyt.get_node(t));
    }

    std::vector<std::vector<std::string>> rows(const Lyt& lyt) const noexcept
    {
        std::vector<std::vector<std::string>> rows{};

        for (auto y = 0ul; y <= lyt.y(); ++y)
        {
            std::vector<std::string> row{};
            for (auto x = 0ul; x <= lyt.x(); ++x) { row.emplace_back(tile_id({x, y})); }
            rows.push_back(row);
        }

        return rows;
    }

    std::vector<std::vector<std::string>> columns(const Lyt& lyt) const noexcept
    {
        std::vector<std::vector<std::string>> columns{};

        for (auto x = 0ul; x <= lyt.x(); ++x)
        {
            std::vector<std::string> col{};
            for (auto y = 0ul; y <= lyt.y(); ++y) { col.emplace_back(tile_id({x, y})); }
            columns.push_back(col);
        }

        return columns;
    }
};

/*! \brief Writes layout in DOT format into output stream
 *
 * An overloaded variant exists that writes the layout into a file.
 *
 * **Required network functions:**
 * - is_pi
 * - foreach_node
 * - foreach_fanin
 *
 * \param lyt Layout
 * \param os Output stream
 */
template <class Lyt, class Drawer = gate_layout_tile_drawer<Lyt>>
void write_dot_layout(const Lyt& lyt, std::ostream& os, const Drawer& drawer = {})
{
    static_assert(mockturtle::is_network_type_v<Lyt>, "Ntk is not a network type");
    static_assert(mockturtle::has_is_pi_v<Lyt>, "Ntk does not implement the is_pi method");
    static_assert(mockturtle::has_foreach_node_v<Lyt>, "Ntk does not implement the foreach_node method");
    static_assert(mockturtle::has_foreach_fanin_v<Lyt>, "Ntk does not implement the foreach_fanin method");

    std::stringstream nodes{}, edges{}, grid{};

    nodes << fmt::format("node [style=filled {}];\n", fmt::join(drawer.additional_node_attributes(), " "));

    // draw tiles
    lyt.foreach_ground_tile(
        [&lyt, &drawer, &nodes](const auto& t)
        {
            nodes << fmt::format("{} [label=\"{}\", fillcolor={}];\n", drawer.tile_id(t), drawer.tile_label(lyt, t),
                                 drawer.tile_fillcolor(lyt, t));
        });

    nodes << "edge [constraint=false];\n";

    // draw connections
    lyt.foreach_node(
        [&lyt, &drawer, &edges](const auto& n)
        {
            lyt.foreach_fanin(n,
                              [&lyt, &drawer, &edges, &n](const auto& f)
                              {
                                  edges << fmt::format("{} -> {} [style={}];\n",
                                                       drawer.tile_id(static_cast<typename Lyt::tile>(f)),
                                                       drawer.tile_id(lyt.get_tile(n)), drawer.signal_style(lyt, f));
                              });
        });

    grid << "edge [constraint=true, style=invis];\n";

    // enforce grid structure
    for (const auto& col : drawer.columns(lyt)) { grid << fmt::format("{};\n", fmt::join(col, " -> ")); }
    for (const auto& row : drawer.rows(lyt)) { grid << fmt::format("rank = same {{ {} }};\n", fmt::join(row, " -> ")); }

    // draw layout
    os << fmt::format("digraph layout {{  // Generated by {} ({})", FICTION_VERSION, FICTION_REPO) << "\n"
       << "rankdir=TB;\n"
       << "splines=ortho;\n"
       << "concentrate=true;\n"
       << nodes.str() << edges.str() << grid.str() << "}\n";
}

/*! \brief Writes layout in DOT format into a file
 *
 * **Required network functions:**
 * - is_pi
 * - foreach_node
 * - foreach_fanin
 *
 * \param lyt Layout
 * \param filename Filename
 */
template <class Lyt, class Drawer = gate_layout_tile_drawer<Lyt>>
void write_dot_layout(const Lyt& lyt, const std::string& filename, const Drawer& drawer = {})
{
    std::ofstream os{filename.c_str(), std::ofstream::out};
    write_dot(lyt, os, drawer);
    os.close();
}

}  // namespace fiction

#endif  // FICTION_DOT_DRAWERS_HPP
