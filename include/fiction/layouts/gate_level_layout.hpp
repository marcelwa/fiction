//
// Created by marcel on 14.05.21.
//

#ifndef FICTION_GATE_LEVEL_LAYOUT_HPP
#define FICTION_GATE_LEVEL_LAYOUT_HPP

#include "../utils/range.hpp"
#include "clocking_scheme.hpp"

#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <mockturtle/networks/detail/foreach.hpp>
#include <mockturtle/networks/events.hpp>
#include <mockturtle/networks/storage.hpp>
#include <mockturtle/traits.hpp>
#include <mockturtle/utils/algorithm.hpp>
#include <mockturtle/utils/truth_table_cache.hpp>

#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <set>
#include <unordered_map>
#include <vector>

namespace fiction
{

template <typename ClockedLayout>
class gate_level_layout : public ClockedLayout
{
  public:
#pragma region Types and constructors

    using tile = typename ClockedLayout::tile;

    template <typename Node, typename Tile>
    struct gate_level_layout_storage_data
    {
        mockturtle::truth_table_cache<kitty::dynamic_truth_table> fn_cache;

        const Tile const0{0x8000000000000000ull};
        const Tile const1{0xc000000000000000ull};

        std::unordered_map<Tile, Node> tile_node_map{
            {{const0, static_cast<Node>(0ull)}, {const1, static_cast<Node>(1ull)}}};
        std::unordered_map<Node, Tile> node_tile_map{
            {{static_cast<Node>(0ull), const0}, {static_cast<Node>(1ull), const1}}};

        uint32_t num_gates = 0ull;
        uint32_t num_wires = 0ull;

        uint32_t trav_id = 0ul;
    };

    /*! \brief gate-level layout node
     *
     * `data[0].h1`: Internal (data-flow independent) fan-out size (MSB indicates dead nodes)
     * `data[0].h2`: Application-specific value
     * `data[1].h1`: Function literal in truth table cache
     * `data[2].h2`: Visited flags
     */
    struct gate_level_layout_storage_node : mockturtle::mixed_fanin_node<2>
    {
        bool operator==(gate_level_layout_storage_node const& other) const
        {
            return data[1].h1 == other.data[1].h1 && children == other.children;
        }
    };

    static constexpr auto min_fanin_size = 1;
    static constexpr auto max_fanin_size = 3;

    using base_type = gate_level_layout;
    using node      = uint32_t;
    using signal    = uint64_t;

    using event_storage = std::shared_ptr<mockturtle::network_events<base_type>>;

    /*! \brief tile-based layout storage container */
    using gate_level_layout_storage =
        mockturtle::storage<gate_level_layout_storage_node, gate_level_layout_storage_data<node, signal>>;

    using storage = std::shared_ptr<gate_level_layout_storage>;

    explicit gate_level_layout(const typename ClockedLayout::aspect_ratio& ar = {}) :
            ClockedLayout(ar),
            strg{std::make_shared<gate_level_layout_storage>()},
            evnts{std::make_shared<typename event_storage::element_type>()}
    {
        initialize_truth_table_cache();
    }

    gate_level_layout(const typename ClockedLayout::aspect_ratio& ar, const clocking_scheme<tile>& scheme) :
            ClockedLayout(ar, scheme),
            strg{std::make_shared<gate_level_layout_storage>()},
            evnts{std::make_shared<typename event_storage::element_type>()}
    {
        initialize_truth_table_cache();
    }

    explicit gate_level_layout(std::shared_ptr<gate_level_layout_storage> s) :
            strg{std::move(s)},
            evnts{std::make_shared<typename event_storage::element_type>()}
    {}

#pragma endregion

#pragma region Primary I / O and constants

    [[nodiscard]] signal get_constant(bool value = false) const noexcept
    {
        // const0 = (1,0,0,0), const1 = (1,1,0,0)
        return value ? strg->data.const1 : strg->data.const0;
    }

