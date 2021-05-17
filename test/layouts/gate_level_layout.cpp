//
// Created by marcel on 31.03.21.
//

#include "gate_level_layout.hpp"

#include "catch.hpp"
#include "clocked_layout.hpp"
#include "tile_based_layout.hpp"

#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <mockturtle/traits.hpp>

#include <type_traits>

using namespace fiction;

TEST_CASE("Creation and usage of constants", "[gate-level]")
{
    // adapted from mockturtle/test/networks/klut.cpp

    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout>>;

    REQUIRE(mockturtle::is_network_type_v<gate_layout>);
    REQUIRE(mockturtle::has_size_v<gate_layout>);
    REQUIRE(mockturtle::has_get_constant_v<gate_layout>);
    REQUIRE(mockturtle::has_is_constant_v<gate_layout>);
    REQUIRE(mockturtle::has_get_node_v<gate_layout>);
    REQUIRE(mockturtle::has_is_complemented_v<gate_layout>);

    gate_layout layout{tile_based_layout::aspect_ratio{2, 2, 1}};

    CHECK(layout.size() == 2);

    const auto c0 = layout.get_constant(false);
    CHECK(layout.is_constant(layout.get_node(c0)));
    CHECK(!layout.is_pi(layout.get_node(c0)));

    CHECK(layout.size() == 2);
    CHECK(std::is_same_v<std::decay_t<decltype(c0)>, gate_layout::signal>);
    CHECK(layout.get_node(c0) == 0);
    CHECK(!layout.is_complemented(c0));

    const auto c1 = layout.get_constant(true);
    CHECK(layout.is_constant(layout.get_node(c1)));

    CHECK(layout.get_node(c1) == 1);
    CHECK(!layout.is_complemented(c1));
}

TEST_CASE("Creation and usage of primary inputs", "[gate-level]")
{
    // adapted from mockturtle/test/networks/klut.cpp

    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout>>;

    REQUIRE(mockturtle::is_network_type_v<gate_layout>);
    REQUIRE(mockturtle::has_size_v<gate_layout>);
    REQUIRE(mockturtle::has_create_pi_v<gate_layout>);
    REQUIRE(mockturtle::has_num_pis_v<gate_layout>);
    REQUIRE(mockturtle::has_num_gates_v<gate_layout>);
    REQUIRE(mockturtle::has_foreach_pi_v<gate_layout>);

    gate_layout layout{tile_based_layout::aspect_ratio{2, 2, 1}};

    auto a = layout.create_pi("a", {0, 0});
    CHECK(layout.is_pi(layout.get_node(a)));

    CHECK(layout.size() == 3);
    CHECK(layout.num_pis() == 1);
    CHECK(layout.num_gates() == 0);
    CHECK(layout.num_wires() == 1);

    CHECK(std::is_same_v<std::decay_t<decltype(a)>, gate_layout::signal>);

    auto b = layout.create_pi("b", {1, 0});
    auto c = layout.create_pi("c", {0, 1});

    CHECK(layout.is_pi(layout.get_node(b)));
    CHECK(layout.is_pi(layout.get_node(c)));

    CHECK(layout.num_pis() == 3);
    CHECK(layout.num_wires() == 3);

    layout.foreach_pi(
        [&](auto pi, auto i)
        {
            const auto check = [&layout, &pi](auto c, auto s)
            {
                auto t = layout.get_tile(pi);
                CHECK(t == c);
                auto n = layout.get_node(static_cast<gate_layout::signal>(t));
                CHECK(n == layout.get_node(s));
                auto tn = layout.get_tile(n);
                CHECK(tn == t);
            };

            CHECK(layout.is_pi(pi));
            CHECK(layout.is_pi_tile(layout.get_tile(pi)));
            CHECK(!layout.is_gate_tile(static_cast<gate_layout::tile>(pi)));
            CHECK(!layout.is_gate(layout.get_node(static_cast<gate_layout::tile>(pi))));

            switch (i)
            {
                case 0:
                {
                    check(gate_layout::tile{0, 0}, a);

                    break;
                }

                case 1:
                {
                    check(gate_layout::tile{1, 0}, b);

                    break;
                }
                case 2:
                {
                    check(gate_layout::tile{0, 1}, c);

                    break;
                }
            }
        });
}

