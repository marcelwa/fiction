//
// Created by marcel on 19.05.21.
//

#ifndef FICTION_PRINT_LAYOUT_HPP
#define FICTION_PRINT_LAYOUT_HPP

#include "../types.hpp"

#include <fmt/color.h>
#include <fmt/format.h>

#include <array>
#include <string>
#include <vector>

namespace fiction
{

namespace detail
{

// Escape color sequence for input colors (green).
static const auto INP_COLOR = fmt::fg(fmt::color::green);
// Escape color sequence for output colors (red).
static const auto OUT_COLOR = fmt::fg(fmt::color::red);
//// Escape color sequence for latch colors (yellow on black).
// static const auto LATCH_COLOR = fmt::fg(fmt::color::yellow) | fmt::bg(fmt::color::black);
// Escape color sequences for clock background colors (white to dark grey).
static const std::array<fmt::text_style, 4> CLOCK_COLOR{{fmt::fg(fmt::color::black) | fmt::bg(fmt::color::white),
                                                         fmt::fg(fmt::color::black) | fmt::bg(fmt::color::light_gray),
                                                         fmt::fg(fmt::color::white) | fmt::bg(fmt::color::gray),
                                                         fmt::fg(fmt::color::white) | fmt::bg(fmt::color::dark_gray)}};

}  // namespace detail

template <typename Lyt>
void print_gate_level_layout(std::ostream& os, const Lyt& layout, const bool io_color = true,
                             const bool clk_color = false) noexcept
{
    // empty layout
    if (layout.num_gates() == 0ul && layout.num_wires() == 0ul)
    {
        os << fmt::format("[i] empty layout");
        return;
    }

    const auto num_cols = layout.x() + 1;
    const auto num_rows = layout.y() + 1;

    // cache operations and directions in a 2d-matrix-like object
    using s_matrix = std::vector<std::vector<std::string>>;
    s_matrix reprs(num_rows, std::vector<std::string>(num_cols));
    s_matrix x_dirs(num_rows, std::vector<std::string>(num_cols + 1u, " "));
    s_matrix y_dirs(num_rows + 1u, std::vector<std::string>(num_cols, " "));

    const auto gate_repr = [&layout](const auto& t)
    {
        if (layout.is_empty_tile(t))
            return "▢";

        if (const auto n = layout.get_node(t); layout.is_and(n))
            return "&";
        else if (layout.is_or(n))
            return "|";
        else if (layout.is_inv(n))
            return "¬";
        else if (layout.is_maj(n))
            return "M";
        else if (layout.is_xor(n))
            return "X";
        else if (layout.is_fanout(n))
            return "Y";
        else if (layout.is_wire(n))
        {
            // second-layer wire indicates a crossing
            if (auto an = layout.get_node(layout.above(t)); an != n && layout.is_wire(an))
                return "+";
            else if (layout.is_pi(n))
                return "I";
            else if (layout.is_po(n))
                return "O";

            return "=";
        }

        return "?";
    };

    for (auto i = 0ull; i < num_rows; ++i)
    {
        for (auto j = 0ull; j < num_cols; ++j)
        {
            auto t1     = typename Lyt::tile{j, i};
            auto t2     = layout.above(t1);
            reprs[i][j] = gate_repr(t1);

            const auto east_west_connections = [&layout, &x_dirs, &t1, &t2, i, j](const auto s)
            {
                const auto ft = static_cast<typename Lyt::tile>(s);
                if (layout.is_east_of(t1, ft) || layout.is_east_of(t2, ft))
                    x_dirs[i][j] = "→";
                if (layout.is_west_of(t1, ft) || layout.is_west_of(t2, ft))
                    x_dirs[i][j - 1] = "←";
            };

            const auto north_south_connections = [&layout, &y_dirs, &t1, &t2, i, j](const auto s)
            {
                const auto ft = static_cast<typename Lyt::tile>(s);
                if (layout.is_north_of(t1, ft) || layout.is_north_of(t2, ft))
                    y_dirs[i][j] = "↑";
                if (layout.is_south_of(t1, ft) || layout.is_south_of(t2, ft))
                    y_dirs[i + 1u][j] = "↓";
            };

            layout.foreach_fanout(layout.get_node(t1), east_west_connections);
            layout.foreach_fanout(layout.get_node(t2), east_west_connections);
            layout.foreach_fanout(layout.get_node(t1), north_south_connections);
            layout.foreach_fanout(layout.get_node(t2), north_south_connections);
        }
    }

    // actual printing
    auto r_ctr = 0u, c_ctr = 0u;
    for (const auto& row : reprs)
    {
        for (auto& d : y_dirs[r_ctr]) os << d << " ";
        os << '\n';

        for (const auto& gate : row)
        {
            const auto t = typename Lyt::tile{c_ctr, r_ctr};

            fmt::text_style color{};

            if (clk_color)
                color = color | detail::CLOCK_COLOR[layout.get_clock_number(t)];
            //            if (io_color && (get_latch(t) > 0u))
            //                os << detail::LATCH_COLOR;
            if (io_color && layout.is_pi_tile(t))
                color = color | detail::INP_COLOR;
            else if (io_color && layout.is_po_tile(t))
                color = color | detail::OUT_COLOR;

            os << fmt::format(color, gate);

            os << x_dirs[r_ctr][c_ctr];
            ++c_ctr;
        }
        c_ctr = 0u;
        os << '\n';

        ++r_ctr;
    }
    // flush stream
    os << std::endl;
}

template <typename Lyt>
void print_cell_level_layout(std::ostream& os, const Lyt& layout, const bool io_color = true,
                             const bool clk_color = false) noexcept
{
    // empty layout
    if (layout.num_cells() == 0ul)
    {
        os << fmt::format("[i] empty layout");
        return;
    }

    for (auto y_pos = 0ull; y_pos <= layout.y(); ++y_pos)
    {
        for (auto x_pos = 0ull; x_pos <= layout.x(); ++x_pos)
        {
            typename Lyt::cell c{x_pos, y_pos};

            fmt::text_style color{};

            if (clk_color)
                color = color | detail::CLOCK_COLOR[layout.get_clock_number(c)];

            // crossing case
            if (const auto ac = layout.above(c); (c != ac) && !layout.is_empty_cell(ac))
            {
                os << fmt::format(color, "x");
            }
            else
            {
                const auto ct = layout.get_cell_type(c);

                //                if (io_color && get_latch(c) > 0u)
                //                    os << detail::LATCH_COLOR;
                if (io_color && Lyt::technology::is_input_cell(ct))
                    color = color | detail::INP_COLOR;
                else if (io_color && Lyt::technology::is_output_cell(ct))
                    color = color | detail::OUT_COLOR;

                os << fmt::format(color,
                                  (Lyt::technology::is_normal_cell(ct) ? "▢" : std::string(1u, static_cast<char>(ct))));
            }
        }
        os << '\n';
    }
    // flush stream
    os << std::endl;
}

}  // namespace fiction

#endif  // FICTION_PRINT_LAYOUT_HPP
