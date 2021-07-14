//
// Created by marcel on 20.05.21.
//

#ifndef FICTION_TOPOLOGY_NETWORK_HPP
#define FICTION_TOPOLOGY_NETWORK_HPP

#include <mockturtle/networks/klut.hpp>

namespace fiction
{

/**
 * Reimplement all node creation functions to disable structural hashing.
 */
class topology_network : public mockturtle::klut_network
{
  public:
#pragma region Types and constructors

    topology_network() : mockturtle::klut_network()
    {
        add_additional_functions();
    }

    explicit topology_network(std::shared_ptr<mockturtle::klut_storage> s) : mockturtle::klut_network(std::move(s))
    {
        add_additional_functions();
    }

#pragma endregion

#pragma region Primary I / O and functions

    [[nodiscard]] bool is_po(node const& n) const
    {
        const auto end = _storage->outputs.begin() + _storage->data.num_pos;

        return std::find_if(_storage->outputs.begin(), end,
                            [this, &n](const auto& p)
                            { return this->get_node(p.index) == n; }) != _storage->outputs.end();
    }

#pragma endregion

#pragma region Create unary functions

    /**
     * Make buffers real nodes.
     */
    signal create_buf(signal const& a)
    {
        return _create_node({a}, 2);
    }

    signal create_not(signal const& a)
    {
        return is_constant(a) ? get_constant(!constant_value(a)) : _create_node({a}, 3);
    }

#pragma endregion

#pragma region Create binary functions
    signal create_and(signal a, signal b)
    {
        return _create_node({a, b}, 4);
    }

    signal create_nand(signal a, signal b)
    {
        return _create_node({a, b}, 5);
    }

    signal create_or(signal a, signal b)
    {
        return _create_node({a, b}, 6);
    }

    signal create_nor(signal a, signal b)
    {
        return _create_node({a, b}, 7);
    }

    signal create_lt(signal a, signal b)
    {
        return _create_node({a, b}, 8);
    }

    signal create_le(signal a, signal b)
    {
        return _create_node({a, b}, 11);
    }

    signal create_xor(signal a, signal b)
    {
        return _create_node({a, b}, 12);
    }
#pragma endregion

#pragma region Create ternary functions

    signal create_maj(signal a, signal b, signal c)
    {
        return _create_node({a, b, c}, 14);
    }

    signal create_ite(signal a, signal b, signal c)
    {
        return _create_node({a, b, c}, 16);
    }

    signal create_xor3(signal a, signal b, signal c)
    {
        return _create_node({a, b, c}, 18);
    }

    signal create_dot(signal a, signal b, signal c)
    {
        return _create_node({a, b, c}, 20);
    }
#pragma endregion

#pragma region Create nary functions

    signal create_nary_and(std::vector<signal> const& fs)
    {
        return mockturtle::tree_reduce(fs.begin(), fs.end(), get_constant(true),
                                       [this](auto const& a, auto const& b) { return create_and(a, b); });
    }

    signal create_nary_or(std::vector<signal> const& fs)
    {
        return mockturtle::tree_reduce(fs.begin(), fs.end(), get_constant(false),
                                       [this](auto const& a, auto const& b) { return create_or(a, b); });
    }

    signal create_nary_xor(std::vector<signal> const& fs)
    {
        return mockturtle::tree_reduce(fs.begin(), fs.end(), get_constant(false),
                                       [this](auto const& a, auto const& b) { return create_xor(a, b); });
    }
#pragma endregion

#pragma region Create arbitrary functions

    /**
     * Disable structural hashing.
     */
    signal _create_node(std::vector<signal> const& children, uint32_t literal)
    {
        storage::element_type::node_type node_data;
        std::copy(children.begin(), children.end(), std::back_inserter(node_data.children));
        node_data.data[1].h1 = literal;

        const auto index = _storage->nodes.size();
        _storage->nodes.push_back(node_data);

        /* increase ref-count to children */
        for (auto c : children) { _storage->nodes[c].data[0].h1++; }

        set_value(index, 0);

        for (auto const& fn : _events->on_add) { (*fn)(index); }

        return index;
    }

    signal create_node(std::vector<signal> const& children, kitty::dynamic_truth_table const& function)
    {
        if (children.size() == 0u)
        {
            assert(function.num_vars() == 0u);
            return get_constant(!kitty::is_const0(function));
        }
        return _create_node(children, _storage->data.cache.insert(function));
    }

    signal clone_node(klut_network const& other, node const& source, std::vector<signal> const& children)
    {
        assert(!children.empty());
        const auto tt = other._storage->data.cache[other._storage->nodes[source].data[1].h1];
        return create_node(children, tt);
    }

#pragma endregion

#pragma region Structural properties

    [[nodiscard]] bool is_buf(node const& n) const noexcept
    {
        return _storage->nodes[n].data[1].h1 == 2 && !is_pi(n);
    }

