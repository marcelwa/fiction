//
// Created by marcel on 29.09.18.
//

#include "port_router.h"

port_router::port_router(fcn_gate_layout_ptr fgl, const fcn::technology t, const fcn::tile_size s) noexcept
        :
        layout{std::move(fgl)},
        tech{t},
        size{s}
{}

void port_router::compute_ports()
{
    switch (tech)
    {
        case fcn::technology::QCA:
        {
            switch (size)
            {
                case fcn::tile_size::FIVE_X_FIVE:
                {
                    compute_qca_5x5_ports();
                    return;
                }
                default:
                {
                    // not supported
                }
            }
        }
        case fcn::technology::INML:
        {
            switch (size)
            {
                case fcn::tile_size::FOUR_X_FOUR:
                {
                    compute_inml_4x4_ports();
                    return;
                }
                default:
                {
                    // not supported
                }
            }
        }
    }
}

void port_router::compute_qca_5x5_ports()
{
    // for all non-free (i.e. occupied) tiles
    for (auto&& t : layout->tiles() |
                    iter::filterfalse([&](const fcn_gate_layout::tile& _t){return layout->is_free_tile(_t);}))
    {
        // distinguish between gates and wires
        if (layout->is_gate_tile(t))
        {
            port_list p{};

            // determine incoming connector ports
            if (layout->is_tile_inp_dir(t, layout::DIR_N))
                p.inp.emplace(2u, 0u);
            if (layout->is_tile_inp_dir(t, layout::DIR_E))
                p.inp.emplace(4u, 2u);
            if (layout->is_tile_inp_dir(t, layout::DIR_S))
                p.inp.emplace(2u, 4u);
            if (layout->is_tile_inp_dir(t, layout::DIR_W))
                p.inp.emplace(0u, 2u);

            // NONE (WEST)
            if (auto op = layout->get_op(t); op != operation::NOT && op != operation::W)
            {
                if (layout->get_tile_inp_dirs(t) == layout::DIR_NONE)
                    p.inp.emplace(0u, 2u);
            }

            // determine outgoing connector ports
            if (layout->is_tile_out_dir(t, layout::DIR_N))
                p.out.emplace(2u, 0u);
            if (layout->is_tile_out_dir(t, layout::DIR_E))
                p.out.emplace(4u, 2u);
            if (layout->is_tile_out_dir(t, layout::DIR_S))
                p.out.emplace(2u, 4u);
            if (layout->is_tile_out_dir(t, layout::DIR_W))
                p.out.emplace(0u, 2u);

            // NONE (EAST)
            if (auto op = layout->get_op(t); op != operation::NOT && op != operation::W)
            {
                if (layout->get_tile_out_dirs(t) == layout::DIR_NONE)
                    p.out.emplace(4u, 2u);
            }

            // assign connector ports
            g_ports[std::make_pair(t, *layout->get_logic_vertex(t))] = p;
        }
        else if (layout->is_wire_tile(t))
        {
            for (auto& e : layout->get_logic_edges(t))
            {
                port_list p{};

                // numeral representation of direction bitset
                switch (auto inp_dir_num = layout->get_wire_inp_dirs(t, e).to_ulong(); inp_dir_num)
                {
                    case 0b0000: { p.inp.emplace(2u, 0u); break; }  // NONE (NORTH)
                    case 0b0001: { p.inp.emplace(0u, 2u); break; }  // WEST
                    case 0b0010: { p.inp.emplace(2u, 4u); break; }  // SOUTH
                    case 0b0100: { p.inp.emplace(4u, 2u); break; }  // EAST
                    case 0b1000: { p.inp.emplace(2u, 0u); break; }  // NORTH
                    default:
                    {
                        throw std::invalid_argument{"unsupported input wire directions in tile"};
                    }
                }
                // numeral representation of direction bitset
                switch (auto out_dir_num = layout->get_wire_out_dirs(t, e).to_ulong(); out_dir_num)
                {
                    case 0b0000: { p.out.emplace(2u, 4u); break; }  // NONE (SOUTH)
                    case 0b0001: { p.out.emplace(0u, 2u); break; }  // WEST
                    case 0b0010: { p.out.emplace(2u, 4u); break; }  // SOUTH
                    case 0b0100: { p.out.emplace(4u, 2u); break; }  // EAST
                    case 0b1000: { p.out.emplace(2u, 0u); break; }  // NORTH
                    default:
                    {
                        throw std::invalid_argument{"unsupported output wire directions in tile"};
                    }
                }

                // assign ports
                w_ports[std::make_pair(t, e)] = p;
            }
        }
    }
}

