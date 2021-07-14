//
// Created by marcel on 26.04.18.
//

#include "fcn_gate_layout.h"


fcn_gate_layout::fcn_gate_layout(const fcn_dimension_xyz& lengths, fcn_clocking_scheme clocking, logic_network_ptr ln, offset o) noexcept
        :
        fcn_layout(lengths, std::move(clocking), o),
        network(std::move(ln))
{}

fcn_gate_layout::fcn_gate_layout(const fcn_dimension_xy& lengths, fcn_clocking_scheme clocking, logic_network_ptr ln, offset o) noexcept
        :
        fcn_layout(fcn_dimension_xyz{lengths[X], lengths[Y], 2}, std::move(clocking), o),
        network(std::move(ln))
{}

fcn_gate_layout::fcn_gate_layout(const fcn_dimension_xy& lengths, logic_network_ptr ln, offset o) noexcept
        :
        fcn_layout(fcn_dimension_xyz{lengths[X], lengths[Y], 2}, std::move(open_4_clocking), o),
        network(std::move(ln))
{}

fcn_gate_layout::fcn_gate_layout(fcn_clocking_scheme clocking, logic_network_ptr ln, offset o) noexcept
        :
        fcn_layout(fcn_dimension_xyz{2, 2, 2}, std::move(clocking), o),
        network(std::move(ln))
{}

fcn_gate_layout::fcn_gate_layout(logic_network_ptr ln, offset o) noexcept
        :
        fcn_layout(fcn_dimension_xyz{2, 2, 2}, std::move(open_4_clocking), o),
        network(std::move(ln))
{}

std::optional<fcn_gate_layout::tile> fcn_gate_layout::random_gate() const noexcept
{
    std::mt19937 rgen(std::random_device{}());
    std::uniform_int_distribution<coord_t> dist(0, v_map.left.size() - 1); // distribution in range [0, |G|]

    const auto rnd_it = std::next(std::begin(v_map.left), dist(rgen));

    return rnd_it == v_map.left.end() ? std::nullopt : std::make_optional(rnd_it->first);
}

bool fcn_gate_layout::is_incoming_clocked(const tile& t1, const tile& t2) const noexcept
{
    if (!tile_clocking(t1) || !tile_clocking(t2))
        return false;

    if (t1 == t2)
        return false;

    return static_cast<fcn_clock::zone>((*tile_clocking(t2) + get_latch(t2) + 1) %
                                        clocking.num_clocks) == *tile_clocking(t1);
}

bool fcn_gate_layout::is_outgoing_clocked(const tile& t1, const tile& t2) const noexcept
{
    if (!tile_clocking(t1) || !tile_clocking(t2))
        return false;

    if (t1 == t2)
        return false;

    return static_cast<fcn_clock::zone>((*tile_clocking(t1) + get_latch(t1) + 1) %
                                        clocking.num_clocks) == *tile_clocking(t2);
}

layout::directions fcn_gate_layout::closest_border(const tile& t) const noexcept
{
    // tile is on the left side of the layout
    if (t[X] < x() / 2)
    {
        // tile is on the upper side of the layout
        if (t[Y] < y() / 2)
        {
            return t[X] < t[Y] ? layout::DIR_W : layout::DIR_N;
        }
        // tile is on the lower side of the layout
        else
        {
            return t[X] < y() - t[Y] ? layout::DIR_W : layout::DIR_S;
        }
    }
    // tile is on the right side of the layout
    else
    {
        // tile is on the upper side of the layout
        if (t[Y] < y() / 2)
        {
            return x() - t[X] <= t[Y] ? layout::DIR_E : layout::DIR_N;
        }
        // tile is on the lower side of the layout
        else
        {
            return x() - t[X] < y() - t[Y] ? layout::DIR_E : layout::DIR_S;
        }
    }
}

std::optional<fcn_clock::zone> fcn_gate_layout::tile_clocking(const tile& t) const noexcept
{
    if (clocking.regular)
    {
        auto x = t[X], y = t[Y];
        return clocking.scheme[y % clocking.cutout_y][x % clocking.cutout_x];
    }
    else  // irregular clocking accesses clocking map
    {
        if (auto it = c_map.find(get_ground(t)); it != c_map.end())
        {
            return it->second;
        }
        else
        {
            return std::nullopt;
        }
    }
}

std::optional<fcn_clock::zone> fcn_gate_layout::tile_clocking(const tile_index t) const noexcept
{
    return tile_clocking(get_vertex(t));
}

void fcn_gate_layout::assign_logic_vertex(const tile& t, const logic_network::vertex v, const bool pi, const bool po) noexcept
{
    dissociate_logic_edges(t);
    dissociate_logic_vertex(t);
    v_map.insert(vertex_map::value_type(t, v));

    // keep track of I/O sets
    if (pi)
        pi_set.emplace(t);
    if (po)
        po_set.emplace(t);
}

