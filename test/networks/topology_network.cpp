//
// Created by marcel on 20.05.21.
//

// This file is based on mockturtle/test/networks/klut.cpp

#include <fiction/networks/topology_network.hpp>
#include <fiction/traits.hpp>

#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/operations.hpp>
#include <kitty/operators.hpp>

#include <vector>

#include <catch.hpp>

using namespace fiction;

TEST_CASE("create and use constants in a topology network", "[topo]")
{
    topology_network topo{};

    CHECK(mockturtle::has_size_v<topology_network>);
    CHECK(mockturtle::has_get_constant_v<topology_network>);
    CHECK(mockturtle::has_is_constant_v<topology_network>);
    CHECK(mockturtle::has_is_pi_v<topology_network>);
    CHECK(mockturtle::has_is_constant_v<topology_network>);
    CHECK(mockturtle::has_get_node_v<topology_network>);
    CHECK(mockturtle::has_is_complemented_v<topology_network>);

    CHECK(topo.size() == 2);

    auto c0 = topo.get_constant(false);
    auto c1 = topo.get_constant(true);

    CHECK(topo.size() == 2);
    CHECK(c0 != c1);
    CHECK(topo.get_node(c0) == 0);
    CHECK(topo.get_node(c1) == 1);
    CHECK(!topo.is_complemented(c0));
    CHECK(!topo.is_complemented(c1));
    CHECK(topo.is_constant(c0));
    CHECK(topo.is_constant(c1));
    CHECK(!topo.is_pi(c0));
    CHECK(!topo.is_pi(c1));
}

TEST_CASE("create and use primary inputs in a topology network", "[topo]")
{
    topology_network topo{};

    CHECK(mockturtle::has_create_pi_v<topology_network>);
    CHECK(mockturtle::has_is_constant_v<topology_network>);
    CHECK(mockturtle::has_is_pi_v<topology_network>);
    CHECK(mockturtle::has_num_pis_v<topology_network>);

    CHECK(topo.num_pis() == 0);

    auto x1 = topo.create_pi();
    auto x2 = topo.create_pi();

    CHECK(topo.size() == 4);
    CHECK(topo.num_pis() == 2);
    CHECK(x1 != x2);
}

TEST_CASE("create and use primary outputs in a topology network", "[topo]")
{
    topology_network topo{};

    CHECK(mockturtle::has_create_po_v<topology_network>);
    CHECK(mockturtle::has_num_pos_v<topology_network>);

    auto c0 = topo.get_constant(false);
    auto c1 = topo.get_constant(true);
    auto x  = topo.create_pi();

    topo.create_po(c0);
    topo.create_po(c1);
    topo.create_po(x);

    CHECK(topo.size() == 3);
    CHECK(topo.num_pis() == 1);
    CHECK(topo.num_pos() == 3);
}

