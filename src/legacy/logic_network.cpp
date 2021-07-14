//
// Created by marcel on 18.05.18.
//

#include "logic_network.h"


logic_network::logic_network() noexcept
        :
        bidirectional_graph(),
        name{""},
        operation_counter(OP_COUNT, 0ul)
{}

logic_network::logic_network(std::string&& name) noexcept
        :
        bidirectional_graph(),
        name{std::move(name)},
        operation_counter(OP_COUNT, 0ul)
{}


logic_network::logic_network(mig_nt&& mig, std::string&& name) noexcept
        :
        bidirectional_graph(),
        name{std::move(name)},
        mig{std::move(mig)},
        operation_counter(OP_COUNT, 0ul)
{
    initialize_network_from_logic();
}

logic_network::num_vertices_t logic_network::vertex_count(const bool ios) const noexcept
{
    auto count = get_vertex_count();
    if (!ios)
        count -= (num_pis() + num_pos());

    return count;
}

logic_network::num_edges_t logic_network::edge_count(const bool ios) const noexcept
{
    auto count = get_edge_count();
    if (!ios)
        count -= (num_pis() + num_pos());

    return count;
}

logic_network::degree_t logic_network::out_degree(const vertex v, const bool ios) const noexcept
{
    auto out = out_edges(v, ios);
    return static_cast<degree_t>(std::distance(out.begin(), out.end()));
}

logic_network::degree_t logic_network::in_degree(const vertex v, const bool ios) const noexcept
{
    auto in = in_edges(v, ios);
    return static_cast<degree_t>(std::distance(in.begin(), in.end()));
}

logic_network::vertex logic_network::create_logic_vertex(const operation o) noexcept
{
    increment_op_counter(o);
    return add_vertex(o);
}

void logic_network::remove_logic_vertex(const vertex v) noexcept
{
    decrement_op_counter(get_op(v));

    pi_set.erase(v);
    po_set.erase(v);
    io_port_map.left.erase(v);

    remove_vertex(v);
}

logic_network::vertex logic_network::create_pi(const std::string& name) noexcept
{
    auto v = create_logic_vertex(operation::PI);
    pi_set.emplace(v);
    io_port_map.insert(port_map::value_type(v, name));

    return v;
}

logic_network::vertex logic_network::create_po(const std::string& name) noexcept
{
    auto v = create_logic_vertex(operation::PO);
    po_set.emplace(v);
    io_port_map.insert(port_map::value_type(v, name));

    return v;
}

logic_network::vertex logic_network::create_po(const vertex a, const std::string& name) noexcept
{
    auto v = create_po(name);
    add_edge(a, v);

    return v;
}

logic_network::vertex logic_network::create_not(const vertex a) noexcept
{
    auto v = create_logic_vertex(operation::NOT);
    add_edge(a, v);

    return v;
}

logic_network::vertex logic_network::create_and(const vertex a, const vertex b) noexcept
{
    auto v = create_logic_vertex(operation::AND);
    add_edge(a, v);
    add_edge(b, v);

    return v;
}

logic_network::vertex logic_network::create_or(const vertex a, const vertex b) noexcept
{
    auto v = create_logic_vertex(operation::OR);
    add_edge(a, v);
    add_edge(b, v);

    return v;
}

logic_network::vertex logic_network::create_maj(const vertex a, const vertex b, const vertex c) noexcept
{
    auto v = create_logic_vertex(operation::MAJ);
    add_edge(a, v);
    add_edge(b, v);
    add_edge(c, v);

    return v;
}

logic_network::vertex logic_network::create_balance_vertex(const edge& e) noexcept
{
    auto v = create_logic_vertex(operation::W);
    auto s = source(e), t = target(e);

    remove_edge(e);
    add_edge(s, v);
    add_edge(v, t);

    return v;
}

void logic_network::assign_op(const vertex v, const operation o) noexcept
{
    properties(v) = o;
}

operation logic_network::get_op(const vertex v) const noexcept
{
    return properties(v);
}

