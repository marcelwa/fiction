//
// Created by marcel on 24.10.19.
//

#ifndef FICTION_CHECK_HPP
#define FICTION_CHECK_HPP

#include "design_checker.h"
#include "fcn_gate_layout.h"

#include <alice/alice.hpp>
#include <nlohmann/json.hpp>

namespace alice
{
/**
 * Performs design rule checks on the active gate layout. Checks for various design rule validations like crossing
 * gates, too many wires in a tile, wrongly assigned directions, etc.
 * See algo/design_checker.h for more details.
 */
class check_command : public command
{
  public:
    /**
     * Standard constructor. Adds descriptive information, options, and flags.
     *
     * @param e alice::environment that specifies stores etc.
     */
    explicit check_command(const environment::ptr& e) :
            command(e, "Performs various design rule checks on the current gate layout in store. "
                       "A full report can be logged and a summary is printed to standard output.")
    {
        add_option("--wire_limit,-w", wire_limit, "Maximum number of wires allowed per tile", true);
    }

  protected:
    /**
     * Function to perform the design rule check call. Generates a report and prints a summary.
     */
    void execute() override
    {
        auto& s = store<fcn_gate_layout_ptr>();

        // error case: empty logic network store
        if (s.empty())
        {
            env->out() << "[w] no gate layout in store" << std::endl;
            reset_flags();
            return;
        }

        design_checker c{s.current(), wire_limit};
        report = c.check(env->out());

        reset_flags();
    }

    /**
     * Logs the resulting information in a log file.
     *
     * @return JSON object containing information about the process.
     */
    nlohmann::json log() const override
    {
        return report;
    }

  private:
    /**
     * Resulting logging information.
     */
    nlohmann::json report;
    /**
     * Maximum number of wires per tile.
     */
    std::size_t wire_limit = 1;

    /**
     * Reset all flags. Necessary for some reason... alice bug?
     */
    void reset_flags()
    {
        wire_limit = 1;
    }
};

ALICE_ADD_COMMAND(check, "Verification")
}  // namespace alice

#endif  // FICTION_CHECK_HPP