void fcn_gate_layout::dissociate_logic_vertex(const tile& t) noexcept
{
    if (is_wire_tile(t))
        return;

    v_map.left.erase(t);

    // keep track of I/O sets
    pi_set.erase(t);
    po_set.erase(t);

    // remove directions associated with t
    inp_dir_map.erase(t);
    out_dir_map.erase(t);
}

std::optional<logic_network::vertex> fcn_gate_layout::get_logic_vertex(const tile& t) const noexcept
{
    if (auto it = v_map.left.find(t); it != v_map.left.end())
    {
        return it->second;
    }
    else
    {
        return std::nullopt;
    }
}

bool fcn_gate_layout::is_gate_tile(const tile& t) const noexcept
{
    return v_map.left.find(t) != v_map.left.end();
}

bool fcn_gate_layout::has_logic_vertex(const tile& t, const logic_network::vertex v) const noexcept
{
    if (auto it = v_map.left.find(t); it != v_map.left.end())
    {
        return it->second == v;
    }
    else
    {
        return false;
    }
}

std::optional<fcn_gate_layout::tile> fcn_gate_layout::get_logic_tile(const logic_network::vertex v) const noexcept
{
    if (auto it = v_map.right.find(v); it != v_map.right.end())
    {
        return it->second;
    }
    else
    {
        return std::nullopt;
    }
}

void fcn_gate_layout::assign_logic_edge(const tile& t, const logic_network::edge& e) noexcept
{
    dissociate_logic_vertex(t);
    e_map[t].emplace(e);
}

void fcn_gate_layout::dissociate_logic_edge(const tile& t, const logic_network::edge& e) noexcept
{
    if (auto it = e_map.find(t); it != e_map.end())
    {
        it->second.erase(e);
        // if this was the last edge assigned, save memory
        if (it->second.empty())
            e_map.erase(t);

        edge_inp_dir_map.erase({t, e});
        edge_out_dir_map.erase({t, e});

        // recalculate tile directions based on the leftover wires
        const auto& edges = get_logic_edges(t);
        inp_dir_map[t] = std::accumulate(edges.cbegin(), edges.cend(), layout::DIR_NONE,
                                         [this, &t](auto val, const auto& _e)
                                         { return val | get_wire_inp_dirs(t, _e); });
        out_dir_map[t] = std::accumulate(edges.cbegin(), edges.cend(), layout::DIR_NONE,
                                         [this, &t](auto val, const auto& _e)
                                         { return val | get_wire_out_dirs(t, _e); });
    }

    // if tile t does not have any edges assigned, do nothing
}

void fcn_gate_layout::dissociate_logic_edges(const tile& t) noexcept
{
    inp_dir_map.erase(t);
    out_dir_map.erase(t);

    for (auto& e : get_logic_edges(t))
    {
        edge_inp_dir_map.erase({t, e});
        edge_out_dir_map.erase({t, e});
    }

    e_map.erase(t);
}

fcn_gate_layout::edge_set fcn_gate_layout::get_logic_edges(const tile& t) const noexcept
{
    if (auto it = e_map.find(t); it != e_map.end())
    {
        return it->second;
    }
    else
    {
        return {};
    }
}

bool fcn_gate_layout::is_wire_tile(const tile& t) const noexcept
{
    if (auto it = e_map.find(t); it != e_map.end())
    {
        return !it->second.empty();
    }
    else
    {
        return false;
    }
}

bool fcn_gate_layout::has_logic_edge(const tile& t, const logic_network::edge& e) const noexcept
{
    if (auto it = e_map.find(t); it != e_map.end())
    {
        return it->second.count(e) > 0u;
    }
    else
    {
        return false;
    }
}

void fcn_gate_layout::clear_tile(const tile& t) noexcept
{
    inp_dir_map.erase(t);
    out_dir_map.erase(t);

    for (auto& e : get_logic_edges(t))
    {
        edge_inp_dir_map.erase({t, e});
        edge_out_dir_map.erase({t, e});
    }

    dissociate_logic_vertex(t);
    dissociate_logic_edges(t);
    assign_latch(t, 0);
}

bool fcn_gate_layout::is_free_tile(const tile& t) const noexcept
{
    return !is_gate_tile(t) && !is_wire_tile(t);
}

void fcn_gate_layout::assign_tile_inp_dir(const tile& t, layout::directions d) noexcept
{
    // do not do anything if t is a free tile
    if (is_free_tile(t))
        return;

    if (d == layout::DIR_NONE)
        inp_dir_map.erase(t);
    else
        inp_dir_map[t] |= d;
}

void fcn_gate_layout::assign_wire_inp_dir(const tile& t, const logic_network::edge& e, layout::directions d) noexcept
{
    // do not do anything if e is not a wire on t
    if (!has_logic_edge(t, e))
        return;

    if (d == layout::DIR_NONE)
    {
        inp_dir_map.erase(t);
        edge_inp_dir_map.erase({t, e});
    }
    else
    {
        inp_dir_map[t] |= d;
        edge_inp_dir_map[{t, e}] |= d;
    }
}

