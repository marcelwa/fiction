//
// Created by marcel on 09.04.20.
//

#if MUGEN

#ifndef FICTION_CMD_ONEPASS_HPP
#define FICTION_CMD_ONEPASS_HPP

#include "../../algo/one_pass_synthesis.h"
#include "fcn_clocking_scheme.h"
#include "fcn_gate_layout.h"

#include <alice/alice.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <nlohmann/json.hpp>

namespace alice
{
/**
 * Executes a SAT-driven topology-based logic re-synthesis, i.e., a one-pass synthesis. Utilizes the synthesis tool
 * Mugen by Winston Haaswijk.
 * See algo/one_pass_synthesis.h for more details.
 */
class onepass_command : public command
{
  public:
    /**
     * Standard constructor. Adds descriptive information, options, and flags.
     *
     * @param e alice::environment that specifies stores etc.
     */
    explicit onepass_command(const environment::ptr& e) :
            command(e, "SAT-driven topology-based logic re-synthesis, i.e., one-pass synthesis. Uses "
                       "Mugen by Winston Haaswijk to synthesize a specification in terms of a truth table or a "
                       "logic network onto a given clocking scheme. Gate types to be used can be specified. If "
                       "none are given, all are enabled, because synthesis without gates cannot work. Layouts "
                       "resulting from this approach might be desynchronized. I/Os are always located at the "
                       "layout's borders.")
    {
        add_option("--clk_scheme,-s", clocking, "Clocking scheme to use {2DDWAVE[3|4], USE, RES, BANCS}", true);
        add_option("--upper_bound,-u", config.upper_bound, "Number of FCN gate tiles to use at maximum");
        add_option("--fixed_size,-f", config.fixed_size, "Execute only one iteration with the given number of tiles");
        add_option("--timeout,-t", config.timeout, "Timeout in seconds");
        add_option("--async,-a", config.num_threads, "Number of threads to use for parallel solving");

        add_flag("--async_max", "Use the maximum number of threads available to the system");
        add_flag("--network,-n", "Re-synthesize the current logic network in store instead of the current truth table");
        add_flag("--and,-A", config.enable_and, "Enable the use of AND gates");
        add_flag("--or,-O", config.enable_or, "Enable the use of OR gates");
        add_flag("--not,-N", config.enable_not, "Enable the use of NOT gates");
        add_flag("--maj,-M", config.enable_maj, "Enable the use of MAJ gates");
        add_flag("--wires,-W", config.enable_wires, "Enable the use of wire segments and fan-outs");
        add_flag("--crossings,-x", config.crossings, "Enable wire crossings");
        add_flag("--io_ports,-i", config.io_ports, "Use I/O port elements instead of gate pins");
    }

  protected:
    /**
     * Function to perform the physical design call.
     * Given arguments are parsed and a placed and routed FCN gate layout is generated if possible.
     */
    void execute() override
    {
        if (this->is_set("network"))
        {
            auto& s = store<logic_network_ptr>();

            // error case: empty logic network store
            if (s.empty())
            {
                env->out() << "[w] no logic network in store" << std::endl;
                reset_flags();
                return;
            }

            auto ln     = s.current();
            config.name = ln->get_name();

            try
            {
                spec = ln->simulate();
            }
            catch (const std::bad_alloc&)
            {
                env->out() << "[e] " << ln->get_name() << " has too many inputs to store its truth table" << std::endl;
                return;
            }
        }
        else
        {
            auto& s = store<kitty::dynamic_truth_table>();

            // error case: empty truth table store
            if (s.empty())
            {
                env->out() << "[w] no truth table in store" << std::endl;
                reset_flags();
                return;
            }

            auto tt     = s.current();
            config.name = kitty::to_hex(tt);
            spec        = {tt};
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

        // if no gate types are specified, enable them all
        if (!config.enable_and && !config.enable_or && !config.enable_not && !config.enable_maj && !config.enable_wires)
        {
            config.enable_and   = true;
            config.enable_or    = true;
            config.enable_not   = true;
            config.enable_maj   = true;
            config.enable_wires = true;
        }

        // choose clocking
        if (auto clk = get_clocking_scheme(clocking))
        {
            if (auto name = clk->name;
                name == "OPEN3" || name == "OPEN4" || name == "TOPOLINANO3" || name == "TOPOLINANO4")
            {
                env->out() << "[e] the \"" << name << "\" clocking scheme is not supported by this approach"
                           << std::endl;
                reset_flags();
                return;
            }

            config.scheme = std::make_shared<fcn_clocking_scheme>(*clk);
            if (config.scheme->name != "RES" && config.enable_maj)
            {
                config.enable_maj = false;
                env->out() << "[w] disabling MAJ gates as they are not supported by the " << config.scheme->name
                           << " clocking scheme" << std::endl;
            }
        }
        else
        {
            env->out() << "[e] \"" << clocking << "\" does not refer to a supported clocking scheme" << std::endl;
            reset_flags();
            return;
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

        // scope to destruct the one_pass_synthesis object to end Python interpreter's lifetime
        {
            auto print_name = config.name;
            // perform one-pass synthesis
            one_pass_synthesis physical_design{std::move(spec), std::move(config)};

            if (auto result = physical_design(); result.success)
            {
                store<fcn_gate_layout_ptr>().extend() = physical_design.get_layout();
                pd_result                             = result.json;
            }
            else
            {
                env->out() << "[e] impossible to synthesize " << print_name << " within the given parameters"
                           << std::endl;
            }
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
    onepass_pd_config config{};
    /**
     * Identifier of clocking scheme to use.
     */
    std::string clocking = "2DDWave";
    /**
     * Specification to synthesize.
     */
    std::vector<kitty::dynamic_truth_table> spec;
    /**
     * Resulting logging information.
     */
    nlohmann::json pd_result;

    /**
     * Reset all flags. Necessary for some reason... alice bug?
     */
    void reset_flags()
    {
        config   = onepass_pd_config{};
        clocking = "2DDWave";
    }
};

ALICE_ADD_COMMAND(onepass, "Physical Design")
}  // namespace alice

#endif  // FICTION_CMD_ONEPASS_HPP

#endif  // MUGEN
