//
// Created by marcel on 12.05.21.
//

#include "catch.hpp"

#include <fiction/layouts/clocking_scheme.hpp>

#include <string>
#include <vector>

using namespace fiction;

TEST_CASE("3-phase 2DDWave", "[clocking scheme]")
{
    CHECK(twoddwave_3_clocking.is_regular());

    CHECK(twoddwave_3_clocking(coord_t{0, 0}) == 0);
    CHECK(twoddwave_3_clocking(coord_t{0, 1}) == 1);
    CHECK(twoddwave_3_clocking(coord_t{0, 2}) == 2);
    CHECK(twoddwave_3_clocking(coord_t{1, 0}) == 1);
    CHECK(twoddwave_3_clocking(coord_t{1, 1}) == 2);
    CHECK(twoddwave_3_clocking(coord_t{1, 2}) == 0);
    CHECK(twoddwave_3_clocking(coord_t{2, 0}) == 2);
    CHECK(twoddwave_3_clocking(coord_t{2, 1}) == 0);
    CHECK(twoddwave_3_clocking(coord_t{2, 2}) == 1);

    CHECK(twoddwave_3_clocking(coord_t{0 + 3, 0}) == 0);
    CHECK(twoddwave_3_clocking(coord_t{0 + 3, 1}) == 1);
    CHECK(twoddwave_3_clocking(coord_t{0 + 3, 2}) == 2);
    CHECK(twoddwave_3_clocking(coord_t{1 + 3, 0}) == 1);
    CHECK(twoddwave_3_clocking(coord_t{1 + 3, 1}) == 2);
    CHECK(twoddwave_3_clocking(coord_t{1 + 3, 2}) == 0);
    CHECK(twoddwave_3_clocking(coord_t{2 + 3, 0}) == 2);
    CHECK(twoddwave_3_clocking(coord_t{2 + 3, 1}) == 0);
    CHECK(twoddwave_3_clocking(coord_t{2 + 3, 2}) == 1);

    CHECK(twoddwave_3_clocking(coord_t{0, 0 + 3}) == 0);
    CHECK(twoddwave_3_clocking(coord_t{0, 1 + 3}) == 1);
    CHECK(twoddwave_3_clocking(coord_t{0, 2 + 3}) == 2);
    CHECK(twoddwave_3_clocking(coord_t{1, 0 + 3}) == 1);
    CHECK(twoddwave_3_clocking(coord_t{1, 1 + 3}) == 2);
    CHECK(twoddwave_3_clocking(coord_t{1, 2 + 3}) == 0);
    CHECK(twoddwave_3_clocking(coord_t{2, 0 + 3}) == 2);
    CHECK(twoddwave_3_clocking(coord_t{2, 1 + 3}) == 0);
    CHECK(twoddwave_3_clocking(coord_t{2, 2 + 3}) == 1);

    CHECK(twoddwave_3_clocking(coord_t{0 + 3, 0 + 3}) == 0);
    CHECK(twoddwave_3_clocking(coord_t{0 + 3, 1 + 3}) == 1);
    CHECK(twoddwave_3_clocking(coord_t{0 + 3, 2 + 3}) == 2);
    CHECK(twoddwave_3_clocking(coord_t{1 + 3, 0 + 3}) == 1);
    CHECK(twoddwave_3_clocking(coord_t{1 + 3, 1 + 3}) == 2);
    CHECK(twoddwave_3_clocking(coord_t{1 + 3, 2 + 3}) == 0);
    CHECK(twoddwave_3_clocking(coord_t{2 + 3, 0 + 3}) == 2);
    CHECK(twoddwave_3_clocking(coord_t{2 + 3, 1 + 3}) == 0);
    CHECK(twoddwave_3_clocking(coord_t{2 + 3, 2 + 3}) == 1);
}