bool fcn_gate_layout::is_tile_inp_dir(const tile& t, const layout::directions& d) const noexcept
{
    if (auto it = inp_dir_map.find(t); it != inp_dir_map.end())
    {
        return (it->second & d) == d;
    }
    else
    {
        return d == layout::DIR_NONE;
    }
}

bool fcn_gate_layout::is_wire_inp_dir(const tile& t, const logic_network::edge& e, const layout::directions& d) const noexcept
{
    if (auto it = edge_inp_dir_map.find({t, e}); it != edge_inp_dir_map.end())
    {
        return (it->second & d) == d;
    }
    else
    {
        return false;
    }
}

layout::directions fcn_gate_layout::get_tile_inp_dirs(const tile& t) const noexcept
{
    if (auto it = inp_dir_map.find(t); it != inp_dir_map.end())
    {
        return it->second;
    }
    else
    {
        return layout::DIR_NONE;
    }
}

layout::directions fcn_gate_layout::get_wire_inp_dirs(const tile& t, const logic_network::edge& e) const noexcept
{
    if (auto it = edge_inp_dir_map.find({t, e}); it != edge_inp_dir_map.end())
    {
        return it->second;
    }
    else
    {
        return layout::DIR_NONE;
    }
}

void fcn_gate_layout::assign_tile_out_dir(const tile& t, layout::directions d) noexcept
{
    // do not do anything if t is a free tile
    if (is_free_tile(t))
        return;

    if (d == layout::DIR_NONE)
        out_dir_map.erase(t);
    else
        out_dir_map[t] |= d;
}

void fcn_gate_layout::assign_wire_out_dir(const tile& t, const logic_network::edge& e, layout::directions d) noexcept
{
    // do not do anything if e is not a wire on t
    if (!has_logic_edge(t, e))
        return;

    if (d == layout::DIR_NONE)
    {
        out_dir_map.erase(t);
        edge_out_dir_map.erase({t, e});
    }
    else
    {
        out_dir_map[t] |= d;
        edge_out_dir_map[{t, e}] |= d;
    }
}

bool fcn_gate_layout::is_tile_out_dir(const tile& t, const layout::directions& d) const noexcept
{
    if (auto it = out_dir_map.find(t); it != out_dir_map.end())
    {
        return (it->second & d) == d;
    }
    else
    {
        return d == layout::DIR_NONE;
    }
}

bool fcn_gate_layout::is_wire_out_dir(const tile& t, const logic_network::edge& e, const layout::directions& d) const noexcept
{
    if (auto it = edge_out_dir_map.find({t, e}); it != edge_out_dir_map.end())
    {
        return (it->second & d) == d;
    }
    else
    {
        return false;
    }
}

layout::directions fcn_gate_layout::get_tile_out_dirs(const tile& t) const noexcept
{
    if (auto it = out_dir_map.find(t); it != out_dir_map.end())
    {
        return it->second;
    }
    else
    {
        return layout::DIR_NONE;
    }
}

layout::directions fcn_gate_layout::get_wire_out_dirs(const tile& t, const logic_network::edge& e) const noexcept
{
    if (auto it = edge_out_dir_map.find({t, e}); it != edge_out_dir_map.end())
    {
        return it->second;
    }
    else
    {
        return layout::DIR_NONE;
    }
}

layout::directions fcn_gate_layout::get_unused_tile_dirs(const tile& t) const noexcept
{
    return ~(get_tile_inp_dirs(t) | get_tile_out_dirs(t));
}

bool fcn_gate_layout::is_tile_dir(const tile& t, const layout::directions& d) const noexcept
{
    return ((get_tile_inp_dirs(t) | get_tile_out_dirs(t)) & d) == d;
}

bool fcn_gate_layout::is_wire_dir(const tile& t, const logic_network::edge& e, const layout::directions& d) const noexcept
{
    return ((get_wire_inp_dirs(t, e) | get_wire_out_dirs(t, e)) & d) == d;
}

