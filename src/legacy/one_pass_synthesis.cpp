//
// Created by marcel on 09.04.20.
//

#include "one_pass_synthesis.h"


const pybind11::scoped_interpreter one_pass_synthesis::instance{};

one_pass_synthesis::one_pass_synthesis(std::vector<kitty::dynamic_truth_table>&& tts, onepass_pd_config&& config)
        :
        physical_design(),
        spec(std::move(tts)),
        config{std::move(config)},
        dit{std::max(this->config.fixed_size, 1ul)}
{
    assert(!spec.empty());
    assert(std::adjacent_find(spec.begin(), spec.end(),
            [](const auto& a, const auto& b){return a.num_vars() != b.num_vars();}) == spec.end());

    // add Mugen's path to Python's sys.path module scope
    pybind11::module::import("sys").attr("path").attr("append")(MUGEN_PATH);
}

physical_design::pd_result one_pass_synthesis::operator()()
{
    if (!test_dependencies())
        return pd_result{false, nlohmann::json{{"runtime (s)", 0.0}}};

    mockturtle::stopwatch<>::duration time{0};

    auto layout_sketch = std::make_shared<fcn_gate_layout>(*config.scheme,
                                                           std::make_shared<logic_network>(std::move(config.name)));

    mugen_handler handler{spec, layout_sketch, config};

    for (; dit <= config.upper_bound; ++dit)  // <= to prevent overflow
    {

#if (PROGRESS_BARS)
        mockturtle::progress_bar bar("[i] examining layout dimensions: {:>2} × {:<2}");
#endif

        auto dimension = *dit;

        if (handler.skippable(dimension))
            continue;

#if (PROGRESS_BARS)
        bar(dimension[X], dimension[Y]);
#endif

        handler.update(dimension);

        try
        {
            auto sat = mockturtle::call_with_stopwatch(time, [&handler]
            {
                return handler.is_satisfiable();
            });

            if (sat)
            {
                layout = layout_sketch;
                return pd_result{true, nlohmann::json{{"runtime (s)", mockturtle::to_seconds(time)}}};
            }

            if (config.timeout)
                update_timeout(handler, time);
        }
        // timeout reached
        catch (...)
        {
            return pd_result{false, nlohmann::json{{"runtime (s)", mockturtle::to_seconds(time)}}};
        }
    }

    return pd_result{false, nlohmann::json{{"runtime (s)", mockturtle::to_seconds(time)}}};
}

bool one_pass_synthesis::test_dependencies() const
{
    namespace py = pybind11;
    using namespace py::literals;

    // test for graphviz
    try
    {
        py::exec("from graphviz import *");
    }
    catch (...)
    {
        std::cout << "[e] Python module 'graphviz' could not be detected" << std::endl;
        return false;
    }

    // test for PySAT
    try
    {
        py::exec("from pysat.solvers import Glucose3");
    }
    catch (...)
    {
        std::cout << "[e] Python module 'PySAT' could not be detected" << std::endl;
        return false;
    }

    // test for wrapt_timeout_decorator
    try
    {
        py::exec("from wrapt_timeout_decorator import *");
    }
    catch (...)
    {
        std::cout << "[e] Python module 'wrapt_timeout_decorator' could not be detected" << std::endl;
        return false;
    }

    return true;
}

one_pass_synthesis::mugen_handler::mugen_handler(const std::vector<kitty::dynamic_truth_table>& s, fcn_gate_layout_ptr fgl, onepass_pd_config c) noexcept
        :
        spec{s},
        num_pis{spec[0].num_vars()},
        layout{std::move(fgl)},
        config{std::move(c)},
        mugen{pybind11::module::import("mugen")}
{}

bool one_pass_synthesis::mugen_handler::skippable(const fcn_dimension_xy& dim) const noexcept
{
    // OPEN clocking optimization: rotated dimensions don't need to be explored
    if (!layout->is_regularly_clocked())
    {
        if (dim[X] != dim[Y] && dim[X] == layout->y() && dim[Y] == layout->x())
            return true;
    }

    return false;
}

void one_pass_synthesis::mugen_handler::update(const fcn_dimension_xy& dim) noexcept
{
    layout->resize(dim);
}

void one_pass_synthesis::mugen_handler::update(const uint32_t timeout) noexcept
{
    config.timeout = timeout;
}

bool one_pass_synthesis::mugen_handler::is_satisfiable()
{
    namespace py = pybind11;
    using namespace py::literals;

    auto scheme_graph = generate_scheme_graph();

    // Mugen modifies its parameters, so you better have a copy
    auto py_spec = as_py_lists(spec);

    auto nets = scheme_graph.attr("synthesize")(scheme_graph, py_spec);
    for (auto net_it = nets.begin(); net_it != nets.end(); ++net_it)
    {
        if (net_it->is_none())
            return false;

        to_gate_layout(*net_it);

        return true;
    }

    return false;
}