TEST_CASE("Creation and usage of primary outputs", "[gate-level]")
{
    // adapted from mockturtle/test/networks/klut.cpp

    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout>>;

    REQUIRE(mockturtle::is_network_type_v<gate_layout>);
    REQUIRE(mockturtle::has_size_v<gate_layout>);
    REQUIRE(mockturtle::has_create_pi_v<gate_layout>);
    REQUIRE(mockturtle::has_create_po_v<gate_layout>);
    REQUIRE(mockturtle::has_num_pis_v<gate_layout>);
    REQUIRE(mockturtle::has_num_pos_v<gate_layout>);
    REQUIRE(mockturtle::has_foreach_po_v<gate_layout>);

    gate_layout layout{tile_based_layout::aspect_ratio{2, 2, 1}};

    const auto x1 = layout.create_pi("x1", gate_layout::tile{0, 0});

    CHECK(layout.size() == 3);
    CHECK(layout.num_pis() == 1);
    CHECK(layout.num_pos() == 0);

    const auto f1 = layout.create_po(x1, "f1", gate_layout::tile{0, 1});
    const auto f2 = layout.create_po(!x1, "f2", gate_layout::tile{1, 1});

    CHECK(layout.is_po(layout.get_node(f1)));
    CHECK(layout.is_po(layout.get_node(f2)));

    CHECK(layout.is_empty_tile({1, 0}));

    CHECK(layout.size() == 5);
    CHECK(layout.num_pos() == 2);
    CHECK(layout.num_wires() == 3);

    layout.foreach_po(
        [&](auto po, auto i)
        {
            const auto check = [&layout, &po](auto c)
            {
                CHECK(static_cast<gate_layout::tile>(po) == c);
                auto n  = layout.get_node(po);
                auto tn = layout.get_tile(n);
                CHECK(tn == static_cast<gate_layout::tile>(po));
            };

            CHECK(layout.is_po(layout.get_node(po)));
            CHECK(layout.is_po_tile(static_cast<gate_layout::tile>(po)));
            CHECK(layout.is_gate_tile(static_cast<gate_layout::tile>(po)));
            CHECK(layout.is_gate(layout.get_node(static_cast<gate_layout::tile>(po))));

            switch (i)
            {
                case 0:
                {
                    check(gate_layout::tile{0, 1});

                    break;
                }
                case 1:
                {
                    check(gate_layout::tile{1, 1});

                    break;
                }
            }
        });
}

TEST_CASE("Creation of unary operations", "[gate-level]")
{
    // adapted from mockturtle/test/networks/klut.cpp

    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout>>;

    REQUIRE(mockturtle::is_network_type_v<gate_layout>);
    REQUIRE(mockturtle::has_size_v<gate_layout>);
    REQUIRE(mockturtle::has_create_pi_v<gate_layout>);
    REQUIRE(mockturtle::has_create_buf_v<gate_layout>);
    REQUIRE(mockturtle::has_create_not_v<gate_layout>);

    gate_layout layout{gate_layout::aspect_ratio{2, 2, 1}};

    auto x1 = layout.create_pi("x1", {0, 0});

    CHECK(layout.size() == 3);

    auto f1 = layout.create_buf(x1, {1, 0});
    auto f2 = layout.create_not(x1, {0, 1});

    CHECK(layout.size() == 5);
    CHECK(layout.num_gates() == 1);
    CHECK(layout.num_wires() == 2);

    auto x2 = layout.create_pi("x2", {1, 1});
    CHECK(layout.is_pi(layout.get_node(x2)));

    auto f1n  = layout.get_node(f1);
    auto t10n = layout.get_node(static_cast<gate_layout::signal>(gate_layout::tile{1, 0}));
    CHECK(f1n == t10n);

    auto f2n  = layout.get_node(f2);
    auto t01n = layout.get_node(static_cast<gate_layout::signal>(gate_layout::tile{0, 1}));
    CHECK(f2n == t01n);

    CHECK(!layout.is_empty_tile({1, 0}));
    CHECK(layout.is_gate_tile({1, 0}));
    CHECK(layout.is_gate(layout.get_node({1, 0})));
    CHECK(layout.is_wire_tile({1, 0}));
    CHECK(layout.is_wire(layout.get_node({1, 0})));

    CHECK(!layout.is_empty_tile({0, 1}));
    CHECK(layout.is_gate_tile({0, 1}));
    CHECK(layout.is_gate(layout.get_node({0, 1})));
    CHECK(!layout.is_wire_tile({0, 1}));
    CHECK(!layout.is_wire(layout.get_node({0, 1})));
}