layout::directions fcn_gate_layout::get_bearing(const tile& t1, const tile& t2) const noexcept
{
    // if x-dimensions are the same
    if (t1[X] == t2[X])
    {
        if (t1[Y] - t2[Y] == 1)
            return layout::DIR_N;
        if (static_cast<int>(t1[Y]) - static_cast<int>(t2[Y]) == -1)
            return layout::DIR_S;
    }
    // if y-dimensions are the same
    if (t1[Y] == t2[Y])
    {
        if (t1[X] - t2[X] == 1)
        {
            if (!is_vertically_shifted())
                return layout::DIR_W;
            else  // vertical shift
            {
                if (is_even_column(t1))
                    return layout::DIR_SW;
                else
                    return layout::DIR_NW;
            }
        }
        if (static_cast<int>(t1[X]) - static_cast<int>(t2[X]) == -1)
        {
            if (!is_vertically_shifted())
                return layout::DIR_E;
            else  // vertical shift
            {
                if (is_even_column(t1))
                    return layout::DIR_ES;
                else
                    return layout::DIR_NE;
            }
        }
    }
    // special cases for vertically shifted layouts
    if (is_vertically_shifted())
    {
        // if t2 is west of t1
        if (t1[X] - t2[X] == 1)
        {
            // and y-dimensions differ by one
            if (t1[Y] - t2[Y] == 1)
            {
                if (is_even_column(t1))
                    return layout::DIR_NW;
            }
            else if (static_cast<int>(t1[Y]) - static_cast<int>(t2[Y]) == -1)
            {
                if (is_odd_column(t1))
                    return layout::DIR_SW;
            }
        }
            // if t2 is east of t1
        else if (static_cast<int>(t1[X]) - static_cast<int>(t2[X]) == -1)
        {
            // and y-dimensions differ by one
            if (t1[Y] - t2[Y] == 1)
            {
                if (is_even_column(t1))
                    return layout::DIR_NE;
            }
            else if (static_cast<int>(t1[Y]) - static_cast<int>(t2[Y]) == -1)
            {
                if (is_odd_column(t1))
                    return layout::DIR_ES;
            }
        }
    }
//    // special cases for horizontally shifted layouts
//    else if (shift == offset::HORIZONTAL)
//    {
//
//    }

    // tiles are not straightly reachable
    return layout::DIR_NONE;
}

std::optional<fcn_gate_layout::gate_or_wire> fcn_gate_layout::is_data_flow(const gate_or_wire& gw, const tile& at, const bool out) const noexcept
{
    // if a gate was passed as gw
    if (auto v1 = std::get_if<logic_network::vertex>(&gw))
    {
        // if at is a gate as well
        if (auto v2 = get_logic_vertex(at))
        {
            // if connection v1 -> v2 or v2 -> v1 exists respectively
            if (out ? network->get_edge(*v1, *v2) : network->get_edge(*v2, *v1))
                return gate_or_wire{*v2};
        }
        // if at is a wire tile instead
        for (auto& e2 : get_logic_edges(at))
        {
            // if v1 is the source/target of e2
            if (out ? (network->source(e2) == *v1) : (network->target(e2) == *v1))
                return gate_or_wire{e2};
        }
    }
    // if a wire was passed as gw
    if (auto e1 = std::get_if<logic_network::edge>(&gw))
    {
        // if v2 is a gate tile
        if (auto v2 = get_logic_vertex(at))
        {
            // if v2 is the target/source of e1
            if (out ? (network->target(*e1) == *v2) : (network->source(*e1) == *v2))
                return gate_or_wire{*v2};
        }
        // if at is a wire tile instead
        if (has_logic_edge(at, *e1))
            return gate_or_wire{*e1};
    }

    // no information flow from t1 to at or non-matching arguments
    return std::nullopt;
}

std::optional<fcn_gate_layout::gate_or_wire> fcn_gate_layout::is_out_data_flow(const gate_or_wire& gw, const tile& at) const noexcept
{
    return is_data_flow(gw, at, true);
}

std::optional<fcn_gate_layout::gate_or_wire> fcn_gate_layout::is_in_data_flow(const gate_or_wire& gw, const tile& at) const noexcept
{
    return is_data_flow(gw, at, false);
}

bool fcn_gate_layout::is_data_flow(const tile& t1, const tile& t2) const noexcept
{
    if (auto v = get_logic_vertex(t1))
    {
        return static_cast<bool>(is_out_data_flow({*v}, t2));
    }
    else if (is_wire_tile(t1))
    {
        for (const auto& e : get_logic_edges(t1))
        {
            if (is_out_data_flow({e}, t2))
                return true;
        }
    }

    return false;
}

std::vector<fcn_gate_layout::tile_assignment> fcn_gate_layout::outgoing_data_flow(const tile& t, const gate_or_wire& gw) const noexcept
{
    std::vector<tile_assignment> odf{}; // outgoing data flow

    // for all outgoing clocked tiles
    for (auto&& oct : outgoing_clocked_tiles(t))
    {
        for (const auto& at : std::set<tile>{{oct, above(oct), below(oct)}})
        {
            if (is_tile_out_dir(t, get_bearing(t, at)))
            {
                if (auto target = is_out_data_flow(gw, at); target)
                    odf.emplace_back(at, *target);
            }
        }
    }

    return odf;
}

std::vector<fcn_gate_layout::tile> fcn_gate_layout::outgoing_data_flow(const tile& t) const noexcept
{
    std::vector<tile> odf{}; // outgoing data flow

    for (auto&& oct : outgoing_clocked_tiles(t))
    {
        for (const auto& at : std::set<tile>{{oct, above(oct), below(oct)}})
        {
            if (is_data_flow(t, at) && is_tile_out_dir(t, get_bearing(t, at)))
                odf.push_back(at);
        }
    }

    return odf;
}

