//
// Created by marcel on 06.01.20.
//

#include "exact.h"


exact::exact(logic_network_ptr ln, exact_pd_config&& config)
        :
        physical_design(std::move(ln)),
        config{config}
{
    // OPEN clocking and RES support fan-out of outdegree 3
    auto fan_out_degree = config.scheme->name == "OPEN3" ||
                          config.scheme->name == "OPEN4" ||
                          config.scheme->name == "RES" ? 3u : 2u;

    network->substitute_fan_outs(fan_out_degree);

    lower_bound = std::max(static_cast<unsigned long>(network->vertex_count(config.io_ports)), 4ul);  // incorporating BGL bug
    dit = dimension_iterator{config.fixed_size ? config.fixed_size : lower_bound};
}

physical_design::pd_result exact::operator()()
{
    if (config.num_threads > 1)
    {
       return run_asynchronously();
    }
    else
    {
        return run_synchronously();
    }
}

exact::smt_handler::smt_handler(ctx_ptr ctx, fcn_gate_layout_ptr fgl, const exact_pd_config& c) noexcept
        :
        ctx{std::move(ctx)},
        layout{std::move(fgl)},
        network{layout->get_network()},
        hierarchy{std::make_shared<network_hierarchy>(network,false)},
        config{c}
{
    hierarchy->unify_output_ranks();
    hierarchy->unify_inv_input_ranks();
}

bool exact::smt_handler::skippable(const fcn_dimension_xy& dim) const noexcept
{
    // OPEN clocking optimization: rotated dimensions don't need to be explored
    if (!layout->is_regularly_clocked())
    {
        if (dim[X] != dim[Y] && dim[X] == layout->y() && dim[Y] == layout->x())
            return true;
    }
    // ToPoliNano optimization: skip all dimensions where X <= levels + number of fan-outs - 1 and
    // Y < maximum of number of PIs and number of POs; this should not be too restrictive
    else if (config.topolinano)
    {
        if (dim[X] <= hierarchy->height() + network->operation_count(operation::F1O2) - 1 ||
            dim[Y] < std::max(network->num_pis(), network->num_pos()))
            return true;
    }

    return false;
}

void exact::smt_handler::update(const fcn_dimension_xy& dim) noexcept
{
    layout->resize(dim);
    check_point = std::make_shared<solver_check_point>(fetch_solver(dim));
    ++lc;
    solver = check_point->state->solver;
}

void exact::smt_handler::set_timeout(const unsigned t)
{
    z3::params p{*ctx};
    p.set("timeout", t);
    solver->set(p);
}

bool exact::smt_handler::is_satisfiable()
{
    generate_smt_instance();

    switch (solver->check(check_point->assumptions))
    {
        case z3::sat:
        {
            // TODO out-of-solver constraints and maybe going back into solver

            // optimize the generated result
            if (auto opt = optimize(); opt != nullptr)
            {
                opt->check();
                assign_layout(opt->get_model());
            }
            else
            {
                assign_layout(solver->get_model());
            }

            return true;
        }
        default:
        {
            return false;
        }
    }
}

void exact::smt_handler::store_solver_state(const fcn_dimension_xy& dim) noexcept
{
    solver_tree[dim] = check_point->state;
}

z3::expr exact::smt_handler::get_lit_e() noexcept
{
    return ctx->bool_const(fmt::format("lit_e_{}", lc).c_str());
}

z3::expr exact::smt_handler::get_lit_s() noexcept
{
    return ctx->bool_const(fmt::format("lit_s_{}", lc).c_str());
}

exact::smt_handler::solver_check_point exact::smt_handler::fetch_solver(const fcn_dimension_xy& dim) noexcept
{
    const auto create_assumptions = [this](const solver_state& state) -> z3::expr_vector
    {
        z3::expr_vector assumptions{*ctx};
        assumptions.push_back(state.lit.s);
        assumptions.push_back(state.lit.e);

        return assumptions;
    };

    // does a solver state for a layout of dimension of size x - 1 * y exist?
    if (auto it_x = solver_tree.find({dim[X] - 1, dim[Y]}); it_x != solver_tree.end())
    {
        // gather additional y-tiles and updated tiles
        std::set<layout_tile> added_tiles{}, updated_tiles{};
        for (auto&& y : iter::range(dim[Y]))
        {
            added_tiles.emplace(layout_tile{dim[X] - 1, y, GROUND});
            updated_tiles.emplace(layout_tile{dim[X] - 2, y, GROUND});
        }

        // deep-copy solver state
        const auto state = it_x->second;
        solver_state new_state = {state->solver, {get_lit_e(), state->lit.s}};

        // reset eastern constraints
        new_state.solver->add(not state->lit.e);

        // remove solver
        solver_tree.erase(it_x);

        return {std::make_shared<solver_state>(new_state), added_tiles, updated_tiles, create_assumptions(new_state)};
    }
    else
    {
        // does a solver state for a layout of dimension of size x * y - 1 exist?
        if (auto it_y = solver_tree.find({dim[X], dim[Y] - 1}); it_y != solver_tree.end())
        {
            // gather additional x-tiles
            std::set<layout_tile> added_tiles{}, updated_tiles{};
            for (auto&& x : iter::range(dim[X]))
            {
                added_tiles.emplace(layout_tile{x, dim[Y] - 1, GROUND});
                updated_tiles.emplace(layout_tile{x, dim[Y] - 2, GROUND});
            }

            // deep-copy solver state
            const auto state = it_y->second;
            solver_state new_state = {state->solver, {state->lit.e, get_lit_s()}};

            // reset southern constraints
            new_state.solver->add(not state->lit.s);

            // remove solver
            solver_tree.erase(it_y);

            return {std::make_shared<solver_state>(new_state), added_tiles, updated_tiles,
                    create_assumptions(new_state)};
        }
        else  // no existing solver state; create a new one
        {
            // all tiles are additional ones
            std::set<layout_tile> added_tiles{};
            for (auto&& y : iter::range(dim[Y]))
            {
                for (auto&& x : iter::range(dim[X]))
                    added_tiles.emplace(layout_tile{x, y, GROUND});
            }

            // create new state
            solver_state new_state{std::make_shared<z3::solver>(*ctx), {get_lit_e(), get_lit_s()}};

            return {std::make_shared<solver_state>(new_state), added_tiles, {}, create_assumptions(new_state)};
        }
    }
}

bool exact::smt_handler::is_added_tile(const layout_tile& t) const noexcept
{
    return check_point->added_tiles.count(t);
}

bool exact::smt_handler::is_updated_tile(const layout_tile& t) const noexcept
{
    return check_point->updated_tiles.count(t);
}

exact::smt_handler::assumption_literals& exact::smt_handler::lit() const noexcept
{
    return check_point->state->lit;
}

z3::expr exact::smt_handler::get_tv(const layout_tile& t, const logic_vertex v) noexcept
{
    return ctx->bool_const(fmt::format("tv_({},{})_{}", t[X], t[Y], v).c_str());
}