TEST_CASE("4-phase 2DDWave", "[clocking scheme]")
{
    CHECK(twoddwave_4_clocking.is_regular());

    CHECK(twoddwave_4_clocking(coord_t{0, 0}) == 0);
    CHECK(twoddwave_4_clocking(coord_t{0, 1}) == 1);
    CHECK(twoddwave_4_clocking(coord_t{0, 2}) == 2);
    CHECK(twoddwave_4_clocking(coord_t{0, 3}) == 3);
    CHECK(twoddwave_4_clocking(coord_t{1, 0}) == 1);
    CHECK(twoddwave_4_clocking(coord_t{1, 1}) == 2);
    CHECK(twoddwave_4_clocking(coord_t{1, 2}) == 3);
    CHECK(twoddwave_4_clocking(coord_t{1, 3}) == 0);
    CHECK(twoddwave_4_clocking(coord_t{2, 0}) == 2);
    CHECK(twoddwave_4_clocking(coord_t{2, 1}) == 3);
    CHECK(twoddwave_4_clocking(coord_t{2, 2}) == 0);
    CHECK(twoddwave_4_clocking(coord_t{2, 3}) == 1);
    CHECK(twoddwave_4_clocking(coord_t{3, 0}) == 3);
    CHECK(twoddwave_4_clocking(coord_t{3, 1}) == 0);
    CHECK(twoddwave_4_clocking(coord_t{3, 2}) == 1);
    CHECK(twoddwave_4_clocking(coord_t{3, 3}) == 2);

    CHECK(twoddwave_4_clocking(coord_t{0 + 4, 0}) == 0);
    CHECK(twoddwave_4_clocking(coord_t{0 + 4, 1}) == 1);
    CHECK(twoddwave_4_clocking(coord_t{0 + 4, 2}) == 2);
    CHECK(twoddwave_4_clocking(coord_t{0 + 4, 3}) == 3);
    CHECK(twoddwave_4_clocking(coord_t{1 + 4, 0}) == 1);
    CHECK(twoddwave_4_clocking(coord_t{1 + 4, 1}) == 2);
    CHECK(twoddwave_4_clocking(coord_t{1 + 4, 2}) == 3);
    CHECK(twoddwave_4_clocking(coord_t{1 + 4, 3}) == 0);
    CHECK(twoddwave_4_clocking(coord_t{2 + 4, 0}) == 2);
    CHECK(twoddwave_4_clocking(coord_t{2 + 4, 1}) == 3);
    CHECK(twoddwave_4_clocking(coord_t{2 + 4, 2}) == 0);
    CHECK(twoddwave_4_clocking(coord_t{2 + 4, 3}) == 1);
    CHECK(twoddwave_4_clocking(coord_t{3 + 4, 0}) == 3);
    CHECK(twoddwave_4_clocking(coord_t{3 + 4, 1}) == 0);
    CHECK(twoddwave_4_clocking(coord_t{3 + 4, 2}) == 1);
    CHECK(twoddwave_4_clocking(coord_t{3 + 4, 3}) == 2);

    CHECK(twoddwave_4_clocking(coord_t{0, 0 + 4}) == 0);
    CHECK(twoddwave_4_clocking(coord_t{0, 1 + 4}) == 1);
    CHECK(twoddwave_4_clocking(coord_t{0, 2 + 4}) == 2);
    CHECK(twoddwave_4_clocking(coord_t{0, 3 + 4}) == 3);
    CHECK(twoddwave_4_clocking(coord_t{1, 0 + 4}) == 1);
    CHECK(twoddwave_4_clocking(coord_t{1, 1 + 4}) == 2);
    CHECK(twoddwave_4_clocking(coord_t{1, 2 + 4}) == 3);
    CHECK(twoddwave_4_clocking(coord_t{1, 3 + 4}) == 0);
    CHECK(twoddwave_4_clocking(coord_t{2, 0 + 4}) == 2);
    CHECK(twoddwave_4_clocking(coord_t{2, 1 + 4}) == 3);
    CHECK(twoddwave_4_clocking(coord_t{2, 2 + 4}) == 0);
    CHECK(twoddwave_4_clocking(coord_t{2, 3 + 4}) == 1);
    CHECK(twoddwave_4_clocking(coord_t{3, 0 + 4}) == 3);
    CHECK(twoddwave_4_clocking(coord_t{3, 1 + 4}) == 0);
    CHECK(twoddwave_4_clocking(coord_t{3, 2 + 4}) == 1);
    CHECK(twoddwave_4_clocking(coord_t{3, 3 + 4}) == 2);

    CHECK(twoddwave_4_clocking(coord_t{0 + 4, 0 + 4}) == 0);
    CHECK(twoddwave_4_clocking(coord_t{0 + 4, 1 + 4}) == 1);
    CHECK(twoddwave_4_clocking(coord_t{0 + 4, 2 + 4}) == 2);
    CHECK(twoddwave_4_clocking(coord_t{0 + 4, 3 + 4}) == 3);
    CHECK(twoddwave_4_clocking(coord_t{1 + 4, 0 + 4}) == 1);
    CHECK(twoddwave_4_clocking(coord_t{1 + 4, 1 + 4}) == 2);
    CHECK(twoddwave_4_clocking(coord_t{1 + 4, 2 + 4}) == 3);
    CHECK(twoddwave_4_clocking(coord_t{1 + 4, 3 + 4}) == 0);
    CHECK(twoddwave_4_clocking(coord_t{2 + 4, 0 + 4}) == 2);
    CHECK(twoddwave_4_clocking(coord_t{2 + 4, 1 + 4}) == 3);
    CHECK(twoddwave_4_clocking(coord_t{2 + 4, 2 + 4}) == 0);
    CHECK(twoddwave_4_clocking(coord_t{2 + 4, 3 + 4}) == 1);
    CHECK(twoddwave_4_clocking(coord_t{3 + 4, 0 + 4}) == 3);
    CHECK(twoddwave_4_clocking(coord_t{3 + 4, 1 + 4}) == 0);
    CHECK(twoddwave_4_clocking(coord_t{3 + 4, 2 + 4}) == 1);
    CHECK(twoddwave_4_clocking(coord_t{3 + 4, 3 + 4}) == 2);
}

