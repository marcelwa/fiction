//
// Created by marcel on 27.07.18.
//

#ifndef FICTION_QCA_ONE_LIBRARY_H
#define FICTION_QCA_ONE_LIBRARY_H

#include "../traits.hpp"
#include "../utils/array.hpp"
#include "cell_ports.hpp"
#include "cell_technologies.hpp"
#include "fcn_gate_library.hpp"

#include <fmt/format.h>

#include <unordered_map>
#include <vector>

namespace fiction
{

/**
 * A concrete FCN gate library based on QCA-ONE proposed in "A Methodology for Standard Cell Design for QCA" by Dayane
 * Alfenas Reis, Caio Araújo T. Campos, Thiago Rodrigues B. S. Soares, Omar Paranaiba V. Neto, and Frank Sill Torres in
 * IEEE International Symposium on Circuits and Systems, 2016. QCA-ONE was originally proposed for the USE clocking
 * scheme. The version used here is an extension to the original QCA-ONE by also theoretically allowing multiple wires
 * in the same tile. Furthermore, it can be used for a range of clocking schemes. Tiles in QCA-ONE are 5 x 5 QCA cells.
 */
class qca_one_library : public fcn_gate_library<qca_technology, 5, 5>
{
  public:
    explicit qca_one_library() = delete;
    /**
     * Overrides the corresponding function in fcn_gate_library. Given a tile t, this function takes all
     * necessary information from the stored grid into account to choose the correct fcn_gate representation for
     * that tile. May it be a gate or wires. Rotation and special marks like input and output, const cells etc.
     * are computed additionally.
     *
     * @param t Tile to be realized in QCA-ONE.
     * @return QCA-ONE gate representation of t including I/Os, rotation, const cells, etc.
     */
    template <typename Lyt>
    [[nodiscard]] static fcn_gate set_up_gate(const Lyt& lyt, const typename Lyt::tile& t)
    {
        const auto n = lyt.get_node(t);
        const auto p = determine_port_routing(lyt, t);

        try
        {
            if constexpr (fiction::has_is_fanout_v<Lyt>)
            {
                if (lyt.is_fanout(n))
                {
                    if (lyt.fanout_size(n) == 2)
                        return fanout_map.at(p);
                    else if (lyt.fanout_size(n) == 3)
                        return fan_out_1_3;
                }
            }
            if constexpr (fiction::has_is_buf_v<Lyt>)
            {
                if (lyt.is_buf(n))
                    return wire_map.at(p);
            }
            if constexpr (fiction::has_is_inv_v<Lyt>)
            {
                if (lyt.is_inv(n))
                    return inverter_map.at(p);
            }
            if constexpr (mockturtle::has_is_and_v<Lyt>)
            {
                if (lyt.is_and(n))
                    return conjunction_map.at(p);
            }
            if constexpr (mockturtle::has_is_or_v<Lyt>)
            {
                if (lyt.is_or(n))
                    return disjunction_map.at(p);
            }
            if constexpr (mockturtle::has_is_maj_v<Lyt>)
            {
                if (lyt.is_maj(n))
                    return majority;
            }
        }
        catch (const std::out_of_range&)
        {
            throw unsupported_gate_orientation_exception(t, p);
        }

        throw unsupported_gate_type_exception(t);
    }

    template <typename Lyt>
    static void assign_via_cells(Lyt& lyt) noexcept
    {
        lyt.foreach_cell_position(
            [&lyt](const auto& c)
            {
                if (lyt.is_crossing_layer(c))
                {
                    if (!lyt.is_empty_cell(c))
                    {
                        // gather adjacent cell positions
                        auto adjacent_cells = lyt.template adjacent_tiles<std::vector<typename Lyt::cell>>(c);
                        // remove all empty cells
                        adjacent_cells.erase(std::remove_if(adjacent_cells.begin(), adjacent_cells.end(),
                                                            [&lyt](const auto& ac) { return lyt.is_empty_cell(ac); }),
                                             adjacent_cells.end());
                        // if there is at most one neighbor left
                        if (std::distance(adjacent_cells.cbegin(), adjacent_cells.cend()) <= 1)
                        {
                            // change cell mode to via
                            lyt.assign_cell_mode(c, qca_technology::cell_mode::VERTICAL);
                            // create a corresponding via ground cell
                            const typename Lyt::cell ground_via_cell{c.x, c.y, 0};
                            lyt.assign_cell_type(ground_via_cell, qca_technology::cell_type::NORMAL);
                            lyt.assign_cell_mode(ground_via_cell, qca_technology::cell_mode::VERTICAL);
                        }
                    }
                }
            });
    }

