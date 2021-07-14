//
// Created by marcel on 25.07.18.
//

#include "fcn_layout.h"


fcn_layout::fcn_layout(const fcn_dimension_xyz& lengths, fcn_clocking_scheme&& clocking, offset o) noexcept
        :
        grid_graph(lengths),
        clocking(std::move(clocking)),
        shift(o)
{
    if (lengths[X] < 2 || lengths[Y] < 2 || lengths[Z] < 2)
        std::cerr << "[w] due to a bug in the BGL, every dimension should have a minimum size of 2 to prevent SEGFAULTs." << std::endl;
}

fcn_layout::fcn_layout(const fcn_dimension_xy& lengths, fcn_clocking_scheme&& clocking, offset o) noexcept
        :
        grid_graph(fcn_dimension_xyz{lengths[X], lengths[Y], 2}),
        clocking(std::move(clocking)),
        shift(o)
{
    if (lengths[X] < 2 || lengths[Y] < 2)
        std::cerr << "[w] due to a bug in the BGL, every dimension should have a minimum size of 2 to prevent SEGFAULTs." << std::endl;
}

fcn_layout::fcn_layout(fcn_clocking_scheme&& clocking, offset o) noexcept
        :
        grid_graph(fcn_dimension_xyz{2, 2, 2}),
        clocking(std::move(clocking)),
        shift(o)
{}

fcn_layout::fcn_layout() noexcept
        :
        grid_graph(fcn_dimension_xyz{2, 2, 2}),
        clocking(open_4_clocking),
        shift(offset::NONE)
{}

fcn_layout::~fcn_layout() = default;

bool fcn_layout::is_vertically_shifted() const noexcept
{
    return shift == offset::VERTICAL;
}

void fcn_layout::resize(const fcn_dimension_xyz& lengths) noexcept
{
    if (lengths[X] < 2 || lengths[Y] < 2 || lengths[Z] < 2)
        std::cerr << "[w] due to a bug in the BGL, every dimension should have a minimum size of 2 to prevent SEGFAULTs." << std::endl;

    resize_grid(lengths);
}

void fcn_layout::resize(const fcn_dimension_xy& lengths) noexcept
{
    resize(fcn_dimension_xyz{lengths[X], lengths[Y], 2});
}

bool fcn_layout::is_even_row(const face& f) const noexcept
{
    return f[Y] % 2 == 0;
}

bool fcn_layout::is_odd_row(const face& f) const noexcept
{
    return f[Y] % 2 == 1;
}

bool fcn_layout::is_even_column(const face& f) const noexcept
{
    return f[X] % 2 == 0;
}

bool fcn_layout::is_odd_column(const face& f) const noexcept
{
    return f[X] % 2 == 1;
}

fcn_layout::face fcn_layout::north(const face& f) const noexcept
{
    return previous(f, Y);
}

fcn_layout::face fcn_layout::east(const face& f) const noexcept
{
    return next(f, X);
}

fcn_layout::face fcn_layout::south(const face& f) const noexcept
{
    return next(f, Y);
}

fcn_layout::face fcn_layout::west(const face& f) const noexcept
{
    return previous(f, X);
}

fcn_layout::face fcn_layout::above(const face& f) const noexcept
{
    return next(f, Z);
}

fcn_layout::face fcn_layout::below(const face& f) const noexcept
{
    return previous(f, Z);
}

std::optional<fcn_layout::face> fcn_layout::north_east(const face& f) const noexcept
{
    auto e = east(f);
    if (e == f)
        return std::nullopt;

    if (is_vertically_shifted() && is_odd_column(f))
        return e;

    auto ne = north(e);
    if (ne == e)
        return std::nullopt;

    return ne;
}

std::optional<fcn_layout::face> fcn_layout::north_west(const face& f) const noexcept
{
    auto w = west(f);
    if (w == f)
        return std::nullopt;

    if (is_vertically_shifted() && is_odd_column(f))
        return w;

    auto nw = north(w);
    if (nw == w)
        return std::nullopt;

    return nw;
}

std::optional<fcn_layout::face> fcn_layout::south_east(const face& f) const noexcept
{
    auto e = east(f);
    if (e == f)
        return std::nullopt;

    if (is_vertically_shifted() && is_even_column(f))
        return e;

    auto se = south(e);
    if (se == e)
        return std::nullopt;

    return se;
}

std::optional<fcn_layout::face> fcn_layout::south_west(const face& f) const noexcept
{
    auto w = west(f);
    if (w == f)
        return std::nullopt;

    if (is_vertically_shifted() && is_even_column(f))
        return w;

    auto sw = south(w);
    if (sw == w)
        return std::nullopt;

    return sw;
}