TEST_CASE("4-phase USE", "[clocking scheme]")
{
    CHECK(use_4_clocking.is_regular());

    CHECK(use_4_clocking(coord_t{0, 0}) == 0);
    CHECK(use_4_clocking(coord_t{0, 1}) == 3);
    CHECK(use_4_clocking(coord_t{0, 2}) == 2);
    CHECK(use_4_clocking(coord_t{0, 3}) == 1);
    CHECK(use_4_clocking(coord_t{1, 0}) == 1);
    CHECK(use_4_clocking(coord_t{1, 1}) == 2);
    CHECK(use_4_clocking(coord_t{1, 2}) == 3);
    CHECK(use_4_clocking(coord_t{1, 3}) == 0);
    CHECK(use_4_clocking(coord_t{2, 0}) == 2);
    CHECK(use_4_clocking(coord_t{2, 1}) == 1);
    CHECK(use_4_clocking(coord_t{2, 2}) == 0);
    CHECK(use_4_clocking(coord_t{2, 3}) == 3);
    CHECK(use_4_clocking(coord_t{3, 0}) == 3);
    CHECK(use_4_clocking(coord_t{3, 1}) == 0);
    CHECK(use_4_clocking(coord_t{3, 2}) == 1);
    CHECK(use_4_clocking(coord_t{3, 3}) == 2);

    CHECK(use_4_clocking(coord_t{0 + 4, 0}) == 0);
    CHECK(use_4_clocking(coord_t{0 + 4, 1}) == 3);
    CHECK(use_4_clocking(coord_t{0 + 4, 2}) == 2);
    CHECK(use_4_clocking(coord_t{0 + 4, 3}) == 1);
    CHECK(use_4_clocking(coord_t{1 + 4, 0}) == 1);
    CHECK(use_4_clocking(coord_t{1 + 4, 1}) == 2);
    CHECK(use_4_clocking(coord_t{1 + 4, 2}) == 3);
    CHECK(use_4_clocking(coord_t{1 + 4, 3}) == 0);
    CHECK(use_4_clocking(coord_t{2 + 4, 0}) == 2);
    CHECK(use_4_clocking(coord_t{2 + 4, 1}) == 1);
    CHECK(use_4_clocking(coord_t{2 + 4, 2}) == 0);
    CHECK(use_4_clocking(coord_t{2 + 4, 3}) == 3);
    CHECK(use_4_clocking(coord_t{3 + 4, 0}) == 3);
    CHECK(use_4_clocking(coord_t{3 + 4, 1}) == 0);
    CHECK(use_4_clocking(coord_t{3 + 4, 2}) == 1);
    CHECK(use_4_clocking(coord_t{3 + 4, 3}) == 2);

    CHECK(use_4_clocking(coord_t{0, 0 + 4}) == 0);
    CHECK(use_4_clocking(coord_t{0, 1 + 4}) == 3);
    CHECK(use_4_clocking(coord_t{0, 2 + 4}) == 2);
    CHECK(use_4_clocking(coord_t{0, 3 + 4}) == 1);
    CHECK(use_4_clocking(coord_t{1, 0 + 4}) == 1);
    CHECK(use_4_clocking(coord_t{1, 1 + 4}) == 2);
    CHECK(use_4_clocking(coord_t{1, 2 + 4}) == 3);
    CHECK(use_4_clocking(coord_t{1, 3 + 4}) == 0);
    CHECK(use_4_clocking(coord_t{2, 0 + 4}) == 2);
    CHECK(use_4_clocking(coord_t{2, 1 + 4}) == 1);
    CHECK(use_4_clocking(coord_t{2, 2 + 4}) == 0);
    CHECK(use_4_clocking(coord_t{2, 3 + 4}) == 3);
    CHECK(use_4_clocking(coord_t{3, 0 + 4}) == 3);
    CHECK(use_4_clocking(coord_t{3, 1 + 4}) == 0);
    CHECK(use_4_clocking(coord_t{3, 2 + 4}) == 1);
    CHECK(use_4_clocking(coord_t{3, 3 + 4}) == 2);

    CHECK(use_4_clocking(coord_t{0 + 4, 0 + 4}) == 0);
    CHECK(use_4_clocking(coord_t{0 + 4, 1 + 4}) == 3);
    CHECK(use_4_clocking(coord_t{0 + 4, 2 + 4}) == 2);
    CHECK(use_4_clocking(coord_t{0 + 4, 3 + 4}) == 1);
    CHECK(use_4_clocking(coord_t{1 + 4, 0 + 4}) == 1);
    CHECK(use_4_clocking(coord_t{1 + 4, 1 + 4}) == 2);
    CHECK(use_4_clocking(coord_t{1 + 4, 2 + 4}) == 3);
    CHECK(use_4_clocking(coord_t{1 + 4, 3 + 4}) == 0);
    CHECK(use_4_clocking(coord_t{2 + 4, 0 + 4}) == 2);
    CHECK(use_4_clocking(coord_t{2 + 4, 1 + 4}) == 1);
    CHECK(use_4_clocking(coord_t{2 + 4, 2 + 4}) == 0);
    CHECK(use_4_clocking(coord_t{2 + 4, 3 + 4}) == 3);
    CHECK(use_4_clocking(coord_t{3 + 4, 0 + 4}) == 3);
    CHECK(use_4_clocking(coord_t{3 + 4, 1 + 4}) == 0);
    CHECK(use_4_clocking(coord_t{3 + 4, 2 + 4}) == 1);
    CHECK(use_4_clocking(coord_t{3 + 4, 3 + 4}) == 2);
}

