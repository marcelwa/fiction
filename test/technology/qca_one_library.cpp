//
// Created by marcel on 03.07.21.
//

#include "../utils/blueprints/layout_blueprints.hpp"
#include "catch.hpp"

#include <fiction/layouts/clocked_layout.hpp>
#include <fiction/layouts/gate_level_layout.hpp>
#include <fiction/layouts/tile_based_layout.hpp>
#include <fiction/technology/qca_one_library.hpp>

using namespace fiction;

TEST_CASE("Setting up input ports and gates", "[qca-one]")
{
    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout>>;

    auto layout = blueprints::or_not_gate_layout<gate_layout>();

    static constexpr const qca_one_library::fcn_gate primary_input_port{
        qca_one_library::cell_list_to_gate<char>({{{' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'i', ' ', ' '},
                                                   {' ', ' ', ' ', ' ', ' '},
                                                   {' ', ' ', ' ', ' ', ' '}}})};

    static constexpr const qca_one_library::fcn_gate primary_output_port{
        qca_one_library::cell_list_to_gate<char>({{{' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'o', ' ', ' '},
                                                   {' ', ' ', ' ', ' ', ' '},
                                                   {' ', ' ', ' ', ' ', ' '}}})};

    static constexpr const qca_one_library::fcn_gate disjunction{
        qca_one_library::cell_list_to_gate<char>({{{' ', ' ', '1', ' ', ' '},
                                                   {' ', ' ', 'x', ' ', ' '},
                                                   {'x', 'x', 'x', 'x', 'x'},
                                                   {' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'x', ' ', ' '}}})};

    static constexpr const qca_one_library::fcn_gate bent_inverter{
        qca_one_library::cell_list_to_gate<char>({{{' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', ' ', 'x', 'x'},
                                                   {' ', ' ', ' ', ' ', ' '},
                                                   {' ', ' ', ' ', ' ', ' '}}})};

    CHECK(qca_one_library::set_up_gate(layout, {0, 1}) == qca_one_library::rotate_90(primary_input_port));
    CHECK(qca_one_library::set_up_gate(layout, {1, 0}) == qca_one_library::rotate_180(primary_input_port));
    CHECK(qca_one_library::set_up_gate(layout, {1, 1}) == qca_one_library::rotate_90(disjunction));
    CHECK(qca_one_library::set_up_gate(layout, {1, 2}) == bent_inverter);
    CHECK(qca_one_library::set_up_gate(layout, {2, 2}) == qca_one_library::rotate_270(primary_output_port));
}

TEST_CASE("Setting up wires", "[qca-one]")
{
    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout>>;

    auto layout = blueprints::crossing_layout<gate_layout>();

    static constexpr const qca_one_library::fcn_gate primary_input_port{
        qca_one_library::cell_list_to_gate<char>({{{' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'i', ' ', ' '},
                                                   {' ', ' ', ' ', ' ', ' '},
                                                   {' ', ' ', ' ', ' ', ' '}}})};

    static constexpr const qca_one_library::fcn_gate primary_output_port{
        qca_one_library::cell_list_to_gate<char>({{{' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'o', ' ', ' '},
                                                   {' ', ' ', ' ', ' ', ' '},
                                                   {' ', ' ', ' ', ' ', ' '}}})};

    static constexpr const qca_one_library::fcn_gate conjunction{
        qca_one_library::cell_list_to_gate<char>({{{' ', ' ', '0', ' ', ' '},
                                                   {' ', ' ', 'x', ' ', ' '},
                                                   {'x', 'x', 'x', 'x', 'x'},
                                                   {' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'x', ' ', ' '}}})};

    static constexpr const qca_one_library::fcn_gate wire{
        qca_one_library::cell_list_to_gate<char>({{{' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'x', ' ', ' '}}})};

    CHECK(qca_one_library::set_up_gate(layout, {0, 1}) == qca_one_library::rotate_90(primary_input_port));
    CHECK(qca_one_library::set_up_gate(layout, {0, 2}) == qca_one_library::rotate_90(primary_input_port));
    CHECK(qca_one_library::set_up_gate(layout, {1, 0}) == qca_one_library::rotate_180(primary_input_port));
    CHECK(qca_one_library::set_up_gate(layout, {2, 0}) == qca_one_library::rotate_180(primary_input_port));
    CHECK(qca_one_library::set_up_gate(layout, {1, 1}) == qca_one_library::rotate_180(conjunction));
    CHECK(qca_one_library::set_up_gate(layout, {2, 2}) == qca_one_library::rotate_180(conjunction));
    CHECK(qca_one_library::set_up_gate(layout, {2, 1}) == wire);
    CHECK(qca_one_library::set_up_gate(layout, {1, 2}) == qca_one_library::rotate_90(wire));
    CHECK(qca_one_library::set_up_gate(layout, {2, 1, 1}) == qca_one_library::rotate_90(wire));
    CHECK(qca_one_library::set_up_gate(layout, {3, 1}) == qca_one_library::rotate_270(primary_output_port));
    CHECK(qca_one_library::set_up_gate(layout, {3, 2}) == qca_one_library::rotate_270(primary_output_port));
}

TEST_CASE("Setting up fanouts", "[qca-one]")
{
    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout>>;

    auto layout = blueprints::fanout_layout<gate_layout>();

    static constexpr const qca_one_library::fcn_gate primary_input_port{
        qca_one_library::cell_list_to_gate<char>({{{' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'i', ' ', ' '},
                                                   {' ', ' ', ' ', ' ', ' '},
                                                   {' ', ' ', ' ', ' ', ' '}}})};

    static constexpr const qca_one_library::fcn_gate primary_output_port{
        qca_one_library::cell_list_to_gate<char>({{{' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'o', ' ', ' '},
                                                   {' ', ' ', ' ', ' ', ' '},
                                                   {' ', ' ', ' ', ' ', ' '}}})};

    static constexpr const qca_one_library::fcn_gate fanout{
        qca_one_library::cell_list_to_gate<char>({{{' ', ' ', ' ', ' ', ' '},
                                                   {' ', ' ', ' ', ' ', ' '},
                                                   {'x', 'x', 'x', 'x', 'x'},
                                                   {' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'x', ' ', ' '}}})};

    static constexpr const qca_one_library::fcn_gate bent_wire{
        qca_one_library::cell_list_to_gate<char>({{{' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'x', ' ', ' '},
                                                   {' ', ' ', 'x', 'x', 'x'},
                                                   {' ', ' ', ' ', ' ', ' '},
                                                   {' ', ' ', ' ', ' ', ' '}}})};

    CHECK(qca_one_library::set_up_gate(layout, {0, 1}) == qca_one_library::rotate_90(primary_input_port));
    CHECK(qca_one_library::set_up_gate(layout, {1, 0}) == qca_one_library::rotate_180(primary_output_port));
    CHECK(qca_one_library::set_up_gate(layout, {2, 0}) == qca_one_library::rotate_180(primary_output_port));
    CHECK(qca_one_library::set_up_gate(layout, {1, 2}) == qca_one_library::rotate_90(primary_output_port));
    CHECK(qca_one_library::set_up_gate(layout, {1, 1}) == qca_one_library::rotate_180(fanout));
    CHECK(qca_one_library::set_up_gate(layout, {2, 1}) == qca_one_library::rotate_90(fanout));
    CHECK(qca_one_library::set_up_gate(layout, {2, 2}) == qca_one_library::rotate_270(bent_wire));
}