bool logic_network::is_pi(const vertex v) const noexcept
{
    return pi_set.count(v) > 0u;
}

bool logic_network::pre_pi(const vertex v) const noexcept
{
    auto pre = inv_adjacent_vertices(v, true);
    return std::any_of(pre.begin(), pre.end(), [this](const vertex _v){return is_pi(_v);});
}

bool logic_network::is_po(const vertex v) const noexcept
{
    return po_set.count(v) > 0u;
}

bool logic_network::post_po(const vertex v) const noexcept
{
    auto post = adjacent_vertices(v, true);
    return std::any_of(post.begin(), post.end(), [this](const vertex _v){return is_po(_v);});
}

bool logic_network::is_io(const vertex v) const noexcept
{
    return get_op(v) == operation::PI || get_op(v) == operation::PO;
}

uint64_t logic_network::operation_count(const operation o) const noexcept
{
    return operation_counter[o];
}

bool logic_network::is_MIG() const noexcept
{
    for (auto&& oc : iter::range(operation_counter.size()))
    {
        switch (oc)
        {
            case operation::MAJ:
            case operation::NOT:
            case operation::F1O2:
            case operation::F1O3:
            case operation::W:
            case operation::PI:
            case operation::PO:
                continue;
            default:
            {
                if (operation_counter[oc] != 0lu)
                    return false;
            }
        }
    }

    return true;
}

bool logic_network::is_AIG() const noexcept
{
    for (auto&& oc : iter::range(operation_counter.size()))
    {
        switch (oc)
        {
            case operation::AND:
            case operation::NOT:
            case operation::F1O2:
            case operation::F1O3:
            case operation::W:
            case operation::PI:
            case operation::PO:
                continue;
            default:
            {
                if (operation_counter[oc] != 0lu)
                    return false;
            }
        }
    }

    return true;
}

bool logic_network::is_OIG() const noexcept
{
    for (auto&& oc : iter::range(operation_counter.size()))
    {
        switch (oc)
        {
            case operation::OR:
            case operation::NOT:
            case operation::F1O2:
            case operation::F1O3:
            case operation::W:
            case operation::PI:
            case operation::PO:
                continue;
            default:
            {
                if (operation_counter[oc] != 0lu)
                    return false;
            }
        }
    }

    return true;
}

bool logic_network::is_AOIG() const noexcept
{
    for (auto&& oc : iter::range(operation_counter.size()))
    {
        switch (oc)
        {
            case operation::AND:
            case operation::OR:
            case operation::NOT:
            case operation::F1O2:
            case operation::F1O3:
            case operation::W:
            case operation::PI:
            case operation::PO:
                continue;
            default:
            {
                if (operation_counter[oc] != 0lu)
                    return false;
            }
        }
    }

    return true;
}

bool logic_network::is_MAOIG() const noexcept
{
    for (auto&& oc : iter::range(operation_counter.size()))
    {
        switch (oc)
        {
            case operation::MAJ:
            case operation::AND:
            case operation::OR:
            case operation::NOT:
            case operation::F1O2:
            case operation::F1O3:
            case operation::W:
            case operation::PI:
            case operation::PO:
                continue;
            default:
            {
                if (operation_counter[oc] != 0lu)
                    return false;
            }
        }
    }

    return true;
}

std::string logic_network::get_port_name(const vertex v) const noexcept
{
    if (auto it = io_port_map.left.find(v); it != io_port_map.left.end())
    {
        return it->second;
    }
    else
    {
        return "";
    }
}

std::string logic_network::get_name() const noexcept
{
    return name;
}

std::vector<logic_network::edge_path> logic_network::get_all_paths(const vertex v, const bool ios) noexcept
{
    if (get_in_degree(v) == 0u)
        return std::vector<edge_path>{edge_path{}};

    std::vector<edge_path> paths{};
    for (auto&& e : in_edges(v, ios))
    {
        auto ps = get_all_paths(source(e), ios);
        for (auto& p : ps)
            p.push_back(e);

        paths.insert(paths.end(), ps.cbegin(), ps.cend());
    }

    return paths;
}