TEST_CASE("4-phase RES", "[clocking scheme]")
{
    CHECK(res_4_clocking.is_regular());

    CHECK(res_4_clocking(coord_t{0, 0}) == 3);
    CHECK(res_4_clocking(coord_t{0, 1}) == 0);
    CHECK(res_4_clocking(coord_t{0, 2}) == 1);
    CHECK(res_4_clocking(coord_t{0, 3}) == 0);
    CHECK(res_4_clocking(coord_t{1, 0}) == 0);
    CHECK(res_4_clocking(coord_t{1, 1}) == 1);
    CHECK(res_4_clocking(coord_t{1, 2}) == 2);
    CHECK(res_4_clocking(coord_t{1, 3}) == 3);
    CHECK(res_4_clocking(coord_t{2, 0}) == 1);
    CHECK(res_4_clocking(coord_t{2, 1}) == 0);
    CHECK(res_4_clocking(coord_t{2, 2}) == 3);
    CHECK(res_4_clocking(coord_t{2, 3}) == 2);
    CHECK(res_4_clocking(coord_t{3, 0}) == 2);
    CHECK(res_4_clocking(coord_t{3, 1}) == 3);
    CHECK(res_4_clocking(coord_t{3, 2}) == 0);
    CHECK(res_4_clocking(coord_t{3, 3}) == 1);

    CHECK(res_4_clocking(coord_t{0 + 4, 0}) == 3);
    CHECK(res_4_clocking(coord_t{0 + 4, 1}) == 0);
    CHECK(res_4_clocking(coord_t{0 + 4, 2}) == 1);
    CHECK(res_4_clocking(coord_t{0 + 4, 3}) == 0);
    CHECK(res_4_clocking(coord_t{1 + 4, 0}) == 0);
    CHECK(res_4_clocking(coord_t{1 + 4, 1}) == 1);
    CHECK(res_4_clocking(coord_t{1 + 4, 2}) == 2);
    CHECK(res_4_clocking(coord_t{1 + 4, 3}) == 3);
    CHECK(res_4_clocking(coord_t{2 + 4, 0}) == 1);
    CHECK(res_4_clocking(coord_t{2 + 4, 1}) == 0);
    CHECK(res_4_clocking(coord_t{2 + 4, 2}) == 3);
    CHECK(res_4_clocking(coord_t{2 + 4, 3}) == 2);
    CHECK(res_4_clocking(coord_t{3 + 4, 0}) == 2);
    CHECK(res_4_clocking(coord_t{3 + 4, 1}) == 3);
    CHECK(res_4_clocking(coord_t{3 + 4, 2}) == 0);
    CHECK(res_4_clocking(coord_t{3 + 4, 3}) == 1);

    CHECK(res_4_clocking(coord_t{0, 0 + 4}) == 3);
    CHECK(res_4_clocking(coord_t{0, 1 + 4}) == 0);
    CHECK(res_4_clocking(coord_t{0, 2 + 4}) == 1);
    CHECK(res_4_clocking(coord_t{0, 3 + 4}) == 0);
    CHECK(res_4_clocking(coord_t{1, 0 + 4}) == 0);
    CHECK(res_4_clocking(coord_t{1, 1 + 4}) == 1);
    CHECK(res_4_clocking(coord_t{1, 2 + 4}) == 2);
    CHECK(res_4_clocking(coord_t{1, 3 + 4}) == 3);
    CHECK(res_4_clocking(coord_t{2, 0 + 4}) == 1);
    CHECK(res_4_clocking(coord_t{2, 1 + 4}) == 0);
    CHECK(res_4_clocking(coord_t{2, 2 + 4}) == 3);
    CHECK(res_4_clocking(coord_t{2, 3 + 4}) == 2);
    CHECK(res_4_clocking(coord_t{3, 0 + 4}) == 2);
    CHECK(res_4_clocking(coord_t{3, 1 + 4}) == 3);
    CHECK(res_4_clocking(coord_t{3, 2 + 4}) == 0);
    CHECK(res_4_clocking(coord_t{3, 3 + 4}) == 1);

    CHECK(res_4_clocking(coord_t{0 + 4, 0 + 4}) == 3);
    CHECK(res_4_clocking(coord_t{0 + 4, 1 + 4}) == 0);
    CHECK(res_4_clocking(coord_t{0 + 4, 2 + 4}) == 1);
    CHECK(res_4_clocking(coord_t{0 + 4, 3 + 4}) == 0);
    CHECK(res_4_clocking(coord_t{1 + 4, 0 + 4}) == 0);
    CHECK(res_4_clocking(coord_t{1 + 4, 1 + 4}) == 1);
    CHECK(res_4_clocking(coord_t{1 + 4, 2 + 4}) == 2);
    CHECK(res_4_clocking(coord_t{1 + 4, 3 + 4}) == 3);
    CHECK(res_4_clocking(coord_t{2 + 4, 0 + 4}) == 1);
    CHECK(res_4_clocking(coord_t{2 + 4, 1 + 4}) == 0);
    CHECK(res_4_clocking(coord_t{2 + 4, 2 + 4}) == 3);
    CHECK(res_4_clocking(coord_t{2 + 4, 3 + 4}) == 2);
    CHECK(res_4_clocking(coord_t{3 + 4, 0 + 4}) == 2);
    CHECK(res_4_clocking(coord_t{3 + 4, 1 + 4}) == 3);
    CHECK(res_4_clocking(coord_t{3 + 4, 2 + 4}) == 0);
    CHECK(res_4_clocking(coord_t{3 + 4, 3 + 4}) == 1);
}

