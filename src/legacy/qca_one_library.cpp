//
// Created by marcel on 27.07.18.
//

#include "qca_one_library.h"

qca_one_library::qca_one_library(fcn_gate_layout_ptr fgl)
        :
        fcn_gate_library
        (
            "QCA-ONE",
            std::move(fgl),
            fcn::technology::QCA,
            fcn::tile_size::FIVE_X_FIVE
        )
{
    p_router->compute_ports();
}

fcn_gate qca_one_library::set_up_gate(const fcn_gate_layout::tile& t)
{
    /**
     * Marks PI/PO cell for single PI/PO gates.
     */
    auto mark_1_io = [this, &t](fcn_gate g, layout::directions d) -> fcn_gate
    {
        if (layout->is_pi(t))
            g = mark_cell(g, dir_to_port(d), fcn::cell_mark::INPUT);
        if (layout->is_po(t))
            g = mark_cell(g, dir_to_port(d), fcn::cell_mark::OUTPUT);

        return g;
    };
    /**
     * Marks PI/PO cells for double PI/PO gates.
     */
    auto mark_2_io = [this, &t](fcn_gate g, layout::directions d1, layout::directions d2) -> fcn_gate
    {
        if (layout->is_pi(t) && layout->is_po(t))
        {
            g = mark_cell(g, dir_to_port(d1), fcn::cell_mark::INPUT);
            g = mark_cell(g, dir_to_port(d2), fcn::cell_mark::OUTPUT);
        }
        else if (layout->is_pi(t))
        {
            g = mark_cell(g, dir_to_port(d1), fcn::cell_mark::INPUT);
            g = mark_cell(g, dir_to_port(d2), fcn::cell_mark::INPUT);
        }
        else if (layout->is_po(t))
        {
            g = mark_cell(g, dir_to_port(d1), fcn::cell_mark::OUTPUT);
            g = mark_cell(g, dir_to_port(d2), fcn::cell_mark::OUTPUT);
        }

        return g;
    };
    /**
     * Marks PI/PO cells for triple PI/PO gates.
     */
    auto mark_3_io_gate = [this, &t](fcn_gate g, layout::directions d1, layout::directions d2,
                                     layout::directions d3) -> fcn_gate
    {
        if (layout->is_pi(t) && layout->is_po(t))
        {
            g = mark_cell(g, dir_to_port(d1), fcn::cell_mark::INPUT);
            g = mark_cell(g, dir_to_port(d2), fcn::cell_mark::INPUT);
            g = mark_cell(g, dir_to_port(d3), fcn::cell_mark::OUTPUT);
        }
        else if (layout->is_pi(t))
        {
            g = mark_cell(g, dir_to_port(d1), fcn::cell_mark::INPUT);
            g = mark_cell(g, dir_to_port(d2), fcn::cell_mark::INPUT);
            g = mark_cell(g, dir_to_port(d3), fcn::cell_mark::INPUT);
        }

        return g;
    };
    /**
     * Marks PI/PO cells for quadruple PI/PO gates.
     */
    auto mark_4_io_gate = [this](fcn_gate g) -> fcn_gate
    {
        g = mark_cell(g, dir_to_port(layout::DIR_N), fcn::cell_mark::INPUT);
        g = mark_cell(g, dir_to_port(layout::DIR_W), fcn::cell_mark::INPUT);
        g = mark_cell(g, dir_to_port(layout::DIR_S), fcn::cell_mark::INPUT);
        g = mark_cell(g, dir_to_port(layout::DIR_E), fcn::cell_mark::OUTPUT);

        return g;
    };

    /**
     * Marks PI/PO cells for triple PI/PO fan-outs.
     */
    auto mark_3_io_fan_out = [this, &t](fcn_gate g, layout::directions d1, layout::directions d2,
                                        layout::directions d3) -> fcn_gate
    {
        if (layout->is_pi(t) && layout->is_po(t))
        {
            g = mark_cell(g, dir_to_port(d1), fcn::cell_mark::INPUT);
            g = mark_cell(g, dir_to_port(d2), fcn::cell_mark::OUTPUT);
            g = mark_cell(g, dir_to_port(d3), fcn::cell_mark::OUTPUT);
        }
        else if (layout->is_po(t))
        {
            g = mark_cell(g, dir_to_port(d1), fcn::cell_mark::OUTPUT);
            g = mark_cell(g, dir_to_port(d2), fcn::cell_mark::OUTPUT);
            g = mark_cell(g, dir_to_port(d3), fcn::cell_mark::OUTPUT);
        }

        return g;
    };
    /**
     * Marks PI/PO cells for quadruple PI/PO fan-outs.
     */
    auto mark_4_io_fan_out = [this](fcn_gate g) -> fcn_gate
    {
        g = mark_cell(g, dir_to_port(layout::DIR_N), fcn::cell_mark::INPUT);
        g = mark_cell(g, dir_to_port(layout::DIR_W), fcn::cell_mark::OUTPUT);
        g = mark_cell(g, dir_to_port(layout::DIR_S), fcn::cell_mark::OUTPUT);
        g = mark_cell(g, dir_to_port(layout::DIR_E), fcn::cell_mark::OUTPUT);

        return g;
    };
    /**
     * Determines rotation of given gate g with respect to its unused given directions.
     * Works for gates with a combined amount of 3 inputs/outputs only (AND, OR, F1O2).
     */
    auto pick_3_io_rotation = [&](const fcn_gate& g, layout::directions unused) -> fcn_gate
    {
        if (unused.count() == 1)
        {
            if (unused == layout::DIR_N)
                return g;
            else if (unused == layout::DIR_E)
                return rotate_90(g);
            else if (unused == layout::DIR_S)
                return rotate_180(g);
            else if (unused == layout::DIR_W)
                return rotate_270(g);
        }
        else if (unused.count() == 2)
        {
            if (unused == layout::DIR_NE)
            {
                if (layout->closest_border(t) == layout::DIR_N)
                    return mark_1_io(rotate_90(g), layout::DIR_N);
                else
                    return mark_1_io(g, layout::DIR_E);
            }
            else if (unused == layout::DIR_NS)
            {
                if (layout->closest_border(t) == layout::DIR_N)
                    return mark_1_io(rotate_180(g), layout::DIR_N);
                else
                    return mark_1_io(g, layout::DIR_S);
            }
            else if (unused == layout::DIR_NW)
            {
                if (layout->closest_border(t) == layout::DIR_N)
                    return mark_1_io(rotate_270(g), layout::DIR_N);
                else
                    return mark_1_io(g, layout::DIR_W);
            }
            else if (unused == layout::DIR_EW)
            {
                if (layout->closest_border(t) == layout::DIR_E)
                    return mark_1_io(rotate_270(g), layout::DIR_E);
                else
                    return mark_1_io(rotate_90(g), layout::DIR_W);
            }
            else if (unused == layout::DIR_ES)
            {
                if (layout->closest_border(t) == layout::DIR_E)
                    return mark_1_io(rotate_180(g), layout::DIR_E);
                else
                    return mark_1_io(rotate_90(g), layout::DIR_S);
            }
            else if (unused == layout::DIR_SW)
            {
                if (layout->closest_border(t) == layout::DIR_S)
                    return mark_1_io(rotate_270(g), layout::DIR_S);
                else
                    return mark_1_io(rotate_180(g), layout::DIR_W);
            }
        }
        else if (unused.count() == 3)
        {
            if (layout->get_op(t) == operation::F1O3)
            {
                if (unused == layout::DIR_NES)
                    return mark_3_io_fan_out(g, layout::DIR_N, layout::DIR_E, layout::DIR_S);
                else if (unused == layout::DIR_NEW)
                    return mark_3_io_fan_out(g, layout::DIR_N, layout::DIR_E, layout::DIR_W);
                else if (unused == layout::DIR_NSW)
                    return mark_3_io_fan_out(g, layout::DIR_N, layout::DIR_S, layout::DIR_W);
                else if (unused == layout::DIR_ESW)
                    return mark_3_io_fan_out(g, layout::DIR_E, layout::DIR_S, layout::DIR_W);
            }
            else
            {
                if (unused == layout::DIR_NEW)
                    return mark_2_io(g, layout::DIR_E, layout::DIR_W);
                else if (unused == layout::DIR_NES)
                    return mark_2_io(g, layout::DIR_E, layout::DIR_S);
                else if (unused == layout::DIR_NSW)
                    return mark_2_io(g, layout::DIR_S, layout::DIR_W);
                else if (unused == layout::DIR_ESW)
                    return mark_2_io(rotate_90(g), layout::DIR_S, layout::DIR_W);
            }
        }

        // unused.count() == 4
        return g;
    };

    switch (layout->get_op(t))
    {
        case operation::NOT:
        {
            auto ports = p_router->get_ports(t, *layout->get_logic_vertex(t));
            auto inv = inverter_map.at(ports);

            if (layout->is_pi(t))
                inv = mark_cell(inv, opposite(*ports.out.cbegin()), fcn::cell_mark::INPUT);
            if (layout->is_po(t))
                inv = mark_cell(inv, opposite(*ports.inp.cbegin()), fcn::cell_mark::OUTPUT);

            return inv;
        }
        case operation::AND:
        {
            auto dirs = layout->get_unused_tile_dirs(t);

            // choose correct orientation first
            auto conj = pick_3_io_rotation(conjunction, dirs);
            // mark PI/POs then
            if (dirs == layout::DIR_NESW)
                return mark_3_io_gate(conj, layout::DIR_W, layout::DIR_S, layout::DIR_E);
            else
                return conj;
        }
        case operation::OR:
        {
            auto dirs = layout->get_unused_tile_dirs(t);

            // choose correct orientation first
            auto disj = pick_3_io_rotation(disjunction, dirs);
            // mark PI/POs then
            if (dirs == layout::DIR_NESW)
                return mark_3_io_gate(disj, layout::DIR_W, layout::DIR_S, layout::DIR_E);
            else
                return disj;
        }
        case operation::MAJ:
        {
            // no need to rotate majority
            auto maj = majority;

            if (auto dirs = layout->get_unused_tile_dirs(t); dirs.count() == 0)
                return maj;
            else if (dirs.count() == 1)
                return mark_1_io(maj, dirs);
            else if (dirs.count() == 2)
            {
                if (dirs == layout::DIR_NE)
                    return mark_2_io(maj, layout::DIR_N, layout::DIR_E);
                else if (dirs == layout::DIR_NS)
                    return mark_2_io(maj, layout::DIR_N, layout::DIR_S);
                else if (dirs == layout::DIR_NW)
                    return mark_2_io(maj, layout::DIR_N, layout::DIR_W);
                else if (dirs == layout::DIR_ES)
                    return mark_2_io(maj, layout::DIR_E, layout::DIR_S);
                else if (dirs == layout::DIR_EW)
                    return mark_2_io(maj, layout::DIR_E, layout::DIR_W);
                else if (dirs == layout::DIR_SW)
                    return mark_2_io(maj, layout::DIR_S, layout::DIR_W);
            }
            else if (dirs.count() == 3)
            {
                if (dirs == layout::DIR_NES)
                    return mark_3_io_gate(maj, layout::DIR_N, layout::DIR_E, layout::DIR_S);
                else if (dirs == layout::DIR_NEW)
                    return mark_3_io_gate(maj, layout::DIR_N, layout::DIR_E, layout::DIR_W);
                else if (dirs == layout::DIR_NSW)
                    return mark_3_io_gate(maj, layout::DIR_N, layout::DIR_S, layout::DIR_W);
                else if (dirs == layout::DIR_ESW)
                    return mark_3_io_gate(maj, layout::DIR_E, layout::DIR_S, layout::DIR_W);
            }
            else if (dirs.count() == 4)
                return mark_4_io_gate(maj);

            break;
        }
        case operation::F1O2:
        {
            auto dirs = layout->get_unused_tile_dirs(t);

            // choose correct orientation first
            auto fan_out = pick_3_io_rotation(fan_out_1_2, dirs);
            // mark PI/POs then
            if (dirs == layout::DIR_NESW)
                return mark_3_io_fan_out(fan_out, layout::DIR_W, layout::DIR_S, layout::DIR_E);
            else
                return fan_out;
        }
        case operation::F1O3:
        {
            // no need to rotate F1O3
            auto fan_out = fan_out_1_3;

            if (auto dirs = layout->get_unused_tile_dirs(t); dirs.count() == 1)
                return mark_1_io(fan_out, dirs);
            else if (dirs == layout::DIR_NESW)
                return mark_4_io_fan_out(fan_out);
            else
                return fan_out;
        }
        case operation::PI:
        {
            if (layout->is_tile_out_dir(t, layout::DIR_E))
                return rotate_90(primary_input_port);
            else if (layout->is_tile_out_dir(t, layout::DIR_S))
                return rotate_180(primary_input_port);
            else if (layout->is_tile_out_dir(t, layout::DIR_W))
                return rotate_270(primary_input_port);

            return primary_input_port;
        }
        case operation::PO:
        {
            if (layout->is_tile_inp_dir(t, layout::DIR_E))
                return rotate_90(primary_output_port);
            else if (layout->is_tile_inp_dir(t, layout::DIR_S))
                return rotate_180(primary_output_port);
            else if (layout->is_tile_inp_dir(t, layout::DIR_W))
                return rotate_270(primary_output_port);

            return primary_output_port;
        }
        case operation::W:
        {
            std::vector <fcn_gate> wires;
            if (layout->is_gate_tile(t))
            {
                if (layout->is_pi(t))
                {
                    if (layout->is_tile_out_dir(t, layout::DIR_E))
                        return rotate_90(primary_input_port);
                    else if (layout->is_tile_out_dir(t, layout::DIR_S))
                        return rotate_180(primary_input_port);
                    else if (layout->is_tile_out_dir(t, layout::DIR_W))
                        return rotate_270(primary_input_port);

                    return primary_input_port;
                }
                else if (layout->is_po(t))
                {
                    if (layout->is_tile_inp_dir(t, layout::DIR_E))
                        return rotate_90(primary_output_port);
                    else if (layout->is_tile_inp_dir(t, layout::DIR_S))
                        return rotate_180(primary_output_port);
                    else if (layout->is_tile_inp_dir(t, layout::DIR_W))
                        return rotate_270(primary_output_port);

                    return primary_output_port;
                }
                else
                {
                    auto v = *layout->get_logic_vertex(t);
                    auto ports = p_router->get_ports(t, v);
                    wires.push_back(wire_map.at(ports));
                }
            }
            else
            {
                for (auto& e : layout->get_logic_edges(t))
                {
                    auto ports = p_router->get_ports(t, e);
                    wires.push_back(wire_map.at(ports));
                }
            }

            return merge(wires);
        }
        case operation::NONE:
        {
            return empty_gate;
        }
        default:
        {
            throw std::invalid_argument("gate type not listed in library");
        }
    }
    throw std::invalid_argument("unsupported gate/direction combination");
}

fcn_gate_library::port qca_one_library::dir_to_port(const layout::directions d) const
{
    if (d == layout::DIR_N)
        return {2, 0};
    else if (d == layout::DIR_E)
        return {4, 2};
    else if (d == layout::DIR_S)
        return {2, 4};
    else if (d == layout::DIR_W)
        return {0, 2};

    throw std::invalid_argument("given direction does not have a single port equivalence.");
}
