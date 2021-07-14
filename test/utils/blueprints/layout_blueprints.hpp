//
// Created by marcel on 24.05.21.
//

#ifndef FICTION_LAYOUT_BLUEPRINTS_HPP
#define FICTION_LAYOUT_BLUEPRINTS_HPP

#include "catch.hpp"

#include <fiction/layouts/clocking_scheme.hpp>

#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <mockturtle/traits.hpp>

namespace blueprints
{

template <typename GateLyt>
GateLyt xor_maj_gate_layout() noexcept
{
    REQUIRE(mockturtle::has_create_node_v<GateLyt>);

    GateLyt layout{typename GateLyt::aspect_ratio{3, 2, 0}, fiction::open_4_clocking};

    layout.assign_clock_number({2, 0}, static_cast<typename GateLyt::clock_number_t>(0));
    layout.assign_clock_number({1, 1}, static_cast<typename GateLyt::clock_number_t>(0));
    layout.assign_clock_number({3, 1}, static_cast<typename GateLyt::clock_number_t>(0));

    layout.assign_clock_number({1, 0}, static_cast<typename GateLyt::clock_number_t>(1));
    layout.assign_clock_number({2, 1}, static_cast<typename GateLyt::clock_number_t>(1));

    layout.assign_clock_number({0, 0}, static_cast<typename GateLyt::clock_number_t>(2));
    layout.assign_clock_number({2, 2}, static_cast<typename GateLyt::clock_number_t>(2));

    const auto a = layout.create_pi("a", {1, 1});
    const auto b = layout.create_pi("b", {2, 0});
    const auto c = layout.create_pi("c", {3, 1});

    kitty::dynamic_truth_table tt_maj(3u), tt_xor(2u);
    kitty::create_from_hex_string(tt_maj, "e8");
    kitty::create_from_hex_string(tt_xor, "6");

    const auto n_maj = layout.create_node({a, b, c}, tt_maj, {2, 1});
    const auto n_xor = layout.create_node({a, b}, tt_xor, {1, 0});

    layout.create_po(n_maj, "f1", {2, 2});
    layout.create_po(n_xor, "f2", {0, 0});

    return layout;
}

template <typename GateLyt>
GateLyt and_or_gate_layout() noexcept
{
    GateLyt layout{typename GateLyt::aspect_ratio{3, 1, 0}, fiction::open_4_clocking};

    layout.assign_clock_number({2, 0}, static_cast<typename GateLyt::clock_number_t>(0));
    layout.assign_clock_number({1, 0}, static_cast<typename GateLyt::clock_number_t>(1));
    layout.assign_clock_number({0, 0}, static_cast<typename GateLyt::clock_number_t>(2));

    layout.assign_clock_number({1, 1}, static_cast<typename GateLyt::clock_number_t>(0));
    layout.assign_clock_number({2, 1}, static_cast<typename GateLyt::clock_number_t>(1));
    layout.assign_clock_number({3, 1}, static_cast<typename GateLyt::clock_number_t>(2));

    const auto x1 = layout.create_pi("x1", {2, 0});
    const auto x2 = layout.create_pi("x2", {1, 1});
    const auto a  = layout.create_and(x1, x2, {1, 0});
    const auto o  = layout.create_or(x2, x1, {2, 1});
    layout.create_po(a, "f1", {0, 0});
    layout.create_po(o, "f2", {3, 1});

    return layout;
}

template <typename GateLyt>
GateLyt and_not_gate_layout() noexcept
{
    GateLyt layout{typename GateLyt::aspect_ratio{3, 1, 0}, fiction::open_4_clocking};

    layout.assign_clock_number({2, 0}, static_cast<typename GateLyt::clock_number_t>(0));
    layout.assign_clock_number({1, 0}, static_cast<typename GateLyt::clock_number_t>(1));
    layout.assign_clock_number({0, 0}, static_cast<typename GateLyt::clock_number_t>(2));

    layout.assign_clock_number({1, 1}, static_cast<typename GateLyt::clock_number_t>(0));
    layout.assign_clock_number({2, 1}, static_cast<typename GateLyt::clock_number_t>(1));
    layout.assign_clock_number({3, 1}, static_cast<typename GateLyt::clock_number_t>(2));

    const auto x1 = layout.create_pi("x1", {2, 0});
    const auto x2 = layout.create_pi("x2", {1, 1});
    const auto a1 = layout.create_and(x1, x2, {1, 0});
    const auto n1 = layout.create_not(x2, {2, 1});
    layout.create_po(a1, "f1", {0, 0});
    layout.create_po(n1, "f2", {3, 1});

    return layout;
}

template <typename GateLyt>
GateLyt or_not_gate_layout() noexcept
{
    GateLyt layout{typename GateLyt::aspect_ratio{2, 2, 0}, fiction::twoddwave_4_clocking};

    const auto x1 = layout.create_pi("x1", {1, 0});
    const auto x2 = layout.create_pi("x2", {0, 1});
    const auto o1 = layout.create_or(x1, x2, {1, 1});
    const auto n1 = layout.create_not(o1, {1, 2});
    layout.create_po(n1, "f1", {2, 2});

    return layout;
}

template <typename GateLyt>
GateLyt crossing_layout() noexcept
{
    GateLyt layout{typename GateLyt::aspect_ratio{3, 2, 1}, fiction::twoddwave_4_clocking};

    const auto x1 = layout.create_pi("x1", {1, 0});
    const auto x2 = layout.create_pi("x2", {0, 1});
    const auto x3 = layout.create_pi("x3", {2, 0});
    const auto x4 = layout.create_pi("x4", {0, 2});

    const auto a1 = layout.create_and(x1, x2, {1, 1});

    const auto b1 = layout.create_buf(x3, {2, 1});
    const auto b2 = layout.create_buf(x4, {1, 2});

    const auto a2 = layout.create_and(b1, b2, {2, 2});

    const auto c = layout.create_buf(a1, {2, 1, 1});

    layout.create_po(c, "f1", {3, 1});
    layout.create_po(a2, "f2", {3, 2});

    return layout;
}

template <typename GateLyt>
GateLyt fanout_layout() noexcept
{
    GateLyt layout{typename GateLyt::aspect_ratio{2, 2, 1}};

    layout.assign_clock_number({0, 0}, static_cast<typename GateLyt::clock_number_t>(3));
    layout.assign_clock_number({0, 1}, static_cast<typename GateLyt::clock_number_t>(0));
    layout.assign_clock_number({0, 2}, static_cast<typename GateLyt::clock_number_t>(1));
    layout.assign_clock_number({1, 0}, static_cast<typename GateLyt::clock_number_t>(2));
    layout.assign_clock_number({1, 1}, static_cast<typename GateLyt::clock_number_t>(1));
    layout.assign_clock_number({1, 2}, static_cast<typename GateLyt::clock_number_t>(0));
    layout.assign_clock_number({2, 0}, static_cast<typename GateLyt::clock_number_t>(3));
    layout.assign_clock_number({2, 1}, static_cast<typename GateLyt::clock_number_t>(2));
    layout.assign_clock_number({2, 2}, static_cast<typename GateLyt::clock_number_t>(3));

    const auto x1 = layout.create_pi("x1", {0, 1});

    const auto f1 = layout.create_buf(x1, {1, 1});
    const auto f2 = layout.create_buf(f1, {2, 1});
    const auto w1 = layout.create_buf(f2, {2, 2});

    layout.create_po(f1, "f1", {1, 0});
    layout.create_po(f2, "f2", {2, 0});
    layout.create_po(w1, "f3", {1, 2});

    return layout;
}

template <typename GateLyt>
GateLyt non_structural_all_function_gate_layout() noexcept
{
    GateLyt layout{typename GateLyt::aspect_ratio{2, 4, 0}, fiction::open_4_clocking};

    layout.assign_clock_number({0, 0}, static_cast<typename GateLyt::clock_number_t>(0));
    layout.assign_clock_number({1, 0}, static_cast<typename GateLyt::clock_number_t>(0));
    layout.assign_clock_number({2, 0}, static_cast<typename GateLyt::clock_number_t>(0));

    layout.assign_clock_number({0, 1}, static_cast<typename GateLyt::clock_number_t>(1));
    layout.assign_clock_number({1, 1}, static_cast<typename GateLyt::clock_number_t>(1));
    layout.assign_clock_number({2, 1}, static_cast<typename GateLyt::clock_number_t>(1));

    layout.assign_clock_number({0, 2}, static_cast<typename GateLyt::clock_number_t>(2));
    layout.assign_clock_number({1, 2}, static_cast<typename GateLyt::clock_number_t>(2));
    layout.assign_clock_number({2, 2}, static_cast<typename GateLyt::clock_number_t>(3));

    layout.assign_clock_number({0, 3}, static_cast<typename GateLyt::clock_number_t>(3));
    layout.assign_clock_number({1, 3}, static_cast<typename GateLyt::clock_number_t>(3));
    layout.assign_clock_number({2, 3}, static_cast<typename GateLyt::clock_number_t>(3));

    const auto x1 = layout.create_pi("x1", {0, 0});
    const auto x2 = layout.create_pi("x2", {1, 0});
    const auto x3 = layout.create_pi("x3", {2, 0});

    layout.create_and(x1, x2, {0, 1});
    layout.create_or(x1, x2, {1, 1});
    layout.create_xor(x1, x2, {2, 1});

    const auto m = layout.create_maj(x1, x2, x3, {0, 2});
    const auto f = layout.create_buf(m, {1, 2});
    const auto w = layout.create_buf(f, {2, 2});

    layout.create_not(w, {2, 3});
    layout.create_po(f, "po", {1, 3});

    layout.create_nand(x1, x2, {0, 4});
    layout.create_nor(x1, x2, {1, 4});

    return layout;
}

}  // namespace blueprints

#endif  // FICTION_LAYOUT_BLUEPRINTS_HPP