z3::expr exact::smt_handler::get_te(const layout_tile& t, const logic_edge& e) noexcept
{
    return ctx->bool_const(fmt::format("te_({},{})_({},{})",
            t[X], t[Y],network->source(e), network->target(e)).c_str());
}

z3::expr exact::smt_handler::get_tc(const layout_tile& t1, const layout_tile& t2) noexcept
{
    return ctx->bool_const(fmt::format("tc_({},{})_({},{})", t1[X], t1[Y], t2[X], t2[Y]).c_str());
}

z3::expr exact::smt_handler::get_tp(const layout_tile& t1, const layout_tile& t2) noexcept
{
    return ctx->bool_const(fmt::format("tp_({},{})_({},{})", t1[X], t1[Y], t2[X], t2[Y]).c_str());
}

z3::expr exact::smt_handler::get_vcl(const logic_vertex v) noexcept
{
    return ctx->int_const(fmt::format("vcl_{}", v).c_str());
}

z3::expr exact::smt_handler::get_tcl(const layout_tile& t) noexcept
{
    return ctx->int_const(fmt::format("tcl_({},{})", t[X], t[Y]).c_str());
}

z3::expr exact::smt_handler::get_tl(const layout_tile& t) noexcept
{
    return ctx->int_const(fmt::format("tl_({},{})", t[X], t[Y]).c_str());
}

z3::expr exact::smt_handler::mk_eq(const z3::expr_vector& v) const noexcept
{
    z3::expr_vector eq{*ctx};
    for (int&& i : iter::range(1u, v.size()))
        eq.push_back(v[i - 1] == v[i]);

    return z3::mk_and(eq);
}

z3::expr exact::smt_handler::mk_as(const z3::expr& constraint, const z3::expr& lit) const noexcept
{
    return z3::implies(lit, constraint);
}

z3::expr exact::smt_handler::mk_as_if_se(const z3::expr& constraint, const layout_tile& t) const noexcept
{
    if (auto east = layout->is_eastern_border(t), south = layout->is_southern_border(t); east && south)
    {
        return mk_as(constraint, lit().e and lit().s);
    }
    else
    {
        if (east)
        {
            return mk_as(constraint, lit().e);
        }
        else if (south)
        {
            return mk_as(constraint, lit().s);
        }
        else
        {
            return constraint;
        }
    }
}

void exact::smt_handler::tile_ite_counters(const logic_edge& e, z3::expr_vector& ve) noexcept
{
    z3::expr one  = ctx->real_val(1u);
    z3::expr zero = ctx->real_val(0u);

    z3::expr num_phases = ctx->real_val(static_cast<unsigned>(layout->num_clocks()));

    for (auto&& t : layout->ground_layer())
    {
        // an artificial latch variable counts as an extra 1 clock cycle (n clock phases)
        if (config.clock_latches)
            ve.push_back(z3::ite(get_te(t, e), get_tl(t) * num_phases + one, zero));
        else
            ve.push_back(z3::ite(get_te(t, e), one, zero));
    }
}

void exact::smt_handler::restrict_tile_elements() noexcept
{
    for (const auto& t : check_point->added_tiles)
    {
        if (config.crossings)
        {
            z3::expr_vector tv{*ctx};
            for (auto&& v : network->vertices(config.io_ports))
                tv.push_back(get_tv(t, v));

            if (!tv.empty())
                solver->add(z3::atmost(tv, 1u));

            z3::expr_vector te{*ctx};
            for (auto&& e : network->edges(config.io_ports))
                te.push_back(get_te(t, e));

            if (!te.empty())
                solver->add(z3::atmost(te, 2u));
        }
        else
        {
            z3::expr_vector ve{*ctx};
            for (auto&& v : network->vertices(config.io_ports))
                ve.push_back(get_tv(t, v));

            for (auto&& e : network->edges(config.io_ports))
                ve.push_back(get_te(t, e));

            if (!ve.empty())
                solver->add(z3::atmost(ve, 1u));
        }
    }
}

void exact::smt_handler::restrict_vertices() noexcept
{
    for (auto&& v : network->vertices(config.io_ports))
    {
        z3::expr_vector ve{*ctx};
        for (auto&& t : layout->ground_layer())
            ve.push_back(get_tv(t, v));

        // use a tracking literal to disable constraints in case of UNSAT
        solver->add(mk_as(z3::atleast(ve, 1u), lit().e and lit().s));
        solver->add(z3::atmost(ve, 1u));
    }
}

void exact::smt_handler::restrict_clocks() noexcept
{
    for (const auto& t : check_point->added_tiles)
    {
        auto cl = get_tcl(t);
        solver->add(ctx->int_val(0) <= cl);
        solver->add(cl < ctx->int_val(static_cast<unsigned>(layout->num_clocks())));
    }
}

void exact::smt_handler::define_adjacent_vertex_tiles() noexcept
{
    for (auto&& t : iter::chain(check_point->added_tiles, check_point->updated_tiles))
    {
        for (auto&& v : network->vertices(config.io_ports))
        {
            auto tv = get_tv(t, v);
            z3::expr_vector conj{*ctx};
            for (auto&& ae : network->out_edges(v, config.io_ports))
            {
                z3::expr_vector disj{*ctx};

                if (auto tgt = network->target(ae); layout->is_regularly_clocked())
                {
                    for (auto&& at : layout->outgoing_clocked_tiles(t))
                        disj.push_back((get_tv(at, tgt) or get_te(at, ae)) and get_tc(t, at));
                }
                else  // irregular clocking
                {
                    for (auto&& at : layout->surrounding_2d(t))
                    {
                        // clocks must differ by 1
                        auto mod = z3::mod(get_tcl(at) - get_tcl(t), layout->num_clocks()) == ctx->int_val(1);
                        disj.push_back(((get_tv(at, tgt) or get_te(at, ae)) and mod) and get_tc(t, at));
                    }
                }

                if (!disj.empty())
                    conj.push_back(z3::mk_or(disj));
            }
            if (!conj.empty())
            {
                solver->add(mk_as_if_se(z3::implies(tv, z3::mk_and(conj)), t));
            }
        }
    }
}

void exact::smt_handler::define_inv_adjacent_vertex_tiles() noexcept
{
    for (auto&& t : iter::chain(check_point->added_tiles, check_point->updated_tiles))
    {
        for (auto&& v : network->vertices(config.io_ports))
        {
            auto tv = get_tv(t, v);
            z3::expr_vector conj{*ctx};
            for (auto&& iae : network->in_edges(v, config.io_ports))
            {
                z3::expr_vector disj{*ctx};

                if (auto src = network->source(iae); layout->is_regularly_clocked())
                {
                    for (auto&& iat : layout->incoming_clocked_tiles(t))
                        disj.push_back((get_tv(iat, src) or get_te(iat, iae)) and get_tc(iat, t));
                }
                else  // irregular clocking
                {
                    for (auto&& iat : layout->surrounding_2d(t))
                    {
                        // clocks must differ by 1
                        auto mod = z3::mod(get_tcl(t) - get_tcl(iat), layout->num_clocks()) == ctx->int_val(1);
                        disj.push_back(((get_tv(iat, src) or get_te(iat, iae)) and mod) and get_tc(iat, t));
                    }
                }

                if (!disj.empty())
                    conj.push_back(z3::mk_or(disj));
            }
            if (!conj.empty())
            {
                solver->add(mk_as_if_se(z3::implies(tv, z3::mk_and(conj)), t));
            }
        }
    }
}