TEST_CASE("Creation of binary operations", "[gate-level]")
{
    // adapted from mockturtle/test/networks/klut.cpp

    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout>>;

    REQUIRE(mockturtle::is_network_type_v<gate_layout>);
    REQUIRE(mockturtle::has_create_pi_v<gate_layout>);
    REQUIRE(mockturtle::has_create_and_v<gate_layout>);
    REQUIRE(mockturtle::has_create_or_v<gate_layout>);

    gate_layout layout{tile_based_layout::aspect_ratio{2, 2, 1}};

    auto x1 = layout.create_pi("x1", {1, 0});
    auto x2 = layout.create_pi("x2", {0, 1});

    CHECK(layout.num_pis() == 2);

    auto a = layout.create_and(x1, x2, {0, 0});
    auto o = layout.create_or(x1, x2, {1, 1});

    CHECK(a != o);
    CHECK(layout.num_gates() == 2);
    CHECK(layout.size() == 6);

    CHECK(layout.get_node({0, 0}) == layout.get_node(a));
    CHECK(layout.get_node({1, 1}) == layout.get_node(o));

    CHECK(layout.is_gate_tile({0, 0}));
    CHECK(layout.is_gate_tile({1, 1}));

    CHECK(layout.is_gate(layout.get_node({0, 0})));
    CHECK(layout.is_gate(layout.get_node({1, 1})));

    CHECK(!layout.is_wire_tile({0, 0}));
    CHECK(!layout.is_wire_tile({1, 1}));

    CHECK(!layout.is_wire(layout.get_node({0, 0})));
    CHECK(!layout.is_wire(layout.get_node({1, 1})));
}

TEST_CASE("Creation of ternary operations", "[gate-level]")
{
    // adapted from mockturtle/test/networks/klut.cpp

    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout>>;

    REQUIRE(mockturtle::is_network_type_v<gate_layout>);
    REQUIRE(mockturtle::has_create_pi_v<gate_layout>);
    REQUIRE(mockturtle::has_num_pis_v<gate_layout>);
    REQUIRE(mockturtle::has_create_maj_v<gate_layout>);

    gate_layout layout{tile_based_layout::aspect_ratio{2, 3, 1}};

    auto x1 = layout.create_pi("x1", {1, 0});
    auto x2 = layout.create_pi("x2", {0, 1});
    auto x3 = layout.create_pi("x3", {1, 2});

    CHECK(layout.num_pis() == 3);

    auto m = layout.create_maj(x1, x2, x3, {1, 1});

    CHECK(x1 != m);
    CHECK(x2 != m);
    CHECK(x3 != m);

    CHECK(layout.num_gates() == 1);
    CHECK(layout.size() == 6);

    CHECK(layout.get_node({1, 1}) == layout.get_node(m));

    CHECK(layout.is_gate_tile({1, 1}));
    CHECK(layout.is_gate(layout.get_node({1, 1})));
    CHECK(!layout.is_wire_tile({1, 1}));
    CHECK(!layout.is_wire(layout.get_node({1, 1})));
}

TEST_CASE("compute functions from AND and NOT gates", "[gate-level]")
{
    // adapted from mockturtle/test/networks/klut.cpp

    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout>>;

    REQUIRE(mockturtle::has_compute_v<gate_layout, kitty::dynamic_truth_table>);

    gate_layout layout{tile_based_layout::aspect_ratio{3, 1, 0}, open_4_clocking};

    layout.assign_clock_number({2, 0}, static_cast<typename gate_layout::clock_number_t>(0));
    layout.assign_clock_number({1, 0}, static_cast<typename gate_layout::clock_number_t>(1));
    layout.assign_clock_number({0, 0}, static_cast<typename gate_layout::clock_number_t>(2));

    layout.assign_clock_number({1, 1}, static_cast<typename gate_layout::clock_number_t>(0));
    layout.assign_clock_number({2, 1}, static_cast<typename gate_layout::clock_number_t>(1));
    layout.assign_clock_number({3, 1}, static_cast<typename gate_layout::clock_number_t>(2));

    const auto x1 = layout.create_pi("x1", {2, 0});
    const auto x2 = layout.create_pi("x2", {1, 1});
    const auto a1 = layout.create_and(x1, x2, {1, 0});
    const auto n1 = layout.create_not(x2, {2, 1});
    layout.create_po(a1, "f1", {0, 0});
    layout.create_po(n1, "f2", {3, 1});

    std::vector<kitty::dynamic_truth_table> xs;
    xs.emplace_back(3u);
    xs.emplace_back(3u);
    kitty::create_nth_var(xs[0], 0);
    kitty::create_nth_var(xs[1], 1);

    const auto sim_n1 = layout.compute(layout.get_node(n1), xs.begin(), xs.begin() + 1);
    const auto sim_a1 = layout.compute(layout.get_node(a1), xs.begin(), xs.end());

    CHECK(sim_n1 == ~xs[0]);
    CHECK(sim_a1 == (xs[0] & xs[1]));
}