    [[nodiscard]] bool is_constant(node const& n) const noexcept
    {
        return n <= 1;
    }

    [[nodiscard]] bool constant_value(node const& n) const noexcept
    {
        return n == 1;
    }

    signal create_pi([[maybe_unused]] const std::string& name = std::string(), const tile& t = {})
    {
        const auto n = static_cast<node>(strg->nodes.size());
        strg->nodes.emplace_back();     // empty node data
        strg->nodes[n].data[1].h1 = 2;  // assign identity function
        strg->inputs.emplace_back(n);
        assign_node(t, n);

        return static_cast<signal>(t);
    }

    signal create_po(const signal& s, [[maybe_unused]] const std::string& name = std::string(), const tile& t = {})
    {
        const auto n = static_cast<node>(strg->nodes.size());
        strg->nodes.emplace_back();     // empty node data
        strg->nodes[n].data[1].h1 = 2;  // assign identity function
        strg->outputs.emplace_back(static_cast<signal>(t));
        assign_node(t, n);

        /* increase ref-count to child */
        strg->nodes[get_node(s)].data[0].h1++;
        strg->nodes[n].children.push_back(s);

        return static_cast<signal>(t);
    }

    [[nodiscard]] bool is_pi(const node& n) const noexcept
    {
        return std::find(strg->inputs.cbegin(), strg->inputs.cend(), n) != strg->inputs.cend();
    }

    [[nodiscard]] bool is_pi_tile(const tile& t) const noexcept
    {
        return is_pi(get_node(t));
    }

    [[nodiscard]] bool is_po(const node& n) const noexcept
    {
        return std::find_if(strg->outputs.cbegin(), strg->outputs.cend(),
                            [this, &n](const auto& p) { return this->get_node(p.index) == n; }) != strg->outputs.cend();
    }

    [[nodiscard]] bool is_po_tile(const tile& t) const noexcept
    {
        return is_po(get_node(t));
    }

    [[nodiscard]] bool is_combinational() const noexcept
    {
        return true;
    }

#pragma endregion

#pragma region Create function tiles

    signal create_buf(signal const& a, const tile& t = {})
    {
        return create_node_from_literal({a}, 2, t);
    }

    signal create_not(signal const& a, const tile& t = {})
    {
        return create_node_from_literal({a}, 3, t);
    }

    signal create_and(signal a, signal b, const tile& t = {})
    {
        return create_node_from_literal({a, b}, 4, t);
    }

    signal create_nand(signal a, signal b, const tile& t = {})
    {
        return create_node_from_literal({a, b}, 5, t);
    }

    signal create_or(signal a, signal b, const tile& t = {})
    {
        return create_node_from_literal({a, b}, 6, t);
    }

    signal create_nor(signal a, signal b, const tile& t = {})
    {
        return create_node_from_literal({a, b}, 7, t);
    }

    signal create_xor(signal a, signal b, const tile& t = {})
    {
        return create_node_from_literal({a, b}, 8, t);
    }

    signal create_xnor(signal a, signal b, const tile& t = {})
    {
        return create_node_from_literal({a, b}, 9, t);
    }

    signal create_maj(signal a, signal b, signal c, const tile& t = {})
    {
        return create_node_from_literal({a, b, c}, 10, t);
    }

    signal create_node(const std::vector<signal>& children, const kitty::dynamic_truth_table& function,
                       const tile& t = {})
    {
        if (children.empty())
        {
            assert(function.num_vars() == 0u);
            return get_constant(!kitty::is_const0(function));
        }

        return create_node_from_literal(children, strg->data.fn_cache.insert(function), t);
    }

#pragma endregion

#pragma region Functional properties

    [[nodiscard]] kitty::dynamic_truth_table node_function(const node& n) const
    {
        return strg->data.fn_cache[strg->nodes[n].data[1].h1];
    }

#pragma endregion

#pragma region Structural properties

