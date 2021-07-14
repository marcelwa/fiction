//
// Created by marcel on 08.07.21.
//

#ifndef FICTION_COUNT_GATE_TYPES_HPP
#define FICTION_COUNT_GATE_TYPES_HPP

#include "../traits.hpp"

#include <mockturtle/traits.hpp>
#include <mockturtle/utils/progress_bar.hpp>

#include <ostream>

namespace fiction
{

struct count_gate_types_stats
{

    uint64_t num_fanout{0}, num_buf{0}, num_inv{0}, num_and2{0}, num_or2{0}, num_nand2{0}, num_nor2{0}, num_xor2{0},
        num_xnor2{0}, num_and3{0}, num_xor_and{0}, num_or_and{0}, num_onehot{0}, num_maj3{0}, num_gamble{0}, num_dot{0},
        num_mux{0}, num_and_xor{0}, num_other{0};

    void report(std::ostream& out = std::cout, const bool detailed = false) const
    {
        out << fmt::format("[i] AND2   = {}\n", num_and2);
        out << fmt::format("[i] OR2    = {}\n", num_or2);
        out << fmt::format("[i] NAND2  = {}\n", num_nand2);
        out << fmt::format("[i] NOR2   = {}\n", num_nor2);
        out << fmt::format("[i] XOR2   = {}\n", num_xor2);
        out << fmt::format("[i] XNOR2  = {}\n", num_xnor2);
        out << fmt::format("[i] MAJ3   = {}\n", num_maj3);
        out << fmt::format("[i] INV    = {}\n", num_inv);
        out << fmt::format("[i] BUF    = {}\n", num_buf);
        out << fmt::format("[i] FANOUT = {}\n", num_fanout);

        if (detailed)
        {
            out << fmt::format("[i] AND3   = {}\n", num_and3);
            out << fmt::format("[i] XORAND = {}\n", num_xor_and);
            out << fmt::format("[i] ORAND  = {}\n", num_or_and);
            out << fmt::format("[i] ONEHOT = {}\n", num_onehot);
            out << fmt::format("[i] GAMBLE = {}\n", num_gamble);
            out << fmt::format("[i] DOT    = {}\n", num_dot);
            out << fmt::format("[i] MUX    = {}\n", num_mux);
            out << fmt::format("[i] ANDXOR = {}\n", num_and_xor);
            out << fmt::format("[i] other  = {}\n", num_other);
        }
        else
        {
            out << fmt::format("[i] other  = {}\n", num_and3 + num_xor_and + num_or_and + num_onehot + num_gamble +
                                                        num_dot + num_mux + num_and_xor + num_other);
        }

        out << fmt::format("[i] total  = {}\n", num_fanout + num_buf + num_inv + num_and2 + num_or2 + num_nand2 +
                                                    num_nor2 + num_xor2 + num_xnor2 + num_and3 + num_xor_and +
                                                    num_or_and + num_onehot + num_maj3 + num_gamble + num_dot +
                                                    num_mux + num_and_xor + num_other);
    }
};

namespace detail
{

template <typename Ntk>
class count_gate_types_impl
{
  public:
    count_gate_types_impl(const Ntk& src, count_gate_types_stats& st) : ntk{src}, pst{st} {}

