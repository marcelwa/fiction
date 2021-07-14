//
// Created by marcel on 25.05.21.
//

#include "catch.hpp"
#include "utils/blueprints/network_blueprints.hpp"
#include "utils/equivalence_checking.hpp"

#include <fiction/algorithms/network_balancing.hpp>
#include <fiction/networks/topology_network.hpp>

#include <kitty/dynamic_truth_table.hpp>
#include <mockturtle/networks/aig.hpp>
#include <mockturtle/networks/mig.hpp>
#include <mockturtle/networks/xag.hpp>

#include <type_traits>

using namespace fiction;

TEST_CASE("Simple network balancing", "[algorithms]")
{
    const auto top = blueprints::unbalanced_and_inv_network<topology_network>();

    const auto balanced = network_balancing<topology_network>(top);

    CHECK(balanced.size() == top.size() + 1);
    CHECK(is_balanced(balanced));

    check_eq(top, balanced);
}

TEST_CASE("Complex network balancing", "[algorithms]")
{
    const auto aig = blueprints::maj4_network<mockturtle::aig_network>();
    const auto mig = blueprints::maj4_network<mockturtle::mig_network>();
    const auto xag = blueprints::maj4_network<mockturtle::xag_network>();
    const auto top = blueprints::maj4_network<fiction::topology_network>();

    SECTION("without primary output unification")
    {
        auto balanced_aig = network_balancing<topology_network>(aig);
        auto balanced_mig = network_balancing<topology_network>(mig);
        auto balanced_xag = network_balancing<topology_network>(xag);
        auto balanced_top = network_balancing<topology_network>(top);

        CHECK(is_balanced(balanced_aig));
        CHECK(is_balanced(balanced_mig));
        CHECK(is_balanced(balanced_xag));
        CHECK(is_balanced(balanced_top));

        CHECK(!is_balanced(balanced_aig, {true}));
        CHECK(!is_balanced(balanced_mig, {true}));
        CHECK(!is_balanced(balanced_xag, {true}));
        CHECK(!is_balanced(balanced_top, {true}));

        check_eq(aig, balanced_aig);
        check_eq(mig, balanced_mig);
        check_eq(xag, balanced_xag);
        check_eq(top, balanced_top);
    }
    SECTION("with primary output unification")
    {
        auto balanced_aig = network_balancing<topology_network>(aig, {true});
        auto balanced_mig = network_balancing<topology_network>(mig, {true});
        auto balanced_xag = network_balancing<topology_network>(xag, {true});
        auto balanced_top = network_balancing<topology_network>(top, {true});

        CHECK(is_balanced(balanced_aig, {true}));
        CHECK(is_balanced(balanced_mig, {true}));
        CHECK(is_balanced(balanced_xag, {true}));
        CHECK(is_balanced(balanced_top, {true}));

        check_eq(aig, balanced_aig);
        check_eq(mig, balanced_mig);
        check_eq(xag, balanced_xag);
        check_eq(top, balanced_top);
    }
}