std::vector<fcn_gate_layout::tile_assignment> fcn_gate_layout::incoming_data_flow(const tile& t, const gate_or_wire& gw) const noexcept
{
    std::vector<tile_assignment> idf{}; // incoming data flow

    // for all incoming clocked tiles
    for (auto&& ict : incoming_clocked_tiles(t))
    {
        for (const auto& iat : std::set<tile>{{ict, above(ict), below(ict)}})
        {
            if (is_tile_inp_dir(t, get_bearing(t, iat)))
            {
                if (auto source = is_in_data_flow(gw, iat); source)
                    idf.emplace_back(iat, *source);
            }
        }
    }

    return idf;
}

std::vector<fcn_gate_layout::tile> fcn_gate_layout::incoming_data_flow(const tile& t) const noexcept
{
    std::vector<tile> idf{}; // incoming data flow

    for (auto&& ict : incoming_clocked_tiles(t))
    {
        for (const auto& iat : std::set<tile>{{ict, above(ict), below(ict)}})
        {
            if (is_data_flow(iat, t) && is_tile_inp_dir(t, get_bearing(t, iat)))
                idf.push_back(iat);
        }
    }

    return idf;
}

operation fcn_gate_layout::get_op(const tile& t) const noexcept
{
    if (auto it = v_map.left.find(t); it != v_map.left.end())
    {
        return network->get_op(it->second);
    }
    else
    {
        return is_wire_tile(t) ? operation::W : operation::NONE;
    }
}

fcn_gate_layout::path_info fcn_gate_layout::signal_delay(const tile& t, const gate_or_wire& gw, delay_cache& dc) const noexcept
{
    if (is_free_tile(t))
        return {};

    if (auto idf = incoming_data_flow(t, gw); idf.empty())
        return {1, *tile_clocking(t), 0};
    else if (auto it = dc.find(t); it != dc.end())  // cache hit
        return it->second;
    else  // cache miss
    {
        // fetch information about all incoming paths
        std::vector<path_info> infos{};
        for (const auto& [_t, _gw] : idf)
            infos.push_back(signal_delay(_t, _gw, dc));

        path_info dominant_path{};

        if (is_pi(t))  // primary input in the circuit
            infos.push_back({1, static_cast<std::size_t>((*tile_clocking(t) + (num_clocks() - 1)) % num_clocks()), 0});

        if (infos.size() == 1)  // size cannot be 0
            dominant_path = infos.front();
        else  // fetch highest delay and difference
        {
            // sort by path length
            std::sort(infos.begin(), infos.end(), [](const auto& i1, const auto& i2) { return i1.length < i2.length; });

            dominant_path.length = infos.back().length;
            dominant_path.delay = infos.back().delay;
            dominant_path.diff = infos.back().delay - infos.front().delay;
        }

        // incorporate self
        ++dominant_path.length;
        ++dominant_path.delay;

        // cache value for gates only
        if (is_gate_tile(t))
            dc[t] = dominant_path;

        return dominant_path;
    }
}

std::pair<std::size_t, std::size_t> fcn_gate_layout::critical_path_length_and_throughput() const noexcept
{
    delay_cache dc{};
    std::size_t critical_path = 0;
    for (auto&& po : get_pos())
        critical_path = std::max(signal_delay(po, gate_or_wire{*get_logic_vertex(po)}, dc).length, critical_path);

    std::size_t throughput = 0;
    if (!dc.empty())
        throughput = std::max_element(dc.begin(), dc.end(),
                                      [](const auto& i1, const auto& i2) { return i1.second.diff < i2.second.diff; })->second.diff;

    // give throughput in cycles, not in phases
    throughput /= num_clocks();

    // convert cycle difference to throughput
    ++throughput;

    return {critical_path, throughput};
}

