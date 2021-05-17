//
// Created by marcel on 31.03.21.
//

#include "tile_based_layout.hpp"

#include "catch.hpp"

#include <map>
#include <set>
#include <sstream>

using namespace fiction;

TEST_CASE("Tiles", "[tile-based")
{
    auto td = tile_based_layout::tile{};
    CHECK(td.is_dead());

    auto t0 = tile_based_layout::tile{0, 0, 0};
    CHECK(!t0.is_dead());

    CHECK(t0 != td);

    auto t1 = tile_based_layout::tile{1, 2, 0};
    auto t2 = tile_based_layout::tile{1, 2};

    CHECK(t0 < t1);
    CHECK(t1 > t0);
    CHECK(t1 >= t0);
    CHECK(t0 <= t1);
    CHECK(t1 == t2);
    CHECK(t2 == t1);

    t1.z += static_cast<uint64_t>(4ul);

    CHECK(t1 == t2);

    t1.y += static_cast<uint64_t>(2147483648ul);

    CHECK(t1 == t2);

    t1.x += static_cast<uint64_t>(2147483648ul);

    CHECK(t1 == t2);

    t1.x++;

    CHECK(t1 != t2);
    CHECK(t1 > t2);
    CHECK(t1 >= t2);
    CHECK(t2 < t1);
    CHECK(t2 <= t1);

    auto t3 = tile_based_layout::tile{0, 0, 1};

    CHECK(t1 < t3);
    CHECK(t2 < t3);

    std::map<uint64_t, tile_based_layout::tile> tile_repr{
        {0x8000000000000000, tile_based_layout::tile{}},
        {0x0000000000000000, tile_based_layout::tile{0, 0, 0}},
        {0x4000000000000000, tile_based_layout::tile{0, 0, 1}},
        {0x4000000080000001, tile_based_layout::tile{1, 1, 1}},
        {0x3fffffffffffffff, tile_based_layout::tile{2147483647, 2147483647, 0}}};

    for (auto [repr, tile] : tile_repr)
    {
        CHECK(static_cast<tile_based_layout::tile>(repr) == tile);
        CHECK(repr == static_cast<uint64_t>(tile));
        CHECK(tile_based_layout::tile{repr} == tile);
    }

    std::ostringstream os{};
    os << tile_based_layout::tile{3, 2, 1};
    CHECK(os.str() == "(3,2,1)");
}

TEST_CASE("Tile iteration", "[tile-based]")
{
    tile_based_layout::aspect_ratio ar{9, 9, 1};

    tile_based_layout layout{ar};

    std::set<tile_based_layout::tile> visited{};

    const auto check1 = [&visited, &ar](const auto& t)
    {
        CHECK(t <= ar);
        //        CHECK(layout.is_empty_tile(t));

        // no tile is visited twice
        CHECK(visited.count(t) == 0);
        visited.insert(t);
    };

    for (auto&& t : layout.tiles()) { check1(t); }
    CHECK(visited.size() == 200);

    visited.clear();

    layout.foreach_tile(check1);
    CHECK(visited.size() == 200);

    visited.clear();

    tile_based_layout::aspect_ratio ar_ground{ar.x, ar.y, 0};

    const auto check2 = [&visited, &ar_ground](const auto& t)
    {
        // iteration stays in ground layer
        CHECK(t.z == 0);
        CHECK(t <= ar_ground);

        // no tile is visited twice
        CHECK(visited.count(t) == 0);
        visited.insert(t);
    };

    for (auto&& t : layout.ground_tiles()) { check2(t); }
    CHECK(visited.size() == 100);

    visited.clear();

    layout.foreach_ground_tile(check2);
    CHECK(visited.size() == 100);

    visited.clear();

    tile_based_layout::tile start{2, 2}, stop{5, 4};

    const auto check3 = [&visited, &start, &stop](const auto& t)
    {
        CHECK(t.z == 0);
        // iteration stays in between the bounds
        CHECK(t >= start);
        CHECK(t < stop);

        // no tile is visited twice
        CHECK(visited.count(t) == 0);
        visited.insert(t);
    };

    for (auto&& t : layout.tiles(start, stop)) { check3(t); }
    CHECK(visited.size() == 23);

    visited.clear();

    layout.foreach_tile(check3, start, stop);
    CHECK(visited.size() == 23);
}

TEST_CASE("Cardinal operations", "[tile-based]")
{
    tile_based_layout::aspect_ratio ar{10, 10, 1};

    tile_based_layout layout{ar};

    auto nt = layout.north({5, 5});
    CHECK(!nt.is_dead());
    CHECK(nt == tile_based_layout::tile{5, 4});
    nt = layout.north({5, 0});
    CHECK(!nt.is_dead());
    CHECK(nt == tile_based_layout::tile{5, 0});
    CHECK(layout.is_border(nt));
    CHECK(layout.is_northern_border(nt));
    CHECK(layout.is_ground_layer(nt));

    auto et = layout.east({5, 5});
    CHECK(!et.is_dead());
    CHECK(et == tile_based_layout::tile{6, 5});
    et = layout.east({10, 5});
    CHECK(!et.is_dead());
    CHECK(et == tile_based_layout::tile{10, 5});
    CHECK(layout.is_border(et));
    CHECK(layout.is_eastern_border(et));
    CHECK(layout.is_ground_layer(et));
    et = layout.east({11, 5});
    CHECK(et.is_dead());

    auto st = layout.south({5, 5});
    CHECK(!st.is_dead());
    CHECK(st == tile_based_layout::tile{5, 6});
    st = layout.south({5, 10});
    CHECK(!st.is_dead());
    CHECK(st == tile_based_layout::tile{5, 10});
    CHECK(layout.is_border(st));
    CHECK(layout.is_southern_border(st));
    CHECK(layout.is_ground_layer(st));
    st = layout.south({5, 11});
    CHECK(st.is_dead());

    auto wt = layout.west({5, 5});
    CHECK(!wt.is_dead());
    CHECK(wt == tile_based_layout::tile{4, 5});
    wt = layout.west({0, 5});
    CHECK(!wt.is_dead());
    CHECK(wt == tile_based_layout::tile{0, 5});
    CHECK(layout.is_border(wt));
    CHECK(layout.is_western_border(wt));
    CHECK(layout.is_ground_layer(wt));

    auto at = layout.above({5, 5, 0});
    CHECK(!at.is_dead());
    CHECK(at == tile_based_layout::tile{5, 5, 1});
    at = layout.above({5, 5, 1});
    CHECK(!at.is_dead());
    CHECK(at == tile_based_layout::tile{5, 5, 1});
    CHECK(layout.is_crossing_layer(at));
    CHECK(!layout.is_border(at));
    // cover corner case
    tile_based_layout planar_layout{{1, 1, 0}};
    at = planar_layout.above({1, 1, 1});
    CHECK(at.is_dead());

    auto bt = layout.below({5, 5, 1});
    CHECK(!bt.is_dead());
    CHECK(bt == tile_based_layout::tile{5, 5, 0});
    bt = layout.below({5, 5, 0});
    CHECK(!bt.is_dead());
    CHECK(bt == tile_based_layout::tile{5, 5, 0});
    CHECK(layout.is_ground_layer(bt));

    auto s1 = layout.adjacent_tiles<std::set<tile_based_layout::tile>>({5, 5});
    auto s2 = std::set<tile_based_layout::tile>{{{4, 5}, {5, 4}, {6, 5}, {5, 6}}};

    CHECK(s1 == s2);
}