TEST_CASE("create and use register in a topology network", "[topo]")
{
    topology_network topo{};

    CHECK(mockturtle::has_foreach_po_v<topology_network>);
    CHECK(mockturtle::has_create_po_v<topology_network>);
    CHECK(mockturtle::has_create_pi_v<topology_network>);
    CHECK(mockturtle::has_create_ro_v<topology_network>);
    CHECK(mockturtle::has_create_ri_v<topology_network>);
    CHECK(mockturtle::has_create_maj_v<topology_network>);

    const auto c0 = topo.get_constant(false);
    const auto x1 = topo.create_pi();
    const auto x2 = topo.create_pi();
    const auto x3 = topo.create_pi();
    const auto x4 = topo.create_pi();

    CHECK(topo.size() == 6);
    CHECK(topo.num_registers() == 0);
    CHECK(topo.num_cis() == 4);
    CHECK(topo.num_cos() == 0);

    const auto f1 = topo.create_maj(x1, x2, x3);
    topo.create_po(f1);

    CHECK(topo.num_pos() == 1);

    const auto s1 = topo.create_ro();  // ntk. input
    topo.create_po(s1);                // po

    const auto f2 = topo.create_maj(f1, x4, c0);
    topo.create_ri(f2);  // ntk. output

    CHECK(topo.num_registers() == 1);
    CHECK(topo.num_cis() == 4 + 1);
    CHECK(topo.num_cos() == 2 + 1);

    topo.foreach_pi(
        [&](auto const& node, auto index)
        {
            CHECK(topo.is_pi(node));
            switch (index)
            {
                case 0:
                    CHECK(node == x1); /* first pi */
                    break;
                case 1:
                    CHECK(node == x2); /* second pi */
                    break;
                case 2:
                    CHECK(node == x3); /* third pi */
                    break;
                case 3:
                    CHECK(node == x4); /* fourth pi */
                    break;
                default: CHECK(false);
            }
        });

    topo.foreach_ci(
        [&](auto const& node, auto index)
        {
            CHECK(topo.is_ci(node));
            switch (index)
            {
                case 0:
                    CHECK(node == x1); /* first pi */
                    break;
                case 1:
                    CHECK(node == x2); /* second pi */
                    break;
                case 2:
                    CHECK(node == x3); /* third pi */
                    break;
                case 3:
                    CHECK(node == x4); /* fourth pi */
                    break;
                case 4:
                    CHECK(node == s1); /* first state-bit */
                    CHECK(topo.is_ci(node));
                    CHECK(!topo.is_pi(node));
                    break;
                default: CHECK(false);
            }
        });

    topo.foreach_po(
        [&](auto const& node, auto index)
        {
            switch (index)
            {
                case 0:
                    CHECK(node == f1); /* first po */
                    break;
                case 1:
                    CHECK(node == s1); /* second po */
                    break;
                default: CHECK(false);
            }
        });

    topo.foreach_co(
        [&](auto const& node, auto index)
        {
            switch (index)
            {
                case 0:
                    CHECK(node == f1); /* first po */
                    break;
                case 1:
                    CHECK(node == s1); /* second po */
                    break;
                case 2:
                    CHECK(node == f2); /* first next-state bit */
                    break;
                default: CHECK(false);
            }
        });
}

TEST_CASE("create unary operations in a topology network", "[topo]")
{
    topology_network topo{};

    CHECK(mockturtle::has_create_buf_v<topology_network>);
    CHECK(mockturtle::has_create_not_v<topology_network>);

    auto x1 = topo.create_pi();

    CHECK(topo.size() == 3);

    auto f1 = topo.create_buf(x1);
    auto f2 = topo.create_not(x1);

    CHECK(topo.size() == 5);
    CHECK(f1 != x1);
    CHECK(f2 != x1);
}

TEST_CASE("create binary operations in a topology network", "[topo]")
{
    topology_network topo{};

    CHECK(mockturtle::has_create_and_v<topology_network>);

    const auto x1 = topo.create_pi();
    const auto x2 = topo.create_pi();

    CHECK(topo.size() == 4);

    topo.create_and(x1, x2);
    CHECK(topo.size() == 5);

    topo.create_and(x1, x2);
    CHECK(topo.size() == 6);  // no structural hashing

    topo.create_and(x2, x1);
    CHECK(topo.size() == 7);  // no structural hashing
}

TEST_CASE("create ternary operations in a topology network", "[topo]")
{
    topology_network topo{};

    REQUIRE(mockturtle::has_create_maj_v<topology_network>);
    REQUIRE(fiction::has_create_dot_v<topology_network>);

    const auto x1 = topo.create_pi();
    const auto x2 = topo.create_pi();
    const auto x3 = topo.create_pi();

    CHECK(topo.size() == 5);

    const auto f1 = topo.create_maj(x1, x2, x3);
    CHECK(topo.size() == 6);

    const auto f2 = topo.create_maj(x1, x2, topo.get_constant(false));
    CHECK(topo.size() == 7);
    CHECK(f1 != f2);

    const auto f3 = topo.create_maj(x1, x2, topo.get_constant(true));
    CHECK(topo.size() == 8);
    CHECK(f2 != f3);


    const auto f4 = topo.create_dot(x1, x2, x3);
    CHECK(topo.size() == 9);

    const auto f5 = topo.create_dot(x1, x2, topo.get_constant(false));
    CHECK(topo.size() == 10);
    CHECK(f4 != f5);

    const auto f6 = topo.create_dot(x1, x2, topo.get_constant(true));
    CHECK(topo.size() == 11);
    CHECK(f5 != f6);
}

TEST_CASE("create n-ary operations in a topology network", "[topo]")
{
    topology_network topo{};

    CHECK(mockturtle::has_create_and_v<topology_network>);

    const auto x1 = topo.create_pi();
    const auto x2 = topo.create_pi();
    const auto x3 = topo.create_pi();
    const auto x4 = topo.create_pi();
    const auto x5 = topo.create_pi();

    CHECK(topo.size() == 7);

    topo.create_nary_and({x1, x2, x3});
    CHECK(topo.size() == 9);

    topo.create_nary_or({x2, x3, x4});
    CHECK(topo.size() == 11);

    topo.create_nary_xor({x3, x4, x5});
    CHECK(topo.size() == 13);
}

