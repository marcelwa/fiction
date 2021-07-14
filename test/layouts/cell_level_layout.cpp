//
// Created by marcel on 24.06.21.
//

#include "catch.hpp"

#include <fiction/layouts/cell_level_layout.hpp>
#include <fiction/layouts/clocked_layout.hpp>
#include <fiction/layouts/tile_based_layout.hpp>
#include <fiction/technology/cell_technologies.hpp>
#include <fiction/traits.hpp>
#include <fiction/types.hpp>

#include <string>
#include <type_traits>

using namespace fiction;

TEST_CASE("Cell technology", "[cell-level]")
{
    std::stringstream s{};

    SECTION("QCA")
    {
        CHECK(qca_technology::is_empty_cell(qca_technology::cell_type::EMPTY));
        CHECK(qca_technology::is_normal_cell(qca_technology::cell_type::NORMAL));
        CHECK(qca_technology::is_input_cell(qca_technology::cell_type::INPUT));
        CHECK(qca_technology::is_output_cell(qca_technology::cell_type::OUTPUT));
        CHECK(qca_technology::is_const_0_cell(qca_technology::cell_type::CONST_0));
        CHECK(qca_technology::is_const_1_cell(qca_technology::cell_type::CONST_1));
        CHECK(qca_technology::is_constant_cell(qca_technology::cell_type::CONST_0));
        CHECK(qca_technology::is_constant_cell(qca_technology::cell_type::CONST_1));

        CHECK(qca_technology::is_normal_cell_mode(qca_technology::cell_mode::NORMAL));
        CHECK(qca_technology::is_rotated_cell_mode(qca_technology::cell_mode::ROTATED));
        CHECK(qca_technology::is_vertical_cell_mode(qca_technology::cell_mode::VERTICAL));
        CHECK(qca_technology::is_crossover_cell_mode(qca_technology::cell_mode::CROSSOVER));

        CHECK(tech_impl_name<qca_technology> == std::string{"QCA"});
    }
    SECTION("iNML")
    {
        // TODO cell types go here

        CHECK(tech_impl_name<inml_technology> == std::string{"iNML"});
    }
    SECTION("SiDB")
    {
        // TODO cell types go here

        CHECK(tech_impl_name<sidb_technology> == std::string{"SiDB"});
    }
}