logic_network::mig_nt fcn_gate_layout::extract() const noexcept
{
    logic_network::mig_nt mig;

    using extraction_cache = std::unordered_map<tile, std::vector<logic_network::mig_nt::signal>, boost::hash<tile>>;
    extraction_cache cache{};

    // check if all but one incoming signals have been cached (the one missing signal is the current one)
    const auto is_discovered = [&cache, this](const auto& _t) -> bool
    {
        if (auto it = cache.find(_t); it != cache.end())
        {
            auto pre = network->in_edges(*get_logic_vertex(_t), true);
            return (it->second.size() + 1ul) == static_cast<std::size_t>(std::distance(pre.begin(), pre.end()));
        }
        else
        {
            return false;
        }
    };

    // recursive function to traverse the layout by following the signal propagation (data flow)
    // when encountering a gate, it is constructed in the logic description if all input gates have been realized already
    const std::function<void(const tile_assignment&, const logic_network::mig_nt::signal&)>
            follow_data_flow = [&](const auto& ta, const auto& s) -> void
    {
        auto& [t, gw] = ta;

        // current tile is a gate
        if (auto g = std::get_if<logic_network::vertex>(&gw))
        {
            switch (auto op = network->get_op(*g); op)
            {
                // two-input gate
                case operation::AND:
                case operation::OR:
                {
                    if (is_discovered(t))
                    {
                        const auto s2 = cache.at(t).front();
                        const auto aos = op == operation::AND ? mig.create_and(s, s2) : mig.create_or(s, s2);

                        for (auto&& odf : outgoing_data_flow(t, gw))
                        {
                            follow_data_flow(odf, aos);
                        }
                    }
                    else
                    {
                        cache[t].push_back(s);
                    }

                    break;
                }
                // three-input gate
                case operation::MAJ:
                {
                    if (is_discovered(t))
                    {
                        const auto s2 = cache.at(t).front(), s3 = cache.at(t).back();
                        const auto ms = mig.create_maj(s, s2, s3);

                        for (auto&& odf : outgoing_data_flow(t, gw))
                        {
                            follow_data_flow(odf, ms);
                        }
                    }
                    else
                    {
                        cache[t].push_back(s);
                    }

                    break;
                }
                // balance node, PI, or fan-out
                case operation::W:
                case operation::PI:
                case operation::F1O2:
                case operation::F1O3:
                {
                    for (auto&& odf : outgoing_data_flow(t, gw))
                    {
                        follow_data_flow(odf, s);
                    }

                    break;
                }
                // NOT gate, invert the signal
                case operation::NOT:
                {
                    for (auto&& odf : outgoing_data_flow(t, gw))
                    {
                        follow_data_flow(odf, !s);
                    }

                    break;
                }
                // cache primary output for post-processing
                case operation::PO:
                {
                    cache[t].push_back(s);

                    break;
                }
                default:
                {
                    break;
                }
            }
        }
        // current tile is a wire (simply propagate the signal)
        else if (auto w = std::get_if<logic_network::edge>(&gw))
        {
            for (auto&& odf : outgoing_data_flow(t, {*w}))
                follow_data_flow(odf, s);
        }
    };

    // pre-processing to keep PI order
    for (auto&& pi : get_pis() | iter::sorted([this](const auto& _t1, const auto& _t2)
                                              { return *get_logic_vertex(_t1) < *get_logic_vertex(_t2); }))
    {
        for (const auto& n : get_inp_names(pi))
            cache[pi].push_back(mig.create_pi(n));
    }

    // extract logic representation by traversing the layout starting at PIs
    for (auto&& pi : get_pis())
    {
        const auto v = *get_logic_vertex(pi);
        const auto s = cache.at(pi).front();
        cache.at(pi).clear();

        follow_data_flow({pi, v}, s);
    }

    // post-processing to keep PO order
    for (auto&& po : get_pos() | iter::sorted([this](const auto& _t1, const auto& _t2)
                                              { return *get_logic_vertex(_t1) < *get_logic_vertex(_t2); }))
    {
        for (const auto& n : get_out_names(po))
            mig.create_po(cache.at(po).back(), n);
    }

    return mig;
}

std::vector<kitty::dynamic_truth_table> fcn_gate_layout::simulate() const
{
    const auto mig = extract();
    return mockturtle::simulate<kitty::dynamic_truth_table>(mig,
            mockturtle::default_simulator<kitty::dynamic_truth_table>(mig.num_pis()));
}

std::vector<std::string> fcn_gate_layout::get_inp_names(const tile& t) const noexcept
{
    std::vector<std::string> inp_names{};

    if (get_op(t) == operation::PI)
    {
        inp_names.push_back(network->get_port_name(*get_logic_vertex(t)));
        return inp_names;
    }
    else
    {
        if (auto v = get_logic_vertex(t))
        {
            for (auto&& iav : network->inv_adjacent_vertices(*v, true))
            {
                if (network->get_op(iav) == operation::PI)
                    inp_names.push_back(network->get_port_name(iav));
            }
        }

        return inp_names;
    }
}

std::vector<std::string> fcn_gate_layout::get_out_names(const tile& t) const noexcept
{
    std::vector<std::string> out_names{};

    if (get_op(t) == operation::PO)
    {
        out_names.push_back(network->get_port_name(*get_logic_vertex(t)));
        return out_names;
    }
    else
    {
        if (auto v = get_logic_vertex(t))
        {
            for (auto&& av : network->adjacent_vertices(*v, true))
            {
                if (network->get_op(av) == operation::PO)
                    out_names.push_back(network->get_port_name(av));
            }
        }

        return out_names;
    }
}

bool fcn_gate_layout::has_io_pins() const noexcept
{
    return std::all_of(pi_set.cbegin(), pi_set.cend(), [this](const auto& pi){return get_op(pi) == operation::PI;}) &&
            std::all_of(po_set.cbegin(), po_set.cend(), [this](const auto& po){return get_op(po) == operation::PO;});
}

std::string fcn_gate_layout::get_name() const noexcept
{
    return network->get_name();
}

