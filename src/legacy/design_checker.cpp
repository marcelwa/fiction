//
// Created by marcel on 26.04.19.
//

#include "design_checker.h"

design_checker::design_checker(fcn_gate_layout_ptr fgl, std::size_t wl)
        :
        layout{std::move(fgl)},
        wire_limit{wl}
{}

nlohmann::json design_checker::check(std::ostream& out) noexcept
{
    drvs = 0, warnings = 0;

    nlohmann::json report{};

    out << "Topology:" << std::endl;
    out << wire_count_check(report) << std::endl;
    out << missing_connections_check(report) << std::endl;
    out << crossing_gates_check(report) << std::endl;
    out << std::endl;

    out << "Clocking:" << std::endl;
    out << tile_clocking_check(report) << std::endl;
    out << direction_check(report) << std::endl;
    out << std::endl;

    out << "I/O ports:" << std::endl;
    out << operation_io_check(report) << std::endl;
    out << io_port_check(report) << std::endl;
    out << border_io_check(report) << std::endl;
    out << std::endl;

    out << fmt::format("DRVs: {}, Warnings: {}", (drvs ? fmt::format(SOME_DRVS, drvs) : ZERO_ISSUES),
                       (warnings ? fmt::format(SOME_WARNINGS, warnings) : ZERO_ISSUES)) << std::endl;

    report["DRVs"] = drvs;
    report["Warnings"] = warnings;
    
    return report;
}

std::size_t design_checker::get_drvs() const noexcept
{
    return drvs;
}

std::size_t design_checker::get_warnings() const noexcept
{
    return warnings;
}

void design_checker::log_tile(const fcn_gate_layout::tile& t, nlohmann::json& report) const noexcept
{
    std::stringstream s{};

    if (auto v = layout->get_logic_vertex(t))
        s << "v: " << *v;  // log vertex
    else if (layout->is_wire_tile(t))
    {
        s << "e: { ";
        for (const auto& e : layout->get_logic_edges(t))
            s << e << " ";  // log edges
        s << "}";
    }
    else  // empty tile
        s << "empty";

    auto clk = layout->tile_clocking(t);
    auto l   = layout->get_latch(t);
    s << fmt::format(", clk: {}{}, inp: {}, out: {}{}{}", (clk ? std::to_string(*clk) : ""),
                     (l ? ", l: " + std::to_string(l) : ""),
                     layout->get_tile_inp_dirs(t), layout->get_tile_out_dirs(t), (layout->is_pi(t) ? ", PI" : ""),
                     (layout->is_po(t) ? ", PO" : ""));

    report[to_string(t)] = s.str();
}

std::string design_checker::summary(std::string&& msg, const bool chk, const bool brk) const noexcept
{
    return fmt::format(" {} {}" + std::move(msg), chk ? CHECK_PASSED : (brk ? CHECK_FAILED : WARNING), chk ? "" : "not ");
}

std::string design_checker::wire_count_check(nlohmann::json& report) noexcept
{
    nlohmann::json wire_report{};

    auto all_matched = true;
    for (auto& [t, e] : layout->e_map)
    {
        if (e.size() > wire_limit)
        {
            all_matched = false;
            log_tile(t, wire_report);
            ++drvs;
        }
    }

    report["Overfull tiles"] = wire_report;

    return summary(fmt::format("all tiles have at most {} wire{} assigned", wire_limit, wire_limit != 1 ? "s" : ""), all_matched, true);
}

std::string design_checker::missing_connections_check(nlohmann::json& report) noexcept
{
    nlohmann::json connections_report{};

    auto all_connected = true;
    for (auto&& t : layout->tiles() | iter::filterfalse([this](const auto& _t){return layout->is_free_tile(_t);}))
    {
        auto odf = layout->outgoing_data_flow(t);
        bool dangling_out_connection = (std::distance(odf.begin(), odf.end()) == 0) && !layout->is_po(t);

        auto idf = layout->incoming_data_flow(t);
        bool dangling_inp_connection = (std::distance(idf.begin(), idf.end()) == 0) && !layout->is_pi(t);

        if (dangling_out_connection || dangling_inp_connection)
        {
            all_connected = false;
            log_tile(t, connections_report);
            ++drvs;
        }
    }

    report["Missing connections"] = connections_report;

    return summary("all occupied tiles are properly connected", all_connected, true);
}