  private:
    template <typename Lyt>
    [[nodiscard]] static port_list determine_port_routing(const Lyt& lyt, const typename Lyt::tile& t) noexcept
    {
        port_list p{};

        // determine incoming connector ports
        if (lyt.has_northern_incoming_signal(t))
            p.inp.emplace(2u, 0u);
        if (lyt.has_eastern_incoming_signal(t))
            p.inp.emplace(4u, 2u);
        if (lyt.has_southern_incoming_signal(t))
            p.inp.emplace(2u, 4u);
        if (lyt.has_western_incoming_signal(t))
            p.inp.emplace(0u, 2u);

        // determine outgoing connector ports
        if (lyt.has_northern_outgoing_signal(t))
            p.out.emplace(2u, 0u);
        if (lyt.has_eastern_outgoing_signal(t))
            p.out.emplace(4u, 2u);
        if (lyt.has_southern_outgoing_signal(t))
            p.out.emplace(2u, 4u);
        if (lyt.has_western_outgoing_signal(t))
            p.out.emplace(0u, 2u);

        // has no connector ports
        if (const auto n = lyt.get_node(t); !lyt.is_wire(n) && !lyt.is_inv(n))
        {
            if (lyt.has_no_incoming_signal(t))
                p.inp.emplace(0u, 2u);

            if (lyt.has_no_outgoing_signal(t))
                p.out.emplace(4u, 2u);
        }

        return p;
    }

    // ************************************************************
    // ************************** Gates ***************************
    // ************************************************************

    static constexpr const fcn_gate straight_inverter{cell_list_to_gate<char>({{{' ', ' ', 'x', ' ', ' '},
                                                                                {' ', 'x', 'x', 'x', ' '},
                                                                                {' ', 'x', ' ', 'x', ' '},
                                                                                {' ', ' ', 'x', ' ', ' '},
                                                                                {' ', ' ', 'x', ' ', ' '}}})};

    static constexpr const fcn_gate bent_inverter{cell_list_to_gate<char>({{{' ', ' ', 'x', ' ', ' '},
                                                                            {' ', ' ', 'x', ' ', ' '},
                                                                            {' ', ' ', ' ', 'x', 'x'},
                                                                            {' ', ' ', ' ', ' ', ' '},
                                                                            {' ', ' ', ' ', ' ', ' '}}})};

    static constexpr const fcn_gate conjunction{cell_list_to_gate<char>({{{' ', ' ', '0', ' ', ' '},
                                                                          {' ', ' ', 'x', ' ', ' '},
                                                                          {'x', 'x', 'x', 'x', 'x'},
                                                                          {' ', ' ', 'x', ' ', ' '},
                                                                          {' ', ' ', 'x', ' ', ' '}}})};

    static constexpr const fcn_gate disjunction{cell_list_to_gate<char>({{{' ', ' ', '1', ' ', ' '},
                                                                          {' ', ' ', 'x', ' ', ' '},
                                                                          {'x', 'x', 'x', 'x', 'x'},
                                                                          {' ', ' ', 'x', ' ', ' '},
                                                                          {' ', ' ', 'x', ' ', ' '}}})};

    static constexpr const fcn_gate majority{cell_list_to_gate<char>({{{' ', ' ', 'x', ' ', ' '},
                                                                       {' ', ' ', 'x', ' ', ' '},
                                                                       {'x', 'x', 'x', 'x', 'x'},
                                                                       {' ', ' ', 'x', ' ', ' '},
                                                                       {' ', ' ', 'x', ' ', ' '}}})};

    static constexpr const fcn_gate fan_out_1_2{cell_list_to_gate<char>({{{' ', ' ', ' ', ' ', ' '},
                                                                          {' ', ' ', ' ', ' ', ' '},
                                                                          {'x', 'x', 'x', 'x', 'x'},
                                                                          {' ', ' ', 'x', ' ', ' '},
                                                                          {' ', ' ', 'x', ' ', ' '}}})};

