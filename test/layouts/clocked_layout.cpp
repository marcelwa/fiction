//
// Created by marcel on 31.03.21.
//

#include "catch.hpp"

#include <fiction/layouts/clocked_layout.hpp>
#include <fiction/layouts/tile_based_layout.hpp>

#include <set>

using namespace fiction;

TEST_CASE("Clocking", "[clocked]")
{
    clocked_layout<tile_based_layout> layout{tile_based_layout::aspect_ratio{2, 2, 0}, twoddwave_4_clocking};

    CHECK(layout.is_clocking_scheme(clock_name::twoddwave4));
    CHECK(layout.is_regularly_clocked());
    CHECK(layout.num_clocks() == 4);

    CHECK(layout.get_clock_number({0, 0}) == 0);
    CHECK(layout.get_clock_number({1, 0}) == 1);
    CHECK(layout.get_clock_number({0, 1}) == 1);
    CHECK(layout.get_clock_number({1, 1}) == 2);

    CHECK(layout.is_incoming_clocked({1, 0}, {0, 0}));
    CHECK(layout.is_incoming_clocked({0, 1}, {0, 0}));
    CHECK(layout.is_incoming_clocked({1, 1}, {0, 1}));
    CHECK(layout.is_incoming_clocked({1, 1}, {1, 0}));
    CHECK(!layout.is_incoming_clocked({1, 1}, {0, 0}));
    CHECK(!layout.is_incoming_clocked({1, 1}, {1, 1}));

    CHECK(layout.is_outgoing_clocked({0, 0}, {1, 0}));
    CHECK(layout.is_outgoing_clocked({0, 0}, {0, 1}));
    CHECK(layout.is_outgoing_clocked({0, 1}, {1, 1}));
    CHECK(layout.is_outgoing_clocked({1, 0}, {1, 1}));
    CHECK(!layout.is_outgoing_clocked({0, 0}, {1, 1}));
    CHECK(!layout.is_outgoing_clocked({1, 1}, {1, 1}));

    layout.assign_clock_number({1, 0}, 2);
    layout.assign_clock_number({0, 1}, 2);
    layout.assign_clock_number({1, 1}, 3);

    CHECK(layout.get_clock_number({0, 0}) == 0);
    CHECK(layout.get_clock_number({1, 0}) == 2);
    CHECK(layout.get_clock_number({0, 1}) == 2);
    CHECK(layout.get_clock_number({1, 1}) == 3);
}

TEST_CASE("Iteration", "[clocked]")
{
    clocked_layout<tile_based_layout> layout{tile_based_layout::aspect_ratio{2, 2, 0}, twoddwave_4_clocking};

    CHECK(layout.incoming_clocked_zones<std::set<tile_based_layout::tile>>({0, 0}).empty());
    CHECK(layout.outgoing_clocked_zones<std::set<tile_based_layout::tile>>({2, 2}).empty());

    auto s1 = layout.incoming_clocked_zones<std::set<tile_based_layout::tile>>({1, 1});
    auto s2 = std::set<tile_based_layout::tile>{{{1, 0}, {0, 1}}};

    CHECK(s1 == s2);

    auto s3 = layout.outgoing_clocked_zones<std::set<tile_based_layout::tile>>({1, 1});
    auto s4 = std::set<tile_based_layout::tile>{{{1, 2}, {2, 1}}};

    CHECK(s3 == s4);
}

TEST_CASE("Structural properties", "[clocked]")
{
    clocked_layout<tile_based_layout> layout{tile_based_layout::aspect_ratio{2, 2, 0}, twoddwave_4_clocking};

    CHECK(layout.in_degree({0, 0}) == static_cast<clocked_layout<tile_based_layout>::degree_t>(0));
    CHECK(layout.in_degree({1, 0}) == static_cast<clocked_layout<tile_based_layout>::degree_t>(1));
    CHECK(layout.in_degree({2, 0}) == static_cast<clocked_layout<tile_based_layout>::degree_t>(1));
    CHECK(layout.in_degree({1, 1}) == static_cast<clocked_layout<tile_based_layout>::degree_t>(2));

    CHECK(layout.out_degree({1, 1}) == static_cast<clocked_layout<tile_based_layout>::degree_t>(2));
    CHECK(layout.out_degree({0, 2}) == static_cast<clocked_layout<tile_based_layout>::degree_t>(1));
    CHECK(layout.out_degree({1, 2}) == static_cast<clocked_layout<tile_based_layout>::degree_t>(1));
    CHECK(layout.out_degree({2, 2}) == static_cast<clocked_layout<tile_based_layout>::degree_t>(0));

    CHECK(layout.degree({0, 0}) == static_cast<clocked_layout<tile_based_layout>::degree_t>(2));
    CHECK(layout.degree({1, 0}) == static_cast<clocked_layout<tile_based_layout>::degree_t>(3));
    CHECK(layout.degree({2, 0}) == static_cast<clocked_layout<tile_based_layout>::degree_t>(2));
    CHECK(layout.degree({1, 1}) == static_cast<clocked_layout<tile_based_layout>::degree_t>(4));
    CHECK(layout.degree({0, 2}) == static_cast<clocked_layout<tile_based_layout>::degree_t>(2));
    CHECK(layout.degree({1, 2}) == static_cast<clocked_layout<tile_based_layout>::degree_t>(3));
    CHECK(layout.degree({2, 2}) == static_cast<clocked_layout<tile_based_layout>::degree_t>(2));
}