logic_network_ptr fcn_gate_layout::get_network() const noexcept
{
    return network;
}

fcn_layout::bounding_box fcn_gate_layout::determine_bounding_box() const noexcept
{
    // calculate min_x
    coord_t min_x = 0u;
    for (coord_t x = 0u; x < this->x(); ++x)
    {
        bool elem_found = false;
        for (coord_t y = 0u; y < this->y(); ++y)
        {
            if (!this->is_free_tile(tile{x, y, GROUND}) || !this->is_free_tile(tile{x, y, CROSSING}))
            {
                elem_found = true;
                break;
            }
        }

        min_x = x;
        if (elem_found)
            break;
    }

    // calculate min_y
    coord_t min_y = 0u;
    for (coord_t y = 0u; y < this->y(); ++y)
    {
        bool elem_found = false;
        for (coord_t x = 0u; x < this->x(); ++x)
        {
            if (!this->is_free_tile(tile{x, y, GROUND}) || !this->is_free_tile(tile{x, y, CROSSING}))
            {
                elem_found = true;
                break;
            }
        }

        min_y = y;
        if (elem_found)
            break;
    }

    // calculate max_x
    coord_t max_x = this->x() - 1;
    for (auto x = static_cast<long>(this->x()) - 1; x >= 0; --x)
    {
        bool elem_found = false;
        for (coord_t y = 0u; y < this->y(); ++y)
        {
            if (!this->is_free_tile(tile{static_cast<coord_t>(x), y, GROUND}) ||
                    !this->is_free_tile(tile{static_cast<coord_t>(x), y, CROSSING}))
            {
                elem_found = true;
                break;
            }
        }

        max_x = static_cast<coord_t>(x);
        if (elem_found)
            break;
    }

    // calculate max_y
    coord_t max_y = this->y() - 1;
    for (auto y = static_cast<long>(this->y()) - 1; y >= 0; --y)
    {
        bool elem_found = false;
        for (coord_t x = 0u; x < this->x(); ++x)
        {
            if (!this->is_free_tile(tile{x, static_cast<coord_t>(y), GROUND}) ||
                    !this->is_free_tile(tile{x, static_cast<coord_t>(y), CROSSING}))
            {
                elem_found = true;
                break;
            }
        }

        max_y = static_cast<coord_t>(y);
        if (elem_found)
            break;
    }

    return bounding_box{min_x, min_y, max_x, max_y};
}

energy::info fcn_gate_layout::calculate_energy() const noexcept
{
    double slow_energy = 0.0, fast_energy = 0.0;

    auto num_wires = wire_count();
    auto num_crossings = crossing_count();

    // adding wire energy (subtract 2 wires for each crossing)
    slow_energy += static_cast<double>(num_wires - num_crossings * 2) * energy::WIRE_SLOW;
    fast_energy += static_cast<double>(num_wires - num_crossings * 2) * energy::WIRE_FAST;

    // adding crossing energy
    slow_energy += num_crossings * energy::CROSSING_SLOW;
    fast_energy += num_crossings * energy::CROSSING_FAST;

    // counting gates
    auto num_inv_s = 0u, num_inv_b = 0u, num_and = 0u, num_or = 0u, num_maj = 0u, num_fan_out = 0u;

    for (auto&& [t, v] : v_map.left)
    {
        (void)v;  // fix compiler warning
        switch (get_op(t))
        {
            case operation::NOT:
            {
                // inputs are opposite to outputs --> straight inverter
                if (get_tile_inp_dirs(t) == layout::opposite(get_tile_out_dirs(t)))
                    ++num_inv_s;
                else  // else bent inverter
                    ++num_inv_b;
                break;
            }
            case operation::AND:
                ++num_and;
                break;
            case operation::OR:
                ++num_or;
                break;
            case operation::MAJ:
                ++num_maj;
                break;
            case operation::F1O2:
            case operation::F1O3:  // TODO do 1-3-Fan-outs have the same energy dissipation?
                ++num_fan_out;
                break;
            default:
                break;
        }
    }

    // adding inverter energy
    slow_energy += num_inv_s * energy::INVERTER_STRAIGHT_SLOW + num_inv_b * energy::INVERTER_BENT_SLOW;
    fast_energy += num_inv_s * energy::INVERTER_STRAIGHT_FAST + num_inv_b * energy::INVERTER_BENT_FAST;

    // adding conjunction energy
    slow_energy += num_and * energy::AND_SLOW;
    fast_energy += num_and * energy::AND_FAST;

    // adding disjunction energy
    slow_energy += num_or * energy::OR_SLOW;
    fast_energy += num_or * energy::OR_FAST;

    // adding majority energy
    slow_energy += num_maj * energy::MAJORITY_SLOW;
    fast_energy += num_maj * energy::MAJORITY_FAST;

    // adding fan-out energy
    slow_energy += num_fan_out * energy::FANOUT_SLOW;
    fast_energy += num_fan_out * energy::FANOUT_FAST;

    return {slow_energy, fast_energy};
}