void exact::smt_handler::define_adjacent_edge_tiles() noexcept
{
    for (auto&& t : iter::chain(check_point->added_tiles, check_point->updated_tiles))
    {
        for (auto&& e : network->edges(config.io_ports))
        {
            auto te = network->target(e);
            z3::expr_vector disj{*ctx};

            if (layout->is_regularly_clocked())
            {
                for (auto&& at : layout->outgoing_clocked_tiles(t))
                    disj.push_back((get_tv(at, te) or get_te(at, e)) and get_tc(t, at));
            }
            else  // irregular clocking
            {
                for (auto&& at : layout->surrounding_2d(t))
                {
                    // clocks must differ by 1
                    auto mod = z3::mod(get_tcl(at) - get_tcl(t), layout->num_clocks()) == ctx->int_val(1);
                    disj.push_back(((get_tv(at, te) or get_te(at, e)) and mod) and get_tc(t, at));
                }
            }

            if (!disj.empty())
            {
                solver->add(mk_as_if_se(z3::implies(get_te(t, e), z3::mk_or(disj)), t));
            }
        }
    }
}

void exact::smt_handler::define_inv_adjacent_edge_tiles() noexcept
{
    for (auto&& t : iter::chain(check_point->added_tiles, check_point->updated_tiles))
    {
        for (auto&& e : network->edges(config.io_ports))
        {
            auto se = network->source(e);
            z3::expr_vector disj{*ctx};

            if (layout->is_regularly_clocked())
            {
                for (auto&& iat : layout->incoming_clocked_tiles(t))
                    disj.push_back((get_tv(iat, se) or get_te(iat, e)) and get_tc(iat, t));
            }
            else  // irregular clocking
            {
                for (auto&& iat : layout->surrounding_2d(t))
                {
                    // clocks must differ by 1
                    auto mod = z3::mod(get_tcl(t) - get_tcl(iat), layout->num_clocks()) == ctx->int_val(1);
                    disj.push_back(((get_tv(iat, se) or get_te(iat, e)) and mod) and get_tc(iat, t));
                }
            }

            if (!disj.empty())
            {
                solver->add(mk_as_if_se(z3::implies(get_te(t, e), z3::mk_or(disj)), t));
            }
        }
    }
}

void exact::smt_handler::establish_sub_paths() noexcept
{
    for (auto&& t : layout->ground_layer())
    {
        if (layout->is_regularly_clocked())
        {
            for (auto&& at : layout->outgoing_clocked_tiles(t))
            {
                // if neither t nor at are in added_tiles, the constraint exists already
                if (is_added_tile(t) || is_added_tile(at))
                    solver->add(z3::implies(get_tc(t, at), get_tp(t, at)));
            }
        }
        else  // irregular clocking
        {
            for (auto&& at : layout->surrounding_2d(t))
            {
                // if neither t nor at are in added_tiles, the constraint exists already
                if (is_added_tile(t) || is_added_tile(at))
                    solver->add(z3::implies(get_tc(t, at), get_tp(t, at)));
            }
        }
    }
}

void exact::smt_handler::establish_transitive_paths() noexcept
{
    for (auto&& t1 : layout->ground_layer())
    {
        for (auto&& t2 : layout->ground_layer())
        {
            if (layout->index(t1) == layout->index(t2))
                continue;

            for (auto&& t3 : layout->ground_layer())
            {
                if (layout->index(t2) == layout->index(t3))
                    continue;

                // if neither t1 nor t2 nor t3 are in added_tiles, the constraint exists already
                if (is_added_tile(t1) || is_added_tile(t2) || is_added_tile(t3))
                    solver->add(z3::implies(get_tp(t1, t2) and get_tp(t2, t3), get_tp(t1, t3)));
            }
        }
    }
}

void exact::smt_handler::eliminate_cycles() noexcept
{
    for (const auto& t : check_point->added_tiles)
        solver->add(not get_tp(t, t));
}

void exact::smt_handler::assign_pi_clockings() noexcept
{
    auto assign = [&](const logic_vertex _v) -> void
    {
        auto cl = get_vcl(_v);
        for (const auto& t : check_point->added_tiles)
        {
            if (layout->is_regularly_clocked())
                solver->add(z3::implies(get_tv(t, _v), cl == ctx->int_val(static_cast<unsigned>(*layout->tile_clocking(t)))));
            else  // irregular clocking
                solver->add(z3::implies(get_tv(t, _v), cl == get_tcl(t)));
        }
    };

    if (!(config.twoddwave && config.border_io))
    {
        if (config.io_ports)
        {
            for (auto&& v : network->get_pis())
                assign(v);
        }
        else
        {
            for (auto&& pi : network->get_pis())
            {
                for (auto&& v : network->adjacent_vertices(pi))
                    assign(v);
            }
        }
    }
}

// TODO only add to recheck instance after a solution has already been found or perform out-of-solver checks
void exact::smt_handler::global_synchronization() noexcept
{
    // restrict PIs to the first c x c tiles of the layout
    const auto restrict_entry_tiles = [this](const logic_vertex _v) -> void
    {
        for (const auto& t : check_point->added_tiles)
        {
            if (t[X] > layout->num_clocks() - 1u || t[Y] > layout->num_clocks() - 1u)
                solver->add(not get_tv(t, _v));
        }
    };

    using logic_edge_path = logic_network::edge_path;
    const auto define_length = [this](const logic_vertex _v) -> void
    {
        auto paths = network->get_all_paths(_v, config.io_ports);
        if (paths.empty())
            return;

        auto max_length = std::max_element(paths.cbegin(), paths.cend(),
                                           [](const logic_edge_path& p1, const logic_edge_path& p2)
                                           {return p1.size() < p2.size();})->size();

        z3::expr_vector all_path_lengths{*ctx};
        for (auto& p : paths)
        {
            z3::expr_vector path_length{*ctx};

            // respect number of vertices as an offset to path length
            // this works because every vertex must be placed
            if (int offset = static_cast<int>(p.size() - max_length); offset)
                path_length.push_back(ctx->real_val(offset));

            for (auto& e : p)
            {
                // respect clock zone of PI if one is involved
                if (auto s = network->source(e); config.io_ports && network->is_pi(s))
                    path_length.push_back(get_vcl(s));
                else if (!config.io_ports)
                {
                    if (network->pre_pi(s))
                        path_length.push_back(get_vcl(s));
                }

                tile_ite_counters(e, path_length);
            }
            all_path_lengths.push_back(z3::sum(path_length));
        }

        // use a tracking literal to disable constraints in case of UNSAT
        solver->add(mk_as(mk_eq(all_path_lengths), lit().e and lit().s));
    };

    // much simpler but equisatisfiable version of the constraint for 2DDWave clocking with border I/Os
    if (config.twoddwave && config.border_io)
    {
        if (config.io_ports)
        {
            for (auto&& pi : network->get_pis())
                restrict_entry_tiles(pi);
        }
        else
        {
            for (auto&& pi : network->get_pis())
            {
                for (auto&& v : network->adjacent_vertices(pi))
                    restrict_entry_tiles(v);
            }
        }
    }
    // normal version for all other configurations
    else
    {
        if (config.io_ports)
        {
            for (auto&& po : network->get_pos())
                define_length(po);
        }
        else
        {
            for (auto&& po : network->get_pos())
            {
                for (auto&& v : network->inv_adjacent_vertices(po))
                    define_length(v);
            }
        }
    }
}