    [[nodiscard]] auto size() const noexcept
    {
        return static_cast<uint32_t>(strg->nodes.size());
    }

    [[nodiscard]] auto num_pis() const noexcept
    {
        return strg->inputs.size();
    }

    [[nodiscard]] auto num_pos() const noexcept
    {
        return strg->outputs.size();
    }

    [[nodiscard]] auto num_gates() const noexcept
    {
        return strg->data.num_gates;
    }

    [[nodiscard]] auto num_wires() const noexcept
    {
        return strg->data.num_wires;
    }

    [[nodiscard]] auto fanin_size(node const& n) const
    {
        return static_cast<uint32_t>(incoming_data_flow<std::set<tile>>(get_tile(n)).size());
    }

    [[nodiscard]] auto fanout_size(node const& n) const
    {
        return static_cast<uint32_t>(outgoing_data_flow<std::set<tile>>(get_tile(n)).size());
    }

#pragma endregion

#pragma region Nodes and signals

    [[nodiscard]] node get_node(const signal& s) const noexcept
    {
        if (auto it = strg->data.tile_node_map.find(s); it != strg->data.tile_node_map.end())
        {
            return it->second;
        }

        return 0;
    }

    [[nodiscard]] node get_node(const tile& t) const noexcept
    {
        return get_node(static_cast<signal>(t));
    }

    [[nodiscard]] tile get_tile(const node& n) const noexcept
    {
        if (auto it = strg->data.node_tile_map.find(n); it != strg->data.node_tile_map.end())
        {
            return static_cast<tile>(it->second);
        }

        return {};
    }

    [[nodiscard]] bool is_dead(const node& n) const noexcept
    {
        return (strg->nodes[n].data[0].h1 >> 31) & 1;
    }

    [[nodiscard]] signal make_signal(const node& n) const noexcept
    {
        return static_cast<signal>(get_tile(n));
    }

    signal move_node(const node& n, const tile& t, const std::vector<signal>& new_children = {}) noexcept
    {
        auto& children = strg->nodes[n].children;
        // decrease ref-count of children
        std::for_each(children.begin(), children.end(),
                      [this](const auto& c) { strg->nodes[get_node(c.index)].data[0].h1--; });
        // clear n's children
        children.clear();
        // clear n's position
        clear_tile(get_tile(n));

        // assign n to its new position
        assign_node(t, n);
        // assign new children
        std::copy(new_children.begin(), new_children.end(), std::back_inserter(children));
        // increase ref-count to new children
        std::for_each(new_children.cbegin(), new_children.cend(),
                      [this](const auto& nc) { strg->nodes[get_node(nc)].data[0].h1++; });

        return static_cast<signal>(t);
    }

    [[nodiscard]] bool is_complemented([[maybe_unused]] const signal& s) const noexcept
    {
        return false;
    }

    [[nodiscard]] uint32_t node_to_index(node const& n) const noexcept
    {
        return static_cast<uint32_t>(n);
    }

    [[nodiscard]] node index_to_node(const uint32_t index) const noexcept
    {
        return index;
    }

    [[nodiscard]] bool is_gate(const node n) const noexcept
    {
        return strg->nodes[n].data[1].h1 >= 2 && !is_pi(n);
    }

    [[nodiscard]] bool is_buf(const node n) const noexcept
    {
        return strg->nodes[n].data[1].h1 == 2;
    }

    [[nodiscard]] bool is_wire(const node n) const noexcept
    {
        return is_buf(n);
    }

    [[nodiscard]] bool is_inv(const node n) const noexcept
    {
        return strg->nodes[n].data[1].h1 == 3;
    }

    [[nodiscard]] bool is_and(node const n) const noexcept
    {
        return strg->nodes[n].data[1].h1 == 4;
    }

    [[nodiscard]] bool is_nand(node const n) const noexcept
    {
        return strg->nodes[n].data[1].h1 == 5;
    }