    static constexpr const fcn_gate fan_out_1_3{cell_list_to_gate<char>({{{' ', ' ', 'x', ' ', ' '},
                                                                          {' ', ' ', 'x', ' ', ' '},
                                                                          {'x', 'x', 'x', 'x', 'x'},
                                                                          {' ', ' ', 'x', ' ', ' '},
                                                                          {' ', ' ', 'x', ' ', ' '}}})};

    // ************************************************************
    // ************************** Wires ***************************
    // ************************************************************

    static constexpr const fcn_gate primary_input_port{cell_list_to_gate<char>({{{' ', ' ', 'x', ' ', ' '},
                                                                                 {' ', ' ', 'x', ' ', ' '},
                                                                                 {' ', ' ', 'i', ' ', ' '},
                                                                                 {' ', ' ', ' ', ' ', ' '},
                                                                                 {' ', ' ', ' ', ' ', ' '}}})};

    static constexpr const fcn_gate primary_output_port{cell_list_to_gate<char>({{{' ', ' ', 'x', ' ', ' '},
                                                                                  {' ', ' ', 'x', ' ', ' '},
                                                                                  {' ', ' ', 'o', ' ', ' '},
                                                                                  {' ', ' ', ' ', ' ', ' '},
                                                                                  {' ', ' ', ' ', ' ', ' '}}})};

    static constexpr const fcn_gate center_wire{cell_list_to_gate<char>({{{' ', ' ', 'x', ' ', ' '},
                                                                          {' ', ' ', 'x', ' ', ' '},
                                                                          {' ', ' ', 'x', ' ', ' '},
                                                                          {' ', ' ', 'x', ' ', ' '},
                                                                          {' ', ' ', 'x', ' ', ' '}}})};

    static constexpr const fcn_gate inner_side_wire{cell_list_to_gate<char>({{{' ', ' ', ' ', 'x', ' '},
                                                                              {' ', ' ', ' ', 'x', ' '},
                                                                              {' ', ' ', ' ', 'x', ' '},
                                                                              {' ', ' ', ' ', 'x', ' '},
                                                                              {' ', ' ', ' ', 'x', ' '}}})};

    static constexpr const fcn_gate outer_side_wire{cell_list_to_gate<char>({{{' ', ' ', ' ', ' ', 'x'},
                                                                              {' ', ' ', ' ', ' ', 'x'},
                                                                              {' ', ' ', ' ', ' ', 'x'},
                                                                              {' ', ' ', ' ', ' ', 'x'},
                                                                              {' ', ' ', ' ', ' ', 'x'}}})};

    static constexpr const fcn_gate center_bent_wire{cell_list_to_gate<char>({{{' ', ' ', 'x', ' ', ' '},
                                                                               {' ', ' ', 'x', ' ', ' '},
                                                                               {' ', ' ', 'x', 'x', 'x'},
                                                                               {' ', ' ', ' ', ' ', ' '},
                                                                               {' ', ' ', ' ', ' ', ' '}}})};

    static constexpr const fcn_gate inner_center_to_inner_center_bent_wire{
        cell_list_to_gate<char>({{{' ', ' ', ' ', 'x', ' '},
                                  {' ', ' ', ' ', 'x', 'x'},
                                  {' ', ' ', ' ', ' ', ' '},
                                  {' ', ' ', ' ', ' ', ' '},
                                  {' ', ' ', ' ', ' ', ' '}}})};

    static constexpr const fcn_gate inner_center_to_center_bent_wire{
        cell_list_to_gate<char>({{{' ', ' ', ' ', 'x', ' '},
                                  {' ', ' ', ' ', 'x', ' '},
                                  {' ', ' ', ' ', 'x', 'x'},
                                  {' ', ' ', ' ', ' ', ' '},
                                  {' ', ' ', ' ', ' ', ' '}}})};