void exact::smt_handler::prevent_insufficiencies() noexcept
{
    for (auto&& t : layout->tiles())
    {
        if (!layout->is_eastern_border(t) && !layout->is_southern_border(t) && !is_updated_tile(t))
            continue;

        if (layout->is_regularly_clocked())
        {
            for (auto&& v : network->vertices(config.io_ports))
            {
                // if vertex v has more adjacent or inversely adjacent elements than tile t
                if (layout->out_degree(t) < network->out_degree(v, config.io_ports) ||
                        layout->in_degree(t) < network->in_degree(v, config.io_ports))
                {
                    // if t is at eastern/southern border, its adjacencies might change
                    if (layout->is_eastern_border(t) || layout->is_southern_border(t))
                    {
                        // add restriction as assumption only
                        check_point->assumptions.push_back(not get_tv(t, v));
                    }
                    else if (is_updated_tile(t))  // nothing's about to change here
                    {
                        // add hard constraint
                        solver->add(not get_tv(t, v));
                    }
                }
            }

            for (auto&& e : network->edges(config.io_ports))
            {
                // if tile t has no adjacent or inversely adjacent tiles
                if (layout->out_degree(t) == 0 || layout->in_degree(t) == 0)
                {
                    // if t is at eastern/southern border, its adjacencies might change
                    if (layout->is_eastern_border(t) || layout->is_southern_border(t))
                    {
                        // add restriction as assumption only
                        check_point->assumptions.push_back(not get_te(t, e));
                    }
                    else if (is_updated_tile(t)) // nothing's about to change here
                    {
                        // add hard constraint
                        solver->add(not get_te(t, e));
                    }
                }
            }
        }
        else  // irregular clocking
        {
            auto surrounding_tiles = layout->surrounding_2d(t);
            auto tile_degree = static_cast<fcn_gate_layout::degree_t>(std::distance(surrounding_tiles.begin(),
                                                                      surrounding_tiles.end()));

            for (auto&& v : network->vertices(config.io_ports))
            {
                // in an irregular clocking scheme, not so strict restrictions can be made
                if (tile_degree < network->out_degree(v, config.io_ports) + network->in_degree(v, config.io_ports))
                {
                    // if t is at eastern/southern border, its adjacencies might change
                    if (layout->is_eastern_border(t) || layout->is_southern_border(t))
                    {
                        // add restriction as assumption only
                        check_point->assumptions.push_back(not get_tv(t, v));
                    }
                    else  // nothing's about to change here
                    {
                        // add hard constraint
                        solver->add(not get_tv(t, v));
                    }
                }
            }
            // cannot restrict wires because each tile has degree >= 2 in an irregular clocking
        }
    }
}

void exact::smt_handler::define_number_of_connections() noexcept
{
    for (auto&& t : iter::chain(check_point->added_tiles, check_point->updated_tiles))
    {
        z3::expr_vector tcc{*ctx};

        // collect (inverse) connection variables
        z3::expr_vector acc{*ctx};
        z3::expr_vector iacc{*ctx};
        if (layout->is_regularly_clocked())
        {
            for (auto&& at : layout->outgoing_clocked_tiles(t))
            {
                auto tc = get_tc(t, at);
                acc.push_back(tc);
                tcc.push_back(tc);
            }

            for (auto&& iat : layout->incoming_clocked_tiles(t))
            {
                auto itc = get_tc(iat, t);
                iacc.push_back(itc);
                tcc.push_back(itc);
            }
        }
        else  // irregular clocking
        {
            for (auto&& at : layout->surrounding_2d(t))
            {
                auto tc = get_tc(t, at);
                acc.push_back(tc);
                tcc.push_back(tc);

                auto itc = get_tc(at, t);
                iacc.push_back(itc);
                tcc.push_back(itc);
            }
        }

        z3::expr_vector ow{*ctx};

        for (auto&& v : network->vertices(config.io_ports))
        {
            auto tv   = get_tv(t, v);
            auto aon  = network->out_degree(v, config.io_ports);
            auto iaon = network->in_degree(v, config.io_ports);

            ow.push_back(tv);

            // if vertex v is assigned to a tile, the number of connections need to correspond
            if (!acc.empty())
            {
                solver->add(mk_as_if_se(z3::implies(tv, z3::atleast(acc, aon) and z3::atmost(acc, aon)), t));
            }
            if (!iacc.empty())
            {
                solver->add(mk_as_if_se(z3::implies(tv, z3::atleast(iacc, iaon) and z3::atmost(iacc, iaon)), t));
            }
        }

        z3::expr_vector wv{*ctx};
        for (auto&& e : network->edges(config.io_ports))
        {
            auto te = get_te(t, e);
            ow.push_back(te);
            wv.push_back(te);
        }

        // if there is any edge assigned to a tile, the number of connections need to correspond
        if (!wv.empty())
        {
            if (!acc.empty())
            {
                solver->add(mk_as_if_se(z3::implies(z3::atleast(wv, 1u) and z3::atmost(wv, 1u),
                                                    z3::atleast(acc, 1u) and z3::atmost(acc, 1u)), t));
            }
            if (!iacc.empty())
            {
                solver->add(mk_as_if_se(z3::implies(z3::atleast(wv, 1u) and z3::atmost(wv, 1u),
                                                    z3::atleast(iacc, 1u) and z3::atmost(iacc, 1u)), t));
            }

            // if crossings are allowed, there must be exactly four connections (one in each direction) for two assigned edges
            if (config.crossings)
            {
                if (!acc.empty())
                {
                    solver->add(mk_as_if_se(z3::implies(z3::atleast(wv, 2u) and z3::atmost(wv, 2u),
                                                        z3::atleast(acc, 2u) and z3::atmost(acc, 2u)), t));
                }
                if (!iacc.empty())
                {
                    solver->add(mk_as_if_se(z3::implies(z3::atleast(wv, 2u) and z3::atmost(wv, 2u),
                                                        z3::atleast(iacc, 2u) and z3::atmost(iacc, 2u)), t));
                }
            }
        }

        // if tile t is empty, there must not be any connection from or to tile t established
        if (ow.size() > 1 && !tcc.empty())  // test for > 1 to exclude single-vertex networks from this constraint
        {
            solver->add(mk_as_if_se(z3::atmost(ow, 0u) == z3::atmost(tcc, 0u), t));
        }
    }

    // lacking connections for irregular clocking
    if (!layout->is_regularly_clocked())
    {
        z3::expr_vector ccp{*ctx};

        // iterate over added_tiles only here to not duplicate constraints
        for (const auto& t : check_point->added_tiles)
        {
            z3::expr_vector ow{*ctx};
            for (auto&& v : network->vertices(config.io_ports))
            {
                ow.push_back(get_tv(t, v));
            }
            for (auto&& e : network->edges(config.io_ports))
            {
                ow.push_back(get_te(t, e));
            }

            // if tile t is empty, the clock number does not matter and can be fixed to 0
            if (!ow.empty())
                solver->add(z3::implies(z3::atmost(ow, 0u), get_tcl(t) == ctx->int_val(0)));
        }
    }
}