    [[nodiscard]] bool is_or(node const n) const noexcept
    {
        return strg->nodes[n].data[1].h1 == 6;
    }

    [[nodiscard]] bool is_nor(node const n) const noexcept
    {
        return strg->nodes[n].data[1].h1 == 7;
    }

    [[nodiscard]] bool is_xor(node const n) const noexcept
    {
        return strg->nodes[n].data[1].h1 == 8;
    }

    [[nodiscard]] bool is_xnor(node const n) const noexcept
    {
        return strg->nodes[n].data[1].h1 == 9;
    }

    [[nodiscard]] bool is_maj(node const n) const noexcept
    {
        return strg->nodes[n].data[1].h1 == 10;
    }

    [[nodiscard]] bool is_fanout(const node n) const noexcept
    {
        return is_wire(n) && fanout_size(n) > 1;
    }

    [[nodiscard]] bool is_function(node const& n) const
    {
        return n > 1;
    }

    [[nodiscard]] bool is_gate_tile(const tile& t) const noexcept
    {
        return is_gate(get_node(static_cast<signal>(t)));
    }

    [[nodiscard]] bool is_wire_tile(const tile& t) const noexcept
    {
        return is_wire(get_node(static_cast<signal>(t)));
    }

    [[nodiscard]] bool is_empty_tile(const tile& t) const noexcept
    {
        return !is_gate_tile(t) && !is_wire_tile(t);
    }

#pragma endregion

#pragma region Iteration

    template <typename Fn>
    void foreach_pi(Fn&& fn) const
    {
        using IteratorType = decltype(strg->inputs.cbegin());
        mockturtle::detail::foreach_element_transform<IteratorType, node>(
            strg->inputs.cbegin(), strg->inputs.cend(), [](const auto& i) { return static_cast<node>(i); }, fn);
    }

    template <typename Fn>
    void foreach_po(Fn&& fn) const
    {
        using IteratorType = decltype(strg->outputs.begin());
        mockturtle::detail::foreach_element_transform<IteratorType, signal>(
            strg->outputs.begin(), strg->outputs.end(), [](const auto& o) { return o.index; }, fn);
    }

    template <typename Fn>
    void foreach_node(Fn&& fn) const
    {
        auto r = mockturtle::range<node>(static_cast<node>(strg->nodes.size()));
        mockturtle::detail::foreach_element_if(
            r.begin(), r.end(), [this](const auto& n) { return !is_dead(n); }, fn);
    }

    template <typename Fn>
    void foreach_gate(Fn&& fn) const
    {
        auto r = mockturtle::range<node>(2u, static_cast<node>(strg->nodes.size()));  // start from 2 to avoid constants
        mockturtle::detail::foreach_element_if(
            r.begin(), r.end(), [this](const auto n) { return is_gate(n) && !is_dead(n); }, fn);
    }

    template <typename Fn>
    void foreach_wire(Fn&& fn) const
    {
        auto r = mockturtle::range<node>(2u, static_cast<node>(strg->nodes.size()));  // start from 2 to avoid constants
        mockturtle::detail::foreach_element_if(
            r.begin(), r.end(), [this](const auto n) { return is_wire(n) && !is_dead(n); }, fn);
    }

    template <typename Fn>
    void foreach_fanin(node const& n, Fn&& fn) const
    {
        if (n <= 1)  // const-0 or const-1
            return;

        const auto fanin = incoming_data_flow<std::set<tile>>(get_tile(n));

        using IteratorType = decltype(fanin.cbegin());
        mockturtle::detail::foreach_element_transform<IteratorType, signal>(
            fanin.cbegin(), fanin.cend(), [](const auto& t) { return static_cast<signal>(t); }, fn);
    }

