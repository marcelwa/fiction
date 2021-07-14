//
// Created by marcel on 25.07.18.
//

#include "fcn_cell_layout.h"


fcn_cell_layout::fcn_cell_layout(const fcn_dimension_xyz& lengths, fcn_clocking_scheme clocking,
                                 fcn::technology tech, const std::string& name) noexcept
        :
        fcn_layout(lengths, std::move(clocking)),
        technology{tech},
        name{name}
{}

fcn_cell_layout::fcn_cell_layout(const fcn_dimension_xy& lengths, fcn_clocking_scheme clocking,
                                 fcn::technology tech, const std::string& name) noexcept
        :
        fcn_layout(lengths, std::move(clocking)),
        technology{tech},
        name{name}
{}

fcn_cell_layout::fcn_cell_layout(fcn_gate_library_ptr&& lib)
        :
        fcn_layout(fcn_dimension_xyz{lib->get_layout()->x() * lib->gate_x_size(),
                                     lib->get_layout()->y() * lib->gate_y_size() +
                                     (lib->get_layout()->is_vertically_shifted() ?
                                     lib->gate_y_size() / 2 : 0),  // add half a tile in y direction if layout is vertically shifted
                                     lib->get_layout()->z()},
                   std::move(lib->get_layout()->clocking)),
        library(std::move(lib)),
        technology{library->get_technology()},
        name{library->get_layout()->get_name()}
{
    if (!clocking.regular)
        map_irregular_clocking();

    map_layout();
}

void fcn_cell_layout::assign_cell_type(const cell& c, const fcn::cell_type t) noexcept
{
    if (t == fcn::EMPTY_CELL)
    {
        type_map.erase(c);
        pi_set.erase(c);
        po_set.erase(c);
        return;
    }
    else if (t == fcn::INPUT_CELL)
        pi_set.insert(c);
    else if (t == fcn::OUTPUT_CELL)
        po_set.insert(c);

    type_map[c] = t;
}

fcn::cell_type fcn_cell_layout::get_cell_type(const cell& c) const noexcept
{
    if (auto it = type_map.find(c); it != type_map.end())
    {
        return it->second;
    }
    else
    {
        return fcn::EMPTY_CELL;
    }
}

bool fcn_cell_layout::is_free_cell(const cell& c) const noexcept
{
    return get_cell_type(c) == fcn::EMPTY_CELL;
}

void fcn_cell_layout::assign_cell_mode(const cell& c, const fcn::cell_mode m) noexcept
{
    if (m == fcn::cell_mode::NORMAL)
        mode_map.erase(c);
    else
        mode_map[c] = m;
}

fcn::cell_mode fcn_cell_layout::get_cell_mode(const cell& c) const noexcept
{
    if (auto it = mode_map.find(c); it != mode_map.end())
    {
        return it->second;
    }
    else
    {
        return fcn::cell_mode::NORMAL;
    }
}

void fcn_cell_layout::assign_cell_name(const cell& c, const std::string& n) noexcept
{
    if (n.empty())
        name_map.erase(c);
    else
        name_map[c] = n;
}

std::string fcn_cell_layout::get_cell_name(const cell& c) const noexcept
{
    if (auto it = name_map.find(c); it != name_map.end())
    {
        return it->second;
    }
    else
    {
        return "";
    }
}


void fcn_cell_layout::print_cell(const cell& c, std::ostream& os) const noexcept
{
    os << get_cell_type(c);
}

std::size_t fcn_cell_layout::cell_count() const noexcept
{
    return type_map.size();
}

std::optional<fcn_clock::zone> fcn_cell_layout::cell_clocking(const cell& c) const noexcept
{
    if (clocking.regular)
    {
        coord_t x = c[X] / library->gate_x_size(), y = c[Y] / library->gate_y_size();
        return clocking.scheme[y % clocking.cutout_y][x % clocking.cutout_x];
    }
    else  // irregular clocking accesses clocking map
    {
        if (auto it = c_map.find(get_ground(c)); it != c_map.end())
        {
            return it->second;
        }
        else
        {
            return std::nullopt;
        }
    }
}