void port_router::compute_inml_4x4_ports()
{
    // for all non-free (i.e. occupied) tiles
    for (auto&& t : layout->tiles() |
                    iter::filterfalse([&](const fcn_gate_layout::tile& _t) { return layout->is_free_tile(_t); }))
    {
        auto op = layout->get_op(t);

        // distinguish between gates and wires
        if (op != operation::W)
        {
            port_list p{};

            // determine incoming connector ports

            // special case: PO determines output according to its predecessor
            if (op == operation::PO)
            {
                // if predecessor is an AND/OR/MAJ gate, input port is at (0,3) and output port at (3,3)
                auto [iat, iaop] = *layout->incoming_data_flow(t, {*layout->get_logic_vertex(t)}).begin();
                (void)iaop;
                if (auto iag = layout->get_op(iat); iag == operation::AND || iag == operation::OR ||
                                                    iag == operation::MAJ)
                {
                    // but only if that gate is SW of the PO
                    if (layout->is_tile_inp_dir(t, layout::DIR_SW))
                    {
                        p.inp.emplace(0u, 3u);
                        p.out.emplace(3u, 3u);

                        // assign connector ports
                        g_ports[{t, *layout->get_logic_vertex(t)}] = p;
                        continue;
                    }
                }
                // if input is north-western, output port is at (3,0)
                else if (layout->is_tile_inp_dir(t, layout::DIR_NW))
                    p.out.emplace(3u, 0u);
                // if input is south-western, output port is at (3,2)
                else if (layout->is_tile_inp_dir(t, layout::DIR_SW))
                    p.out.emplace(3u, 2u);
            }

            if (layout->is_tile_inp_dir(t, layout::DIR_NW))
                p.inp.emplace(0u, 0u);
            if (layout->is_tile_inp_dir(t, layout::DIR_SW))
            {
                if (op == operation::NOT || op == operation::PO)
                {
                    // special case: if predecessor is AND, OR, MAJ, input port is at (0,3)
                    auto [iat, iaop] = *layout->incoming_data_flow(t, {*layout->get_logic_vertex(t)}).begin();
                    (void)iaop;
                    if (auto iag = layout->get_op(iat); iag == operation::AND ||
                                                        iag == operation::OR  || iag == operation::MAJ)
                        p.inp.emplace(0u, 3u);
                    else
                        p.inp.emplace(0u, 2u);
                }
                else
                    p.inp.emplace(0u, 2u);
            }
            // special case: MAJ needs to have an input at the bottom
            if (layout->is_tile_inp_dir(t, layout::DIR_N) && layout->get_op(t) == operation::MAJ)
                p.inp.emplace(1u, 3u);

            // special case: PI determines input according to successor
            if (op == operation::PI)
            {
                // if successor is a fan-out, input port is at (0,3) and output port at (3,3)
                auto [at, aop] = *layout->outgoing_data_flow(t, {*layout->get_logic_vertex(t)}).begin();
                (void)aop;
                if (layout->get_op(at) == operation::F1O2)
                {
                    p.inp.emplace(0u, 3u);
                    p.out.emplace(3u, 3u);

                    // assign connector ports
                    g_ports[{t, *layout->get_logic_vertex(t)}] = p;
                    continue;
                }
                // if output is north-eastern, input port is at (0,0)
                else if (layout->is_tile_out_dir(t, layout::DIR_NE))
                    p.inp.emplace(0u, 0u);
                // if output is south-eastern, input port is at (0,2)
                else if (layout->is_tile_out_dir(t, layout::DIR_ES))
                    p.inp.emplace(0u, 2u);
            }

            // determine outgoing connector ports
            if (layout->is_tile_out_dir(t, layout::DIR_NE))
            {
                // special case: output port of AND, OR, MAJ is fixed at pos (3,1)
                if (op == operation::AND || op == operation::OR || op == operation::MAJ)
                    p.out.emplace(3u, 1u);
                else
                    p.out.emplace(3u, 0u);
            }
            if (layout->is_tile_out_dir(t, layout::DIR_ES))
                p.out.emplace(3u, 2u);

            // assign connector ports
            g_ports[{t, *layout->get_logic_vertex(t)}] = p;
        }
        else  // op == operation::W
        {
            port_list p{};

            auto assign_inp_ports = [this, &p, &t](auto input_numeral, auto assigned)
            {
                switch (input_numeral)
                {
                    case 0b1000: { p.inp.emplace(0u, 0u); break; }  // NORTH
                    case 0b0010: { p.inp.emplace(0u, 3u); break; }  // SOUTH
                    case 0b1001: { p.inp.emplace(0u, 0u); break; }  // NORTH-WEST
                    case 0b0011:                                    // SOUTH-WEST
                    {
                        // special case: if predecessor is AND, OR, MAJ, input port is at (0,3)
                        auto idf = layout->incoming_data_flow(t, {assigned});
                        auto [iat, iaop] = *idf.begin();
                        (void)iaop;
                        if (auto iag = layout->get_op(iat); iag == operation::AND || iag == operation::OR ||
                                                            iag == operation::MAJ)
                            p.inp.emplace(0u, 3u);
                        else
                            p.inp.emplace(0u, 2u);

                        break;
                    }
                    default:
                    {
                        p.inp.emplace(0u, 0u);
                    }
                }
            };

            auto assign_out_ports = [this, &p, &t](auto output_numeral, auto assigned)
            {
                switch (output_numeral)
                {
                    case 0b0010: { p.out.emplace(3u, 3u); break; }  // SOUTH
                    case 0b1100: { p.out.emplace(3u, 0u); break; }  // NORTH-EAST
                    case 0b0110:                                    // SOUTH-EAST
                    {
                        // special case: if successor is a fan-out, output is at (3,3)
                        auto [at, aop] = *layout->outgoing_data_flow(t, {assigned}).begin();
                        (void)aop;
                        if (layout->get_op(at) == operation::F1O2)
                            p.out.emplace(3u, 3u);
                        else
                            p.out.emplace(3u, 2u);
                        break;
                    }
                    case 0b1000:                                    // NORTH
                    {
                        // special case: if northern tile is MAJ, output port is at (1,0)
                        if (auto gn = layout->get_op(layout->north(t)); gn == operation::MAJ)
                            p.out.emplace(1u, 0u);
                        else
                            p.out.emplace(3u, 0u);
                        break;
                    }
                    default:
                    {
                        p.out.emplace(3u, 2u);
                    }
                }
            };

            // regular wires
            if (layout->is_wire_tile(t))
            {
                for (auto& e : layout->get_logic_edges(t))
                {
                    // numeral representation of direction bitset
                    auto inp_dir_num = layout->get_wire_inp_dirs(t, e).to_ulong();
                    assign_inp_ports(inp_dir_num, e);

                    // numeral representation of direction bitset
                    auto out_dir_num = layout->get_wire_out_dirs(t, e).to_ulong();
                    assign_out_ports(out_dir_num, e);

                    // assign ports
                    w_ports[{t, e}] = p;
                    // reset p
                    p = {};
                }
            }
            else if (auto b = layout->get_logic_vertex(t); b)  // balance wire vertex
            {
                // numeral representation of direction bitset
                auto inp_dir_num = layout->get_tile_inp_dirs(t).to_ulong();
                assign_inp_ports(inp_dir_num, *b);

                // numeral representation of direction bitset
                auto out_dir_num = layout->get_tile_out_dirs(t).to_ulong();
                assign_out_ports(out_dir_num, *b);

                // assign ports
                g_ports[{t, *b}] = p;
            }
        }
    }
}