TEST_CASE("clone a node in a topology network", "[topo]")
{
    topology_network topo1, topo2;

    CHECK(mockturtle::has_clone_node_v<topology_network>);

    auto a1 = topo1.create_pi();
    auto b1 = topo1.create_pi();
    auto f1 = topo1.create_and(a1, b1);
    CHECK(topo1.size() == 5);

    auto a2 = topo2.create_pi();
    auto b2 = topo2.create_pi();
    CHECK(topo2.size() == 4);

    auto f2 = topo2.clone_node(topo1, topo1.get_node(f1), {a2, b2});
    CHECK(topo2.size() == 5);

    topo2.foreach_fanin(topo2.get_node(f2), [&](auto const& s) { CHECK(!topo2.is_complemented(s)); });
}

TEST_CASE("compute functions from AND and NOT gates in topology networks", "[topo]")
{
    topology_network topo{};

    const auto a = topo.create_pi();
    const auto b = topo.create_pi();

    const auto f1 = topo.create_not(a);
    const auto f2 = topo.create_and(a, b);

    std::vector<kitty::dynamic_truth_table> xs;
    xs.emplace_back(3u);
    xs.emplace_back(3u);
    kitty::create_nth_var(xs[0], 0);
    kitty::create_nth_var(xs[1], 1);

    const auto sim_f1 = topo.compute(topo.get_node(f1), xs.begin(), xs.begin() + 1);
    const auto sim_f2 = topo.compute(topo.get_node(f2), xs.begin(), xs.end());

    CHECK(sim_f1 == ~xs[0]);
    CHECK(sim_f2 == (xs[0] & xs[1]));
}

TEST_CASE("create nodes and compute a function in a topology network", "[topo]")
{
    topology_network topo{};

    CHECK(mockturtle::has_create_node_v<topology_network>);
    CHECK(mockturtle::has_compute_v<topology_network, kitty::dynamic_truth_table>);

    const auto a = topo.create_pi();
    const auto b = topo.create_pi();
    const auto c = topo.create_pi();

    kitty::dynamic_truth_table tt_maj(3u), tt_xor(3u), tt_const0(0u);
    kitty::create_from_hex_string(tt_maj, "e8");
    kitty::create_from_hex_string(tt_xor, "96");

    CHECK(topo.size() == 5);

    const auto _const0 = topo.create_node({}, tt_const0);
    const auto _const1 = topo.create_node({}, ~tt_const0);
    CHECK(_const0 == topo.get_constant(false));
    CHECK(_const1 == topo.get_constant(true));

    const auto _maj = topo.create_node({a, b, c}, tt_maj);
    const auto _xor = topo.create_node({a, b, c}, tt_xor);

    CHECK(topo.size() == 7);

    std::vector<kitty::dynamic_truth_table> xs;
    xs.emplace_back(3u);
    xs.emplace_back(3u);
    xs.emplace_back(3u);
    kitty::create_nth_var(xs[0], 0);
    kitty::create_nth_var(xs[1], 1);
    kitty::create_nth_var(xs[2], 2);

    const auto sim_maj = topo.compute(topo.get_node(_maj), xs.begin(), xs.end());
    const auto sim_xor = topo.compute(topo.get_node(_xor), xs.begin(), xs.end());

    CHECK(sim_maj == kitty::ternary_majority(xs[0], xs[1], xs[2]));
    CHECK(sim_xor == (xs[0] ^ xs[1] ^ xs[2]));
}

