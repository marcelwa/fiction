//
// Created by marcel on 18.05.21.
//

#ifndef FICTION_TYPES_HPP
#define FICTION_TYPES_HPP

#include "clocked_layout.hpp"
#include "gate_level_layout.hpp"
#include "tile_based_layout.hpp"

#include <mockturtle/io/write_dot.hpp>
#include <mockturtle/networks/aig.hpp>
#include <mockturtle/views/depth_view.hpp>
#include <mockturtle/views/names_view.hpp>
#include <kitty/dynamic_truth_table.hpp>

#include <variant>

namespace fiction
{
/**
 * Truth tables.
 */
using tt  = kitty::dynamic_truth_table;
using tt_ptr = std::shared_ptr<tt>;

//using truth_table_t = std:variant<tt_ptr>;
using truth_table_t = tt_ptr;

/**
 * Logic networks.
 */
using aig_nt  = mockturtle::names_view<mockturtle::depth_view<mockturtle::aig_network>>;
using aig_ptr = std::shared_ptr<aig_nt>;

using logic_network_t = std::variant<aig_ptr>;

/**
 * FCN gate-level layouts.
 */
using tile_clk_lyt = mockturtle::names_view<fiction::gate_level_layout<fiction::clocked_layout<fiction::tile_based_layout>>>;
using tile_clk_lyt_ptr = std::shared_ptr<tile_clk_lyt>;

using gate_layout_t = std::variant<tile_clk_lyt_ptr>;

}  // namespace fiction

#endif  // FICTION_TYPES_HPP