pybind11::list one_pass_synthesis::mugen_handler::as_py_lists(const std::vector<kitty::dynamic_truth_table>& tts) const
{
    namespace py = pybind11;

    py::list spec_py_list{};

    for (const auto& tt : tts)
    {
        auto tt_py_list = py::list();

        for (auto&& b : iter::reversed(kitty::to_binary(tt)))
        {
            tt_py_list.append(py::int_(b == '0' ? 0 : 1));
        }

        spec_py_list.append(tt_py_list);
    }

    return spec_py_list;
}

pybind11::object one_pass_synthesis::mugen_handler::generate_scheme_graph()
{
    namespace py = pybind11;
    using namespace py::literals;

    auto scheme_graph = mugen.attr("scheme_graph");
    scheme_graph.attr("__init__")(scheme_graph, "shape"_a = py::make_tuple(layout->x(), layout->y()),
            "enable_wire"_a = config.enable_wires, "enable_not"_a = config.enable_not,
            "enable_and"_a = config.enable_and, "enable_or"_a = config.enable_or, "enable_maj"_a = config.enable_maj,
            "enable_crossings"_a = config.crossings, "designated_pi"_a = config.io_ports,
            "designated_po"_a = config.io_ports, "nr_threads"_a = config.num_threads, "timeout"_a = config.timeout);

    for (auto&& t : layout->ground_layer())
    {
        for (auto&& at : layout->outgoing_clocked_tiles(t))
        {
            scheme_graph.attr("add_virtual_edge")(scheme_graph, py::make_tuple(t[X], t[Y]), py::make_tuple(at[X], at[Y]));
        }
    }

    return scheme_graph;
}