    static constexpr const fcn_gate inner_center_to_outer_center_bent_wire{
        cell_list_to_gate<char>({{{' ', ' ', ' ', 'x', ' '},
                                  {' ', ' ', ' ', 'x', ' '},
                                  {' ', ' ', ' ', 'x', ' '},
                                  {' ', ' ', ' ', 'x', 'x'},
                                  {' ', ' ', ' ', ' ', ' '}}})};

    static constexpr const fcn_gate inner_center_to_outer_side_bent_wire{
        cell_list_to_gate<char>({{{' ', ' ', ' ', 'x', ' '},
                                  {' ', ' ', ' ', 'x', ' '},
                                  {' ', ' ', ' ', 'x', ' '},
                                  {' ', ' ', ' ', 'x', ' '},
                                  {' ', ' ', ' ', 'x', 'x'}}})};

    static constexpr const fcn_gate center_to_inner_center_bent_wire{
        cell_list_to_gate<char>({{{' ', ' ', 'x', ' ', ' '},
                                  {' ', ' ', 'x', 'x', 'x'},
                                  {' ', ' ', ' ', ' ', ' '},
                                  {' ', ' ', ' ', ' ', ' '},
                                  {' ', ' ', ' ', ' ', ' '}}})};

    static constexpr const fcn_gate outer_center_to_center_bent_wire{
        cell_list_to_gate<char>({{{' ', 'x', ' ', ' ', ' '},
                                  {' ', 'x', ' ', ' ', ' '},
                                  {' ', 'x', 'x', 'x', 'x'},
                                  {' ', ' ', ' ', ' ', ' '},
                                  {' ', ' ', ' ', ' ', ' '}}})};

    static constexpr const fcn_gate outer_center_to_outer_center_bent_wire{
        cell_list_to_gate<char>({{{' ', 'x', ' ', ' ', ' '},
                                  {' ', 'x', ' ', ' ', ' '},
                                  {' ', 'x', ' ', ' ', ' '},
                                  {' ', 'x', 'x', 'x', 'x'},
                                  {' ', ' ', ' ', ' ', ' '}}})};

    static constexpr const fcn_gate outer_side_to_outer_side_bent_wire{
        cell_list_to_gate<char>({{{'x', ' ', ' ', ' ', ' '},
                                  {'x', ' ', ' ', ' ', ' '},
                                  {'x', ' ', ' ', ' ', ' '},
                                  {'x', ' ', ' ', ' ', ' '},
                                  {'x', 'x', 'x', 'x', 'x'}}})};