TEST_CASE("3-phase BANCS", "[clocking scheme]")
{
    CHECK(bancs_3_clocking.is_regular());

    CHECK(bancs_3_clocking(coord_t{0, 0}) == 0);
    CHECK(bancs_3_clocking(coord_t{0, 1}) == 2);
    CHECK(bancs_3_clocking(coord_t{0, 2}) == 2);
    CHECK(bancs_3_clocking(coord_t{0, 3}) == 1);
    CHECK(bancs_3_clocking(coord_t{0, 4}) == 1);
    CHECK(bancs_3_clocking(coord_t{0, 5}) == 0);
    CHECK(bancs_3_clocking(coord_t{1, 0}) == 1);
    CHECK(bancs_3_clocking(coord_t{1, 1}) == 1);
    CHECK(bancs_3_clocking(coord_t{1, 2}) == 0);
    CHECK(bancs_3_clocking(coord_t{1, 3}) == 0);
    CHECK(bancs_3_clocking(coord_t{1, 4}) == 2);
    CHECK(bancs_3_clocking(coord_t{1, 5}) == 2);
    CHECK(bancs_3_clocking(coord_t{2, 0}) == 2);
    CHECK(bancs_3_clocking(coord_t{2, 1}) == 0);
    CHECK(bancs_3_clocking(coord_t{2, 2}) == 1);
    CHECK(bancs_3_clocking(coord_t{2, 3}) == 2);
    CHECK(bancs_3_clocking(coord_t{2, 4}) == 0);
    CHECK(bancs_3_clocking(coord_t{2, 5}) == 1);

    CHECK(bancs_3_clocking(coord_t{0 + 3, 0}) == 0);
    CHECK(bancs_3_clocking(coord_t{0 + 3, 1}) == 2);
    CHECK(bancs_3_clocking(coord_t{0 + 3, 2}) == 2);
    CHECK(bancs_3_clocking(coord_t{0 + 3, 3}) == 1);
    CHECK(bancs_3_clocking(coord_t{0 + 3, 4}) == 1);
    CHECK(bancs_3_clocking(coord_t{0 + 3, 5}) == 0);
    CHECK(bancs_3_clocking(coord_t{1 + 3, 0}) == 1);
    CHECK(bancs_3_clocking(coord_t{1 + 3, 1}) == 1);
    CHECK(bancs_3_clocking(coord_t{1 + 3, 2}) == 0);
    CHECK(bancs_3_clocking(coord_t{1 + 3, 3}) == 0);
    CHECK(bancs_3_clocking(coord_t{1 + 3, 4}) == 2);
    CHECK(bancs_3_clocking(coord_t{1 + 3, 5}) == 2);
    CHECK(bancs_3_clocking(coord_t{2 + 3, 0}) == 2);
    CHECK(bancs_3_clocking(coord_t{2 + 3, 1}) == 0);
    CHECK(bancs_3_clocking(coord_t{2 + 3, 2}) == 1);
    CHECK(bancs_3_clocking(coord_t{2 + 3, 3}) == 2);
    CHECK(bancs_3_clocking(coord_t{2 + 3, 4}) == 0);
    CHECK(bancs_3_clocking(coord_t{2 + 3, 5}) == 1);

    CHECK(bancs_3_clocking(coord_t{0, 0 + 6}) == 0);
    CHECK(bancs_3_clocking(coord_t{0, 1 + 6}) == 2);
    CHECK(bancs_3_clocking(coord_t{0, 2 + 6}) == 2);
    CHECK(bancs_3_clocking(coord_t{0, 3 + 6}) == 1);
    CHECK(bancs_3_clocking(coord_t{0, 4 + 6}) == 1);
    CHECK(bancs_3_clocking(coord_t{0, 5 + 6}) == 0);
    CHECK(bancs_3_clocking(coord_t{1, 0 + 6}) == 1);
    CHECK(bancs_3_clocking(coord_t{1, 1 + 6}) == 1);
    CHECK(bancs_3_clocking(coord_t{1, 2 + 6}) == 0);
    CHECK(bancs_3_clocking(coord_t{1, 3 + 6}) == 0);
    CHECK(bancs_3_clocking(coord_t{1, 4 + 6}) == 2);
    CHECK(bancs_3_clocking(coord_t{1, 5 + 6}) == 2);
    CHECK(bancs_3_clocking(coord_t{2, 0 + 6}) == 2);
    CHECK(bancs_3_clocking(coord_t{2, 1 + 6}) == 0);
    CHECK(bancs_3_clocking(coord_t{2, 2 + 6}) == 1);
    CHECK(bancs_3_clocking(coord_t{2, 3 + 6}) == 2);
    CHECK(bancs_3_clocking(coord_t{2, 4 + 6}) == 0);
    CHECK(bancs_3_clocking(coord_t{2, 5 + 6}) == 1);

    CHECK(bancs_3_clocking(coord_t{0 + 3, 0 + 6}) == 0);
    CHECK(bancs_3_clocking(coord_t{0 + 3, 1 + 6}) == 2);
    CHECK(bancs_3_clocking(coord_t{0 + 3, 2 + 6}) == 2);
    CHECK(bancs_3_clocking(coord_t{0 + 3, 3 + 6}) == 1);
    CHECK(bancs_3_clocking(coord_t{0 + 3, 4 + 6}) == 1);
    CHECK(bancs_3_clocking(coord_t{0 + 3, 5 + 6}) == 0);
    CHECK(bancs_3_clocking(coord_t{1 + 3, 0 + 6}) == 1);
    CHECK(bancs_3_clocking(coord_t{1 + 3, 1 + 6}) == 1);
    CHECK(bancs_3_clocking(coord_t{1 + 3, 2 + 6}) == 0);
    CHECK(bancs_3_clocking(coord_t{1 + 3, 3 + 6}) == 0);
    CHECK(bancs_3_clocking(coord_t{1 + 3, 4 + 6}) == 2);
    CHECK(bancs_3_clocking(coord_t{1 + 3, 5 + 6}) == 2);
    CHECK(bancs_3_clocking(coord_t{2 + 3, 0 + 6}) == 2);
    CHECK(bancs_3_clocking(coord_t{2 + 3, 1 + 6}) == 0);
    CHECK(bancs_3_clocking(coord_t{2 + 3, 2 + 6}) == 1);
    CHECK(bancs_3_clocking(coord_t{2 + 3, 3 + 6}) == 2);
    CHECK(bancs_3_clocking(coord_t{2 + 3, 4 + 6}) == 0);
    CHECK(bancs_3_clocking(coord_t{2 + 3, 5 + 6}) == 1);
}

TEST_CASE("3-phase ToPoliNano", "[clocking scheme]")
{
    CHECK(topolinano_3_clocking.is_regular());

    CHECK(topolinano_3_clocking(coord_t{0, 0}) == 0);
    CHECK(topolinano_3_clocking(coord_t{0, 1}) == 0);
    CHECK(topolinano_3_clocking(coord_t{0, 2}) == 0);
    CHECK(topolinano_3_clocking(coord_t{1, 0}) == 1);
    CHECK(topolinano_3_clocking(coord_t{1, 1}) == 1);
    CHECK(topolinano_3_clocking(coord_t{1, 2}) == 1);
    CHECK(topolinano_3_clocking(coord_t{2, 0}) == 2);
    CHECK(topolinano_3_clocking(coord_t{2, 1}) == 2);
    CHECK(topolinano_3_clocking(coord_t{2, 2}) == 2);

    CHECK(topolinano_3_clocking(coord_t{0 + 3, 0}) == 0);
    CHECK(topolinano_3_clocking(coord_t{0 + 3, 1}) == 0);
    CHECK(topolinano_3_clocking(coord_t{0 + 3, 2}) == 0);
    CHECK(topolinano_3_clocking(coord_t{1 + 3, 0}) == 1);
    CHECK(topolinano_3_clocking(coord_t{1 + 3, 1}) == 1);
    CHECK(topolinano_3_clocking(coord_t{1 + 3, 2}) == 1);
    CHECK(topolinano_3_clocking(coord_t{2 + 3, 0}) == 2);
    CHECK(topolinano_3_clocking(coord_t{2 + 3, 1}) == 2);
    CHECK(topolinano_3_clocking(coord_t{2 + 3, 2}) == 2);

    CHECK(topolinano_3_clocking(coord_t{0, 0 + 3}) == 0);
    CHECK(topolinano_3_clocking(coord_t{0, 1 + 3}) == 0);
    CHECK(topolinano_3_clocking(coord_t{0, 2 + 3}) == 0);
    CHECK(topolinano_3_clocking(coord_t{1, 0 + 3}) == 1);
    CHECK(topolinano_3_clocking(coord_t{1, 1 + 3}) == 1);
    CHECK(topolinano_3_clocking(coord_t{1, 2 + 3}) == 1);
    CHECK(topolinano_3_clocking(coord_t{2, 0 + 3}) == 2);
    CHECK(topolinano_3_clocking(coord_t{2, 1 + 3}) == 2);
    CHECK(topolinano_3_clocking(coord_t{2, 2 + 3}) == 2);

    CHECK(topolinano_3_clocking(coord_t{0 + 3, 0 + 3}) == 0);
    CHECK(topolinano_3_clocking(coord_t{0 + 3, 1 + 3}) == 0);
    CHECK(topolinano_3_clocking(coord_t{0 + 3, 2 + 3}) == 0);
    CHECK(topolinano_3_clocking(coord_t{1 + 3, 0 + 3}) == 1);
    CHECK(topolinano_3_clocking(coord_t{1 + 3, 1 + 3}) == 1);
    CHECK(topolinano_3_clocking(coord_t{1 + 3, 2 + 3}) == 1);
    CHECK(topolinano_3_clocking(coord_t{2 + 3, 0 + 3}) == 2);
    CHECK(topolinano_3_clocking(coord_t{2 + 3, 1 + 3}) == 2);
    CHECK(topolinano_3_clocking(coord_t{2 + 3, 2 + 3}) == 2);
}

