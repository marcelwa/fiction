//
// Created by marcel on 29.07.19.
//

#include "topolinano_library.h"

topolinano_library::topolinano_library(fcn_gate_layout_ptr fgl)
        :
        fcn_gate_library
        (
            "ToPoliNano",
            std::move(fgl),
            fcn::technology::INML,
            fcn::tile_size::FOUR_X_FOUR
        )
{
    p_router->compute_ports();
}

fcn_gate topolinano_library::set_up_gate(const fcn_gate_layout::tile& t)
{
    switch (auto op = layout->get_op(t); op)
    {
        case operation::NOT:
        {
            auto ports = p_router->get_ports(t, *layout->get_logic_vertex(t));
            auto inv = inverter_map.at(ports);

            if (layout->is_pi(t))
            {
                if (layout->get_tile_inp_dirs(t) == layout::DIR_NONE)
                    inv = mark_cell(inv, opposite(*ports.out.begin()), fcn::cell_mark::INPUT);
            }
            if (layout->is_po(t))
            {
                if (layout->get_tile_out_dirs(t) == layout::DIR_NONE)
                    inv = mark_cell(inv, opposite(*ports.inp.begin()), fcn::cell_mark::OUTPUT);
            }

            return inv;
        }
        case operation::AND:
        {
            auto conj = conjunction;
            if (layout->is_pi(t))
            {
                if (!layout->is_tile_inp_dir(t, layout::DIR_NW))
                    conj = mark_cell(conj, dir_to_port(layout::DIR_NW), fcn::cell_mark::INPUT);
                if (!layout->is_tile_inp_dir(t, layout::DIR_SW))
                    conj = mark_cell(conj, dir_to_port(layout::DIR_SW), fcn::cell_mark::INPUT);
            }
            if (layout->is_po(t))
            {
                if (!layout->is_tile_out_dir(t, layout::DIR_E))
                    conj = mark_cell(conj, dir_to_port(layout::DIR_E), fcn::cell_mark::OUTPUT);
            }

            return conj;
        }
        case operation::OR:
        {
            auto disj = disjunction;
            if (layout->is_pi(t))
            {
                if (!layout->is_tile_inp_dir(t, layout::DIR_NW))
                    disj = mark_cell(disj, dir_to_port(layout::DIR_NW), fcn::cell_mark::INPUT);
                if (!layout->is_tile_inp_dir(t, layout::DIR_SW))
                    disj = mark_cell(disj, dir_to_port(layout::DIR_SW), fcn::cell_mark::INPUT);
            }
            if (layout->is_po(t))
            {
                if (!layout->is_tile_out_dir(t, layout::DIR_E))
                    disj = mark_cell(disj, dir_to_port(layout::DIR_E), fcn::cell_mark::OUTPUT);
            }

            return disj;
        }
        case operation::MAJ:
        {
            return majority;
        }
        case operation::F1O2:
        {
            auto coup = coupler;
            if (layout->is_pi(t))
            {
                if (!layout->is_tile_inp_dir(t, layout::DIR_W))
                    coup = mark_cell(coup, dir_to_port(layout::DIR_W), fcn::cell_mark::INPUT);
            }

            return coup;
        }
        case operation::PI:
        {
            auto ports = p_router->get_ports(t, *layout->get_logic_vertex(t));
            auto inp_mark_pos = ports.inp.empty() ? opposite(*ports.out.begin()) : *ports.inp.begin();
            return mark_cell(wire_map.at(ports), inp_mark_pos, fcn::cell_mark::INPUT);
        }
        case operation::PO:
        {
            auto ports = p_router->get_ports(t, *layout->get_logic_vertex(t));
            auto out_mark_pos = ports.out.empty() ? opposite(*ports.inp.begin()) : *ports.out.begin();
            return mark_cell(wire_map.at(ports), out_mark_pos, fcn::cell_mark::OUTPUT);
        }
        case operation::W:
        {
            if (layout->is_gate_tile(t))
            {
                auto ports = p_router->get_ports(t, *layout->get_logic_vertex(t));
                auto w = wire_map.at(ports);

                if (layout->is_pi(t))
                {
                    if (layout->get_tile_inp_dirs(t) == layout::DIR_NONE)
                        w = mark_cell(w, ports.inp.empty() ? opposite(*ports.out.begin()) : *ports.inp.begin(), fcn::cell_mark::INPUT);
                }
                if (layout->is_po(t))
                {
                    if (layout->get_tile_out_dirs(t) == layout::DIR_NONE)
                        w = mark_cell(w, ports.out.empty() ? opposite(*ports.inp.begin()) : *ports.out.begin(), fcn::cell_mark::OUTPUT);
                }
                return w;
            }
            else
            {
                if (layout->get_op(layout->above(t)) == operation::W)
                {
                    return crosswire;
                }

                std::vector<fcn_gate> wires;
                for (auto& e : layout->get_logic_edges(t))
                {
                    auto ports = p_router->get_ports(t, e);
                    wires.push_back(wire_map.at(ports));
                }

                return merge(wires);
            }
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

    // just because compiler complains
    throw std::invalid_argument("gate type not listed in library");
}

fcn_gate_library::port topolinano_library::dir_to_port(const layout::directions d) const
{
    if (d == layout::DIR_E)
        return {3, 1};
    else if (d == layout::DIR_W)
        return {0, 1};
    else if (d == layout::DIR_NW)
        return {0, 0};
    else if (d == layout::DIR_NE)
        return {3, 0};
    else if (d == layout::DIR_ES)
        return {3, 2};
    else if (d == layout::DIR_SW)
        return {0, 2};

    throw std::invalid_argument("given direction does not have a single port equivalence.");
}