std::vector<kitty::dynamic_truth_table> logic_network::simulate() const
{
    return mockturtle::simulate<kitty::dynamic_truth_table>(mig,
            mockturtle::default_simulator<kitty::dynamic_truth_table>(mig.num_pis()));
}

void logic_network::substitute_fan_outs(const std::size_t degree, const substitution_strategy stgy, const std::size_t threshold) noexcept
{
    auto connect = [&, this](std::vector<vertex> targets) -> std::vector<vertex>
    {
        std::vector<vertex> new_targets;

        while (!targets.empty())
        {
            if (targets.size() == 1)
            {
                new_targets.push_back(targets.front());
                return new_targets;
            }

            auto fan_out_degree = std::min(targets.size(), degree);

            auto fo = create_logic_vertex(fan_out_degree == 2 ? operation::F1O2 : operation::F1O3);
            std::for_each(targets.end() - fan_out_degree, targets.end(), [_fo = fo, this](auto v){ add_edge(_fo, v); });

            if (stgy == substitution_strategy::BREADTH)
            {
                new_targets.push_back(fo);
                targets.erase(targets.end() - fan_out_degree, targets.end());
                continue;
            }
            else if (stgy == substitution_strategy::DEPTH)
            {
                std::copy(targets.begin(), targets.end() - fan_out_degree, std::back_inserter(new_targets));
                new_targets.push_back(fo);
                return new_targets;
            }
        }

        return new_targets;
    };

    for (auto&& v : vertices(true) | iter::filterfalse([this](const auto _v)
                                            { return get_op(_v) == operation::F1O2 || get_op(_v) == operation::F1O3; }))
    {
        std::size_t specific_threshold = (get_op(v) == operation::NOT || get_op(v) == operation::PI) ? 1 : threshold;

        if (get_out_degree(v) > specific_threshold)
        {
            auto oe = out_edges(v, true);
            std::vector<edge> edges(oe.begin(), oe.end());
            // remove the last (specific_threshold - 1) elements which won't be substituted
            edges.erase(edges.end() - (specific_threshold - 1), edges.end());

            std::vector<vertex> targets{};
            // figure out targets and remove edges which will be substituted
            std::for_each(edges.begin(), edges.end(),
                    [this, &targets](auto _oe){ targets.push_back(target(_oe)); remove_edge(_oe); });

            while (targets.size() > 1)
            {
                targets = connect(targets);
            }

            add_edge(v, targets.front());
        }
    }
}

void logic_network::write_dot(std::ostream& os) const noexcept
{
    os << "digraph G {\n";

    for (auto&& v : vertices())
        os << fmt::format("{} [label=<<B>{}</B><br/>{}>];\n", v, v, name_str(get_op(v)));

    for (auto&& pi : get_pis())
        os << fmt::format("{} [label=<<B>{}</B><br/>{}>,shape=invtriangle,color=green];\n", pi, pi, get_port_name(pi));

    for (auto&& po : get_pos())
        os << fmt::format("{} [label=<<B>{}</B><br/>{}>,shape=triangle,color=red];\n", po, po, get_port_name(po));

    os << "{ rank = same; ";
    for (auto&& pi : get_pis())
        os << pi << "; ";
    os << "}\n{ rank = same; ";
    for (auto&& po : get_pos())
        os << po << "; ";
    os << "}\n";

    for (auto&& e : edges(true))
        os << fmt::format("{}->{};\n", source(e), target(e));

    os << "}\n" << std::endl;
}

