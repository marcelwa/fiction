//
// Created by marcel on 26.07.18.
//

#include "fcn_gate_library.h"


fcn_gate_library::fcn_gate_library(std::string&& name, fcn_gate_layout_ptr&& fgl,
                                   const fcn::technology tech, const fcn::tile_size size) noexcept
        :
        name{std::move(name)},
        layout{std::move(fgl)},
        technology{tech},
        tile_size{size},
        x_size{static_cast<gate_size_t>(size)},
        y_size{static_cast<gate_size_t>(size)},
        empty_gate{fcn_gate(y_size, std::vector<fcn::cell_type>(x_size, fcn::EMPTY_CELL))},
        p_router{std::make_shared<port_router>(layout, tech, size)}
{}

fcn_gate fcn_gate_library::rotate_90(const fcn_gate& g) const noexcept
{
    return reverse_columns(transpose(g));
}

fcn_gate fcn_gate_library::rotate_180(const fcn_gate& g) const noexcept
{
    return reverse_columns(reverse_rows(g));
}

fcn_gate fcn_gate_library::rotate_270(const fcn_gate& g) const noexcept
{
    return transpose(reverse_columns(g));
}

fcn_gate fcn_gate_library::merge(const std::vector<fcn_gate>& gates) const noexcept
{
    auto merged = empty_gate;

    for (auto&& i : iter::range(x_size))
    {
        for (auto&& j : iter::range(y_size))
        {
            for (auto& g : gates)
            {
                if (g[i][j] != fcn::EMPTY_CELL)
                    merged[i][j] = g[i][j];
            }
        }
    }

    return merged;
}

std::string fcn_gate_library::get_name() const noexcept
{
    return name;
}

fcn_gate_layout_ptr fcn_gate_library::get_layout() const noexcept
{
    return layout;
}

fcn::technology fcn_gate_library::get_technology() const noexcept
{
    return technology;
}

fcn::tile_size fcn_gate_library::get_tile_size() const noexcept
{
    return tile_size;
}

fcn_gate_library::gate_size_t fcn_gate_library::gate_x_size() const noexcept
{
    return x_size;
}

fcn_gate_library::gate_size_t fcn_gate_library::gate_y_size() const noexcept
{
    return y_size;
}

port_router_ptr fcn_gate_library::get_port_router() const noexcept
{
    return p_router;
}

fcn_gate fcn_gate_library::transpose(const fcn_gate& g) const noexcept
{
    auto trans = empty_gate;

    for (auto&& i : iter::range(x_size))
    {
        for (auto&& j : iter::range(y_size))
            trans[j][i] = g[i][j];
    }

    return trans;
}

fcn_gate fcn_gate_library::reverse_columns(const fcn_gate& g) const noexcept
{
    fcn_gate rev_cols = g;

    std::for_each(std::begin(rev_cols), std::end(rev_cols),
                  [](auto& i) {std::reverse(std::begin(i), std::end(i));});
    return rev_cols;
}

fcn_gate fcn_gate_library::reverse_rows(const fcn_gate& g) const noexcept
{
    fcn_gate rev_rows = g;

    std::reverse(std::begin(rev_rows), std::end(rev_rows));

    return rev_rows;
}

fcn_gate fcn_gate_library::mark_cell(const fcn_gate& g, const port& p, const fcn::cell_mark mark) const noexcept
{
    auto marked_gate = g;

    marked_gate[p.y][p.x] = static_cast<fcn::cell_type>(mark);

    return marked_gate;
}

fcn_gate_library::port fcn_gate_library::opposite(const port& p) const
{
    using port_port_map = std::unordered_map<port, port, boost::hash<port>>;
    static const port_port_map pp_map =
    {
        // QCA 5x5 ports
        {port(2, 0), port(2, 4)},
        {port(4, 2), port(0, 2)},
        {port(2, 4), port(2, 0)},
        {port(0, 2), port(4, 2)},
        // iNML 4x4 ports
        {port(0, 0), port(3, 0)},
        {port(0, 1), port(3, 1)},
        {port(0, 2), port(3, 2)},
        {port(0, 3), port(3, 3)},
        {port(3, 0), port(0, 0)},
        {port(3, 1), port(0, 1)},
        {port(3, 2), port(0, 2)},
        {port(3, 3), port(0, 3)},
    };

    return pp_map.at(p);
}

std::ostream& operator<<(std::ostream& os, const fcn_gate& g)
{
    for (const auto& x : g)
    {
        for (const auto& y : x)
            os << y;
        os << std::endl;
    }
    os << std::endl;

    return os;
}