    void run()
    {
#if (PROGRESS_BARS)
        // initialize a progress bar
        mockturtle::progress_bar bar{static_cast<uint32_t>(ntk.size()), "[i] counting gate types: |{0}|"};
#endif
        ntk.foreach_node(
            [&, this](const auto& n, [[maybe_unused]] auto i)
            {
#if (PROGRESS_BARS)
                // update progress
                bar(i);
#endif

                if (!ntk.is_constant(n))
                {
                    if constexpr (fiction::has_is_fanout_v<Ntk>)
                    {
                        if (ntk.is_fanout(n))
                        {
                            ++pst.num_fanout;
                            return true;
                        }
                    }
                    if constexpr (fiction::has_is_buf_v<Ntk>)
                    {
                        if (ntk.is_buf(n))
                        {
                            ++pst.num_buf;
                            return true;
                        }
                    }
                    if constexpr (fiction::has_is_inv_v<Ntk>)
                    {
                        if (ntk.is_inv(n))
                        {
                            ++pst.num_inv;
                            return true;
                        }
                    }
                    if constexpr (mockturtle::has_is_and_v<Ntk>)
                    {
                        if (ntk.is_and(n))
                        {
                            ++pst.num_and2;
                            return true;
                        }
                    }
                    if constexpr (mockturtle::has_is_or_v<Ntk>)
                    {
                        if (ntk.is_or(n))
                        {
                            ++pst.num_or2;
                            return true;
                        }
                    }
                    if constexpr (fiction::has_is_nand_v<Ntk>)
                    {
                        if (ntk.is_nand(n))
                        {
                            ++pst.num_nand2;
                            return true;
                        }
                    }
                    if constexpr (fiction::has_is_nor_v<Ntk>)
                    {
                        if (ntk.is_nor(n))
                        {
                            ++pst.num_nor2;
                            return true;
                        }
                    }
                    if constexpr (mockturtle::has_is_xor_v<Ntk>)
                    {
                        if (ntk.is_xor(n))
                        {
                            ++pst.num_xor2;
                            return true;
                        }
                    }
                    if constexpr (fiction::has_is_xnor_v<Ntk>)
                    {
                        if (ntk.is_xnor(n))
                        {
                            ++pst.num_xnor2;
                            return true;
                        }
                    }
                    if constexpr (fiction::has_is_and3_v<Ntk>)
                    {
                        if (ntk.is_and3(n))
                        {
                            ++pst.num_and3;
                            return true;
                        }
                    }
                    if constexpr (fiction::has_is_xor_and_v<Ntk>)
                    {
                        if (ntk.is_xor_and(n))
                        {
                            ++pst.num_xor_and;
                            return true;
                        }
                    }
                    if constexpr (fiction::has_is_or_and_v<Ntk>)
                    {
                        if (ntk.is_or_and(n))
                        {
                            ++pst.num_or_and;
                            return true;
                        }
                    }
                    if constexpr (fiction::has_is_onehot_v<Ntk>)
                    {
                        if (ntk.is_onehot(n))
                        {
                            ++pst.num_onehot;
                            return true;
                        }
                    }
                    if constexpr (mockturtle::has_is_maj_v<Ntk>)
                    {
                        if (ntk.is_maj(n))
                        {
                            ++pst.num_maj3;
                            return true;
                        }
                    }
                    if constexpr (fiction::has_is_gamble_v<Ntk>)
                    {
                        if (ntk.is_gamble(n))
                        {
                            ++pst.num_gamble;
                            return true;
                        }
                    }
                    if constexpr (fiction::has_is_dot_v<Ntk>)
                    {
                        if (ntk.is_dot(n))
                        {
                            ++pst.num_dot;
                            return true;
                        }
                    }
                    if constexpr (mockturtle::has_is_ite_v<Ntk>)
                    {
                        if (ntk.is_ite(n))
                        {
                            ++pst.num_mux;
                            return true;
                        }
                    }
                    if constexpr (fiction::has_is_and_xor_v<Ntk>)
                    {
                        if (ntk.is_and_xor(n))
                        {
                            ++pst.num_and_xor;
                            return true;
                        }
                    }
                    // TODO more gate types go here

                    // don't count PIs
                    if (ntk.is_pi(n))
                    {
                        return true;
                    }

                    ++pst.num_other;
                }

                return true;
            });
    }

  private:
    Ntk ntk;

    count_gate_types_stats& pst;
};

}  // namespace detail

template <typename Ntk>
void count_gate_types(const Ntk& ntk, count_gate_types_stats* pst = nullptr)
{
    static_assert(mockturtle::is_network_type_v<Ntk>, "Ntk is not a network type");
    static_assert(mockturtle::has_foreach_node_v<Ntk>, "Ntk does not implement the foreach_node function");
    static_assert(mockturtle::has_is_constant_v<Ntk>, "Ntk does not implement the is_constant function");

    count_gate_types_stats        st{};
    detail::count_gate_types_impl p{ntk, st};

    p.run();

    if (pst)
    {
        *pst = st;
    }
}

}  // namespace fiction

#endif  // FICTION_COUNT_GATE_TYPES_HPP
