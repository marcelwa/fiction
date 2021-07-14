//
// Created by marcel on 16.07.18.
//

#ifndef FICTION_STORES_HPP
#define FICTION_STORES_HPP

#include <fiction/io/dot_drawers.hpp>
#include <fiction/io/print_layout.hpp>
#include <fiction/io/write_svg_layout.hpp>
#include <fiction/technology/cell_technologies.hpp>
#include <fiction/types.hpp>

#include <alice/alice.hpp>
#include <fmt/format.h>
#include <kitty/print.hpp>
#include <mockturtle/views/depth_view.hpp>

#include <type_traits>
#include <variant>

namespace alice
{
/**
 * Truth tables.
 *
 * Parts of this code are originally from: https://github.com/msoeken/cirkit/blob/cirkit3/cli/stores/tt.hpp.
 */
ALICE_ADD_STORE(fiction::truth_table_t, "truth_table", "t", "truth table", "truth tables")

ALICE_DESCRIBE_STORE(fiction::truth_table_t, tt)
{
    if (tt->num_vars() <= 6)
        return fmt::format("{} vars, hex: {}, bin: {}", tt->num_vars(), kitty::to_hex(*tt), kitty::to_binary(*tt));
    else
        return fmt::format("{} vars, (description omitted due to truth table size)", tt->num_vars());
}

ALICE_PRINT_STORE_STATISTICS(fiction::truth_table_t, os, tt)
{
    if (tt->num_vars() <= 6)
        os << fmt::format("[i] {} vars, hex: {}, bin: {}\n", tt->num_vars(), kitty::to_hex(*tt), kitty::to_binary(*tt));
    else
        os << fmt::format("[i] {} vars, (description omitted due to truth table size)\n", tt->num_vars());
}

ALICE_LOG_STORE_STATISTICS(fiction::truth_table_t, tt)
{
    return {{"vars", tt->num_vars()}, {"hex", kitty::to_hex(*tt)}, {"binary", kitty::to_binary(*tt)}};
}

ALICE_PRINT_STORE(fiction::truth_table_t, os, tt)
{
    os << fmt::format("[i] {} vars, hex: {}, bin: {}", tt->num_vars(), kitty::to_hex(*tt), kitty::to_binary(*tt));
}

/**
 * Logic networks.
 */
ALICE_ADD_STORE(fiction::logic_network_t, "network", "n", "logic network", "logic networks")

ALICE_DESCRIBE_STORE(fiction::logic_network_t, ln)
{
    const auto describe = [](auto&& net)
    {
        using Ntk = typename std::decay_t<decltype(net)>::element_type;

        mockturtle::depth_view depth_net{*net};

        return fmt::format("{} ({}) - I/O: {}/{}, gates: {}, level: {}", net->get_network_name(),
                           fiction::ntk_type_name<Ntk>, net->num_pis(), net->num_pos(), net->num_gates(),
                           depth_net.depth());
    };

    return std::visit(describe, ln);
}

ALICE_PRINT_STORE_STATISTICS(fiction::logic_network_t, os, ln)
{
    const auto print_statistics = [&os](auto&& net)
    {
        using Ntk = typename std::decay_t<decltype(net)>::element_type;

        mockturtle::depth_view depth_net{*net};

        os << fmt::format("[i] {} ({}) - I/O: {}/{}, gates: {}, level: {}\n", net->get_network_name(),
                          fiction::ntk_type_name<Ntk>, net->num_pis(), net->num_pos(), net->num_gates(),
                          depth_net.depth());
    };

    std::visit(print_statistics, ln);
}

ALICE_LOG_STORE_STATISTICS(fiction::logic_network_t, ln)
{
    const auto log_statistics = [](auto&& net)
    {
        using Ntk = typename std::decay_t<decltype(net)>::element_type;

        mockturtle::depth_view depth_net{*net};

        return nlohmann::json{{"name", net->get_network_name()}, {"type", fiction::ntk_type_name<Ntk>},
                              {"inputs", net->num_pis()},        {"outputs", net->num_pos()},
                              {"gates", net->num_gates()},       {"level", depth_net.depth()}};
    };

    return std::visit(log_statistics, ln);
}

template <>
bool can_show<fiction::logic_network_t>(std::string& extension, [[maybe_unused]] command& cmd)
{
    extension = "dot";

    // indexes for both logic networks and gate layouts
    cmd.add_flag("--indexes,-i", "Show node indexes")->group("logic_network (-n) / gate_layout (-g)");

    return true;
}

template <>
void show<fiction::logic_network_t>(std::ostream& os, const fiction::logic_network_t& element, const command& cmd)
{
    const auto show_net = [&os, &cmd](auto&& net)
    {
        try
        {
            mockturtle::depth_view depth_net{*net};

            using Ntk = typename std::decay_t<decltype(depth_net)>;

            if (cmd.is_set("indexes"))
            {
                mockturtle::write_dot(depth_net, os, fiction::topology_dot_drawer<Ntk, true>());
            }
            else
            {
                mockturtle::write_dot(depth_net, os, fiction::topology_dot_drawer<Ntk, false>());
            }
        }
        catch (const std::invalid_argument& e)
        {
            cmd.env->out() << "[e] " << e.what() << std::endl;
        }
    };

    std::visit(show_net, element);
}

/**
 * FCN gate-level layouts.
 */
ALICE_ADD_STORE(fiction::gate_layout_t, "gate_layout", "g", "gate layout", "gate layouts")

ALICE_PRINT_STORE(fiction::gate_layout_t, os, layout)
{
    const auto print = [&os](auto&& lyt) { fiction::print_gate_level_layout(os, *lyt); };

    std::visit(print, layout);
}

ALICE_DESCRIBE_STORE(fiction::gate_layout_t, layout)
{
    const auto describe = [](auto&& lyt)
    {
        return fmt::format("{} - {} × {}, I/O: {}/{}, gates: {}, wires: {}", lyt->get_network_name(), lyt->x() + 1,
                           lyt->y() + 1, lyt->num_pis(), lyt->num_pos(), lyt->num_gates(), lyt->num_wires());
    };

    return std::visit(describe, layout);
}

ALICE_PRINT_STORE_STATISTICS(fiction::gate_layout_t, os, layout)
{
    // TODO crossings, latches, critical path, throughput
    const auto print_statistics = [&os](auto&& lyt)
    {
        mockturtle::depth_view depth_lyt{*lyt};

        os << fmt::format("[i] {} - {} × {}, I/O: {}/{}, gates: {}, wires: {}, CP: {}\n", lyt->get_network_name(),
                          lyt->x() + 1, lyt->y() + 1, lyt->num_pis(), lyt->num_pos(), lyt->num_gates(),
                          lyt->num_wires(), depth_lyt.depth());
    };

    std::visit(print_statistics, layout);
}

ALICE_LOG_STORE_STATISTICS(fiction::gate_layout_t, layout)
{
    const auto log_statistics = [](auto&& lyt)
    {
        mockturtle::depth_view depth_lyt{*lyt};

        return nlohmann::json{
            {"name", lyt->get_network_name()},
            {"inputs", lyt->num_pis()},
            {"outputs", lyt->num_pos()},
            {"gates", lyt->num_gates()},
            {"wires", lyt->num_wires()},
            {"layout", {{"x-size", lyt->x() + 1}, {"y-size", lyt->y() + 1}, {"area", lyt->area()}}},
            // {"bounding box", {{"x-size", bb.x_size}, {"y-size", bb.y_size}, {"area", bb.area()}}},
            // {"free tiles", area - (gate_tiles + wire_tiles - crossings)},  // free tiles in ground layer
            // {"crossings", crossings},
            // {"latches", layout->latch_count()},
            {"critical path", depth_lyt.depth()}
            // {"throughput", fmt::format("1/{}", tp)}};
        };
    };

    return std::visit(log_statistics, layout);
}

template <>
bool can_show<fiction::gate_layout_t>(std::string& extension, [[maybe_unused]] command& cmd)
{
    extension = "dot";

    // already added for logic network; alice doesn't allow for both
    // cmd.add_flag("--indexes,-i", "Show node indexes")->group("gate_layout (-g)");

    return true;
}

template <>
void show<fiction::gate_layout_t>(std::ostream& os, const fiction::gate_layout_t& element, const command& cmd)
{
    const auto show_lyt = [&os, &cmd](auto&& lyt)
    {
        try
        {
            using Lyt = typename std::decay_t<decltype(lyt)>::element_type;

            if (cmd.is_set("indexes"))
            {
                fiction::write_dot_layout(*lyt, os, fiction::gate_layout_tile_drawer<Lyt, true>());
            }
            else
            {
                fiction::write_dot_layout(*lyt, os, fiction::gate_layout_tile_drawer<Lyt, false>());
            }
        }
        catch (const std::invalid_argument& e)
        {
            cmd.env->out() << "[e] " << e.what() << std::endl;
        }
    };

    std::visit(show_lyt, element);
}

/**
 * FCN cell-level layouts.
 */
ALICE_ADD_STORE(fiction::cell_layout_t, "cell_layout", "c", "cell layout", "cell layouts")

ALICE_PRINT_STORE(fiction::cell_layout_t, os, layout)
{
    const auto print = [&os](auto&& lyt) { fiction::print_cell_level_layout(os, *lyt); };

    std::visit(print, layout);
}

ALICE_DESCRIBE_STORE(fiction::cell_layout_t, layout)
{
    const auto describe = [](auto&& lyt)
    {
        using Lyt = typename std::decay_t<decltype(lyt)>::element_type;

        return fmt::format("{} ({}) - {} × {}, I/O: {}/{}, cells: {}", lyt->get_layout_name(),
                           fiction::tech_impl_name<typename Lyt::technology>, lyt->x() + 1, lyt->y() + 1,
                           lyt->num_pis(), lyt->num_pos(), lyt->num_cells());
    };

    return std::visit(describe, layout);
}

ALICE_PRINT_STORE_STATISTICS(fiction::cell_layout_t, os, layout)
{
    const auto print_statistics = [&os](auto&& lyt)
    {
        using Lyt = typename std::decay_t<decltype(lyt)>::element_type;

        os << fmt::format("[i] {} ({}) - {} × {}, I/O: {}/{}, cells: {}\n", lyt->get_layout_name(),
                          fiction::tech_impl_name<typename Lyt::technology>, lyt->x() + 1, lyt->y() + 1, lyt->num_pis(),
                          lyt->num_pos(), lyt->num_cells());
    };

    std::visit(print_statistics, layout);
}

ALICE_LOG_STORE_STATISTICS(fiction::cell_layout_t, layout)
{
    const auto log_statistics = [](auto&& lyt)
    {
        using Lyt = typename std::decay_t<decltype(lyt)>::element_type;

        return nlohmann::json{{"name", lyt->get_layout_name()},
                              {"technology", fiction::tech_impl_name<typename Lyt::technology>},
                              {"inputs", lyt->num_pis()},
                              {"outputs", lyt->num_pos()},
                              {"cells", lyt->num_cells()},
                              {"layout", {{"x-size", lyt->x() + 1}, {"y-size", lyt->y() + 1}, {"area", lyt->area()}}}};
    };

    return std::visit(log_statistics, layout);
}

template <>
bool can_show<fiction::cell_layout_t>(std::string& extension, [[maybe_unused]] command& cmd)
{
    extension = "svg";

    return true;
}

template <>
void show<fiction::cell_layout_t>(std::ostream& os, const fiction::cell_layout_t& element,
                                  const command& cmd)  // const & for pointer because alice says so...
{
    const auto show_lyt = [&os, &cmd](auto&& lyt)
    {
        using Lyt = typename std::decay_t<decltype(lyt)>::element_type;

        if constexpr (!std::is_same_v<typename Lyt::technology, fiction::qca_technology>)
        {
            cmd.env->out() << fmt::format("[e] {} is not a QCA layout", lyt->get_layout_name()) << std::endl;
        }
        else
        {
            try
            {
                fiction::write_qca_layout_svg(*lyt, os, {cmd.is_set("simple")});
            }
            catch (const fiction::unsupported_cell_type_exception& e)
            {
                cmd.env->out() << fmt::format("[e] unsupported cell type at cell position {}", e.where()) << std::endl;
            }
            catch (...)
            {
                cmd.env->out() << "[e] an error occurred while writing the SVG description to the output stream"
                               << std::endl;
            }
        }
    };

    std::visit(show_lyt, element);
}

}  // namespace alice

#endif  // FICTION_STORES_HPP