TEST_CASE("create fanouts and nodes and compute a function in a topology network", "[topo]")
{
    topology_network topo{};

    CHECK(mockturtle::has_create_node_v<topology_network>);
    CHECK(mockturtle::has_compute_v<topology_network, kitty::dynamic_truth_table>);

    const auto a = topo.create_pi();
    const auto b = topo.create_pi();
    const auto c = topo.create_pi();

    kitty::dynamic_truth_table tt_maj(3u), tt_xor(3u);
    kitty::create_from_hex_string(tt_maj, "e8");
    kitty::create_from_hex_string(tt_xor, "96");

    const auto _foa  = topo.create_buf(a);
    const auto _fob  = topo.create_buf(b);
    const auto _foc  = topo.create_buf(c);
    const auto _maj = topo.create_node({_foa, _fob, _foc}, tt_maj);
    const auto _xor = topo.create_node({_foa, _fob, _foc}, tt_xor);

    std::vector<kitty::dynamic_truth_table> xs;
    xs.emplace_back(3u);
    xs.emplace_back(3u);
    xs.emplace_back(3u);
    kitty::create_nth_var(xs[0], 0);
    kitty::create_nth_var(xs[1], 1);
    kitty::create_nth_var(xs[2], 2);

    const auto sim_maj = topo.compute(topo.get_node(_maj), xs.begin(), xs.end());
    const auto sim_xor = topo.compute(topo.get_node(_xor), xs.begin(), xs.end());

    CHECK(sim_maj == kitty::ternary_majority(xs[0], xs[1], xs[2]));
    CHECK(sim_xor == (xs[0] ^ xs[1] ^ xs[2]));
}

TEST_CASE("hash nodes in topology network", "[topo]")
{
    topology_network topo{};

    const auto a = topo.create_pi();
    const auto b = topo.create_pi();
    const auto c = topo.create_pi();

    kitty::dynamic_truth_table tt_maj(3u), tt_xor(3u);
    kitty::create_from_hex_string(tt_maj, "e8");
    kitty::create_from_hex_string(tt_xor, "96");

    topo.create_node({a, b, c}, tt_maj);
    topo.create_node({a, b, c}, tt_xor);

    CHECK(topo.size() == 7);

    topo.create_node({a, b, c}, tt_maj);

    CHECK(topo.size() == 8);  // no structural hashing
}

TEST_CASE("subsitute node by another", "[topo]")
{
    topology_network topo{};

    const auto c0 = topo.get_node(topo.get_constant(false));
    const auto c1 = topo.get_node(topo.get_constant(true));
    const auto a  = topo.create_pi();
    const auto b  = topo.create_pi();

    kitty::dynamic_truth_table tt_nand(2u), tt_le(2u), tt_ge(2u), tt_or(2u);
    kitty::create_from_hex_string(tt_nand, "7");
    kitty::create_from_hex_string(tt_le, "2");
    kitty::create_from_hex_string(tt_ge, "4");
    kitty::create_from_hex_string(tt_or, "e");

    // XOR with NAND
    const auto n1 = topo.create_node({a, b}, tt_nand);
    const auto n2 = topo.create_node({a, n1}, tt_nand);
    const auto n3 = topo.create_node({b, n1}, tt_nand);
    const auto n4 = topo.create_node({n2, n3}, tt_nand);
    topo.create_po(n4);

    std::vector<mockturtle::node<topology_network>> nodes;
    topo.foreach_node([&](auto node) { nodes.push_back(node); });

    CHECK(nodes == std::vector<topology_network::node>{c0, c1, a, b, n1, n2, n3, n4});
    CHECK(topo.fanout_size(n4) == 1);
    topo.foreach_po(
        [&](auto f)
        {
            CHECK(f == n4);
            return false;
        });

    // XOR with AND and OR
    const auto n5 = topo.create_node({a, b}, tt_le);
    const auto n6 = topo.create_node({a, b}, tt_ge);
    const auto n7 = topo.create_node({n5, n6}, tt_or);

    nodes.clear();
    topo.foreach_node([&](auto node) { nodes.push_back(node); });

    CHECK(nodes == std::vector{c0, c1, a, b, n1, n2, n3, n4, n5, n6, n7});
    CHECK(topo.fanout_size(n7) == 0);

    // substitute nodes
    topo.substitute_node(n4, n7);

    CHECK(topo.size() == 11);
    CHECK(topo.fanout_size(n4) == 0);
    CHECK(topo.fanout_size(n7) == 1);
    topo.foreach_po(
        [&](auto f)
        {
            CHECK(f == n7);
            return false;
        });
}