TEST_CASE("4-phase ToPoliNano", "[clocking scheme]")
{
    CHECK(topolinano_4_clocking.is_regular());

    CHECK(topolinano_4_clocking(coord_t{0, 0}) == 0);
    CHECK(topolinano_4_clocking(coord_t{0, 1}) == 0);
    CHECK(topolinano_4_clocking(coord_t{0, 2}) == 0);
    CHECK(topolinano_4_clocking(coord_t{0, 3}) == 0);
    CHECK(topolinano_4_clocking(coord_t{1, 0}) == 1);
    CHECK(topolinano_4_clocking(coord_t{1, 1}) == 1);
    CHECK(topolinano_4_clocking(coord_t{1, 2}) == 1);
    CHECK(topolinano_4_clocking(coord_t{1, 3}) == 1);
    CHECK(topolinano_4_clocking(coord_t{2, 0}) == 2);
    CHECK(topolinano_4_clocking(coord_t{2, 1}) == 2);
    CHECK(topolinano_4_clocking(coord_t{2, 2}) == 2);
    CHECK(topolinano_4_clocking(coord_t{2, 3}) == 2);
    CHECK(topolinano_4_clocking(coord_t{3, 0}) == 3);
    CHECK(topolinano_4_clocking(coord_t{3, 1}) == 3);
    CHECK(topolinano_4_clocking(coord_t{3, 2}) == 3);
    CHECK(topolinano_4_clocking(coord_t{3, 3}) == 3);

    CHECK(topolinano_4_clocking(coord_t{0 + 4, 0}) == 0);
    CHECK(topolinano_4_clocking(coord_t{0 + 4, 1}) == 0);
    CHECK(topolinano_4_clocking(coord_t{0 + 4, 2}) == 0);
    CHECK(topolinano_4_clocking(coord_t{0 + 4, 3}) == 0);
    CHECK(topolinano_4_clocking(coord_t{1 + 4, 0}) == 1);
    CHECK(topolinano_4_clocking(coord_t{1 + 4, 1}) == 1);
    CHECK(topolinano_4_clocking(coord_t{1 + 4, 2}) == 1);
    CHECK(topolinano_4_clocking(coord_t{1 + 4, 3}) == 1);
    CHECK(topolinano_4_clocking(coord_t{2 + 4, 0}) == 2);
    CHECK(topolinano_4_clocking(coord_t{2 + 4, 1}) == 2);
    CHECK(topolinano_4_clocking(coord_t{2 + 4, 2}) == 2);
    CHECK(topolinano_4_clocking(coord_t{2 + 4, 3}) == 2);
    CHECK(topolinano_4_clocking(coord_t{3 + 4, 0}) == 3);
    CHECK(topolinano_4_clocking(coord_t{3 + 4, 1}) == 3);
    CHECK(topolinano_4_clocking(coord_t{3 + 4, 2}) == 3);
    CHECK(topolinano_4_clocking(coord_t{3 + 4, 3}) == 3);

    CHECK(topolinano_4_clocking(coord_t{0, 0 + 4}) == 0);
    CHECK(topolinano_4_clocking(coord_t{0, 1 + 4}) == 0);
    CHECK(topolinano_4_clocking(coord_t{0, 2 + 4}) == 0);
    CHECK(topolinano_4_clocking(coord_t{0, 3 + 4}) == 0);
    CHECK(topolinano_4_clocking(coord_t{1, 0 + 4}) == 1);
    CHECK(topolinano_4_clocking(coord_t{1, 1 + 4}) == 1);
    CHECK(topolinano_4_clocking(coord_t{1, 2 + 4}) == 1);
    CHECK(topolinano_4_clocking(coord_t{1, 3 + 4}) == 1);
    CHECK(topolinano_4_clocking(coord_t{2, 0 + 4}) == 2);
    CHECK(topolinano_4_clocking(coord_t{2, 1 + 4}) == 2);
    CHECK(topolinano_4_clocking(coord_t{2, 2 + 4}) == 2);
    CHECK(topolinano_4_clocking(coord_t{2, 3 + 4}) == 2);
    CHECK(topolinano_4_clocking(coord_t{3, 0 + 4}) == 3);
    CHECK(topolinano_4_clocking(coord_t{3, 1 + 4}) == 3);
    CHECK(topolinano_4_clocking(coord_t{3, 2 + 4}) == 3);
    CHECK(topolinano_4_clocking(coord_t{3, 3 + 4}) == 3);

    CHECK(topolinano_4_clocking(coord_t{0 + 4, 0 + 4}) == 0);
    CHECK(topolinano_4_clocking(coord_t{0 + 4, 1 + 4}) == 0);
    CHECK(topolinano_4_clocking(coord_t{0 + 4, 2 + 4}) == 0);
    CHECK(topolinano_4_clocking(coord_t{0 + 4, 3 + 4}) == 0);
    CHECK(topolinano_4_clocking(coord_t{1 + 4, 0 + 4}) == 1);
    CHECK(topolinano_4_clocking(coord_t{1 + 4, 1 + 4}) == 1);
    CHECK(topolinano_4_clocking(coord_t{1 + 4, 2 + 4}) == 1);
    CHECK(topolinano_4_clocking(coord_t{1 + 4, 3 + 4}) == 1);
    CHECK(topolinano_4_clocking(coord_t{2 + 4, 0 + 4}) == 2);
    CHECK(topolinano_4_clocking(coord_t{2 + 4, 1 + 4}) == 2);
    CHECK(topolinano_4_clocking(coord_t{2 + 4, 2 + 4}) == 2);
    CHECK(topolinano_4_clocking(coord_t{2 + 4, 3 + 4}) == 2);
    CHECK(topolinano_4_clocking(coord_t{3 + 4, 0 + 4}) == 3);
    CHECK(topolinano_4_clocking(coord_t{3 + 4, 1 + 4}) == 3);
    CHECK(topolinano_4_clocking(coord_t{3 + 4, 2 + 4}) == 3);
    CHECK(topolinano_4_clocking(coord_t{3 + 4, 3 + 4}) == 3);
}