void exact::smt_handler::utilize_hierarchical_information() noexcept
{
    // symmetry breaking for ToPoliNano clocking
    if (config.topolinano)
    {
        // restrict vertex placement according to their hierarchy level
        if (config.io_ports && config.border_io)
        {
            for (auto&& v : network->vertices(config.io_ports))
            {
                auto l  = hierarchy->get_level(v);
                auto il = hierarchy->get_inv_level(v);
                auto oe = network->out_edges(v, config.io_ports);
                auto ie = network->in_edges(v, config.io_ports);

                // cannot be placed with too little distance to western border
                for (auto column : iter::range(l))
                {
                    for (auto row : iter::range(layout->y()))
                    {
                        if (auto t = layout_tile{column, row, GROUND}; is_added_tile(t))
                        {
                            solver->add(not get_tv(t, v));

                            // same for the outgoing edges
                            for (auto&& e : oe)
                                solver->add(not get_te(t, e));
                        }
                    }
                }

                // cannot be placed with too little distance to eastern border
                for (auto column : iter::range(layout->x() - il, layout->x()))
                {
                    for (auto row : iter::range(layout->y()))
                    {
                        auto t = layout_tile{column, row, GROUND};

                        // use assumptions here because the south-east corner moves away in the following iterations
                        check_point->assumptions.push_back(not get_tv(t, v));

                        // same for the incoming edges
                        for (auto&& e : ie)
                            check_point->assumptions.push_back(not get_te(t, e));
                    }
                }
            }
        }
    }
    // symmetry breaking for 2DDWave clocking
    else if (config.twoddwave)
    {
        // restrict vertex placement according to their hierarchy level
        if (config.io_ports && config.border_io)
        {
            for (auto&& v : network->vertices(config.io_ports))
            {
                auto l = hierarchy->get_level(v);
                auto il = hierarchy->get_inv_level(v);
                auto oe = network->out_edges(v, config.io_ports);
                auto ie = network->in_edges(v, config.io_ports);

                // cannot be placed with too little distance to north-west corner
                for (const auto& t : check_point->added_tiles)
                {
                    if (t[X] + t[Y] < l)
                    {
                        solver->add(not get_tv(t, v));

                        // same for the outgoing edges
                        for (auto&& e : oe)
                            solver->add(not get_te(t, e));
                    }
                    // cannot be placed with too little distance to south-east corner
                    if (layout->x() - 1 - t[X] + layout->y() - 1 - t[Y] < il)
                    {
                        // use assumptions here because the south-east corner moves away in the following iterations
                        check_point->assumptions.push_back(not get_tv(t, v));

                        // same for the incoming edges
                        for (auto&& e : ie)
                            check_point->assumptions.push_back(not get_te(t, e));
                    }

                }
            }
        }
    }
}

void exact::smt_handler::enforce_border_io() noexcept
{
    auto assign_border = [this](const logic_vertex _v)
    {
        for (auto&& t : iter::chain(check_point->added_tiles, check_point->updated_tiles))
        {
            if (!layout->is_border(t))
                solver->add(not get_tv(t, _v));
        }
    };

    auto assign_west = [this](const logic_vertex _v)
    {
        // no need to iterate over updated_tiles, because nothing changes there
        for (const auto& t : check_point->added_tiles)
        {
            if (!layout->is_western_border(t))
                solver->add(not get_tv(t, _v));
        }
    };

    auto assign_east = [this](const logic_vertex _v)
    {
        for (auto&& t : iter::chain(check_point->added_tiles, check_point->updated_tiles))
        {
            if (!layout->is_eastern_border(t))
                solver->add(not get_tv(t, _v));
        }
    };

    if (config.io_ports)
    {
        for (auto&& pi : network->get_pis())
            config.topolinano ? assign_west(pi) : assign_border(pi);
        for (auto&& po : network->get_pos())
            config.topolinano ? assign_east(po) : assign_border(po);
    }
    else
    {
        for (auto&& pi : network->get_pis())
        {
            for (auto&& v : network->adjacent_vertices(pi))
                config.topolinano ? assign_west(v) : assign_border(v);
        }
        for (auto&& po : network->get_pos())
        {
            for (auto&& v : network->inv_adjacent_vertices(po))
                config.topolinano ? assign_east(v) : assign_border(v);
        }
    }
}

