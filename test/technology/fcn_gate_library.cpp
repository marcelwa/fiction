//
// Created by marcel on 03.07.21.
//

#include "catch.hpp"

#include <fiction/technology/cell_ports.hpp>
#include <fiction/technology/fcn_gate_library.hpp>

#include <type_traits>

using namespace fiction;

TEST_CASE("Construction & traits", "[fcn-gate-library]")
{
    using lib_t = fcn_gate_library<qca_technology, 2, 3>;

    CHECK(std::is_same_v<lib_t::technology, qca_technology>);
    CHECK(lib_t::gate_x_size() == 2);
    CHECK(lib_t::gate_y_size() == 3);
}

TEST_CASE("Gate rotation", "[fcn-gate-library]")
{
    using lib_t = fcn_gate_library<qca_technology, 3, 3>;

    constexpr const typename lib_t::fcn_gate wire{
        lib_t::cell_list_to_gate<char>({{{' ', 'x', ' '},
                                         {' ', 'x', 'x'},
                                         {' ', ' ', ' '}}})};

    constexpr const typename lib_t::fcn_gate wire_rotated_90{
        lib_t::cell_list_to_gate<char>({{{' ', ' ', ' '},
                                         {' ', 'x', 'x'},
                                         {' ', 'x', ' '}}})};

    constexpr const typename lib_t::fcn_gate wire_rotated_180{
        lib_t::cell_list_to_gate<char>({{{' ', ' ', ' '},
                                         {'x', 'x', ' '},
                                         {' ', 'x', ' '}}})};

    constexpr const typename lib_t::fcn_gate wire_rotated_270{
        lib_t::cell_list_to_gate<char>({{{' ', 'x', ' '},
                                         {'x', 'x', ' '},
                                         {' ', ' ', ' '}}})};

    CHECK(lib_t::rotate_90(wire) == wire_rotated_90);
    CHECK(lib_t::rotate_180(wire) == wire_rotated_180);
    CHECK(lib_t::rotate_270(wire) == wire_rotated_270);

    CHECK(lib_t::rotate_90(lib_t::rotate_90(wire)) == wire_rotated_180);
    CHECK(lib_t::rotate_90(lib_t::rotate_90(lib_t::rotate_90(wire))) == wire_rotated_270);
    CHECK(lib_t::rotate_90(lib_t::rotate_90(lib_t::rotate_90(lib_t::rotate_90(wire)))) == wire);

    CHECK(lib_t::rotate_180(lib_t::rotate_180(wire)) == wire);
    CHECK(lib_t::rotate_270(lib_t::rotate_270(lib_t::rotate_270(lib_t::rotate_270(wire)))) == wire);
}

TEST_CASE("Gate merging", "[fcn-gate-library]")
{
    using lib_t = fcn_gate_library<qca_technology, 3, 3>;

    constexpr const typename lib_t::fcn_gate wire{
        lib_t::cell_list_to_gate<char>({{{' ', 'x', ' '},
                                         {' ', 'x', 'x'},
                                         {' ', ' ', ' '}}})};

    constexpr const typename lib_t::fcn_gate wire_rotated_90{
        lib_t::cell_list_to_gate<char>({{{' ', ' ', ' '},
                                         {' ', 'x', 'x'},
                                         {' ', 'x', ' '}}})};

    constexpr const typename lib_t::fcn_gate wire_rotated_180{
        lib_t::cell_list_to_gate<char>({{{' ', ' ', ' '},
                                         {'x', 'x', ' '},
                                         {' ', 'x', ' '}}})};

    constexpr const typename lib_t::fcn_gate wire_rotated_270{
        lib_t::cell_list_to_gate<char>({{{' ', 'x', ' '},
                                         {'x', 'x', ' '},
                                         {' ', ' ', ' '}}})};

    constexpr const typename lib_t::fcn_gate merged_wire{
        lib_t::cell_list_to_gate<char>({{{' ', 'x', ' '},
                                         {'x', 'x', 'x'},
                                         {' ', 'x', ' '}}})};

    CHECK(lib_t::merge({{wire, wire_rotated_90, wire_rotated_180, wire_rotated_270}}) == merged_wire);

    constexpr const typename lib_t::fcn_gate pi_pin{
        lib_t::cell_list_to_gate<char>({{{' ', ' ', ' '},
                                         {'i', 'x', ' '},
                                         {' ', ' ', ' '}}})};

    constexpr const typename lib_t::fcn_gate po_pin{
        lib_t::cell_list_to_gate<char>({{{' ', ' ', ' '},
                                         {' ', 'x', 'o'},
                                         {' ', ' ', ' '}}})};

    constexpr const typename lib_t::fcn_gate io_pin{
        lib_t::cell_list_to_gate<char>({{{' ', ' ', ' '},
                                         {'i', 'x', 'o'},
                                         {' ', ' ', ' '}}})};

    CHECK(lib_t::merge({{pi_pin, po_pin}}) == io_pin);
}

TEST_CASE("Cell marking", "[fcn-gate-library]")
{
    using lib_t = fcn_gate_library<qca_technology, 3, 3>;

    constexpr const typename lib_t::fcn_gate empty{
        lib_t::cell_list_to_gate<char>({{{' ', ' ', ' '},
                                         {' ', ' ', ' '},
                                         {' ', ' ', ' '}}})};

    constexpr const typename lib_t::fcn_gate wire{
        lib_t::cell_list_to_gate<char>({{{' ', 'x', ' '},
                                         {' ', 'x', ' '},
                                         {' ', 'x', ' '}}})};

    constexpr const typename lib_t::fcn_gate pi_wire{
        lib_t::cell_list_to_gate<char>({{{' ', 'i', ' '},
                                         {' ', 'x', ' '},
                                         {' ', 'x', ' '}}})};

    constexpr const typename lib_t::fcn_gate io_wire{
        lib_t::cell_list_to_gate<char>({{{' ', 'i', ' '},
                                         {' ', 'x', ' '},
                                         {' ', 'o', ' '}}})};

    CHECK(lib_t::mark_cell(wire, port(1, 0), qca_technology::cell_mark::INPUT) == pi_wire);
    CHECK(lib_t::mark_cell(pi_wire, port(1, 2), qca_technology::cell_mark::OUTPUT) == io_wire);
    CHECK(lib_t::mark_cell(lib_t::mark_cell(wire, port(1, 0), qca_technology::cell_mark::INPUT), port(1, 2),
                           qca_technology::cell_mark::OUTPUT) == io_wire);
    CHECK(lib_t::mark_cell(lib_t::mark_cell(lib_t::mark_cell(wire, port(1, 0), qca_technology::cell_mark::EMPTY),
                                            port(1, 1), qca_technology::cell_mark::EMPTY), port(1, 2),
                           qca_technology::cell_mark::EMPTY) == empty);
}