TEST_CASE("structural properties of a topology network", "[topo]")
{
    topology_network topo{};

    CHECK(mockturtle::has_size_v<topology_network>);
    CHECK(mockturtle::has_num_pis_v<topology_network>);
    CHECK(mockturtle::has_num_pos_v<topology_network>);
    CHECK(mockturtle::has_num_gates_v<topology_network>);
    CHECK(mockturtle::has_fanin_size_v<topology_network>);
    CHECK(mockturtle::has_fanout_size_v<topology_network>);

    const auto x1 = topo.create_pi();
    const auto x2 = topo.create_pi();

    const auto f1 = topo.create_and(x1, x2);
    const auto f2 = topo.create_and(x2, x1);

    topo.create_po(f1);
    topo.create_po(f2);

    CHECK(topo.size() == 6);
    CHECK(topo.num_pis() == 2);
    CHECK(topo.num_pos() == 2);
    CHECK(topo.num_gates() == 2);
    CHECK(topo.fanin_size(topo.get_node(x1)) == 0);
    CHECK(topo.fanin_size(topo.get_node(x2)) == 0);
    CHECK(topo.fanin_size(topo.get_node(f1)) == 2);
    CHECK(topo.fanin_size(topo.get_node(f2)) == 2);
    CHECK(topo.fanout_size(topo.get_node(x1)) == 2);
    CHECK(topo.fanout_size(topo.get_node(x2)) == 2);
    CHECK(topo.fanout_size(topo.get_node(f1)) == 1);
    CHECK(topo.fanout_size(topo.get_node(f2)) == 1);
}

TEST_CASE("Node functions of a topology network", "[topo]")
{
    topology_network topo{};

    CHECK(mockturtle::has_is_and_v<topology_network>);
    CHECK(mockturtle::has_is_or_v<topology_network>);
    CHECK(mockturtle::has_is_or_v<topology_network>);
    CHECK(mockturtle::has_is_xor_v<topology_network>);
    CHECK(mockturtle::has_is_maj_v<topology_network>);
    CHECK(mockturtle::has_is_ite_v<topology_network>);
    CHECK(mockturtle::has_is_xor3_v<topology_network>);

    const auto x1 = topo.create_pi();
    const auto x2 = topo.create_pi();
    const auto x3 = topo.create_pi();

    const auto and_signal  = topo.create_and(x1, x2);
    const auto nand_signal = topo.create_nand(x1, x2);
    const auto or_signal   = topo.create_or(x1, x2);
    const auto nor_signal  = topo.create_nor(x1, x2);
    const auto xor2_signal = topo.create_xor(x1, x2);
    const auto maj_signal  = topo.create_maj(x1, x2, x3);
    const auto dot_signal  = topo.create_dot(x1, x2, x3);
    const auto ite_signal  = topo.create_ite(x1, x2, x3);
    const auto xor3_signal = topo.create_xor3(x1, x2, x3);

    topo.create_po(and_signal);
    topo.create_po(nand_signal);
    topo.create_po(or_signal);
    topo.create_po(nor_signal);
    topo.create_po(xor2_signal);
    topo.create_po(maj_signal);
    topo.create_po(dot_signal);
    topo.create_po(ite_signal);
    topo.create_po(xor3_signal);

    CHECK(topo.is_and(topo.get_node(and_signal)));
    CHECK(topo.is_nand(topo.get_node(nand_signal)));
    CHECK(topo.is_or(topo.get_node(or_signal)));
    CHECK(topo.is_nor(topo.get_node(nor_signal)));
    CHECK(topo.is_xor(topo.get_node(xor2_signal)));
    CHECK(topo.is_maj(topo.get_node(maj_signal)));
    CHECK(topo.is_dot(topo.get_node(dot_signal)));
    CHECK(topo.is_ite(topo.get_node(ite_signal)));
    CHECK(topo.is_xor3(topo.get_node(xor3_signal)));
}