void exact::smt_handler::enforce_straight_inverters() noexcept
{
    for (auto&& t : iter::chain(check_point->added_tiles, check_point->updated_tiles))
    {
        for (auto&& inv : network->vertices())
        {
            // skip all operations except for NOT
            if (network->get_op(inv) != operation::NOT)
                continue;

            // I/Os inverters are always straight, so they can be skipped as well
            if (!config.io_ports && (network->pre_pi(inv) || network->post_po(inv)))
                continue;

            auto n = layout->north(t), e = layout->east(t),
                 s = layout->south(t), w = layout->west(t);

            auto ne = layout->north_east(t), se = layout->south_east(t),
                 sw = layout->south_west(t), nw = layout->north_west(t);

            // vector to store possible direction combinations
            z3::expr_vector ve{*ctx};

            if (layout->is_regularly_clocked())
            {
                if (layout->is_incoming_clocked(t, n) && layout->is_outgoing_clocked(t, s))
                {
                    // enforce north to south connection if inverter gets placed here
                    ve.push_back(get_tc(n, t) and get_tc(t, s));
                }
                else if (layout->is_incoming_clocked(t, s) && layout->is_outgoing_clocked(t, n))
                {
                    // enforce south to north connection if inverter gets placed here
                    ve.push_back(get_tc(s, t) and get_tc(t, n));
                }

                // if layout is vertically shifted
                if (config.vertical_offset)
                {
                    // if t is in an even column
                    if (layout->is_even_column(t))
                    {
                        if (ne && nw)
                        {
                            if (layout->is_incoming_clocked(t, *nw) && layout->is_outgoing_clocked(t, *ne))
                            {
                                // enforce north-west to north-east connection if inverter gets placed here
                                ve.push_back(get_tc(*nw, t) and get_tc(t, *ne));
                            }
                            else if (layout->is_incoming_clocked(t, *ne) && layout->is_outgoing_clocked(t, *nw))
                            {
                                // enforce north-east to north-west connection if inverter gets placed here
                                ve.push_back(get_tc(*ne, t) and get_tc(t, *nw));
                            }
                        }
                    }
                    else  // odd column
                    {
                        if (se && sw)
                        {
                            if (layout->is_incoming_clocked(t, *sw) && layout->is_outgoing_clocked(t, *se))
                            {
                                // enforce south-west to south-east connection if inverter gets placed here
                                ve.push_back(get_tc(*sw, t) and get_tc(t, *se));
                            }
                            else if (layout->is_incoming_clocked(t, *se) && layout->is_outgoing_clocked(t, *sw))
                            {
                                // enforce south-east to south-west connection if inverter gets placed here
                                ve.push_back(get_tc(*se, t) and get_tc(t, *sw));
                            }
                        }
                    }
                }
                // west-east and east-west is possible regardless of shift
                if (layout->is_incoming_clocked(t, e) && layout->is_outgoing_clocked(t, w))
                {
                    // enforce east to west connection if inverter gets placed here
                    ve.push_back(get_tc(e, t) and get_tc(t, w));
                }
                else if (layout->is_incoming_clocked(t, w) && layout->is_outgoing_clocked(t, e))
                {
                    // enforce west to east connection if inverter gets placed here
                    ve.push_back(get_tc(w, t) and get_tc(t, e));
                }
            }
            else  // irregular clocking
            {
                // t is neither at northern nor at southern border
                if (t != n && t != s)
                {
                    ve.push_back(get_tc(n, t) and get_tc(t, s));
                    ve.push_back(get_tc(s, t) and get_tc(t, n));
                }
                // if layout is vertically shifted
                if (config.vertical_offset)
                {
                    // if t is in an even column
                    if (layout->is_even_column(t))
                    {
                        if (ne && nw)
                        {
                            ve.push_back(get_tc(*ne, t) and get_tc(t, *nw));
                            ve.push_back(get_tc(*nw, t) and get_tc(t, *ne));
                        }
                    }
                    else  // odd column
                    {
                        if (se && sw)
                        {
                            ve.push_back(get_tc(*se, t) and get_tc(t, *sw));
                            ve.push_back(get_tc(*sw, t) and get_tc(t, *se));
                        }
                    }
                }
                // t is neither at eastern nor at western border (possible regardless of shift)
                if (t != e && t != w)
                {
                    ve.push_back(get_tc(e, t) and get_tc(t, w));
                    ve.push_back(get_tc(w, t) and get_tc(t, e));
                }
            }

            if (!ve.empty())
            {
                // inverter can be placed here; enforce any of the direction combinations found possible above
                solver->add(mk_as_if_se(z3::implies(get_tv(t, inv), z3::mk_or(ve)), t));
            }
            else
            {
                // inverter cannot be placed here, add constraint to avoid this case and speed up solving
                solver->add(mk_as_if_se(not get_tv(t, inv), t));
            }
        }
    }
}

void exact::smt_handler::restrict_clock_latches() noexcept
{
    auto zero = ctx->int_val(0u);

    for (const auto& t : check_point->added_tiles)
    {
        // latches must be positive
        auto l = get_tl(t);
        solver->add(l >= zero);

        // tiles without wires cannot have latches
        z3::expr_vector te{*ctx};
        for (auto&& e : network->edges(config.io_ports))
            te.push_back(get_te(t, e));

        solver->add(z3::implies(z3::atmost(te, 0u), l == zero));
    }
}

void exact::smt_handler::topology_specific_constraints() noexcept
{
    if (config.topolinano && config.vertical_offset)
    {
        // fan-outs (couplers) cannot be followed by fan-outs and can only have inputs from their north-western tiles
        for (auto&& fo : network->vertices())
        {
            // only argue about 1-to-2-fan-outs
            if (network->get_op(fo) != operation::F1O2)
                continue;

            // prohibit succeeding fan-outs
            for (auto&& afo : network->adjacent_vertices(fo))
            {
                if (network->get_op(afo) != operation::F1O2)
                    continue;

                for (auto&& t : layout->ground_layer())
                {
                    for (auto&& at : layout->outgoing_clocked_tiles(t))
                    {
                        if (is_added_tile(at))
                            solver->add(z3::implies(get_tv(t, fo), not get_tv(at, afo)));
                    }
                }
            }

            // do not argue about fan-outs without predecessors in the following
            if (network->in_degree(fo, config.io_ports) == 0)
                continue;

            // enforce north-western input
            for (const auto& t : check_point->added_tiles)
            {
                // if fo gets placed here, its predecessor must be on the north-western tile
                if (auto nw = layout->north_west(t); nw)
                {
                    solver->add(z3::implies(get_tv(t, fo), get_tc(*nw, t)));

                    // additionally, no crossing can precede a fan-out
                    z3::expr_vector wv{*ctx};
                    for (auto&& e : network->edges(config.io_ports))
                        wv.push_back(get_te(*nw, e));

                    solver->add(z3::implies(get_tv(t, fo), z3::atmost(wv, 1u)));
                }
                // if tile t doesn't have a north-western adjacent tile, fo cannot be placed here
                else
                {
                    solver->add(not get_tv(t, fo));
                }
            }
        }

        for (auto&& v1 : network->vertices())
        {
            // AND/OR/MAJ gates cannot be followed directly by an AND/OR/MAJ gate or a fan-out (coupler)
            // additionally, if straight inverters are enforced, AND/OR/MAJ cannot be followed by NOT either
            if (auto op1 = network->get_op(v1); op1 != operation::AND && op1 != operation::OR && op1 != operation::MAJ)
                continue;

            for (auto&& v2 : network->adjacent_vertices(v1))
            {
                // only argue about AND/OR/MAJ/fan-outs and additionally about NOT if straight inverters are enforced
                if (auto op2 = network->get_op(v2); op2 != operation::AND && op2 != operation::OR &&
                                                    op2 != operation::MAJ && op2 != operation::F1O2 &&
                                                    !(config.straight_inverters && op2 == operation::NOT))
                {
                    continue;
                }

                for (auto&& t : layout->ground_layer())
                {
                    for (auto&& at : layout->outgoing_clocked_tiles(t))
                    {
                        if (is_added_tile(at))
                            solver->add(z3::implies(get_tv(t, v1), not get_tv(at, v2)));
                    }
                }
            }

            // AND/OR/MAJ gates cannot be followed by a south-eastern connection
            // and cannot be followed by a crossing directly
            for (auto&& t : layout->ground_layer())
            {
                if (auto ne = layout->north_east(t); !ne)
                {
                    // no north-eastern tile, do not place v1 here
                    check_point->assumptions.push_back(not get_tv(t, v1));
                }
                else if (ne)
                {
                    // north-eastern tile exists, do not create a crossing here
                    z3::expr_vector wv{*ctx};
                    for (auto&& e : network->edges(config.io_ports))
                        wv.push_back(get_te(*ne, e));

                    solver->add(z3::implies(get_tv(t, v1), z3::atmost(wv, 1u)));
                }
                if (auto se = layout->south_east(t); se)
                {
                    // south-eastern tile exists, do not route a connection here
                    if (is_added_tile(*se))
                        solver->add(z3::implies(get_tv(t, v1), not get_tc(t, *se)));
                }
            }
        }
    }
}