    template <typename Fn>
    void foreach_fanout(node const& n, Fn&& fn) const
    {
        if (n <= 1)  // const-0 or const-1
            return;

        const auto fanout = outgoing_data_flow<std::set<tile>>(get_tile(n));

        using IteratorType = decltype(fanout.cbegin());
        mockturtle::detail::foreach_element_transform<IteratorType, signal>(
            fanout.cbegin(), fanout.cend(), [](const auto& t) { return static_cast<signal>(t); }, fn);
    }

    template <typename Fn>
    void foreach_ci(Fn&& fn) const
    {
        foreach_pi(fn);
    }

    template <typename Fn>
    void foreach_co(Fn&& fn) const
    {
        foreach_po(fn);
    }

#pragma endregion

#pragma region Simulate values

    template <typename Iterator>
    mockturtle::iterates_over_t<Iterator, bool> compute(node const& n, Iterator begin, Iterator end) const
    {
        uint32_t index{0};
        while (begin != end)
        {
            index <<= 1;
            index ^= *begin++ ? 1 : 0;
        }

        return kitty::get_bit(strg->data.cache[strg->nodes[n].data[1].h1], index);
    }

    template <typename Iterator>
    mockturtle::iterates_over_truth_table_t<Iterator> compute(node const& n, Iterator begin, Iterator end) const
    {
        std::vector<typename Iterator::value_type> tts{begin, end};

        const auto num_fanin = fanin_size(n);
        assert(tts.size() == num_fanin);
        assert(num_fanin != 0ul);

        /* resulting truth table has the same size as any of the children */
        auto       result  = tts.front().construct();
        const auto gate_tt = strg->data.fn_cache[strg->nodes[n].data[1].h1];

        for (uint32_t i = 0u; i < static_cast<uint32_t>(result.num_bits()); ++i)
        {
            uint32_t pattern = 0u;
            for (uint32_t j = 0u; j < num_fanin; ++j)
            {
                pattern |= static_cast<uint32_t>(kitty::get_bit(tts[j], i)) << j;
            }

            if (kitty::get_bit(gate_tt, pattern))
            {
                kitty::set_bit(result, i);
            }
        }

        return result;
    }

#pragma endregion

#pragma region Cardinal operations

    [[nodiscard]] bool is_incoming_signal(const tile& t, const signal& s) const noexcept
    {
        const auto incoming = incoming_data_flow<std::set<tile>>(t);
        return std::any_of(incoming.cbegin(), incoming.cend(),
                           [this, &s](const auto& i)
                           { return i == s || ClockedLayout::above(i) == s || ClockedLayout::below(i) == s; });
    }

    [[nodiscard]] bool has_northern_incoming_signal(const tile& t) const noexcept
    {
        return is_incoming_signal(t, static_cast<signal>(ClockedLayout::north(t)));
    }

    [[nodiscard]] bool has_eastern_incoming_signal(const tile& t) const noexcept
    {
        return is_incoming_signal(t, static_cast<signal>(ClockedLayout::east(t)));
    }

    [[nodiscard]] bool has_southern_incoming_signal(const tile& t) const noexcept
    {
        return is_incoming_signal(t, static_cast<signal>(ClockedLayout::south(t)));
    }

    [[nodiscard]] bool has_western_incoming_signal(const tile& t) const noexcept
    {
        return is_incoming_signal(t, static_cast<signal>(ClockedLayout::west(t)));
    }

    [[nodiscard]] bool has_no_incoming_signal(const tile& t) const noexcept
    {
        return incoming_data_flow<std::set<tile>>(t).empty();
    }

    [[nodiscard]] bool is_outgoing_signal(const tile& t, const signal& s) const noexcept
    {
        const auto outgoing = outgoing_data_flow<std::set<tile>>(t);
        return std::any_of(outgoing.cbegin(), outgoing.cend(),
                           [this, &s](const auto& o)
                           { return o == s || ClockedLayout::above(o) == s || ClockedLayout::below(o) == s; });
    }

