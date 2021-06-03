//
// Created by marcel on 19.05.21.
//

#include "../utils/blueprints/layout_blueprints.hpp"
#include "catch.hpp"

#include <fiction/io/print_layout.hpp>
#include <fiction/layouts/clocked_layout.hpp>
#include <fiction/layouts/gate_level_layout.hpp>
#include <fiction/layouts/tile_based_layout.hpp>

#include <sstream>

using namespace fiction;

TEST_CASE("Print simple layout", "[gate-level]")
{
    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout>>;

    gate_layout layout{tile_based_layout::aspect_ratio{3, 1, 0}, open_4_clocking};

    const auto x1 = layout.create_pi("x1", {2, 0});
    const auto x2 = layout.create_pi("x2", {1, 1});
    const auto a1 = layout.create_and(x1, x2, {1, 0});
    const auto n1 = layout.create_not(x2, {2, 1});
    layout.create_po(a1, "f1", {0, 0});
    layout.create_po(n1, "f2", {3, 1});

    SECTION("No clocking")
    {
        const auto layout_print = "        \n"
                                  "O & I ▢ \n"
                                  "        \n"
                                  "▢ I ¬ O \n"
                                  "\n";

        std::stringstream print_stream{};

        print_gate_level_layout(print_stream, layout, false, false);

        CHECK(layout_print == print_stream.str());
    }

    SECTION("Irregular clocking")
    {
        layout.assign_clock_number({2, 0}, static_cast<typename gate_layout::clock_number_t>(0));
        layout.assign_clock_number({1, 0}, static_cast<typename gate_layout::clock_number_t>(1));
        layout.assign_clock_number({0, 0}, static_cast<typename gate_layout::clock_number_t>(2));
        layout.assign_clock_number({1, 1}, static_cast<typename gate_layout::clock_number_t>(0));
        layout.assign_clock_number({2, 1}, static_cast<typename gate_layout::clock_number_t>(1));
        layout.assign_clock_number({3, 1}, static_cast<typename gate_layout::clock_number_t>(2));

        const auto layout_print = "        \n"
                                  "O←&←I ▢ \n"
                                  "  ↑     \n"
                                  "▢ Y→¬→O \n"
                                  "\n";

        std::stringstream print_stream{};

        print_gate_level_layout(print_stream, layout, false, false);

        CHECK(layout_print == print_stream.str());
    }
}

TEST_CASE("Print crossing layout", "[gate-level]")
{
    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout>>;

    auto layout = blueprints::crossing_layout<gate_layout>();

    const auto layout_print = "        \n"
                              "▢ I I ▢ \n"
                              "  ↓ ↓   \n"
                              "I→&→+→O \n"
                              "    ↓   \n"
                              "I→=→&→O \n"
                              "\n";

    std::stringstream print_stream{};

    print_gate_level_layout(print_stream, layout, false, false);

    CHECK(layout_print == print_stream.str());
}