TEST_CASE("create nodes and compute their functions", "[gate-level]")
{
    // adapted from mockturtle/test/networks/klut.cpp

    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout>>;

    REQUIRE(mockturtle::has_create_node_v<gate_layout>);
    REQUIRE(mockturtle::has_compute_v<gate_layout, kitty::dynamic_truth_table>);

    gate_layout layout{tile_based_layout::aspect_ratio{3, 2, 0}, open_4_clocking};

    layout.assign_clock_number({2, 0}, static_cast<typename gate_layout::clock_number_t>(0));
    layout.assign_clock_number({1, 1}, static_cast<typename gate_layout::clock_number_t>(0));
    layout.assign_clock_number({3, 1}, static_cast<typename gate_layout::clock_number_t>(0));

    layout.assign_clock_number({1, 0}, static_cast<typename gate_layout::clock_number_t>(1));
    layout.assign_clock_number({2, 1}, static_cast<typename gate_layout::clock_number_t>(1));

    layout.assign_clock_number({0, 0}, static_cast<typename gate_layout::clock_number_t>(2));
    layout.assign_clock_number({2, 2}, static_cast<typename gate_layout::clock_number_t>(2));

    const auto a = layout.create_pi("a", {1, 1});
    const auto b = layout.create_pi("b", {2, 0});
    const auto c = layout.create_pi("c", {3, 1});

    kitty::dynamic_truth_table tt_maj(3u), tt_xor(2u), tt_const0(0u);
    kitty::create_from_hex_string(tt_maj, "e8");
    kitty::create_from_hex_string(tt_xor, "6");

    CHECK(layout.size() == 5);

    const auto const0 = layout.create_node({}, tt_const0);
    const auto const1 = layout.create_node({}, ~tt_const0);
    CHECK(const0 == layout.get_constant(false));
    CHECK(const1 == layout.get_constant(true));

    const auto n_maj = layout.create_node({a, b, c}, tt_maj, {2, 1});
    const auto n_xor = layout.create_node({a, b}, tt_xor, {1, 0});

    layout.create_po(n_maj, "f1", {2, 2});
    layout.create_po(n_xor, "f2", {0, 0});

    CHECK(layout.size() == 9);

    std::vector<kitty::dynamic_truth_table> xs;
    xs.emplace_back(3u);
    xs.emplace_back(3u);
    xs.emplace_back(3u);
    kitty::create_nth_var(xs[0], 0);
    kitty::create_nth_var(xs[1], 1);
    kitty::create_nth_var(xs[2], 2);

    const auto sim_maj = layout.compute(layout.get_node(n_maj), xs.begin(), xs.end());
    const auto sim_xor = layout.compute(layout.get_node(n_xor), xs.begin(), xs.begin() + 2);

    CHECK(sim_maj == kitty::ternary_majority(xs[0], xs[1], xs[2]));
    CHECK(sim_xor == (xs[0] ^ xs[1]));
}