    [[nodiscard]] bool has_northern_outgoing_signal(const tile& t) const noexcept
    {
        return is_outgoing_signal(t, static_cast<signal>(ClockedLayout::north(t)));
    }

    [[nodiscard]] bool has_eastern_outgoing_signal(const tile& t) const noexcept
    {
        return is_outgoing_signal(t, static_cast<signal>(ClockedLayout::east(t)));
    }

    [[nodiscard]] bool has_southern_outgoing_signal(const tile& t) const noexcept
    {
        return is_outgoing_signal(t, static_cast<signal>(ClockedLayout::south(t)));
    }

    [[nodiscard]] bool has_western_outgoing_signal(const tile& t) const noexcept
    {
        return is_outgoing_signal(t, static_cast<signal>(ClockedLayout::west(t)));
    }

    [[nodiscard]] bool has_no_outgoing_signal(const tile& t) const noexcept
    {
        return outgoing_data_flow<std::set<tile>>(t).empty();
    }

    [[nodiscard]] bool has_opposite_incoming_and_outgoing_signals(const tile& t) const noexcept
    {
        return (has_northern_incoming_signal(t) && has_southern_outgoing_signal(t)) ||
               (has_eastern_incoming_signal(t) && has_western_outgoing_signal(t)) ||
               (has_southern_incoming_signal(t) && has_northern_outgoing_signal(t)) ||
               (has_western_incoming_signal(t) && has_eastern_outgoing_signal((t)));
    }

#pragma endregion

#pragma region Custom node values

    void clear_values() const noexcept
    {
        std::for_each(strg->nodes.begin(), strg->nodes.end(), [](auto& n) { n.data[0].h2 = 0; });
    }

    [[nodiscard]] uint32_t value(node const& n) const
    {
        return strg->nodes[n].data[0].h2;
    }

    void set_value(node const& n, uint32_t v) const
    {
        strg->nodes[n].data[0].h2 = v;
    }

    uint32_t incr_value(node const& n) const
    {
        return static_cast<uint32_t>(strg->nodes[n].data[0].h2++);
    }

    uint32_t decr_value(node const& n) const
    {
        return static_cast<uint32_t>(--strg->nodes[n].data[0].h2);
    }

#pragma endregion

#pragma region Visited flags

    void clear_visited() const
    {
        std::for_each(strg->nodes.begin(), strg->nodes.end(), [](auto& n) { n.data[1].h2 = 0; });
    }

    [[nodiscard]] auto visited(node const& n) const
    {
        return strg->nodes[n].data[1].h2;
    }

    void set_visited(node const& n, uint32_t v) const
    {
        strg->nodes[n].data[1].h2 = v;
    }

    [[nodiscard]] uint32_t trav_id() const
    {
        return strg->data.trav_id;
    }

    void incr_trav_id() const
    {
        ++strg->data.trav_id;
    }

#pragma endregion

#pragma region General methods

    auto& events() const
    {
        return *evnts;
    }

#pragma endregion

  private:
    storage strg;

    event_storage evnts;

    inline void initialize_truth_table_cache()
    {
        /* reserve the second node for constant 1 */
        strg->nodes.emplace_back();

        kitty::dynamic_truth_table tt_zero(0);
        strg->data.fn_cache.insert(tt_zero);

        strg->nodes[0].data[1].h1 = 0;
        strg->nodes[1].data[1].h1 = 1;

        /* reserve some truth tables for nodes */
        const auto create_and_cache = [this](const auto& literal, auto n)
        {
            kitty::dynamic_truth_table tt(static_cast<uint32_t>(n));
            kitty::create_from_words(tt, &literal, &literal + 1);
            strg->data.fn_cache.insert(tt);
        };

        static constexpr const uint64_t lit_not = 0x1, lit_and = 0x8, lit_or = 0xe, lit_xor = 0x6, lit_maj = 0xe8;

        create_and_cache(lit_not, 1);  // since NOT is not normal, its complement, i.e., the identity, is stored
        create_and_cache(lit_and, 2);
        create_and_cache(lit_or, 2);
        create_and_cache(lit_xor, 2);
        create_and_cache(lit_maj, 3);
    }