TEST_CASE("node and signal iteration in a topology network", "[topo]")
{
    topology_network topo{};

    CHECK(mockturtle::has_foreach_node_v<topology_network>);
    CHECK(mockturtle::has_foreach_pi_v<topology_network>);
    CHECK(mockturtle::has_foreach_po_v<topology_network>);
    CHECK(mockturtle::has_foreach_fanin_v<topology_network>);

    const auto x1 = topo.create_pi();
    const auto x2 = topo.create_pi();
    const auto f1 = topo.create_and(x1, x2);
    const auto f2 = topo.create_and(x2, x1);
    topo.create_po(f1);
    topo.create_po(f2);

    CHECK(topo.size() == 6);

    /* iterate over nodes */
    uint32_t mask{0}, counter{0};
    topo.foreach_node(
        [&](auto n, auto i)
        {
            mask |= (1u << n);
            counter += i;
        });
    CHECK(mask == 63);
    CHECK(counter == 15);

    mask = 0;
    topo.foreach_node([&](auto n) { mask |= (1u << n); });
    CHECK(mask == 63);

    mask = counter = 0;
    topo.foreach_node(
        [&](auto n, auto i)
        {
            mask |= (1u << n);
            counter += i;
            return false;
        });
    CHECK(mask == 1);
    CHECK(counter == 0);

    mask = 0;
    topo.foreach_node(
        [&](auto n)
        {
            mask |= (1u << n);
            return false;
        });
    CHECK(mask == 1);

    /* iterate over PIs */
    mask = counter = 0;
    topo.foreach_pi(
        [&](auto n, auto i)
        {
            mask |= (1u << n);
            counter += i;
        });
    CHECK(mask == 12);
    CHECK(counter == 1);

    mask = 0;
    topo.foreach_pi([&](auto n) { mask |= (1u << n); });
    CHECK(mask == 12);

    mask = counter = 0;
    topo.foreach_pi(
        [&](auto n, auto i)
        {
            mask |= (1u << n);
            counter += i;
            return false;
        });
    CHECK(mask == 4);
    CHECK(counter == 0);

    mask = 0;
    topo.foreach_pi(
        [&](auto n)
        {
            mask |= (1u << n);
            return false;
        });
    CHECK(mask == 4);

    /* iterate over POs */
    mask = counter = 0;
    topo.foreach_po(
        [&](auto s, auto i)
        {
            mask |= (1u << topo.get_node(s));
            counter += i;
        });
    CHECK(mask == 48);
    CHECK(counter == 1);

    mask = 0;
    topo.foreach_po([&](auto s) { mask |= (1u << topo.get_node(s)); });
    CHECK(mask == 48);

    mask = counter = 0;
    topo.foreach_po(
        [&](auto s, auto i)
        {
            mask |= (1u << topo.get_node(s));
            counter += i;
            return false;
        });
    CHECK(mask == 16);
    CHECK(counter == 0);

    mask = 0;
    topo.foreach_po(
        [&](auto s)
        {
            mask |= (1u << topo.get_node(s));
            return false;
        });
    CHECK(mask == 16);
}

TEST_CASE("custom node values in topology networks", "[topo]")
{
    topology_network topo{};

    CHECK(mockturtle::has_clear_values_v<topology_network>);
    CHECK(mockturtle::has_value_v<topology_network>);
    CHECK(mockturtle::has_set_value_v<topology_network>);
    CHECK(mockturtle::has_incr_value_v<topology_network>);
    CHECK(mockturtle::has_decr_value_v<topology_network>);

    const auto x1 = topo.create_pi();
    const auto x2 = topo.create_pi();
    const auto f1 = topo.create_and(x1, x2);
    const auto f2 = topo.create_and(x2, x1);
    topo.create_po(f1);
    topo.create_po(f2);

    CHECK(topo.size() == 6);

    topo.clear_values();
    topo.foreach_node(
        [&](auto n)
        {
            CHECK(topo.value(n) == 0);
            topo.set_value(n, static_cast<uint32_t>(n));
            CHECK(topo.value(n) == n);
            CHECK(topo.incr_value(n) == n);
            CHECK(topo.value(n) == n + 1);
            CHECK(topo.decr_value(n) == n);
            CHECK(topo.value(n) == n);
        });
    topo.clear_values();
    topo.foreach_node([&](auto n) { CHECK(topo.value(n) == 0); });
}

TEST_CASE("visited values in topology networks", "[topo]")
{
    topology_network topo{};

    CHECK(mockturtle::has_clear_visited_v<topology_network>);
    CHECK(mockturtle::has_visited_v<topology_network>);
    CHECK(mockturtle::has_set_visited_v<topology_network>);

    const auto x1 = topo.create_pi();
    const auto x2 = topo.create_pi();
    const auto f1 = topo.create_and(x1, x2);
    const auto f2 = topo.create_and(x2, x1);
    topo.create_po(f1);
    topo.create_po(f2);

    CHECK(topo.size() == 6);

    topo.clear_visited();
    topo.foreach_node(
        [&](auto n)
        {
            CHECK(topo.visited(n) == 0);
            topo.set_visited(n, static_cast<uint32_t>(n));
            CHECK(topo.visited(n) == n);
        });
    topo.clear_visited();
    topo.foreach_node([&](auto n) { CHECK(topo.visited(n) == 0); });
}