fcn_layout::ground fcn_layout::get_ground(const face& f) const noexcept
{
    return ground{f[X], f[Y]};
}

coord_t fcn_layout::x() const noexcept
{
    return length(X);
}

coord_t fcn_layout::y() const noexcept
{
    return length(Y);
}

coord_t fcn_layout::z() const noexcept
{
    return length(Z);
}

fcn_layout::face fcn_layout::random_face() const noexcept
{
    std::mt19937 rgen(std::random_device{}());
    std::uniform_int_distribution<coord_t> dist(0, get_vertex_count() - 1); // distribution in range [0, |V|]

    return get_by_index(dist(rgen));
}

fcn_layout::face fcn_layout::random_face(const layer_t n) const noexcept
{
    const auto f = random_face();

    return face{f[X], f[Y], n};
}

coord_t fcn_layout::area() const noexcept
{
    return x() * y();
}

fcn_clock::number fcn_layout::num_clocks() const noexcept
{
    return clocking.num_clocks;
}

bool fcn_layout::is_regularly_clocked() const noexcept
{
    return clocking.regular;
}

bool fcn_layout::is_clocking(std::string&& name) const noexcept
{
    return clocking.name == name;
}

void fcn_layout::assign_clocking(const face& f, const fcn_clock::number c) noexcept
{
    if (!clocking.regular && c <= clocking.num_clocks)
        c_map[get_ground(f)] = c;
}

void fcn_layout::assign_clocking(const face_index f, const fcn_clock::number c) noexcept
{
    assign_clocking(get_vertex(f), c);
}

bool fcn_layout::is_pi(const face& f) const noexcept
{
    return pi_set.count(f) > 0u;
}

bool fcn_layout::is_po(const face& f) const noexcept
{
    return po_set.count(f) > 0u;
}

void fcn_layout::assign_latch(const face& f, const latch_delay l) noexcept
{
    if (l == 0u)
        l_map.erase(get_ground(f));
    else
        l_map[get_ground(f)] = l;
}

fcn_layout::latch_delay fcn_layout::get_latch(const face& f) const noexcept
{
    if (auto it = l_map.find(get_ground(f)); it != l_map.end())
    {
        return it->second;
    }
    else
    {
        return 0u;
    }
}

std::vector<std::string> fcn_layout::latch_str_reprs() const noexcept
{
    std::vector<std::string> reprs{};
    for (const auto& [t, l] : l_map)
    {
        std::stringstream ss{};
        ss << "l@" << t << "=" << l;
        reprs.push_back(ss.str());
    }

    return reprs;
}

bool fcn_layout::is_northern_border(const face& f, const std::optional<bounding_box>& bb) const noexcept
{
    if (bb)  // bounding box given, do calculations based on that
    {
        return f[Y] == (*bb).min_y;
    }
    else  // no bounding box given, use layout's border
    {
        return f[Y] == 0;
    }
}

bool fcn_layout::is_eastern_border(const face& f, const std::optional<bounding_box>& bb) const noexcept
{
    if (bb)  // bounding box given, do calculations based on that
    {
        return f[X] == (*bb).max_x;
    }
    else  // no bounding box given, use layout's border
    {
        return f[X] == x() - 1;
    }
}

bool fcn_layout::is_southern_border(const face& f, const std::optional<bounding_box>& bb) const noexcept
{
    if (bb)  // bounding box given, do calculations based on that
    {
        return f[Y] == (*bb).max_y;
    }
    else  // no bounding box given, use layout's border
    {
        return f[Y] == y() - 1;
    }
}

bool fcn_layout::is_western_border(const face& f, const std::optional<bounding_box>& bb) const noexcept
{
    if (bb)  // bounding box given, do calculations based on that
    {
        return f[X] == (*bb).min_x;
    }
    else  // no bounding box given, use layout's border
    {
        return f[X] == 0;
    }
}

bool fcn_layout::is_border(const face& f, const std::optional<bounding_box>& bb) const noexcept
{
    return is_northern_border(f, bb) || is_eastern_border(f, bb) || is_southern_border(f, bb) || is_western_border(f, bb);
}

void fcn_layout::shrink_to_fit() noexcept
{
    auto bb = determine_bounding_box();
    resize(fcn_dimension_xyz{std::max(bb.max_x + 1, coord_t{2}), std::max(bb.max_y + 1, coord_t{2}), z()});  // incorporate BGL bug
}

void fcn_layout::clear_layout() noexcept
{
    c_map.clear();
    pi_set.clear();
    po_set.clear();
    l_map.clear();
}
