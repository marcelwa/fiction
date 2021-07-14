//
// Created by marcel on 06.01.20.
//

#ifndef FICTION_CMD_EXACT_HPP
#define FICTION_CMD_EXACT_HPP

#include "../../algo/exact.h"
#include "fcn_clocking_scheme.h"
#include "fcn_gate_layout.h"
#include "logic_network.h"

#include <alice/alice.hpp>
#include <nlohmann/json.hpp>

#include <thread>

namespace alice
{
/**
 * Executes an exact physical design approach utilizing the SMT solver Z3.
 * See algo/exact.h for more details.
 */
class exact_command : public command
{
  public:
    /**
     * Standard constructor. Adds descriptive information, options, and flags.
     *
     * @param e alice::environment that specifies stores etc.
     */
    explicit exact_command(const environment::ptr& e) :
            command(e, "Performs exact placement and routing of the current logic network in store. "
                       "A minimum FCN layout will be found that meets all given constraints.")
    {
        add_option("--clk_scheme,-s", clocking,
                   "Clocking scheme to use {OPEN[3|4], 2DDWAVE[3|4], USE, RES, BANCS, TOPOLINANO[3|4]}", true);
        add_option("--upper_bound,-u", config.upper_bound, "Number of FCN gate tiles to use at maximum");
        add_option("--fixed_size,-f", config.fixed_size, "Execute only one iteration with the given number of tiles");
        add_option("--timeout,-t", config.timeout, "Timeout in seconds");
        add_option("--async,-a", config.num_threads, "Number of layout dimensions to examine in parallel");

        add_flag("--async_max,", "Examine as many layout dimensions in parallel as threads are available");
        add_flag("--crossings,-x", config.crossings, "Enable wire crossings");
        add_flag("--io_ports,-i", config.io_ports, "Use I/O port elements instead of gate pins");
        add_flag("--border_io,-b", config.border_io, "Enforce primary I/O to be placed at the layout's borders");
        add_flag("--straight_inverters,-n", config.straight_inverters,
                 "Enforce NOT gates to be routed non-bending only");
        add_flag("--desynchronize,-d", config.desynchronize,
                 "Do not enforce fan-in path balancing (reduces area and throughput, speeds up runtime)");
        add_flag("--minimize_wires,-w", config.minimize_wires,
                 "Minimize the number of wire tiles to be used (slightly runtime expensive)");
        add_flag("--minimize_crossings,-c", config.minimize_crossings,
                 "Minimize the number of crossing tiles to be used (slightly runtime expensive)");
        add_flag("--clock_latches,-l", config.clock_latches,
                 "Allow clock latches to satisfy global synchronization (runtime expensive!)");
    }

  protected:
    /**
     * Function to perform the physical design call.
     * Given arguments are parsed and a placed and routed FCN gate layout is generated if possible.
     */
    void execute() override
    {
        auto& s = store<logic_network_ptr>();

        // error case: empty logic network store
        if (s.empty())
        {
            env->out() << "[w] no logic network in store" << std::endl;
            reset_flags();
            return;
        }

        // error case: -f and -u are both set
        if (this->is_set("fixed_size") && this->is_set("upper_bound"))
        {
            env->out() << "[e] -u and -f cannot be set together" << std::endl;
            reset_flags();
            return;
        }
        // set the value of fixed_size as the upper bound if set
        else if (this->is_set("fixed_size"))
        {
            config.upper_bound = config.fixed_size;
        }

        // choose clocking
        if (auto clk = get_clocking_scheme(clocking))
        {
            config.scheme = std::make_shared<fcn_clocking_scheme>(*clk);
        }
        else
        {
            env->out() << "[e] \"" << clocking << "\" does not refer to a supported clocking scheme" << std::endl;
            reset_flags();
            return;
        }
        // if clocking is a ToPoliNano one, set a respective flag
        if (config.scheme->name == "TOPOLINANO3" || config.scheme->name == "TOPOLINANO4")
        {
            config.topolinano      = true;
            config.vertical_offset = true;
        }
        // if clocking is a 2DDwave one, set a respective flag
        else if (config.scheme->name == "2DDWAVE3" || config.scheme->name == "2DDWAVE4")
        {
            config.twoddwave = true;
        }

        // fetch number of threads available on the system
        if (this->is_set("async_max"))
        {
            if (auto threads_available = std::thread::hardware_concurrency(); threads_available == 0)
            {
                env->out() << "[w] could not detect the number of threads available to your system" << std::endl;
            }
            else
            {
                config.num_threads = threads_available;
            }
        }

        // convert timeout entered in seconds to milliseconds
        config.timeout *= 1000;

        // perform exact physical design
        exact physical_design{s.current(), std::move(config)};

        if (auto result = physical_design(); result.success)
        {
            store<fcn_gate_layout_ptr>().extend() = physical_design.get_layout();
            pd_result                             = result.json;
        }
        else
        {
            env->out() << "[e] impossible to place and route " << s.current()->get_name()
                       << " within the given parameters" << std::endl;
        }

        reset_flags();
    }

    /**
     * Logs the resulting information in a log file.
     *
     * @return JSON object containing information about the solving process.
     */
    nlohmann::json log() const override
    {
        return pd_result;
    }

  private:
    /**
     * Configuration object extracted from arguments and flags.
     */
    exact_pd_config config{};
    /**
     * Identifier of clocking scheme to use.
     */
    std::string clocking = "OPEN4";
    /**
     * Resulting logging information.
     */
    nlohmann::json pd_result;

    /**
     * Reset all flags. Necessary for some reason... alice bug?
     */
    void reset_flags()
    {
        config   = exact_pd_config{};
        clocking = "OPEN4";
    }
};

ALICE_ADD_COMMAND(exact, "Physical Design")
}  // namespace alice

#endif  // FICTION_CMD_EXACT_HPP