void fcn_gate_layout::write_layout(std::ostream& os, const bool io_color, const bool clk_color) const noexcept
{
    // empty layout
    if (!area())
    {
        os << "[i] empty layout" << std::endl;
        return;
    }
    // shifted layout
    if (shift != offset::NONE)
    {
        os << "[w] cannot print shifted layouts (yet)" << std::endl;
        return;
    }

    const auto num_cols = x();
    const auto num_rows = y();

    // cache operations and directions in a 2d-matrix-like object
    using s_matrix = std::vector<std::vector<std::string>>;
    s_matrix ops(num_rows, std::vector<std::string>(num_cols));
    s_matrix x_dirs(num_rows, std::vector<std::string>(num_cols + 1u, " "));
    s_matrix y_dirs(num_rows + 1u, std::vector<std::string>(num_cols, " "));

    for (auto i : iter::range(num_rows))
    {
        for (auto j : iter::range(num_cols))
        {
            auto t1 = tile{j, i, GROUND};
            auto t2 = above(t1);
            auto o  = get_op(t1);

            // a wire above indicates a crossing
            if (o == operation::W && get_op(t2) == operation::W)
                ops[i][j] = "+";
            else
                ops[i][j] = str(o);

            // determine outgoing directions
            if ((is_tile_out_dir(t1, layout::DIR_E) && is_tile_out_dir({j + 1, i, GROUND}, layout::DIR_W)) ||
                (is_tile_out_dir(t2, layout::DIR_E) && is_tile_out_dir({j + 1, i, GROUND + 1}, layout::DIR_W)))
                x_dirs[i][j] = "↔";
            else if (is_tile_out_dir(t1, layout::DIR_E) || is_tile_out_dir(t2, layout::DIR_E))
                x_dirs[i][j] = "→";
            if ((is_tile_out_dir(t1, layout::DIR_W) || is_tile_out_dir(t2, layout::DIR_W)) && j > 0u) // safety check to prevent SEGFAULT
                x_dirs[i][j - 1u] = "←";

            if ((is_tile_out_dir(t1, layout::DIR_N) && is_tile_out_dir({j, i - 1, GROUND}, layout::DIR_S)) ||
                (is_tile_out_dir(t2, layout::DIR_N) && is_tile_out_dir({j, i - 1, GROUND + 1}, layout::DIR_S)))
                y_dirs[i][j] = "↕";
            else if (is_tile_out_dir(t1, layout::DIR_N) || is_tile_out_dir(t2, layout::DIR_N))
                y_dirs[i][j] = "↑";
            if (is_tile_out_dir(t1, layout::DIR_S) || is_tile_out_dir(t2, layout::DIR_S))
                y_dirs[i + 1u][j] = "↓";
        }
    }

    // Escape color sequence for input colors (green).
    const char* INP_COLOR = "\033[38;5;28m";
    // Escape color sequence for output colors (red).
    const char* OUT_COLOR = "\033[38;5;166m";
    // Escape color sequence for latch colors (yellow on black).
    const char* LATCH_COLOR = "\033[48;5;232;38;5;226m";
    // Escape color sequence for resetting colors.
    const char* COLOR_RESET = "\033[0m";
    // Escape color sequences for clock background colors (white to dark grey).
    std::vector<const char*> CLOCK_COLORS{{"\033[48;5;255;38;5;232m",  // white tile, black text
                                           "\033[48;5;248;38;5;232m",  // light grey tile, black text
                                           "\033[48;5;240;38;5;255m",  // grey tile, white text
                                           "\033[48;5;236;38;5;255m"   // dark grey tile, white text
                                          }};

    // actual printing
    auto r_ctr = 0u, c_ctr = 0u;
    for (auto& row : ops)
    {
        for (auto& d : y_dirs[r_ctr])
            os << d << " ";
        os << '\n';

        for (auto& o : row)
        {
            auto t = tile{c_ctr, r_ctr, GROUND};

            if (clk_color && tile_clocking(t))
                os << CLOCK_COLORS[*tile_clocking(t)];
            if (io_color && (get_latch(t) > 0u))
                os << LATCH_COLOR;
            if (io_color && is_pi(t))
                os << INP_COLOR;
            else if (io_color && is_po(t))
                os << OUT_COLOR;

            os << o << COLOR_RESET;

            os << x_dirs[r_ctr][c_ctr];
            ++c_ctr;
        }
        c_ctr = 0u;
        os << '\n';

        ++r_ctr;
    }
    // flush stream
    os << std::endl;
}

void fcn_gate_layout::clear_layout() noexcept
{
    fcn_layout::clear_layout();
    v_map.clear();
    e_map.clear();
    inp_dir_map.clear();
    out_dir_map.clear();
    edge_inp_dir_map.clear();
    edge_out_dir_map.clear();
}
