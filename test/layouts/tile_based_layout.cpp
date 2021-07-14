//
// Created by marcel on 31.03.21.
//

#include "catch.hpp"

#include <fiction/layouts/tile_based_layout.hpp>

#include <map>
#include <set>
#include <sstream>

using namespace fiction;

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#endif

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

    t1.z += uint64_t{4ul};

    CHECK(t1 == t2);

    t1.y += uint64_t{2147483648ul};

    CHECK(t1 == t2);

    t1.x += uint64_t{2147483648ul};

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
        {0x0000000000000002, tile_based_layout::tile{2, 0, 0}},
        {0x3fffffffffffffff, tile_based_layout::tile{2147483647, 2147483647, 0}}};

    for (auto [repr, tile] : tile_repr)
    {
        CHECK(static_cast<tile_based_layout::tile>(repr) == tile);
        CHECK(repr == static_cast<uint64_t>(tile));
        CHECK(tile_based_layout::tile{repr} == tile);
        CHECK(tile_based_layout::tile{tile} == tile);
        CHECK(tile_based_layout::tile{static_cast<uint64_t>(tile)} == tile);
    }

    std::ostringstream os{};
    os << tile_based_layout::tile{3, 2, 1};
    CHECK(os.str() == "(3,2,1)");
}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

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

    const auto check = [&](const auto& t, const auto& at1, const auto& at2, const auto& b, const auto& bt)
    {
        CHECK(!at1.is_dead());
        CHECK(at1 == at2);
        CHECK(layout.is_adjacent_of(t, at1));
        CHECK(layout.is_adjacent_of(at1, t));

        CHECK(layout.is_border(b));
        CHECK(!bt.is_dead());
        CHECK(b == bt);
        CHECK(layout.is_ground_layer(bt));
        CHECK(layout.is_border(bt));
    };

    auto t = tile_based_layout::tile{5, 5};

    auto nt  = tile_based_layout::tile{5, 4};
    auto bnt = tile_based_layout::tile{5, 0};

    check(t, layout.north(t), nt, bnt, layout.north(bnt));
    CHECK(layout.is_north_of(t, nt));
    CHECK(layout.is_northwards_of(t, nt));
    CHECK(layout.is_northwards_of(t, bnt));
    CHECK(layout.is_northern_border(bnt));
    CHECK(layout.northern_border_of(t) == bnt);

    auto et  = tile_based_layout::tile{6, 5};
    auto bet = tile_based_layout::tile{10, 5};

    check(t, layout.east(t), et, bet, layout.east(bet));
    CHECK(layout.is_east_of(t, et));
    CHECK(layout.is_eastwards_of(t, et));
    CHECK(layout.is_eastwards_of(t, bet));
    CHECK(layout.is_eastern_border(bet));
    CHECK(layout.eastern_border_of(t) == bet);

    auto st  = tile_based_layout::tile{5, 6};
    auto bst = tile_based_layout::tile{5, 10};

    check(t, layout.south(t), st, bst, layout.south(bst));
    CHECK(layout.is_south_of(t, st));
    CHECK(layout.is_southwards_of(t, st));
    CHECK(layout.is_southwards_of(t, bst));
    CHECK(layout.is_southern_border(bst));
    CHECK(layout.southern_border_of(t) == bst);

    auto wt  = tile_based_layout::tile{4, 5};
    auto bwt = tile_based_layout::tile{0, 5};

    check(t, layout.west(t), wt, bwt, layout.west(bwt));
    CHECK(layout.is_west_of(t, wt));
    CHECK(layout.is_westwards_of(t, wt));
    CHECK(layout.is_westwards_of(t, bwt));
    CHECK(layout.is_western_border(bwt));
    CHECK(layout.western_border_of(t) == bwt);

    auto at  = tile_based_layout::tile{5, 5, 1};
    auto bat = layout.above(at);

    CHECK(!at.is_dead());
    CHECK(layout.is_above_of(t, at));
    CHECK(at == bat);
    CHECK(layout.is_crossing_layer(bat));
    CHECK(!layout.is_border(at));

    // cover corner case
    tile_based_layout planar_layout{{1, 1, 0}};
    auto              dat = planar_layout.above({1, 1, 1});
    CHECK(dat.is_dead());

    auto bt  = layout.below(at);
    auto bbt = layout.below(bt);

    CHECK(!bt.is_dead());
    CHECK(bt == t);
    CHECK(layout.is_below_of(at, bt));
    CHECK(!bbt.is_dead());
    CHECK(bt == bbt);
    CHECK(layout.is_ground_layer(bbt));

    auto s1 = layout.adjacent_tiles<std::set<tile_based_layout::tile>>({5, 5});
    auto s2 = std::set<tile_based_layout::tile>{{{4, 5}, {5, 4}, {6, 5}, {5, 6}}};

    CHECK(s1 == s2);
}