TEST_CASE("Cell type assignment", "[cell-level]")
{
    using cell_layout =
        fiction::cell_level_layout<fiction::qca_technology, fiction::clocked_layout<fiction::tile_based_layout>>;

    REQUIRE(fiction::has_get_layout_name_v<cell_layout>);
    REQUIRE(fiction::has_set_layout_name_v<cell_layout>);

    cell_layout layout{fiction::tile_based_layout::aspect_ratio{4, 4}, "AND"};

    layout.assign_cell_type({0, 2}, fiction::qca_technology::cell_type::INPUT);
    layout.assign_cell_type({2, 4}, fiction::qca_technology::cell_type::INPUT);
    layout.assign_cell_type({2, 0}, fiction::qca_technology::cell_type::CONST_0);
    layout.assign_cell_type({2, 1}, fiction::qca_technology::cell_type::NORMAL);
    layout.assign_cell_type({2, 2}, fiction::qca_technology::cell_type::NORMAL);
    layout.assign_cell_type({2, 3}, fiction::qca_technology::cell_type::NORMAL);
    layout.assign_cell_type({1, 2}, fiction::qca_technology::cell_type::NORMAL);
    layout.assign_cell_type({3, 2}, fiction::qca_technology::cell_type::NORMAL);
    layout.assign_cell_type({4, 2}, fiction::qca_technology::cell_type::OUTPUT);

    layout.assign_cell_name({0, 2}, "a");
    layout.assign_cell_name({2, 4}, "b");
    layout.assign_cell_name({4, 2}, "f");

    CHECK(std::is_same_v<typename cell_layout::technology, fiction::qca_technology>);

    CHECK(layout.get_layout_name() == "AND");
    CHECK(layout.get_cell_name({0, 2}) == "a");
    CHECK(layout.get_cell_name({2, 4}) == "b");
    CHECK(layout.get_cell_name({4, 2}) == "f");

    CHECK(layout.num_cells() == 9);
    CHECK(layout.num_pis() == 2);
    CHECK(layout.num_pos() == 1);

    CHECK(layout.get_cell_type({2, 0}) == fiction::qca_technology::cell_type::CONST_0);
    CHECK(layout.get_cell_type({2, 4}) == fiction::qca_technology::cell_type::INPUT);
    CHECK(layout.get_cell_type({0, 2}) == fiction::qca_technology::cell_type::INPUT);
    CHECK(layout.get_cell_type({2, 1}) == fiction::qca_technology::cell_type::NORMAL);
    CHECK(layout.get_cell_type({2, 2}) == fiction::qca_technology::cell_type::NORMAL);
    CHECK(layout.get_cell_type({2, 3}) == fiction::qca_technology::cell_type::NORMAL);
    CHECK(layout.get_cell_type({1, 2}) == fiction::qca_technology::cell_type::NORMAL);
    CHECK(layout.get_cell_type({3, 2}) == fiction::qca_technology::cell_type::NORMAL);
    CHECK(layout.get_cell_type({4, 2}) == fiction::qca_technology::cell_type::OUTPUT);

    CHECK(layout.is_empty_cell({0, 0}));
    CHECK(layout.is_empty_cell({0, 1}));
    CHECK(layout.is_empty_cell({1, 0}));
    CHECK(layout.is_empty_cell({1, 1}));
    CHECK(layout.is_empty_cell({3, 0}));
    CHECK(layout.is_empty_cell({3, 1}));
    CHECK(layout.is_empty_cell({4, 0}));
    CHECK(layout.is_empty_cell({4, 1}));
    CHECK(layout.is_empty_cell({0, 3}));
    CHECK(layout.is_empty_cell({1, 3}));
    CHECK(layout.is_empty_cell({0, 4}));
    CHECK(layout.is_empty_cell({1, 0}));
    CHECK(layout.is_empty_cell({3, 3}));
    CHECK(layout.is_empty_cell({3, 4}));
    CHECK(layout.is_empty_cell({4, 3}));
    CHECK(layout.is_empty_cell({4, 4}));

    CHECK(!layout.is_empty_cell({2, 0}));
    CHECK(!layout.is_empty_cell({2, 4}));
    CHECK(!layout.is_empty_cell({0, 2}));
    CHECK(!layout.is_empty_cell({2, 1}));
    CHECK(!layout.is_empty_cell({2, 2}));
    CHECK(!layout.is_empty_cell({2, 3}));
    CHECK(!layout.is_empty_cell({1, 2}));
    CHECK(!layout.is_empty_cell({3, 2}));
    CHECK(!layout.is_empty_cell({4, 2}));
}