void fcn_cell_layout::assign_gate(const cell& c, const fcn_gate& g, const latch_delay l,
                                  const std::vector<std::string>& inp_names, const std::vector<std::string>& out_names)
{
    auto start_x = c[X];
    auto start_y = c[Y];
    auto layer   = c[Z];

    auto inp_counter = 0u, out_counter = 0u;

    for (auto y = 0ul; y < g.size(); ++y)
    {
        for (auto x = 0ul; x < g[y].size(); ++x)
        {
            const auto pos = cell{start_x + x, start_y + y, layer};
            const auto type = g[y][x];

            assign_cell_type(pos, type);
            assign_latch(pos, l);

            if (type == fcn::INPUT_CELL)
            {
                assign_cell_name(pos, (inp_names.size() > inp_counter) ? inp_names[inp_counter] : "Input");
                ++inp_counter;
            }
            else if (type == fcn::OUTPUT_CELL)
            {
                assign_cell_name(pos, (out_names.size() > out_counter) ? out_names[out_counter] : "Output");
                ++out_counter;
            }
        }
    }
}

fcn_gate_library_ptr fcn_cell_layout::get_library() const noexcept
{
    return library;
}

fcn::technology fcn_cell_layout::get_technology() const noexcept
{
    return technology;
}

std::string fcn_cell_layout::get_name() const noexcept
{
    return name;
}

std::size_t fcn_cell_layout::magcad_magnet_count() const noexcept
{
    if (technology != fcn::technology::INML)
        return 0ul;

    auto num_inv_cells = std::count_if(type_map.cbegin(), type_map.cend(),
            [](const auto _ct){ return _ct.second == fcn::inml::INVERTER_MAGNET; });

    return cell_count() + num_inv_cells / 4;
}

fcn_layout::bounding_box fcn_cell_layout::determine_bounding_box() const noexcept
{
    // calculate min_x
    coord_t min_x = 0u;
    for (coord_t x = 0u; x < this->x(); ++x)
    {
        bool elem_found = false;
        for (coord_t y = 0u; y < this->y(); ++y)
        {
            if (!this->is_free_cell(cell{x, y, GROUND}) || !this->is_free_cell(cell{x, y, 1}))
            {
                elem_found = true;
                break;
            }
        }

        min_x = x;
        if (elem_found)
            break;
    }

    // calculate min_y
    coord_t min_y = 0u;
    for (coord_t y = 0u; y < this->y(); ++y)
    {
        bool elem_found = false;
        for (coord_t x = 0u; x < this->x(); ++x)
        {
            if (!this->is_free_cell(cell{x, y, GROUND}) || !this->is_free_cell(cell{x, y, 1}))
            {
                elem_found = true;
                break;
            }
        }

        min_y = y;
        if (elem_found)
            break;
    }

    // calculate max_x
    coord_t max_x = this->x() - 1;
    for (auto x = static_cast<long>(this->x()) - 1; x >= 0; --x)
    {
        bool elem_found = false;
        for (coord_t y = 0u; y < this->y(); ++y)
        {
            if (!this->is_free_cell(cell{static_cast<coord_t>(x), y, GROUND}) ||
                !this->is_free_cell(cell{static_cast<coord_t>(x), y, 1}))
            {
                elem_found = true;
                break;
            }
        }

        max_x = static_cast<coord_t>(x);
        if (elem_found)
            break;
    }

    // calculate max_y
    coord_t max_y = this->y() - 1;
    for (auto y = static_cast<long>(this->y()) - 1; y >= 0; --y)
    {
        bool elem_found = false;
        for (coord_t x = 0u; x < this->x(); ++x)
        {
            if (!this->is_free_cell(cell{x, static_cast<coord_t>(y), GROUND}) ||
                !this->is_free_cell(cell{x, static_cast<coord_t>(y), 1}))
            {
                elem_found = true;
                break;
            }
        }

        max_y = static_cast<coord_t>(y);
        if (elem_found)
            break;
    }

    return bounding_box{min_x, min_y, max_x, max_y};
}