TEST_CASE("node and signal iteration", "[gate-level]")
{
    // adapted from mockturtle/test/networks/klut.cpp

    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout>>;

    REQUIRE(mockturtle::has_foreach_node_v<gate_layout>);
    REQUIRE(mockturtle::has_foreach_pi_v<gate_layout>);
    REQUIRE(mockturtle::has_foreach_po_v<gate_layout>);
    REQUIRE(mockturtle::has_foreach_fanin_v<gate_layout>);
    REQUIRE(mockturtle::has_foreach_fanout_v<gate_layout>);

    gate_layout layout{tile_based_layout::aspect_ratio{3, 1, 0}, open_4_clocking};

    layout.assign_clock_number({2, 0}, static_cast<typename gate_layout::clock_number_t>(0));
    layout.assign_clock_number({1, 0}, static_cast<typename gate_layout::clock_number_t>(1));
    layout.assign_clock_number({0, 0}, static_cast<typename gate_layout::clock_number_t>(2));

    layout.assign_clock_number({1, 1}, static_cast<typename gate_layout::clock_number_t>(0));
    layout.assign_clock_number({2, 1}, static_cast<typename gate_layout::clock_number_t>(1));
    layout.assign_clock_number({3, 1}, static_cast<typename gate_layout::clock_number_t>(2));

    const auto x1 = layout.create_pi("x1", {2, 0});
    const auto x2 = layout.create_pi("x2", {1, 1});
    const auto a1 = layout.create_and(x1, x2, {1, 0});
    const auto a2 = layout.create_and(x2, x1, {2, 1});
    layout.create_po(a1, "f1", {0, 0});
    layout.create_po(a2, "f2", {3, 1});

    CHECK(layout.size() == 8);

    /* iterate over nodes */
    uint32_t mask{0}, counter{0};
    layout.foreach_node(
        [&](auto n, auto i)
        {
            mask |= (1 << n);
            counter += i;
        });
    CHECK(mask == 255);
    CHECK(counter == 28);

    mask = 0;
    layout.foreach_node([&](auto n) { mask |= (1 << n); });
    CHECK(mask == 255);

    mask = counter = 0;
    layout.foreach_node(
        [&](auto n, auto i)
        {
            mask |= (1 << n);
            counter += i;
            return false;
        });
    CHECK(mask == 1);
    CHECK(counter == 0);

    mask = 0;
    layout.foreach_node(
        [&](auto n)
        {
            mask |= (1 << n);
            return false;
        });
    CHECK(mask == 1);

    /* iterate over gates */
    mask = counter = 0;
    layout.foreach_gate(
        [&](auto n, auto i)
        {
            mask |= (1 << n);
            counter += i;
        });
    CHECK(mask == 240);
    CHECK(counter == 6);

    mask = 0;
    layout.foreach_gate([&](auto n) { mask |= (1 << n); });
    CHECK(mask == 240);

    mask = counter = 0;
    layout.foreach_gate(
        [&](auto n, auto i)
        {
            mask |= (1 << n);
            counter += i;
            return false;
        });
    CHECK(mask == 16);
    CHECK(counter == 0);

    mask = 0;
    layout.foreach_gate(
        [&](auto n)
        {
            mask |= (1 << n);
            return false;
        });
    CHECK(mask == 16);

    /* iterate over wires */
    mask = counter = 0;
    layout.foreach_wire(
        [&](auto n, auto i)
        {
            mask |= (1 << n);
            counter += i;
        });
    CHECK(mask == 204);
    CHECK(counter == 6);

    mask = 0;
    layout.foreach_wire([&](auto n) { mask |= (1 << n); });
    CHECK(mask == 204);

    mask = counter = 0;
    layout.foreach_wire(
        [&](auto n, auto i)
        {
            mask |= (1 << n);
            counter += i;
            return false;
        });
    CHECK(mask == 4);
    CHECK(counter == 0);

    mask = 0;
    layout.foreach_wire(
        [&](auto n)
        {
            mask |= (1 << n);
            return false;
        });
    CHECK(mask == 4);

    /* iterate over PIs */
    mask = counter = 0;
    layout.foreach_pi(
        [&](auto n, auto i)
        {
            mask |= (1 << n);
            counter += i;
        });
    CHECK(mask == 12);
    CHECK(counter == 1);

    mask = 0;
    layout.foreach_pi([&](auto n) { mask |= (1 << n); });
    CHECK(mask == 12);

    mask = counter = 0;
    layout.foreach_pi(
        [&](auto n, auto i)
        {
            mask |= (1 << n);
            counter += i;
            return false;
        });
    CHECK(mask == 4);
    CHECK(counter == 0);

    mask = 0;
    layout.foreach_pi(
        [&](auto n)
        {
            mask |= (1 << n);
            return false;
        });
    CHECK(mask == 4);

    /* iterate over POs */
    mask = counter = 0;
    layout.foreach_po(
        [&](auto s, auto i)
        {
            mask |= (1 << layout.get_node(s));
            counter += i;
        });
    CHECK(mask == 192);
    CHECK(counter == 1);

    mask = 0;
    layout.foreach_po([&](auto s) { mask |= (1 << layout.get_node(s)); });
    CHECK(mask == 192);

    mask = counter = 0;
    layout.foreach_po(
        [&](auto s, auto i)
        {
            mask |= (1 << layout.get_node(s));
            counter += i;
            return false;
        });
    CHECK(mask == 64);
    CHECK(counter == 0);

    mask = 0;
    layout.foreach_po(
        [&](auto s)
        {
            mask |= (1 << layout.get_node(s));
            return false;
        });
    CHECK(mask == 64);

    /* iterate over fanins */
    mask = counter = 0;
    layout.foreach_fanin(layout.get_node(a1),
                         [&](auto s, auto i)
                         {
                             mask |= (1 << layout.get_node(s));
                             counter += i;
                         });
    CHECK(mask == 12);
    CHECK(counter == 1);

    mask = 0;
    layout.foreach_fanin(layout.get_node(a1), [&](auto s) { mask |= (1 << layout.get_node(s)); });
    CHECK(mask == 12);

    mask = counter = 0;
    layout.foreach_fanin(layout.get_node(a1),
                         [&](auto s, auto i)
                         {
                             mask |= (1 << layout.get_node(s));
                             counter += i;
                             return false;
                         });
    CHECK(mask == 4);
    CHECK(counter == 0);

    mask = 0;
    layout.foreach_fanin(layout.get_node(a1),
                         [&](auto s)
                         {
                             mask |= (1 << layout.get_node(s));
                             return false;
                         });
    CHECK(mask == 4);

    /* iterate over fanouts */
    mask = counter = 0;
    layout.foreach_fanout(layout.get_node(a2),
                          [&](auto s, auto i)
                          {
                              mask |= (1 << layout.get_node(s));
                              counter += i;
                          });
    CHECK(mask == 128);
    CHECK(counter == 0);

    mask = 0;
    layout.foreach_fanout(layout.get_node(a2), [&](auto s) { mask |= (1 << layout.get_node(s)); });
    CHECK(mask == 128);

    mask = counter = 0;
    layout.foreach_fanout(layout.get_node(a2),
                          [&](auto s, auto i)
                          {
                              mask |= (1 << layout.get_node(s));
                              counter += i;
                              return false;
                          });
    CHECK(mask == 128);
    CHECK(counter == 0);

    mask = 0;
    layout.foreach_fanout(layout.get_node(a2),
                          [&](auto s)
                          {
                              mask |= (1 << layout.get_node(s));
                              return false;
                          });
    CHECK(mask == 128);
}

