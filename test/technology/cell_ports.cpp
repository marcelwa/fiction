//
// Created by marcel on 03.07.21.
//

#include "catch.hpp"

#include <fiction/technology/cell_ports.hpp>

#include <unordered_set>

using namespace fiction;

TEST_CASE("Ports", "[cell-ports]")
{
    port p_00{0, 0}, p_10{1, 0}, p_01{0, 1};

    CHECK(p_00 == port{});
    CHECK(p_10 == port{1, 0});
    CHECK(p_01 == port{0, 1});
    CHECK(!(p_00 == p_01));
    CHECK(!(p_00 == p_10));
    CHECK(!(p_10 == p_01));
    CHECK(p_00 < p_10);
    CHECK(p_00 < p_01);
    CHECK(p_01 < p_10);

    std::unordered_set<port> port_set{};

    port_set.insert(p_00);
    port_set.insert(p_10);
    port_set.insert(p_01);

    CHECK(port_set.size() == 3);

    port_set.insert(p_00);
    port_set.insert(p_10);
    port_set.insert(p_01);

    CHECK(port_set.size() == 3);
}

TEST_CASE("Port lists", "[cell-ports]")
{
    port p_02{0, 2}, p_24{2, 4};

    port_list p_list_02_24{{p_02}, {p_24}};
    port_list p_list_24_02{{p_24}, {p_02}};

    CHECK(!(p_list_02_24 == p_list_24_02));
    CHECK(!(port_list{} == p_list_24_02));
    CHECK(!(p_list_02_24 == port_list{}));

    CHECK(p_list_02_24 == port_list{{p_02}, {p_24}});
    CHECK(p_list_24_02 == port_list{{p_24}, {p_02}});
}