TEST_CASE("3-phase open clocking", "[clocking scheme]")
{
    auto open_clocking = open_3_clocking;

    CHECK(!open_clocking.is_regular());

    CHECK(open_clocking(coord_t{0, 0}) == 0);
    CHECK(open_clocking(coord_t{0, 1}) == 0);
    CHECK(open_clocking(coord_t{0, 2}) == 0);
    CHECK(open_clocking(coord_t{0, 3}) == 0);
    CHECK(open_clocking(coord_t{1, 0}) == 0);
    CHECK(open_clocking(coord_t{1, 1}) == 0);
    CHECK(open_clocking(coord_t{1, 2}) == 0);
    CHECK(open_clocking(coord_t{1, 3}) == 0);
    CHECK(open_clocking(coord_t{2, 0}) == 0);
    CHECK(open_clocking(coord_t{2, 1}) == 0);
    CHECK(open_clocking(coord_t{2, 2}) == 0);
    CHECK(open_clocking(coord_t{2, 3}) == 0);
    CHECK(open_clocking(coord_t{3, 0}) == 0);
    CHECK(open_clocking(coord_t{3, 1}) == 0);
    CHECK(open_clocking(coord_t{3, 2}) == 0);
    CHECK(open_clocking(coord_t{3, 3}) == 0);

    open_clocking.override_clock_number(coord_t{0, 0}, 0);
    open_clocking.override_clock_number(coord_t{0, 1}, 1);
    open_clocking.override_clock_number(coord_t{0, 2}, 2);
    open_clocking.override_clock_number(coord_t{0, 3}, 3);
    open_clocking.override_clock_number(coord_t{0, 4}, 4);
    open_clocking.override_clock_number(coord_t{0, 5}, 5);
    open_clocking.override_clock_number(coord_t{0, 6}, 6);

    CHECK(!open_clocking.is_regular());

    CHECK(open_clocking(coord_t{0, 0}) == 0);
    CHECK(open_clocking(coord_t{0, 1}) == 1);
    CHECK(open_clocking(coord_t{0, 2}) == 2);
    CHECK(open_clocking(coord_t{0, 3}) == 0);
    CHECK(open_clocking(coord_t{0, 4}) == 1);
    CHECK(open_clocking(coord_t{0, 5}) == 2);
    CHECK(open_clocking(coord_t{0, 6}) == 0);
    CHECK(open_clocking(coord_t{1, 0}) == 0);
    CHECK(open_clocking(coord_t{1, 1}) == 0);
    CHECK(open_clocking(coord_t{1, 2}) == 0);
    CHECK(open_clocking(coord_t{1, 3}) == 0);
    CHECK(open_clocking(coord_t{2, 0}) == 0);
    CHECK(open_clocking(coord_t{2, 1}) == 0);
    CHECK(open_clocking(coord_t{2, 2}) == 0);
    CHECK(open_clocking(coord_t{2, 3}) == 0);
    CHECK(open_clocking(coord_t{3, 0}) == 0);
    CHECK(open_clocking(coord_t{3, 1}) == 0);
    CHECK(open_clocking(coord_t{3, 2}) == 0);
    CHECK(open_clocking(coord_t{3, 3}) == 0);
}