    void assign_node(const tile& t, const node& n)
    {
        if (!t.is_dead())
        {
            clear_tile(t);

            strg->data.tile_node_map[static_cast<signal>(t)] = n;

            strg->data.node_tile_map[n] = static_cast<signal>(t);

            // keep track of number of gates and wire segments
            if (strg->nodes[n].data[1].h1 == 2)
            {
                strg->data.num_wires++;
            }
            else
            {
                strg->data.num_gates++;
            }
        }
    }

    void kill_node(const node& n)
    {
        if (!is_constant(n))
        {
            strg->nodes[n].data[0].h1 |= UINT32_C(0x80000000);
        }
    }

    signal create_node_from_literal(const std::vector<signal>& children, uint32_t literal, const tile& t)
    {
        typename storage::element_type::node_type node_data;
        std::copy(children.begin(), children.end(), std::back_inserter(node_data.children));
        node_data.data[1].h1 = literal;

        const auto n = static_cast<node>(strg->nodes.size());
        strg->nodes.push_back(node_data);

        /* increase ref-count to children */
        for (const auto& c : children) { strg->nodes[get_node(c)].data[0].h1++; }

        set_value(n, 0);

        assign_node(t, n);

        for (auto const& fn : evnts->on_add) { (*fn)(n); }

        return static_cast<signal>(t);
    }

    [[nodiscard]] bool is_child(const node& n, const signal& s) const noexcept
    {
        const auto& node_data = strg->nodes[n];
        return std::find(node_data.children.cbegin(), node_data.children.cend(), s) != node_data.children.cend();
    }

    template <typename Container>
    [[nodiscard]] Container incoming_data_flow(const tile& t) const noexcept
    {
        auto incoming = ClockedLayout::template incoming_clocked_zones<Container>(t);

        Container data_flow{};

        for (const auto& in : incoming)
        {
            const std::set<tile> incoming_above_below{
                std::initializer_list<tile>{in, ClockedLayout::above(in), ClockedLayout::below(in)}};

            std::copy_if(std::cbegin(incoming_above_below), std::cend(incoming_above_below),
                         std::inserter(data_flow, std::cend(data_flow)),
                         [this, &t](const auto& dt) { return is_child(get_node(t), static_cast<signal>(dt)); });
        }

        return data_flow;
    }

    template <typename Container>
    [[nodiscard]] Container outgoing_data_flow(const tile& t) const noexcept
    {
        auto outgoing = ClockedLayout::template outgoing_clocked_zones<Container>(t);

        Container data_flow{};

        for (const auto& out : outgoing)
        {
            const std::set<tile> outgoing_above_below{
                std::initializer_list<tile>{out, ClockedLayout::above(out), ClockedLayout::below(out)}};

            std::copy_if(std::cbegin(outgoing_above_below), std::cend(outgoing_above_below),
                         std::inserter(data_flow, std::cend(data_flow)),
                         [this, &t](const auto& dt) { return is_child(get_node(dt), static_cast<signal>(t)); });
        }

        return data_flow;
    }

    void clear_tile(const tile& t) noexcept
    {
        if (auto it = strg->data.tile_node_map.find(static_cast<signal>(t)); it != strg->data.tile_node_map.end())
        {
            // decrease gate count
            if (is_gate(it->second))
                --strg->data.num_gates;
            // decrease wire count
            if (is_wire(it->second))
                --strg->data.num_wires;
            // mark node as dead
            kill_node(it->second);

            // remove node-tile
            strg->data.node_tile_map.erase(it->second);
            // remove tile-node
            strg->data.tile_node_map.erase(it);
        }
    }
};

}  // namespace fiction

#endif  // FICTION_GATE_LEVEL_LAYOUT_HPP