void exact::smt_handler::minimize_wires(optimize_ptr optimize) noexcept
{
    z3::expr_vector wire_counter{*ctx};
    for (auto&& t : layout->ground_layer())
    {
        for (auto&& e : network->edges(config.io_ports))
            wire_counter.push_back(z3::ite(get_te(t, e), ctx->real_val(1u), ctx->real_val(0u)));
    }

    optimize->minimize(z3::sum(wire_counter));
}

void exact::smt_handler::minimize_crossings(optimize_ptr optimize) noexcept
{
    z3::expr_vector crossings_counter{*ctx};
    for (auto&& t : layout->ground_layer())
    {
        z3::expr_vector wv{*ctx};
        for (auto&& e : network->edges(config.io_ports))
            wv.push_back(get_te(t, e));

        crossings_counter.push_back(z3::ite(z3::atleast(wv, 2u), ctx->real_val(1u), ctx->real_val(0u)));
    }

    optimize->minimize(z3::sum(crossings_counter));
}

void exact::smt_handler::minimize_clock_latches(optimize_ptr optimize) noexcept
{
    z3::expr_vector latch_counter{*ctx};
    for (auto&& t : layout->ground_layer())
    {
        latch_counter.push_back(get_tl(t));
    }

    optimize->minimize(z3::sum(latch_counter));
}

void exact::smt_handler::generate_smt_instance() noexcept
{
    // placement constraints
    restrict_tile_elements();
    restrict_vertices();

    // local synchronization constraints
    define_adjacent_vertex_tiles();
    define_inv_adjacent_vertex_tiles();
    define_adjacent_edge_tiles();
    define_inv_adjacent_edge_tiles();

    // global synchronization constraints
    if (!config.desynchronize && !config.topolinano)
    {
        assign_pi_clockings();
        global_synchronization();
    }

    // open clocking scheme constraints
    if (!layout->is_regularly_clocked())
        restrict_clocks();

    // path/cycle constraints
    if (!config.topolinano && !config.twoddwave)  // linear schemes; no cycles by definition
    {
        establish_sub_paths();
        establish_transitive_paths();
        eliminate_cycles();
    }

    // I/O pin constraints
    if (config.border_io)
        enforce_border_io();

    // straight inverter constraints
    if (config.straight_inverters)
        enforce_straight_inverters();

    // clock latch constraints
    if (config.clock_latches && !config.desynchronize)
        restrict_clock_latches();

    // topology-specific constraints
    topology_specific_constraints();

    // symmetry breaking constraints
    prevent_insufficiencies();
    define_number_of_connections();
    utilize_hierarchical_information();
}

exact::optimize_ptr exact::smt_handler::optimize() noexcept
{
    if (auto wires = config.minimize_wires, cross = config.minimize_crossings,
                latch = config.clock_latches && !config.desynchronize; !wires && !cross && !latch)
    {
        return nullptr;
    }
    else
    {
        auto optimize = std::make_shared<z3::optimize>(*ctx);

        // add all solver constraints
        for (const auto& e : solver->assertions())
            optimize->add(e);

        // add assumptions as constraints, too, because optimize::check with assumptions is broken
        for (const auto& e : check_point->assumptions)
            optimize->add(e);

        // wire minimization constraints
        if (wires)
            minimize_wires(optimize);

        // crossing minimization constraints
        if (cross)
            minimize_crossings(optimize);

        // clock latches minimization constraints
        if (latch)
            minimize_clock_latches(optimize);

        return optimize;
    }
}

void exact::smt_handler::assign_layout(const z3::model& model) noexcept
{
    // assign vertices to tiles
    for (auto&& t : layout->ground_layer())
    {
        auto element_placed = false;
        for (auto&& v : network->vertices(config.io_ports))
        {
            // if vertex v is set to tile t
            if (model.eval(get_tv(t, v)).bool_value() == Z3_L_TRUE)
            {
                // if this is a balance vertex
                if (auto b = hierarchy->get_balance_edge(v); b)
                {
                    layout->assign_logic_edge(t, *b);
                }
                else  // regular gate
                {
                    // check for I/Os
                    bool pi, po;
                    if (config.io_ports)
                    {
                        pi = network->is_pi(v);
                        po = network->is_po(v);
                    }
                    else
                    {
                        pi = network->pre_pi(v);
                        po = network->post_po(v);
                    }

                    layout->assign_logic_vertex(t, v, pi, po);
                }

                // there should not be two elements on the same tile
                element_placed = true;
                break;
            }
        }

        if (element_placed)
            continue;

        // assign edges to tiles
        for (auto&& e : network->edges(config.io_ports))
        {
            // if edge e is set to tile t
            if (model.eval(get_te(t, e)).bool_value() == Z3_L_TRUE)
            {
                // there is already an edge in ground level
                if (layout->is_wire_tile(t))
                    // assign to above tile
                    layout->assign_logic_edge(layout->above(t), e);
                    // no other edge yet
                else
                    // assign to ground level
                    layout->assign_logic_edge(t, e);
            }
        }
    }

    // assign clock zones to tiles of open schemes
    if (!layout->is_regularly_clocked())
    {
        for (auto&& t : layout->ground_layer())
            layout->assign_clocking(t, static_cast<fcn_clock::zone>(model.eval(get_tcl(t), true).get_numeral_int()));
    }  // from now on, a clocking scheme is assigned and no distinction between regular and irregular must be made


    // assign directions to tiles
    for (auto&& t1 : layout->ground_layer())
    {
        for (auto&& t2 : layout->outgoing_clocked_tiles(t1))
        {
            // if connection t1 -> t2 is established
            if (model.eval(get_tc(t1, t2)).bool_value() == Z3_L_TRUE)
            {
                layout_tile _t1, _t2;
                if (layout->is_data_flow(t1, t2))
                { _t1 = t1; _t2 = t2; }
                else if (layout->is_data_flow(t1, layout->above(t2)))
                { _t1 = t1; _t2 = layout->above(t2); }
                else if (layout->is_data_flow(layout->above(t1), t2))
                { _t1 = layout->above(t1); _t2 = t2; }
                else if (layout->is_data_flow(layout->above(t1), layout->above(t2)))
                { _t1 = layout->above(t1); _t2 = layout->above(t2); }

                // assign outgoing directions
                for (auto& e : layout->get_logic_edges(_t1))
                    layout->assign_wire_out_dir(_t1, e, layout->get_bearing(_t1, _t2));
                layout->assign_tile_out_dir(_t1, layout->get_bearing(_t1, _t2));

                // assign incoming directions
                for (auto& e : layout->get_logic_edges(_t2))
                    layout->assign_wire_inp_dir(_t2, e, layout->get_bearing(_t2, _t1));
                layout->assign_tile_inp_dir(_t2, layout->get_bearing(_t2, _t1));
            }
        }
    }

    // adjust wires for ToPoliNano clocking as multi wires are supported
    if (config.topolinano)
    {
        for (auto&& t : layout->crossing_layers())
        {
            for (auto& e : layout->get_logic_edges(t))
            {
                if (auto inp = layout->get_wire_inp_dirs(t, e), out = layout->get_wire_out_dirs(t, e);
                        inp != layout::opposite(out))
                {
                    // move edge to ground layer
                    auto g = layout_tile{t[X], t[Y], GROUND};
                    layout->dissociate_logic_edge(t, e);
                    layout->assign_logic_edge(g, e);
                    layout->assign_wire_inp_dir(g, e, inp);
                    layout->assign_wire_out_dir(g, e, out);
                }
            }
        }
    }

    // assign artificial latches if there were any in use
    if (config.clock_latches)
    {
        for (auto&& t : layout->ground_layer())
            layout->assign_latch(t, static_cast<fcn_gate_layout::latch_delay>
                                    (model.eval(get_tl(t), true).get_numeral_int()) * layout->num_clocks());
    }
}

