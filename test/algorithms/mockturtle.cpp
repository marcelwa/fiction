//
// Created by marcel on 17.05.21.
//

#include "catch.hpp"
#include "utils/blueprints/layout_blueprints.hpp"

#include <fiction/layouts/clocked_layout.hpp>
#include <fiction/layouts/gate_level_layout.hpp>
#include <fiction/layouts/tile_based_layout.hpp>

#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <mockturtle/algorithms/simulation.hpp>
#include <mockturtle/traits.hpp>

#include <type_traits>

using namespace fiction;

TEST_CASE("Simulation", "[mockturtle]")
{
    // adapted from mockturtle/test/networks/klut.cpp

    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout>>;

    REQUIRE(mockturtle::has_compute_v<gate_layout, kitty::dynamic_truth_table>);

    auto and_or_layout   = blueprints::and_or_gate_layout<gate_layout>();
    auto xor_maj_layout  = blueprints::xor_maj_gate_layout<gate_layout>();
    auto crossing_layout = blueprints::crossing_layout<gate_layout>();

    kitty::dynamic_truth_table tt_maj(3u), tt_and(2u), tt_or(2u);
    kitty::create_from_hex_string(tt_maj, "e8");
    kitty::create_from_hex_string(tt_and, "8");
    kitty::create_from_hex_string(tt_or, "e");

    auto and_or_tts = mockturtle::simulate<kitty::dynamic_truth_table>(
        and_or_layout,
        mockturtle::default_simulator<kitty::dynamic_truth_table>(static_cast<unsigned>(and_or_layout.num_pis())));

    REQUIRE(and_or_tts.size() == 2);
    CHECK(and_or_tts[0] == tt_and);
    CHECK(and_or_tts[1] == tt_or);

    auto xor_maj_tts = mockturtle::simulate<kitty::dynamic_truth_table>(
        xor_maj_layout,
        mockturtle::default_simulator<kitty::dynamic_truth_table>(static_cast<unsigned>(xor_maj_layout.num_pis())));

    kitty::dynamic_truth_table tt_3xor2(3u);
    kitty::create_from_hex_string(tt_3xor2, "66");  // 2-input XOR in three variables (1 xor 2)

    REQUIRE(xor_maj_tts.size() == 2);
    CHECK(xor_maj_tts[0] == tt_maj);
    CHECK(xor_maj_tts[1] == tt_3xor2);

    auto crossing_tts = mockturtle::simulate<kitty::dynamic_truth_table>(
        crossing_layout,
        mockturtle::default_simulator<kitty::dynamic_truth_table>(static_cast<unsigned>(crossing_layout.num_pis())));

    kitty::dynamic_truth_table tt_and12(4u), tt_and34(4u);

    kitty::create_from_hex_string(tt_and12, "8888");  // 2-input AND in four variables (1 and 2)
    kitty::create_from_hex_string(tt_and34, "f000");  // 2-input AND in four variables (3 and 4)

    REQUIRE(crossing_tts.size() == 2);
    CHECK(crossing_tts[0] == tt_and12);
    CHECK(crossing_tts[1] == tt_and34);
}