TEST_CASE("Structural properties", "[gate-level]")
{
    // adapted from mockturtle/test/networks/klut.cpp

    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout>>;

    REQUIRE(mockturtle::has_size_v<gate_layout>);
    REQUIRE(mockturtle::has_num_pis_v<gate_layout>);
    REQUIRE(mockturtle::has_num_pos_v<gate_layout>);
    REQUIRE(mockturtle::has_num_gates_v<gate_layout>);
    REQUIRE(mockturtle::has_fanin_size_v<gate_layout>);
    REQUIRE(mockturtle::has_fanout_size_v<gate_layout>);

    gate_layout layout{tile_based_layout::aspect_ratio{3, 1, 0}, open_4_clocking};

    layout.assign_clock_number({2, 0}, static_cast<typename gate_layout::clock_number_t>(0));
    layout.assign_clock_number({1, 0}, static_cast<typename gate_layout::clock_number_t>(1));
    layout.assign_clock_number({0, 0}, static_cast<typename gate_layout::clock_number_t>(2));
    layout.assign_clock_number({1, 1}, static_cast<typename gate_layout::clock_number_t>(0));
    layout.assign_clock_number({2, 1}, static_cast<typename gate_layout::clock_number_t>(1));
    layout.assign_clock_number({3, 1}, static_cast<typename gate_layout::clock_number_t>(2));

    const auto x1 = layout.create_pi("x1", {2, 0});
    const auto x2 = layout.create_pi("x2", {1, 1});
    const auto a1 = layout.create_and(x1, x2, {1, 0});
    const auto n1 = layout.create_not(x2, {2, 1});
    const auto f1 = layout.create_po(a1, "f1", {0, 0});
    const auto f2 = layout.create_po(n1, "f2", {3, 1});

    CHECK(layout.size() == 8);
    CHECK(layout.num_pis() == 2);
    CHECK(layout.num_pos() == 2);
    CHECK(layout.num_gates() == 2);
    CHECK(layout.fanin_size(layout.get_node(x1)) == 0);
    CHECK(layout.fanin_size(layout.get_node(x2)) == 0);
    CHECK(layout.fanin_size(layout.get_node(a1)) == 2);
    CHECK(layout.fanin_size(layout.get_node(n1)) == 1);
    CHECK(layout.fanin_size(layout.get_node(f1)) == 1);
    CHECK(layout.fanin_size(layout.get_node(f2)) == 1);
    CHECK(layout.fanout_size(layout.get_node(x1)) == 1);
    CHECK(layout.fanout_size(layout.get_node(x2)) == 2);
    CHECK(layout.fanout_size(layout.get_node(a1)) == 1);
    CHECK(layout.fanout_size(layout.get_node(n1)) == 1);
    CHECK(layout.fanout_size(layout.get_node(f1)) == 0);
    CHECK(layout.fanout_size(layout.get_node(f2)) == 0);
}