void exact::update_timeout(smt_handler& handler, mockturtle::stopwatch<>::duration time) const
{
    auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(time).count();
    auto time_left = (config.timeout - time_elapsed > 0 ? static_cast<unsigned>(config.timeout - time_elapsed) : 0u);

    if (!time_left)
        throw z3::exception("timeout");

    handler.set_timeout(time_left);
}

fcn_gate_layout_ptr exact::explore_asynchronously(const unsigned t_num, std::shared_ptr<std::vector<thread_info>> ti_list) noexcept
{
    // timeout measurement
    mockturtle::stopwatch<>::duration time{0};

    auto ctx = std::make_shared<z3::context>();
    auto layout_sketch = std::make_shared<fcn_gate_layout>(*config.scheme, network, config.vertical_offset ?
                                                                                    fcn_layout::offset::VERTICAL :
                                                                                    fcn_layout::offset::NONE);
    smt_handler handler{ctx,layout_sketch, config};
    (*ti_list)[t_num].ctx = ctx;

    while (true)
    {
        fcn_dimension_xy dimension;

        // mutually exclusive access to the dimension iterator
        {
            std::lock_guard<std::mutex> guard(dit_mutex);

            ++dit;
            dimension = *dit;  // operations ++ and * are split to prevent a vector copy construction
        }

        if (area(dimension) > config.upper_bound)
            return nullptr;

        if (handler.skippable(dimension))
            continue;

        // mutually exclusive access to the result dimension
        {
            std::lock_guard<std::mutex> guard(rd_mutex);

            // a result is available already
            if (result_dimension)
            {
                // stop working if its area is smaller or equal to the one currently at hand
                if (area(*result_dimension) <= area(dimension))
                    return nullptr;
            }
        }

        // update dimension in the thread_info list and the handler
        (*ti_list)[t_num].worker_dimension = dimension;
        handler.update(dimension);

        try
        {
            mockturtle::stopwatch stop{time};

            if (handler.is_satisfiable())  // found a layout
            {
                // mutually exclusive access to the result_dimension
                {
                    std::lock_guard<std::mutex> guard(rd_mutex);

                    // update the result_dimension if there is none
                    if (!result_dimension)
                    {
                        result_dimension = dimension;
                    }
                    else  // or if the own one is smaller
                    {
                        if (area(*result_dimension) > area(dimension))
                            result_dimension = dimension;
                        else
                            return nullptr;
                    }
                }

                // interrupt other threads that are working on higher dimensions
                for (const auto& ti : *ti_list)
                {
                    if (area(dimension) <= area(ti.worker_dimension))
                        ti.ctx->interrupt();
                }

                return layout_sketch;
            }
            else  // no layout with this dimension possible
            {
                // One could assume that interrupting other threads that are working on real smaller (not bigger in
                // any dimension) layouts could be beneficial here. However, testing revealed that this code was hardly
                // ever triggered and if it was, it impacted performance negatively because no solver state could be
                // stored that could positively influence performance of later SMT calls

                handler.store_solver_state(dimension);
            }
        }
        catch (const z3::exception&)  // timed out or interrupted
        {
            return nullptr;
        }

        update_timeout(handler, time);
    }

    // unreachable code, but compiler complains if it's not there
    return nullptr;
}

exact::pd_result exact::run_asynchronously() noexcept
{
    mockturtle::stopwatch<>::duration time{0};

    {
        mockturtle::stopwatch stop{time};

        using fut_layout = std::future<fcn_gate_layout_ptr>;
        std::vector<fut_layout> fut(config.num_threads);
        auto ti_list = std::make_shared<std::vector<thread_info>>(config.num_threads);

#if (PROGRESS_BARS)
        mockturtle::progress_bar thread_bar("[i] examining layout dimensions using {} threads");
        thread_bar(config.num_threads);

        auto post_toggle = false;
        mockturtle::progress_bar post_bar("[i] some layout has been found; waiting for threads examining smaller dimensions to terminate");
#endif

        for (auto&& i : iter::range(config.num_threads))
        {
            fut[i] = std::async(std::launch::async, &exact::explore_asynchronously, this, i, ti_list);
        }

        // wait for all tasks to finish running (can be made much prettier in C++20...)
        for (auto still_running = true; still_running; )
        {
            still_running = false;
            for (auto&& i : iter::range(config.num_threads))
            {
                using namespace std::chrono_literals;
                if (fut[i].wait_for(10ms) == std::future_status::timeout)
                {
                    still_running = true;
                }
#if (PROGRESS_BARS)
                else if (!post_toggle)
                {
                    thread_bar.done();
                    post_bar(true);
                    post_toggle = true;
                }
#endif
            }
        }

        if (result_dimension)
        {
            auto result_dim_val = *result_dimension;
            // extract the layout from the futures
            for (auto& f : fut)
            {
                if (auto l = f.get(); l != nullptr)
                {
                    // in case multiple returned, get the actual winner
                    if (l->x() == result_dim_val[X] && l->y() == result_dim_val[Y])
                        layout = l;
                }
            }
        }
    }

    return pd_result{result_dimension.has_value(), nlohmann::json{{"runtime (s)", mockturtle::to_seconds(time)}}};
}

exact::pd_result exact::run_synchronously() noexcept
{
    mockturtle::stopwatch<>::duration time{0};

    auto layout_sketch = std::make_shared<fcn_gate_layout>(*config.scheme, network, config.vertical_offset ?
                                                                                    fcn_layout::offset::VERTICAL :
                                                                                    fcn_layout::offset::NONE);
    smt_handler handler{std::make_shared<z3::context>(), layout_sketch, config};

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
            else
            {
                handler.store_solver_state(dimension);
            }

            update_timeout(handler, time);
        }
        catch (const z3::exception&)
        {
            return pd_result{false, nlohmann::json{{"runtime (s)", mockturtle::to_seconds(time)}}};
        }
    }

    return pd_result{false, nlohmann::json{{"runtime (s)", mockturtle::to_seconds(time)}}};
}