    [[nodiscard]] bool is_fanout(const node n) const noexcept
    {
        return is_buf(n) && fanout_size(n) > 1;
    }

    [[nodiscard]] bool is_inv(node const& n) const noexcept
    {
        return _storage->nodes[n].data[1].h1 == 3;
    }

    [[nodiscard]] bool is_and(node const& n) const noexcept
    {
        return _storage->nodes[n].data[1].h1 == 4;
    }

    [[nodiscard]] bool is_nand(node const& n) const noexcept
    {
        return _storage->nodes[n].data[1].h1 == 5;
    }

    [[nodiscard]] bool is_or(node const& n) const noexcept
    {
        return _storage->nodes[n].data[1].h1 == 6;
    }

    [[nodiscard]] bool is_nor(node const& n) const noexcept
    {
        return _storage->nodes[n].data[1].h1 == 7;
    }

    [[nodiscard]] bool is_xor(node const& n) const noexcept
    {
        return _storage->nodes[n].data[1].h1 == 12;
    }

    [[nodiscard]] bool is_xnor(node const& n) const noexcept
    {
        return _storage->nodes[n].data[1].h1 == 13;
    }

    [[nodiscard]] bool is_maj(node const& n) const noexcept
    {
        return _storage->nodes[n].data[1].h1 == 14;
    }

    [[nodiscard]] bool is_ite(node const& n) const noexcept
    {
        return _storage->nodes[n].data[1].h1 == 16;
    }

    [[nodiscard]] bool is_xor3(node const& n) const noexcept
    {
        return _storage->nodes[n].data[1].h1 == 18;
    }

    [[nodiscard]] bool is_dot(node const& n) const noexcept
    {
        return _storage->nodes[n].data[1].h1 == 20;
    }

    [[nodiscard]] bool is_and3(node const& n) const noexcept
    {
        return _storage->nodes[n].data[1].h1 == 22;
    }

    [[nodiscard]] bool is_xor_and(node const& n) const noexcept
    {
        return _storage->nodes[n].data[1].h1 == 24;
    }

    [[nodiscard]] bool is_or_and(node const& n) const noexcept
    {
        return _storage->nodes[n].data[1].h1 == 26;
    }

    [[nodiscard]] bool is_onehot(node const& n) const noexcept
    {
        return _storage->nodes[n].data[1].h1 == 28;
    }

    [[nodiscard]] bool is_gamble(node const& n) const noexcept
    {
        return _storage->nodes[n].data[1].h1 == 31;  // 31 since gamble is not normal
    }

    [[nodiscard]] bool is_and_xor(node const& n) const noexcept
    {
        return _storage->nodes[n].data[1].h1 == 32;
    }

#pragma endregion

  protected:
    void add_additional_functions() noexcept
    {
        // create dot function: a xor (c or a and b)
        static uint64_t dot = 0x52;

        kitty::dynamic_truth_table tt_dot(3);
        kitty::create_from_words(tt_dot, &dot, &dot + 1);
        _storage->data.cache.insert(tt_dot);

        // create and3 function: a and b and c
        static uint64_t and3 = 0x80;

        kitty::dynamic_truth_table tt_and3(3);
        kitty::create_from_words(tt_and3, &and3, &and3 + 1);
        _storage->data.cache.insert(tt_and3);

        // create xor_and function: a and (b xor c)
        static uint64_t xor_and = 0x28;

        kitty::dynamic_truth_table tt_xor_and(3);
        kitty::create_from_words(tt_xor_and, &xor_and, &xor_and + 1);
        _storage->data.cache.insert(tt_xor_and);

        // create or_and function: a and (b or c)
        static uint64_t or_and = 0xa8;

        kitty::dynamic_truth_table tt_or_and(3);
        kitty::create_from_words(tt_or_and, &or_and, &or_and + 1);
        _storage->data.cache.insert(tt_or_and);

        // create onehot function: exactly one of a,b,c
        static uint64_t onehot = 0x16;

        kitty::dynamic_truth_table tt_onehot(3);
        kitty::create_from_words(tt_onehot, &onehot, &onehot + 1);
        _storage->data.cache.insert(tt_onehot);

        // create gamble function: all or nothing of a,b,c
        static uint64_t gamble = 0x81;

        kitty::dynamic_truth_table tt_gamble(3);
        kitty::create_from_words(tt_gamble, &gamble, &gamble + 1);
        _storage->data.cache.insert(tt_gamble);

        // create and_xor function: a xor b and c
        static uint64_t and_xor = 0x6a;

        kitty::dynamic_truth_table tt_and_xor(3);
        kitty::create_from_words(tt_and_xor, &and_xor, &and_xor + 1);
        _storage->data.cache.insert(tt_and_xor);
    }
};

}  // namespace fiction

#endif  // FICTION_TOPOLOGY_NETWORK_HPP