std::string design_checker::crossing_gates_check(nlohmann::json& report) noexcept
{
    nlohmann::json crossing_report{};

    auto all_wire_crossings = true;
    for (auto&& cross : layout->crossing_layers() | iter::filterfalse([this](const auto& _t){return layout->is_free_tile(_t);}))
    {
        if (!layout->is_wire_tile(layout->below(cross)))
        {
            all_wire_crossings = false;
            log_tile(cross, crossing_report);
            ++drvs;
        }
    }

    report["Wires crossing gates"] = crossing_report;

    return summary("all wire crossings cross over other wires only", all_wire_crossings, true);
}

std::string design_checker::tile_clocking_check(nlohmann::json& report) noexcept
{
    nlohmann::json clock_report{};

    auto all_clocked = true;
    if (!layout->is_regularly_clocked())
    {
        for (auto&& t : layout->tiles() | iter::filterfalse([this](const auto& _t){return layout->is_free_tile(_t);}))
        {
            if (!layout->tile_clocking(t))
            {
                all_clocked = false;
                log_tile(t, clock_report);
                ++warnings;
            }
        }
    }

    report["Unclocked non-empty tiles"] = clock_report;

    return summary("all occupied tiles are clocked", all_clocked, false);
}

std::string design_checker::direction_check(nlohmann::json& report) noexcept
{
    nlohmann::json direction_report{};

    auto correct_directions = true;
    for (auto&& t : layout->tiles() | iter::filterfalse([this](const auto& _t){return layout->is_free_tile(_t);}))
    {
        layout::directions out_dirs{}, inp_dirs{};

        for (auto&& odf : layout->outgoing_data_flow(t))
            out_dirs |= layout->get_bearing(t, odf);

        for (auto&& idf : layout->incoming_data_flow(t))
            inp_dirs |= layout->get_bearing(t, idf);

        if ((out_dirs != layout->get_tile_out_dirs(t)) || (inp_dirs != layout->get_tile_inp_dirs(t)))
        {
            correct_directions = false;
            log_tile(t, direction_report);
            ++drvs;
        }
    }

    report["Directions against data flow"] = direction_report;

    return summary("all tiles' directions respect data flow", correct_directions, true);
}

std::string design_checker::operation_io_check(nlohmann::json& report) noexcept
{
    nlohmann::json wire_io_report{};

    auto all_operation = true;
    for (auto&& io : iter::chain(layout->get_pis(), layout->get_pos()))
    {
        if (!layout->is_gate_tile(io))
        {
            all_operation = false;
            log_tile(io, wire_io_report);
            ++drvs;
        }
    }

    report["Wire I/O ports"] = wire_io_report;

    return summary("all I/O ports are assigned to some operation", all_operation, true);
}

std::string design_checker::io_port_check(nlohmann::json& report) noexcept
{
    nlohmann::json port_report{};

    auto all_pin = true;
    for (auto&& io : iter::chain(layout->get_pis(), layout->get_pos()))
    {
        if (layout->get_op(io) != operation::PI && layout->get_op(io) != operation::PO)
        {
            all_pin = false;
            log_tile(io, port_report);
            ++warnings;
        }
    }

    report["Gate I/O ports"] = port_report;

    return summary("all I/O ports are realized by designated pins", all_pin, false);
}

std::string design_checker::border_io_check(nlohmann::json& report) noexcept
{
    nlohmann::json border_report{};

    auto all_border = true;
    for (auto&& io : iter::chain(layout->get_pis(), layout->get_pos()))
    {
        if (!layout->is_border(io))
        {
            all_border = false;
            log_tile(io, border_report);
            ++warnings;
        }
    }

    report["Border I/O ports"] = border_report;

    return summary("all I/O ports are located at layout's borders", all_border, false);
}
