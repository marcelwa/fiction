//
// Created by marcel on 24.10.19.
//

#ifndef FICTION_GATES_HPP
#define FICTION_GATES_HPP

#include <fiction/algorithms/count_gate_types.hpp>
#include <fiction/types.hpp>

#include <alice/alice.hpp>

#include <variant>

namespace alice
{
/**
 * Prints numbers of gates for a current logic network.
 */
class gates_command : public command
{
  public:
    /**
     * Standard constructor. Adds descriptive information, options, and flags.
     *
     * @param e alice::environment that specifies stores etc.
     */
    explicit gates_command(const environment::ptr& e) :
            command(e, "Prints the gate count for each different node type "
                       "of the current logic network or gate-level layout in store.")
    {
        add_flag("--gate_layout,-g", "Count gates in current gate-level layout");
        add_flag("--network,-n", "Count gates in current logic network");
        add_flag("--detailed", "Display a detailed listing of all supported gate types");
    }

  protected:
    /**
     * Function to perform the gates call. Outputs gates counts for each node type.
     */
    void execute() override
    {
        if (!is_set("gate_layout") && !is_set("network"))
        {
            env->out() << "[w] at least one store must be specified" << std::endl;
            return;
        }

        const auto count = [this](auto&& ntk_or_lyt)
        {
            fiction::count_gate_types_stats st{};
            fiction::count_gate_types(*ntk_or_lyt, &st);

            st.report(env->out(), is_set("detailed"));
        };

        if (is_set("gate_layout"))
        {
            auto& s = store<fiction::gate_layout_t>();

            // error case: empty gate-level layout store
            if (s.empty())
            {
                env->out() << "[w] no gate layout in store" << std::endl;
                return;
            }

            std::visit(count, s.current());
        }

        if (is_set("network"))
        {
            auto& s = store<fiction::logic_network_t>();

            // error case: empty logic network store
            if (s.empty())
            {
                env->out() << "[w] no logic network in store" << std::endl;
                return;
            }

            std::visit(count, s.current());
        }
    }
};

ALICE_ADD_COMMAND(gates, "Logic")
}  // namespace alice

#endif  // FICTION_GATES_HPP