TEST_CASE("Custom node values", "[gate-level]")
{
    // adapted from mockturtle/test/networks/klut.cpp

    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout>>;

    REQUIRE(mockturtle::has_clear_values_v<gate_layout>);
    REQUIRE(mockturtle::has_value_v<gate_layout>);
    REQUIRE(mockturtle::has_set_value_v<gate_layout>);
    REQUIRE(mockturtle::has_incr_value_v<gate_layout>);
    REQUIRE(mockturtle::has_decr_value_v<gate_layout>);

    gate_layout layout{tile_based_layout::aspect_ratio{3, 1, 0}};

    const auto x1 = layout.create_pi("x1", {2, 0});
    const auto x2 = layout.create_pi("x2", {1, 1});
    const auto a1 = layout.create_and(x1, x2, {1, 0});
    const auto a2 = layout.create_and(x2, x1, {2, 1});
    layout.create_po(a1, "f1", {0, 0});
    layout.create_po(a2, "f2", {3, 1});

    CHECK(layout.size() == 8);

    layout.clear_values();
    layout.foreach_node(
        [&](auto n)
        {
            CHECK(layout.value(n) == 0);
            layout.set_value(n, static_cast<uint32_t>(n));
            CHECK(layout.value(n) == n);
            CHECK(layout.incr_value(n) == n);
            CHECK(layout.value(n) == n + 1);
            CHECK(layout.decr_value(n) == n);
            CHECK(layout.value(n) == n);
        });
    layout.clear_values();
    layout.foreach_node([&](auto n) { CHECK(layout.value(n) == 0); });
}

TEST_CASE("Visited values", "[gate-level]")
{
    // adapted from mockturtle/test/networks/klut.cpp

    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout>>;

    REQUIRE(mockturtle::has_clear_visited_v<gate_layout>);
    REQUIRE(mockturtle::has_visited_v<gate_layout>);
    REQUIRE(mockturtle::has_set_visited_v<gate_layout>);

    gate_layout layout{tile_based_layout::aspect_ratio{3, 1, 0}};

    const auto x1 = layout.create_pi("x1", {2, 0});
    const auto x2 = layout.create_pi("x2", {1, 1});
    const auto a1 = layout.create_and(x1, x2, {1, 0});
    const auto a2 = layout.create_and(x2, x1, {2, 1});
    layout.create_po(a1, "f1", {0, 0});
    layout.create_po(a2, "f2", {3, 1});

    CHECK(layout.size() == 8);

    layout.clear_visited();
    layout.foreach_node(
        [&](auto n)
        {
            CHECK(layout.visited(n) == 0);
            layout.set_visited(n, static_cast<uint32_t>(n));
            CHECK(layout.visited(n) == n);
        });
    layout.clear_visited();
    layout.foreach_node([&](auto n) { CHECK(layout.visited(n) == 0); });
}