void logic_network::write_simple_dot(std::ostream& os) const noexcept
{
    os << "digraph G {\nnode[label=\"\",style=filled,shape=circle];\n";

    for (auto&& v : vertices())
    {
        if (auto op = get_op(v); op == operation::W)
        {
            os << fmt::format("{} [shape=point,fillcolor=black,width=0.2,height=0.2];\n", v, name_str(op));
        }
        else
        {
            os << fmt::format("{} [fillcolor=white];\n", v, name_str(op));
        }
    }

    for (auto&& pi : get_pis())
        os << fmt::format("{} [shape=invtriangle,fillcolor=black];\n", pi, pi, get_port_name(pi));

    for (auto&& po : get_pos())
        os << fmt::format("{} [shape=triangle,fillcolor=black];\n", po, po, get_port_name(po));

    os << "{ rank = same; ";
    for (auto&& pi : get_pis())
        os << pi << "; ";
    os << "}\n{ rank = same; ";
    for (auto&& po : get_pos())
        os << po << "; ";
    os << "}\n";

    for (auto&& e : edges(true))
        os << fmt::format("{}->{};\n", source(e), target(e));

    os << "}\n" << std::endl;
}

void logic_network::clear_network() noexcept
{
    clear_graph();

    for (auto i = 0u; i < OP_COUNT; ++i)
        operation_counter[i] = 0;
}

void logic_network::increment_op_counter(const operation o) noexcept
{
    ++operation_counter[o];
}

void logic_network::decrement_op_counter(const operation o) noexcept
{
    --operation_counter[o];
}

void logic_network::initialize_network_from_logic() noexcept
{
    clear_network();

    mockturtle::topo_view<mig_nt> topo_mig(mig);
    std::unordered_map<typename mockturtle::topo_view<mig_nt>::node, vertex> node_to_vertex{};

    auto pi_c = 0ul, po_c = 0ul;

    // create PIs
    topo_mig.foreach_pi([&](const auto& pi)
    {
        const auto s = topo_mig.make_signal(topo_mig.node_to_index(pi));
        const std::string port_name = topo_mig.has_name(s) ? topo_mig.get_name(s) : fmt::format("pi{}", pi_c++);

        node_to_vertex.emplace(pi, create_pi(port_name));
    });
    // create gates
    topo_mig.foreach_gate([&](const auto& g)
    {
        // figure out MAJ type (MAJ, AND, OR) and predecessors
        operation maj_op = operation::MAJ;

        std::vector<vertex> predecessors{};
        topo_mig.foreach_fanin(g, [&](const mockturtle::topo_view<mig_nt>::signal& f) -> void
        {
            const auto f_n = topo_mig.get_node(f);
            if (topo_mig.is_constant(f_n))
            {
                assert(maj_op == operation::MAJ && "MIG nodes must not have more than one constant input");
                maj_op = topo_mig.is_complemented(f) ? operation::OR : operation::AND;
            }
            else
            {
                predecessors.push_back(topo_mig.is_complemented(f) ? create_not(node_to_vertex.at(f_n)) : node_to_vertex.at(f_n));
            }
        });
        // create gate vertex
        switch (maj_op)
        {
            case operation::MAJ:
            {
                node_to_vertex.emplace(g, create_maj(predecessors[0], predecessors[1], predecessors[2]));
                break;
            }
            case operation::AND:
            {
                node_to_vertex.emplace(g, create_and(predecessors[0], predecessors[1]));
                break;
            }
            case operation::OR:
            {
                node_to_vertex.emplace(g, create_or(predecessors[0], predecessors[1]));
                break;
            }
            default:  // cannot occur
            {
                break;
            }
        }
    });
    // create POs
    topo_mig.foreach_po([&](const auto& po, auto i)
    {
        const std::string port_name = topo_mig.has_output_name(i) ? topo_mig.get_output_name(i) : fmt::format("po{}", po_c++);

        const auto po_n = topo_mig.get_node(po);

        if (topo_mig.is_constant(po_n))
            std::cout << "[w] omitted constant output " << port_name << std::endl;
        else
            create_po(topo_mig.is_complemented(po) ? create_not(node_to_vertex.at(po_n)) : node_to_vertex.at(po_n), port_name);
    });
}