TEST_CASE("Cell mode assignment", "[cell-level]")
{
    using cell_layout =
        fiction::cell_level_layout<fiction::qca_technology, fiction::clocked_layout<fiction::tile_based_layout>>;

    cell_layout layout{fiction::tile_based_layout::aspect_ratio{4, 4, 1}, "Crossover"};

    layout.assign_cell_type({0, 2, 0}, fiction::qca_technology::cell_type::INPUT);
    layout.assign_cell_type({2, 0, 0}, fiction::qca_technology::cell_type::INPUT);
    layout.assign_cell_type({1, 2, 0}, fiction::qca_technology::cell_type::NORMAL);
    layout.assign_cell_type({2, 2, 0}, fiction::qca_technology::cell_type::NORMAL);
    layout.assign_cell_type({3, 2, 0}, fiction::qca_technology::cell_type::NORMAL);
    layout.assign_cell_type({2, 1, 1}, fiction::qca_technology::cell_type::NORMAL);
    layout.assign_cell_type({2, 2, 1}, fiction::qca_technology::cell_type::NORMAL);
    layout.assign_cell_type({2, 3, 1}, fiction::qca_technology::cell_type::NORMAL);
    layout.assign_cell_type({2, 4, 0}, fiction::qca_technology::cell_type::OUTPUT);
    layout.assign_cell_type({4, 2, 0}, fiction::qca_technology::cell_type::OUTPUT);

    layout.assign_cell_mode({2, 1, 1}, fiction::qca_technology::cell_mode::CROSSOVER);
    layout.assign_cell_mode({2, 2, 1}, fiction::qca_technology::cell_mode::CROSSOVER);
    layout.assign_cell_mode({2, 3, 1}, fiction::qca_technology::cell_mode::CROSSOVER);

    layout.assign_cell_name({0, 2}, "a");
    layout.assign_cell_name({2, 0}, "b");
    layout.assign_cell_name({4, 2}, "a'");
    layout.assign_cell_name({2, 4}, "b'");

    CHECK(layout.get_layout_name() == "Crossover");
    CHECK(layout.get_cell_name({0, 2}) == "a");
    CHECK(layout.get_cell_name({2, 0}) == "b");
    CHECK(layout.get_cell_name({4, 2}) == "a'");
    CHECK(layout.get_cell_name({2, 4}) == "b'");

    CHECK(layout.num_cells() == 10);
    CHECK(layout.num_pis() == 2);
    CHECK(layout.num_pos() == 2);

    CHECK(layout.get_cell_mode({0, 2, 0}) == fiction::qca_technology::cell_mode::NORMAL);
    CHECK(layout.get_cell_mode({2, 0, 0}) == fiction::qca_technology::cell_mode::NORMAL);
    CHECK(layout.get_cell_mode({1, 2, 0}) == fiction::qca_technology::cell_mode::NORMAL);
    CHECK(layout.get_cell_mode({2, 2, 0}) == fiction::qca_technology::cell_mode::NORMAL);
    CHECK(layout.get_cell_mode({3, 2, 0}) == fiction::qca_technology::cell_mode::NORMAL);
    CHECK(layout.get_cell_mode({2, 4, 0}) == fiction::qca_technology::cell_mode::NORMAL);
    CHECK(layout.get_cell_mode({4, 2, 0}) == fiction::qca_technology::cell_mode::NORMAL);

    CHECK(layout.get_cell_mode({2, 1, 1}) == fiction::qca_technology::cell_mode::CROSSOVER);
    CHECK(layout.get_cell_mode({2, 2, 1}) == fiction::qca_technology::cell_mode::CROSSOVER);
    CHECK(layout.get_cell_mode({2, 3, 1}) == fiction::qca_technology::cell_mode::CROSSOVER);
}

TEST_CASE("Clocking", "[cell-level]")
{
    cell_level_layout<qca_technology, clocked_layout<tile_based_layout>> layout{
        tile_based_layout::aspect_ratio{4, 4, 0}, twoddwave_4_clocking, "Lyt", 2, 2};

    CHECK(layout.get_clock_number({0, 0}) == 0);
    CHECK(layout.get_clock_number({0, 1}) == 0);
    CHECK(layout.get_clock_number({1, 0}) == 0);
    CHECK(layout.get_clock_number({1, 1}) == 0);
    CHECK(layout.get_clock_number({2, 0}) == 1);
    CHECK(layout.get_clock_number({2, 1}) == 1);
    CHECK(layout.get_clock_number({3, 0}) == 1);
    CHECK(layout.get_clock_number({3, 1}) == 1);
    CHECK(layout.get_clock_number({0, 2}) == 1);
    CHECK(layout.get_clock_number({0, 3}) == 1);
    CHECK(layout.get_clock_number({1, 2}) == 1);
    CHECK(layout.get_clock_number({1, 3}) == 1);
    CHECK(layout.get_clock_number({2, 2}) == 2);
    CHECK(layout.get_clock_number({2, 3}) == 2);
    CHECK(layout.get_clock_number({3, 2}) == 2);
    CHECK(layout.get_clock_number({3, 3}) == 2);
    CHECK(layout.get_clock_number({4, 0}) == 2);
    CHECK(layout.get_clock_number({4, 1}) == 2);
    CHECK(layout.get_clock_number({4, 2}) == 3);
    CHECK(layout.get_clock_number({4, 3}) == 3);
    CHECK(layout.get_clock_number({0, 4}) == 2);
    CHECK(layout.get_clock_number({1, 4}) == 2);
    CHECK(layout.get_clock_number({2, 4}) == 3);
    CHECK(layout.get_clock_number({3, 4}) == 3);
    CHECK(layout.get_clock_number({4, 4}) == 0);
}
