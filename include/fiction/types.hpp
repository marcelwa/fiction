//
// Created by marcel on 18.05.21.
//

#ifndef FICTION_TYPES_HPP
#define FICTION_TYPES_HPP

#include "layouts/cell_level_layout.hpp"
#include "layouts/clocked_layout.hpp"
#include "layouts/gate_level_layout.hpp"
#include "layouts/tile_based_layout.hpp"
#include "networks/topology_network.hpp"
#include "technology/cell_technologies.hpp"

#include <kitty/dynamic_truth_table.hpp>
#include <mockturtle/io/write_dot.hpp>
#include <mockturtle/networks/aig.hpp>
#include <mockturtle/networks/mig.hpp>
#include <mockturtle/views/names_view.hpp>

#include <memory>
#include <type_traits>
#include <variant>

namespace fiction
{
/**
 * Truth tables.
 */
using tt     = kitty::dynamic_truth_table;
using tt_ptr = std::shared_ptr<tt>;

// using truth_table_t = std:variant<tt_ptr>;
using truth_table_t = tt_ptr;

/**
 * Logic networks.
 */
using aig_nt  = mockturtle::names_view<mockturtle::aig_network>;
using aig_ptr = std::shared_ptr<aig_nt>;

constexpr const char* aig_name = "AIG";

using mig_nt  = mockturtle::names_view<mockturtle::mig_network>;
using mig_ptr = std::shared_ptr<mig_nt>;

constexpr const char* mig_name = "MIG";

using top_nt  = mockturtle::names_view<fiction::topology_network>;
using top_ptr = std::shared_ptr<top_nt>;

constexpr const char* top_name = "TOP";

using logic_network_t = std::variant<aig_ptr, mig_ptr, top_ptr>;

template <class Ntk>
inline constexpr const char* ntk_type_name = std::is_same_v<std::decay_t<Ntk>, fiction::aig_nt> ? fiction::aig_name :
                                             std::is_same_v<std::decay_t<Ntk>, fiction::mig_nt> ? fiction::mig_name :
                                             std::is_same_v<std::decay_t<Ntk>, fiction::top_nt> ? fiction::top_name :
                                                                                                  "?";

/**
 * FCN gate-level layouts.
 */
using gate_clk_lyt =
    mockturtle::names_view<fiction::gate_level_layout<fiction::clocked_layout<fiction::tile_based_layout>>>;
using gate_clk_lyt_ptr = std::shared_ptr<gate_clk_lyt>;

using gate_layout_t = std::variant<gate_clk_lyt_ptr>;


/**
 * FCN technologies.
 */
constexpr const char* qca_name  = "QCA";
constexpr const char* inml_name = "iNML";
constexpr const char* sidb_name = "SiDB";

template <class Tech>
inline constexpr const char* tech_impl_name =
    std::is_same_v<std::decay_t<Tech>, fiction::qca_technology>  ? fiction::qca_name :
    std::is_same_v<std::decay_t<Tech>, fiction::inml_technology> ? fiction::inml_name :
    std::is_same_v<std::decay_t<Tech>, fiction::sidb_technology> ? fiction::sidb_name :
                                                                   "?";

/**
 * FCN cell-level layouts.
 */
using qca_cell_clk_lyt =
    fiction::cell_level_layout<fiction::qca_technology, fiction::clocked_layout<fiction::tile_based_layout>>;
using qca_cell_clk_lyt_ptr = std::shared_ptr<qca_cell_clk_lyt>;

using cell_layout_t = std::variant<qca_cell_clk_lyt_ptr>;

}  // namespace fiction

#endif  // FICTION_TYPES_HPP
