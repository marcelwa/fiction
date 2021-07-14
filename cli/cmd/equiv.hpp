//
// Created by marcel on 04.03.20.
//

#ifndef FICTION_EQUIV_HPP
#define FICTION_EQUIV_HPP

#include <fiction/types.hpp>

#include <alice/alice.hpp>
#include <fmt/format.h>
#include <kitty/print.hpp>
#include <mockturtle/algorithms/equivalence_checking.hpp>
#include <mockturtle/algorithms/miter.hpp>
#include <mockturtle/networks/klut.hpp>
#include <mockturtle/utils/stopwatch.hpp>
#include <nlohmann/json.hpp>

#include <variant>

namespace alice
{

/**
 * Performs equality checks on logic networks; either between specification and gate layout or between two gate
 * layouts.
 */
class equiv_command : public command
{
  public:
    /**
     * Standard constructor. Adds descriptive information, options, and flags.
     *
     * @param e alice::environment that specifies stores etc.
     */
    explicit equiv_command(const environment::ptr& e) :
            command(e, "Performs logical and delay equivalence checks of gate layouts against a specification "
                       "that can either be its own associated logic network or another gate layout.")
    {
        add_flag("--logic_network,-n", "Perform equivalence checking with a logic network");
        add_flag("--gate_layout,-g", "Perform equivalence checking with a gate-level layout");
    }

  protected:
    /**
     * Function to perform the equivalence check call. Fetches the layout(s) and calls the checker.
     */
    void execute() override
    {
        result = {};

        if (is_set("logic_network") && is_set("gate_layout"))
        {
            auto& sn = store<fiction::logic_network_t>();

            if (sn.empty())
            {
                env->out() << "[w] no logic network in store" << std::endl;
                return;
            }

            auto& sg = store<fiction::gate_layout_t>();

            if (sg.empty())
            {
                env->out() << "[w] no gate layout in store" << std::endl;
                return;
            }

            equivalence_checking(sn.current(), sg.current());
        }
        else if (is_set("logic_network"))
        {
            auto& sn = store<fiction::logic_network_t>();

            if (sn.size() < 2)
            {
                env->out() << "[w] need at least two logic networks in store" << std::endl;
                return;
            }

            equivalence_checking(sn[sn.size() - 1], sn[sn.size() - 2]);
        }
        else if (is_set("gate_layout"))
        {
            auto& sg = store<fiction::gate_layout_t>();

            if (sg.size() < 2)
            {
                env->out() << "[w] need at least two gate layouts in store" << std::endl;
                return;
            }

            equivalence_checking(sg[sg.size() - 1], sg[sg.size() - 2]);
        }
        else
        {
            env->out() << "[w] at least one store must be specified" << std::endl;
        }

        //
        //        env->out() << "[i] the layout is "
        //                   << (result.eq == equivalence_checker::equiv_result::eq_type::NONE ? "NOT" :
        //                       result.eq == equivalence_checker::equiv_result::eq_type::WEAK ? "WEAKLY" :
        //                                                                                       "STRONGLY")
        //                   << " equivalent to its specification";
        //
        //        if (result.eq == equivalence_checker::equiv_result::eq_type::NONE && !result.counter_example.empty())
        //        {
        //            env->out() << " with counter example ";
        //            for (const auto c : result.counter_example) env->out() << c;
        //        }
        //        else if (result.eq == equivalence_checker::equiv_result::eq_type::WEAK)
        //        {
        //            if (!is_set("gate_layout"))
        //                env->out() << " with a delay of " << result.delay << " cycles";
        //        }
        //        env->out() << std::endl;
    }

    /**
     * Logs the resulting information in a log file.
     *
     * @return JSON object containing information about the equivalence checking process.
     */
    nlohmann::json log() const override
    {
        return nlohmann::json{{"equivalence type", result.eq == equiv_result::eq_type::NO   ? "NOT EQ" :
                                                   result.eq == equiv_result::eq_type::WEAK ? "WEAK" :
                                                                                              "STRONG"},
                              {"counter example", result.counter_example},
                              {"delay", result.delay},
                              {"runtime (s)", mockturtle::to_seconds(result.runtime)}};
    }

  private:
    /**
     * Encapsulates the equivalence call information.
     */
    struct equiv_result
    {
        /**
         * The different equivalence types possible. See above for more information.
         */
        enum class eq_type
        {
            NO,
            WEAK,
            STRONG
        };
        /**
         * Stores the equivalence type.
         */
        eq_type eq;
        /**
         * Delay value at which weak equivalence manifests.
         */
        std::size_t delay = 0ul;
        /**
         * Stores a possible counter example.
         */
        std::vector<bool> counter_example{};
        /**
         * Stores the runtime.
         */
        mockturtle::stopwatch<>::duration runtime{0};
    };

    equiv_result result{};

    template <typename Ntk1, typename Ntk2>
    void equivalence_checking(const Ntk1& ntk1, const Ntk2& ntk2)
    {
        const auto equiv_check = [this](auto&& net_or_lyt1, auto&& net_or_lyt2) -> bool
        {
            mockturtle::stopwatch stop{result.runtime};

            auto miter = mockturtle::miter<mockturtle::klut_network>(*net_or_lyt1, *net_or_lyt2);

            if (miter)
            {
                mockturtle::equivalence_checking_stats st;

                auto eq = mockturtle::equivalence_checking(*miter, {}, &st);

                if (eq.has_value())
                {
                    result.eq = *eq ? equiv_result::eq_type::STRONG : equiv_result::eq_type::NO;

                    if (!(*eq))
                        result.counter_example = st.counter_example;
                }
                else
                {
                    env->out() << "[e] resource limit exceeded" << std::endl;
                    return false;
                }
            }
            else
            {
                env->out() << "[w] both networks/layouts must have the same number of primary inputs and outputs"
                           << std::endl;
                return false;
            }

            return true;
        };

        const auto get_name = [](auto&& net_or_lyt) { return net_or_lyt->get_network_name(); };

        bool success = std::visit(equiv_check, ntk1, ntk2);

        if (success)
        {
            env->out() << fmt::format("[i] {} and {} are{} equivalent", std::visit(get_name, ntk1),
                                      std::visit(get_name, ntk2), result.eq == equiv_result::eq_type::NO ? " NOT" : "")
                       << std::endl;
        }
    }
};

ALICE_ADD_COMMAND(equiv, "Verification")

}  // namespace alice

#endif  // FICTION_EQUIV_HPP