void one_pass_synthesis::mugen_handler::to_gate_layout(pybind11::handle net) const
{
    namespace py = pybind11;
    using namespace py::literals;

    using vertex_map = std::map<py::handle, logic_network::vertex>;
    vertex_map v_map{};

    using crossing_map = std::map<py::handle, py::handle>;
    crossing_map c_map{};

    auto network = layout->get_network();

    // extracts the gate type from a node
    const auto get_gate_type = [](const auto& node) -> operation
    {
        static const std::map<std::string, operation> op_map
        {
            {std::string("None"),  operation::NONE},
            {std::string("AND"),   operation::AND},
            {std::string("OR"),    operation::OR},
            {std::string("NOT"),   operation::NOT},
            {std::string("MAJ"),   operation::MAJ},
            {std::string("WIRE"),  operation::W},
            {std::string("CROSS"), operation::W}
        };

        return op_map.at(py::str(node.attr("gate_type")));
    };

    // returns whether a node is a crossing node
    const auto is_crossing = [](const auto& node) -> bool
    {
        return py::str(node.attr("gate_type")).equal(py::str("CROSS"));
    };

    // returns whether fanin_node -> node -> fanout_node is a valid crossing path, where fanout_node is available in c_map
    const auto is_crossing_path = [this, &c_map](const auto& node, const auto& fanin_node) -> bool
    {
        static const std::map<std::string, layout::directions> str_to_dir
        {
            {std::string("NORTH"), layout::DIR_N},
            {std::string("EAST"),  layout::DIR_E},
            {std::string("SOUTH"), layout::DIR_S},
            {std::string("WEST"),  layout::DIR_W}
        };

        auto dir_map = node.attr("dir_map");
        auto fanout_node = c_map.at(node);
        auto get_direction = mugen.attr("get_direction");

        auto inp_dir = get_direction(node.attr("coords"), fanin_node.attr("coords"))[py::int_(0)];
        auto out_dir = get_direction(node.attr("coords"), fanout_node.attr("coords"))[py::int_(0)];

        return str_to_dir.at(py::str(dir_map[inp_dir])) == str_to_dir.at(py::str(out_dir));
    };

    // extract a Python tuple representing coordinates from a node and converts it to a tile
    const auto get_tile = [](const auto& node, const bool second_layer = false) -> fcn_gate_layout::tile
    {
        const auto tuple = py::tuple(node.attr("coords"));
        return {py::int_(tuple[0]), py::int_(tuple[1]), second_layer ? CROSSING : GROUND};
    };

    // checks if the given node is PI
    const auto is_pi = [](const auto& node) -> bool
    {
        return py::bool_(node.attr("is_pi"));
    };

    // checks if the given node is PO
    const auto is_po = [](const auto& node)->bool
    {
        return py::bool_(node.attr("is_po"));
    };

    // returns the ID of a PI node
    const auto get_pi_id = [](const auto& node)
    {
        return py::int_(node.attr("coords"));
    };

    // pre-compute PIs and store them
    std::vector<logic_network::vertex> pis(num_pis);
    for (auto&& i : iter::range(num_pis))
    {
        pis[i] = network->create_pi(fmt::format("pi{}", i));
    }

    // checks whether a node has a PI as fan-in
    const auto has_pi_fanin = [&pis, &is_pi, &get_pi_id](const auto& node) -> std::optional<logic_network::vertex>
    {
        auto fanin = node.attr("fanin");
        for (auto fanin_it = fanin.begin(); fanin_it != fanin.end(); ++fanin_it)
        {
            auto fanin_n = fanin[*fanin_it];
            if (is_pi(fanin_n))
                return pis[get_pi_id(fanin_n)];
        }

        return std::nullopt;
    };

    // assigns tile directions to both given tiles assuming they are adjacent as fanin_t -> cur_t
    const auto assign_directions = [this](const auto& fanin_t, const auto& cur_t) -> void
    {
        layout->assign_tile_out_dir(fanin_t, layout->get_bearing(fanin_t, cur_t));
        layout->assign_tile_inp_dir(cur_t, layout->get_bearing(cur_t, fanin_t));
    };

    auto oc = 0u;
    const std::function<void(const py::handle&, const logic_network::vertex, const fcn_gate_layout::tile&)>
            traverse_and_construct = [&](const py::handle& node, const logic_network::vertex v, const fcn_gate_layout::tile& t)
    {
        auto fanin = node.attr("fanin");
        for (auto fanin_it = fanin.begin(); fanin_it != fanin.end(); ++fanin_it)
        {
            // access the dict
            auto fanin_n = fanin[*fanin_it];

            // make sure that only the intended fan-ins of crossing nodes are explored further
            if (is_crossing(node) && !is_crossing_path(node, fanin_n))
            {
                continue;
            }

            auto n_type = get_gate_type(fanin_n);

            // exit condition
            if (is_pi(fanin_n))
            {
                network->create_edge(pis[get_pi_id(fanin_n)], v);
                continue;
            }

            auto second_layer = false;

            // has fanin node been created already?
            if (auto vm_it = v_map.find(fanin_n); vm_it != v_map.end())
            {
                // if a crossing node is revisited, just flag the tile to be used in second layer
                if (is_crossing(fanin_n))
                {
                    second_layer = true;
                }
                // else, add connections (and update wire type)
                else
                {
                    network->create_edge(vm_it->second, v);
                    auto fanin_t = *layout->get_logic_tile(vm_it->second);
                    assign_directions(fanin_t, t);

                    // update operation as fan-out increases
                    if (n_type == operation::W || n_type == operation::PI)
                        network->assign_op(vm_it->second, operation::F1O2);
                    else if (n_type == operation::F1O2)
                        network->assign_op(vm_it->second, operation::F1O3);

                    continue;
                }
            }

            // is input pin?
            if (auto pi = has_pi_fanin(fanin_n); pi && n_type == operation::W)
            {
                v_map[fanin_n] = *pi;
                auto fanin_t = get_tile(fanin_n);
                layout->assign_logic_vertex(fanin_t, *pi, true, is_po(fanin_n));
                assign_directions(fanin_t, t);

                // recurse with the current vertex instead of *pi to not create self-loops in exit condition branch
                traverse_and_construct(fanin_n, v, fanin_t);

                continue;
            }

            auto fanin_v = network->create_logic_vertex(n_type);
            network->create_edge(fanin_v, v);
            v_map[fanin_n] = fanin_v;

            auto fanin_t = get_tile(fanin_n, second_layer);
            layout->assign_logic_vertex(fanin_t, fanin_v, has_pi_fanin(fanin_n).has_value());
            assign_directions(fanin_t, t);

            // store crossing fan-out for later crossing path checks
            if (is_crossing(fanin_n))
                c_map[fanin_n] = node;

            traverse_and_construct(fanin_n, fanin_v, fanin_t);
        }
    };

    auto pos = net.attr("po_map");
    for (auto po_it = pos.begin(); po_it != pos.end(); ++po_it)
    {
        // for some reason, pybind11 seems to not like lists of tuples; it could not infer the type of *po_it itself
        auto po = py::reinterpret_borrow<py::tuple>(*po_it)[0];
        auto n_type = get_gate_type(po);

        logic_network::vertex v;
        if (n_type == operation::W)
        {
            v = network->create_po(fmt::format("po{}", oc++));
        }
        else
        {
            v = network->create_logic_vertex(n_type);
            network->create_po(v, fmt::format("po{}", oc++));
        }

        v_map[po] = v;

        auto tile = get_tile(po);
        layout->assign_logic_vertex(tile, v, is_pi(po), true);
        traverse_and_construct(po, v, tile);
    }
}

void one_pass_synthesis::update_timeout(mugen_handler& handler, mockturtle::stopwatch<>::duration time) const noexcept
{
    auto time_elapsed = std::chrono::duration_cast<std::chrono::seconds>(time).count();
    // remaining time must be 1 because 0 means unlimited time
    auto time_left = (config.timeout - time_elapsed > 0 ? static_cast<uint32_t>(config.timeout - time_elapsed) : 1u);

    handler.update(time_left);
}