void fcn_cell_layout::write_layout(std::ostream& os, const bool io_color, const bool clk_color) const noexcept
{
    // empty layout
    if (!area())
    {
        os << "[i] empty layout" << std::endl;
        return;
    }

    // Escape color sequence for input colors (green).
    const char* INP_COLOR = "\033[38;5;28m";
    // Escape color sequence for output colors (red).
    const char* OUT_COLOR = "\033[38;5;166m";
    // Escape color sequence for latch colors (yellow on black).
    const char* LATCH_COLOR = "\033[48;5;232;38;5;226m";
    // Escape color sequence for resetting colors.
    const char* COLOR_RESET = "\033[0m";
    // Escape color sequences for clock background colors (white to dark grey).
    std::vector<const char*> CLOCK_COLORS{{"\033[48;5;255;38;5;232m",  // white tile, black text
                                           "\033[48;5;248;38;5;232m",  // light grey tile, black text
                                           "\033[48;5;240;38;5;255m",  // grey tile, white text
                                           "\033[48;5;236;38;5;255m"   // dark grey tile, white text
                                          }};

    for (auto y_pos : iter::range(y()))
    {
        for (auto x_pos : iter::range(x()))
        {
            cell c{x_pos, y_pos, GROUND};

            auto type_0 = get_cell_type(c);
            auto type_1 = get_cell_type(above(c));

            if (clk_color && cell_clocking(c))
                os << CLOCK_COLORS[*cell_clocking(c)];

            // is crossing
            if (type_1 != fcn::EMPTY_CELL)
                os << std::string(1u, type_1);
            else
            {
                if (io_color && get_latch(c) > 0u)
                    os << LATCH_COLOR;
                if (io_color && type_0 == fcn::INPUT_CELL)
                    os << INP_COLOR;
                else if (io_color && type_0 == fcn::OUTPUT_CELL)
                    os << OUT_COLOR;

                os << (type_0 == fcn::NORMAL_CELL ? "▢" : std::string(1u, type_0)) << COLOR_RESET;
            }
        }
        os << '\n';
    }
    // flush stream
    os << std::endl;
}

void fcn_cell_layout::map_irregular_clocking() noexcept
{
    auto layout = library->get_layout();

    for (auto&& c : this->ground_layer())
    {
        coord_t x = c[X] / library->gate_x_size(), y = c[Y] / library->gate_y_size();

        if (auto t = fcn_gate_layout::tile{x, y, GROUND}; auto clk = layout->tile_clocking(t))
            assign_clocking(c, clk.value_or(0));
    }
}

void fcn_cell_layout::assign_vias() noexcept
{
    for (auto&& c : crossing_layers() | iter::filterfalse([this](const cell& _c){return is_free_cell(_c);}))
    {
        // if number of surrounding cells is 1 or less, it is a via cell
        if (auto surrounding = surrounding_2d(c) | iter::filterfalse([this](const cell& _c)
                { return is_free_cell(_c); }); std::distance(surrounding.begin(), surrounding.end()) <= 1u)
        {
            // change mode to via
            assign_cell_mode(c, fcn::cell_mode::VERTICAL);
            // create a via ground cell
            auto ground_via = cell{c[X], c[Y], GROUND};
            assign_cell_type(ground_via, fcn::NORMAL_CELL);
            assign_cell_mode(ground_via, fcn::cell_mode::VERTICAL);
        }
    }
}

void fcn_cell_layout::optimize() noexcept
{
    // no optimizations for QCA layouts
    if (technology == fcn::technology::QCA)
        return;

    if (technology == fcn::technology::INML)
    {
        if (is_clocking("TOPOLINANO3") || is_clocking("TOPOLINANO4"))
        {
            // optimize slight cosmetic issues in the ToPoliNano library mapping
            clean_up_topolinano();
            // optimize wires that can be cut; those occur due to row-layout emulation by shifted tile-layouts
            cut_optimization();
            shrink_to_fit();
        }
    }
}

void fcn_cell_layout::map_layout()
{
    auto layout = library->get_layout();

    for (auto&& t : layout->tiles())
    {
        if (layout->is_free_tile(t))
            continue;

        assign_gate({t[X] * library->gate_x_size(),
                     t[Y] * library->gate_y_size() + (layout->is_vertically_shifted() && (is_odd_column(t)) ?
                     library->gate_y_size() / 2 : 0), technology == fcn::technology::INML ? GROUND : t[Z]},
                    library->set_up_gate(t), layout->get_latch(t), layout->get_inp_names(t), layout->get_out_names(t));
    }

    if (technology == fcn::technology::QCA)
        assign_vias();

    optimize();
}
