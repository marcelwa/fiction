//
// Created by marcel on 02.06.21.
//

#include "catch.hpp"
#include "utils/blueprints/network_blueprints.hpp"
#include "utils/equivalence_checking.hpp"

#include <fiction/algorithms/fanout_substitution.hpp>
#include <fiction/algorithms/orthogonal.hpp>
#include <fiction/io/print_layout.hpp>
#include <fiction/layouts/clocked_layout.hpp>
#include <fiction/layouts/gate_level_layout.hpp>
#include <fiction/layouts/tile_based_layout.hpp>
#include <fiction/networks/topology_network.hpp>

#include <mockturtle/networks/aig.hpp>
#include <mockturtle/networks/mig.hpp>
#include <mockturtle/views/fanout_view.hpp>

#include <type_traits>

using namespace fiction;

TEST_CASE("Number of constant fanins", "[algorithms]")
{
    const auto maj4 = blueprints::maj4_network<mockturtle::mig_network>();

    maj4.foreach_node([&maj4](const auto& n) { CHECK(detail::num_constant_fanins(maj4, n) == 0ul); });

    const auto and_inv = blueprints::unbalanced_and_inv_network<mockturtle::mig_network>();

    CHECK(detail::num_constant_fanins(and_inv, 3) == 1ul);
}

TEST_CASE("High-degree fanin nodes", "[algorithms]")
{
    const auto maj4 = blueprints::maj4_network<mockturtle::mig_network>();

    CHECK(detail::has_high_degree_fanin_nodes(maj4, 2));
    CHECK(!detail::has_high_degree_fanin_nodes(maj4, 3));

    const auto and_inv = blueprints::unbalanced_and_inv_network<mockturtle::mig_network>();

    CHECK(detail::has_high_degree_fanin_nodes(and_inv, 1));
    CHECK(!detail::has_high_degree_fanin_nodes(and_inv, 2));
    CHECK(!detail::has_high_degree_fanin_nodes(and_inv, 3));
}

TEST_CASE("East-south coloring", "[algorithms]")
{
    const auto check = [](const auto& net)
    {
        auto container = detail::east_south_coloring(net);
        CHECK(detail::is_east_south_colored(container.color_ntk));
    };

    check(mockturtle::fanout_view{
        fanout_substitution<topology_network>(blueprints::unbalanced_and_inv_network<mockturtle::aig_network>())});
    check(mockturtle::fanout_view{
        fanout_substitution<topology_network>(blueprints::maj1_network<mockturtle::aig_network>())});
    check(mockturtle::fanout_view{
        fanout_substitution<topology_network>(blueprints::maj4_network<mockturtle::aig_network>())});
    check(mockturtle::fanout_view{
        fanout_substitution<topology_network>(blueprints::se_coloring_corner_case_network<topology_network>())});
    check(mockturtle::fanout_view{fanout_substitution<topology_network>(
        blueprints::fanout_substitution_corner_case_network<topology_network>())});
    check(mockturtle::fanout_view{
        fanout_substitution<topology_network>(blueprints::nary_operation_network<topology_network>())});
}

TEST_CASE("Layout equivalence", "[algorithms]")
{
    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout>>;

    const auto check = [](const auto& net)
    {
        orthogonal_physical_design_params params{true};
        orthogonal_physical_design_stats  stats{};

        auto layout = orthogonal<gate_layout>(net, params, &stats);

        print_gate_level_layout(std::cout, layout);

        check_eq(net, layout);
    };

    check(mockturtle::fanout_view{
        fanout_substitution<topology_network>(blueprints::unbalanced_and_inv_network<mockturtle::aig_network>())});
    check(mockturtle::fanout_view{
        fanout_substitution<topology_network>(blueprints::maj1_network<mockturtle::aig_network>())});
    check(mockturtle::fanout_view{
        fanout_substitution<topology_network>(blueprints::maj4_network<mockturtle::aig_network>())});
    check(mockturtle::fanout_view{
        fanout_substitution<topology_network>(blueprints::se_coloring_corner_case_network<topology_network>())});
    check(mockturtle::fanout_view{fanout_substitution<topology_network>(
        blueprints::fanout_substitution_corner_case_network<topology_network>())});
    check(mockturtle::fanout_view{
        fanout_substitution<topology_network>(blueprints::nary_operation_network<topology_network>())});

    // constant input network
    check(mockturtle::fanout_view{
        fanout_substitution<topology_network>(blueprints::unbalanced_and_inv_network<mockturtle::mig_network>())});
}