TEST_CASE("4-phase open clocking", "[clocking scheme]")
{
    auto open_clocking = open_4_clocking;

    CHECK(!open_clocking.is_regular());

    CHECK(open_clocking(coord_t{0, 0}) == 0);
    CHECK(open_clocking(coord_t{0, 1}) == 0);
    CHECK(open_clocking(coord_t{0, 2}) == 0);
    CHECK(open_clocking(coord_t{0, 3}) == 0);
    CHECK(open_clocking(coord_t{1, 0}) == 0);
    CHECK(open_clocking(coord_t{1, 1}) == 0);
    CHECK(open_clocking(coord_t{1, 2}) == 0);
    CHECK(open_clocking(coord_t{1, 3}) == 0);
    CHECK(open_clocking(coord_t{2, 0}) == 0);
    CHECK(open_clocking(coord_t{2, 1}) == 0);
    CHECK(open_clocking(coord_t{2, 2}) == 0);
    CHECK(open_clocking(coord_t{2, 3}) == 0);
    CHECK(open_clocking(coord_t{3, 0}) == 0);
    CHECK(open_clocking(coord_t{3, 1}) == 0);
    CHECK(open_clocking(coord_t{3, 2}) == 0);
    CHECK(open_clocking(coord_t{3, 3}) == 0);

    open_clocking.override_clock_number(coord_t{0, 0}, 0);
    open_clocking.override_clock_number(coord_t{0, 1}, 1);
    open_clocking.override_clock_number(coord_t{0, 2}, 2);
    open_clocking.override_clock_number(coord_t{0, 3}, 3);
    open_clocking.override_clock_number(coord_t{0, 4}, 4);
    open_clocking.override_clock_number(coord_t{0, 5}, 5);
    open_clocking.override_clock_number(coord_t{0, 6}, 6);

    CHECK(!open_clocking.is_regular());

    CHECK(open_clocking(coord_t{0, 0}) == 0);
    CHECK(open_clocking(coord_t{0, 1}) == 1);
    CHECK(open_clocking(coord_t{0, 2}) == 2);
    CHECK(open_clocking(coord_t{0, 3}) == 3);
    CHECK(open_clocking(coord_t{0, 4}) == 0);
    CHECK(open_clocking(coord_t{0, 5}) == 1);
    CHECK(open_clocking(coord_t{0, 6}) == 2);
    CHECK(open_clocking(coord_t{1, 0}) == 0);
    CHECK(open_clocking(coord_t{1, 1}) == 0);
    CHECK(open_clocking(coord_t{1, 2}) == 0);
    CHECK(open_clocking(coord_t{1, 3}) == 0);
    CHECK(open_clocking(coord_t{2, 0}) == 0);
    CHECK(open_clocking(coord_t{2, 1}) == 0);
    CHECK(open_clocking(coord_t{2, 2}) == 0);
    CHECK(open_clocking(coord_t{2, 3}) == 0);
    CHECK(open_clocking(coord_t{3, 0}) == 0);
    CHECK(open_clocking(coord_t{3, 1}) == 0);
    CHECK(open_clocking(coord_t{3, 2}) == 0);
    CHECK(open_clocking(coord_t{3, 3}) == 0);
}

TEST_CASE("Override clocking", "[clocking scheme]")
{
    auto twoddwave_clocking = twoddwave_4_clocking;

    CHECK(twoddwave_clocking.is_regular());

    twoddwave_clocking.override_clock_number(coord_t{0, 0}, 0);

    CHECK(!twoddwave_clocking.is_regular());

    twoddwave_clocking.override_clock_number(coord_t{0, 1}, 1);
    twoddwave_clocking.override_clock_number(coord_t{0, 2}, 2);
    twoddwave_clocking.override_clock_number(coord_t{0, 3}, 3);
    twoddwave_clocking.override_clock_number(coord_t{0, 4}, 4);
    twoddwave_clocking.override_clock_number(coord_t{0, 5}, 5);
    twoddwave_clocking.override_clock_number(coord_t{0, 6}, 6);

    CHECK(twoddwave_clocking(coord_t{0, 0}) == 0);
    CHECK(twoddwave_clocking(coord_t{0, 1}) == 1);
    CHECK(twoddwave_clocking(coord_t{0, 2}) == 2);
    CHECK(twoddwave_clocking(coord_t{0, 3}) == 3);
    CHECK(twoddwave_clocking(coord_t{0, 4}) == 0);
    CHECK(twoddwave_clocking(coord_t{0, 5}) == 1);
    CHECK(twoddwave_clocking(coord_t{0, 6}) == 2);
    CHECK(twoddwave_clocking(coord_t{1, 0}) == 1);
    CHECK(twoddwave_clocking(coord_t{1, 1}) == 2);
    CHECK(twoddwave_clocking(coord_t{1, 2}) == 3);
    CHECK(twoddwave_clocking(coord_t{1, 3}) == 0);
    CHECK(twoddwave_clocking(coord_t{2, 0}) == 2);
    CHECK(twoddwave_clocking(coord_t{2, 1}) == 3);
    CHECK(twoddwave_clocking(coord_t{2, 2}) == 0);
    CHECK(twoddwave_clocking(coord_t{2, 3}) == 1);
    CHECK(twoddwave_clocking(coord_t{3, 0}) == 3);
    CHECK(twoddwave_clocking(coord_t{3, 1}) == 0);
    CHECK(twoddwave_clocking(coord_t{3, 2}) == 1);
    CHECK(twoddwave_clocking(coord_t{3, 3}) == 2);
}

TEST_CASE("Clocking lookup", "[clocking scheme]")
{
    auto check = [](const std::vector<std::string>& vec, const auto& name)
    {
        for (const auto& n : vec)
        {
            auto cs = get_clocking_scheme<coord_t>(n);
            REQUIRE(cs.has_value());
            CHECK(cs == name);
        }
    };

    check({"open3", "OPEN3", "oPeN3", "OpEn3"}, clock_name::open3);
    check({"open4", "OPEN4", "oPeN4", "OpEn4", "open", "OPEN"}, clock_name::open4);
    check({"2ddwave3", "2DDwave3", "2ddWAVE3", "2dDwAvE3", "2DdWaVe3", "DIAG3", "diag3"}, clock_name::twoddwave3);
    check({"2ddwave4", "2DdWaVe4", "DIAG4", "diag4", "2ddwave", "2DDWAVE", "2DDWave"}, clock_name::twoddwave4);
    check({"use", "USE", "uSe", "UsE"}, clock_name::use);
    check({"res", "RES", "rEs", "ReS"}, clock_name::res);
    check({"bancs", "BANCS", "BaNCs", "banCS"}, clock_name::bancs);
    check({"topolinano3", "TOPOLINANO3", "ToPoliNano3", "topolinano"}, clock_name::topolinano3);
    check({"topolinano4", "TOPOLINANO4", "ToPoliNano4"}, clock_name::topolinano4);

    CHECK(!get_clocking_scheme<coord_t>("").has_value());
    CHECK(!get_clocking_scheme<coord_t>("TwoDDWave").has_value());
    CHECK(!get_clocking_scheme<coord_t>("BNCS").has_value());
    CHECK(!get_clocking_scheme<coord_t>("ToPolino").has_value());
}