    using port_gate_map = std::unordered_map<port_list, fcn_gate>;
    /**
     * Lookup table for wire rotations. Maps ports to corresponding wires.
     */
    static inline const port_gate_map wire_map = {
        // primary inputs
        {{{}, {port(2, 0)}}, primary_input_port},
        {{{}, {port(4, 2)}}, rotate_90(primary_input_port)},
        {{{}, {port(2, 4)}}, rotate_180(primary_input_port)},
        {{{}, {port(0, 2)}}, rotate_270(primary_input_port)},
        // primary outputs
        {{{port(2, 0)}, {}}, primary_output_port},
        {{{port(4, 2)}, {}}, rotate_90(primary_output_port)},
        {{{port(2, 4)}, {}}, rotate_180(primary_output_port)},
        {{{port(0, 2)}, {}}, rotate_270(primary_output_port)},
        // center wire
        {{{port(2, 0)}, {port(2, 4)}}, center_wire},
        {{{port(2, 4)}, {port(2, 0)}}, center_wire},
        {{{port(0, 2)}, {port(4, 2)}}, rotate_90(center_wire)},
        {{{port(4, 2)}, {port(0, 2)}}, rotate_90(center_wire)},
        // inner side wire
        {{{port(3, 0)}, {port(3, 4)}}, inner_side_wire},
        {{{port(3, 4)}, {port(3, 0)}}, inner_side_wire},
        {{{port(0, 3)}, {port(4, 3)}}, rotate_90(inner_side_wire)},
        {{{port(4, 3)}, {port(0, 3)}}, rotate_90(inner_side_wire)},
        {{{port(1, 0)}, {port(1, 4)}}, rotate_180(inner_side_wire)},
        {{{port(1, 4)}, {port(1, 0)}}, rotate_180(inner_side_wire)},
        {{{port(0, 1)}, {port(4, 1)}}, rotate_270(inner_side_wire)},
        {{{port(4, 1)}, {port(0, 1)}}, rotate_270(inner_side_wire)},
        // outer side wire
        {{{port(4, 0)}, {port(4, 4)}}, outer_side_wire},
        {{{port(4, 4)}, {port(4, 0)}}, outer_side_wire},
        {{{port(0, 4)}, {port(4, 4)}}, rotate_90(outer_side_wire)},
        {{{port(4, 4)}, {port(0, 4)}}, rotate_90(outer_side_wire)},
        {{{port(0, 0)}, {port(0, 4)}}, rotate_180(outer_side_wire)},
        {{{port(0, 4)}, {port(0, 0)}}, rotate_180(outer_side_wire)},
        {{{port(0, 0)}, {port(4, 0)}}, rotate_270(outer_side_wire)},
        {{{port(4, 0)}, {port(0, 0)}}, rotate_270(outer_side_wire)},
        // center bent wire
        {{{port(2, 0)}, {port(4, 2)}}, center_bent_wire},
        {{{port(4, 2)}, {port(2, 0)}}, center_bent_wire},
        {{{port(4, 2)}, {port(2, 4)}}, rotate_90(center_bent_wire)},
        {{{port(2, 4)}, {port(4, 2)}}, rotate_90(center_bent_wire)},
        {{{port(0, 2)}, {port(2, 4)}}, rotate_180(center_bent_wire)},
        {{{port(2, 4)}, {port(0, 2)}}, rotate_180(center_bent_wire)},
        {{{port(2, 0)}, {port(0, 2)}}, rotate_270(center_bent_wire)},
        {{{port(0, 2)}, {port(2, 0)}}, rotate_270(center_bent_wire)}
        // TODO more wires go here!
    };
    /**
     * Lookup table for inverter rotations. Maps ports to corresponding inverters.
     */
    static inline const port_gate_map inverter_map = {// straight inverters
                                                      {{{port(2, 0)}, {port(2, 4)}}, straight_inverter},
                                                      {{{port(4, 2)}, {port(0, 2)}}, rotate_90(straight_inverter)},
                                                      {{{port(2, 4)}, {port(2, 0)}}, rotate_180(straight_inverter)},
                                                      {{{port(0, 2)}, {port(4, 2)}}, rotate_270(straight_inverter)},
                                                      // without outputs
                                                      {{{port(2, 0)}, {}}, straight_inverter},
                                                      {{{port(4, 2)}, {}}, rotate_90(straight_inverter)},
                                                      {{{port(2, 4)}, {}}, rotate_180(straight_inverter)},
                                                      {{{port(0, 2)}, {}}, rotate_270(straight_inverter)},
                                                      // without inputs
                                                      {{{}, {port(2, 4)}}, straight_inverter},
                                                      {{{}, {port(0, 2)}}, rotate_90(straight_inverter)},
                                                      {{{}, {port(2, 0)}}, rotate_180(straight_inverter)},
                                                      {{{}, {port(4, 2)}}, rotate_270(straight_inverter)},
                                                      // bent inverters
                                                      {{{port(2, 0)}, {port(4, 2)}}, bent_inverter},
                                                      {{{port(4, 2)}, {port(2, 0)}}, bent_inverter},
                                                      {{{port(4, 2)}, {port(2, 4)}}, rotate_90(bent_inverter)},
                                                      {{{port(2, 4)}, {port(4, 2)}}, rotate_90(bent_inverter)},
                                                      {{{port(0, 2)}, {port(2, 4)}}, rotate_180(bent_inverter)},
                                                      {{{port(2, 4)}, {port(0, 2)}}, rotate_180(bent_inverter)},
                                                      {{{port(2, 0)}, {port(0, 2)}}, rotate_270(bent_inverter)},
                                                      {{{port(0, 2)}, {port(2, 0)}}, rotate_270(bent_inverter)}};
    /**
     * Lookup table for conjunction rotations. Maps ports to corresponding AND gates.
     */
    static inline const port_gate_map conjunction_map = {
        {{{port(0, 2), port(2, 4)}, {port(4, 2)}}, conjunction},
        {{{port(0, 2), port(4, 2)}, {port(2, 4)}}, conjunction},
        {{{port(2, 4), port(4, 2)}, {port(0, 2)}}, conjunction},

        {{{port(0, 2), port(2, 4)}, {port(2, 0)}}, rotate_90(conjunction)},
        {{{port(0, 2), port(2, 0)}, {port(2, 4)}}, rotate_90(conjunction)},
        {{{port(2, 4), port(2, 0)}, {port(0, 2)}}, rotate_90(conjunction)},

        {{{port(0, 2), port(4, 2)}, {port(2, 0)}}, rotate_180(conjunction)},
        {{{port(0, 2), port(2, 0)}, {port(4, 2)}}, rotate_180(conjunction)},
        {{{port(4, 2), port(2, 0)}, {port(0, 2)}}, rotate_180(conjunction)},

        {{{port(2, 4), port(4, 2)}, {port(2, 0)}}, rotate_270(conjunction)},
        {{{port(2, 4), port(2, 0)}, {port(4, 2)}}, rotate_270(conjunction)},
        {{{port(4, 2), port(2, 0)}, {port(2, 4)}}, rotate_270(conjunction)}};
    /**
     * Lookup table for disjunction rotations. Maps ports to corresponding OR gates.
     */
    static inline const port_gate_map disjunction_map = {
        {{{port(0, 2), port(2, 4)}, {port(4, 2)}}, disjunction},
        {{{port(0, 2), port(4, 2)}, {port(2, 4)}}, disjunction},
        {{{port(2, 4), port(4, 2)}, {port(0, 2)}}, disjunction},

        {{{port(0, 2), port(2, 4)}, {port(2, 0)}}, rotate_90(disjunction)},
        {{{port(0, 2), port(2, 0)}, {port(2, 4)}}, rotate_90(disjunction)},
        {{{port(2, 4), port(2, 0)}, {port(0, 2)}}, rotate_90(disjunction)},

        {{{port(0, 2), port(4, 2)}, {port(2, 0)}}, rotate_180(disjunction)},
        {{{port(0, 2), port(2, 0)}, {port(4, 2)}}, rotate_180(disjunction)},
        {{{port(4, 2), port(2, 0)}, {port(0, 2)}}, rotate_180(disjunction)},

        {{{port(2, 4), port(4, 2)}, {port(2, 0)}}, rotate_270(disjunction)},
        {{{port(2, 4), port(2, 0)}, {port(4, 2)}}, rotate_270(disjunction)},
        {{{port(4, 2), port(2, 0)}, {port(2, 4)}}, rotate_270(disjunction)}};
    /**
     * Lookup table for fan-out rotations. Maps ports to corresponding fan-out gates.
     */
    static inline const port_gate_map fanout_map = {
        {{{port(4, 2)}, {port(0, 2), port(2, 4)}}, fan_out_1_2},
        {{{port(2, 4)}, {port(0, 2), port(4, 2)}}, fan_out_1_2},
        {{{port(0, 2)}, {port(2, 4), port(4, 2)}}, fan_out_1_2},

        {{{port(2, 0)}, {port(0, 2), port(2, 4)}}, rotate_90(fan_out_1_2)},
        {{{port(2, 4)}, {port(0, 2), port(2, 0)}}, rotate_90(fan_out_1_2)},
        {{{port(0, 2)}, {port(2, 4), port(2, 0)}}, rotate_90(fan_out_1_2)},

        {{{port(2, 0)}, {port(0, 2), port(4, 2)}}, rotate_180(fan_out_1_2)},
        {{{port(4, 2)}, {port(0, 2), port(2, 0)}}, rotate_180(fan_out_1_2)},
        {{{port(0, 2)}, {port(4, 2), port(2, 0)}}, rotate_180(fan_out_1_2)},

        {{{port(2, 0)}, {port(2, 4), port(4, 2)}}, rotate_270(fan_out_1_2)},
        {{{port(4, 2)}, {port(2, 4), port(2, 0)}}, rotate_270(fan_out_1_2)},
        {{{port(2, 4)}, {port(4, 2), port(2, 0)}}, rotate_270(fan_out_1_2)}};
};

}  // namespace fiction

#endif  // FICTION_QCA_ONE_LIBRARY_H
