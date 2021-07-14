//
// Created by marcel on 24.10.19.
//

#ifndef FICTION_ORTHO_HPP
#define FICTION_ORTHO_HPP

#include <fiction/algorithms/orthogonal.hpp>
#include <fiction/traits.hpp>

#include <alice/alice.hpp>
#include <nlohmann/json.hpp>

#include <memory>

namespace alice
{
/**
 * Executes a heuristic physical approach based on "Improved Orthogonal Drawing of 3-graphs" by Therese Biedl, 1996.
 * See algo/orthogonal.hpp for more details.
 */
class ortho_command : public command
{
  public:
    /**
     * Standard constructor. Adds descriptive information, options, and flags.
     *
     * @param e alice::environment that specifies stores etc.
     */
    explicit ortho_command(const environment::ptr& e) :
            command(e, "Performs scalable placement and routing of the current logic network in store. "
                       "An FCN layout that is not minimal will be found in reasonable runtime.")
    {
        add_option("--clock_numbers,-n", ps.number_of_clock_phases, "Number of clock phases to be used {3 or 4}");
        add_flag("--verbose,-v", ps.verbose, "Be verbose");
    }

  protected:
    /**
     * Function to perform the physical design call. Generates a placed and routed FCN gate layout.
     */
    void execute() override
    {
        auto& s = store<fiction::logic_network_t>();

        // error case: empty logic network store
        if (s.empty())
        {
            env->out() << "[w] no logic network in store" << std::endl;
            ps = {};
            return;
        }
        // error case: phases out of range
        if (ps.number_of_clock_phases != 3u && ps.number_of_clock_phases != 4u)
        {
            env->out() << "[e] only 3- and 4-phase clocking schemes are supported" << std::endl;
            ps = {};
            return;
        }

        const auto get_name = [](auto&& net) -> std::string { return net->get_network_name(); };

        const auto orthogonal_physical_design = [this](auto&& net)
        {
            using gate_layout = fiction::gate_level_layout<fiction::clocked_layout<fiction::tile_based_layout>>;

            return fiction::orthogonal<gate_layout>(*net, ps, &st);
        };

        const auto& net = s.current();

        try
        {
            auto lyt = std::visit(orthogonal_physical_design, net);
            store<fiction::gate_layout_t>().extend() =
                std::make_shared<fiction::gate_clk_lyt>(lyt, std::visit(get_name, net));
        }
        catch (const fiction::high_degree_fanin_exception& e)
        {
            env->out() << fmt::format("[e] {}", e.what()) << std::endl;
        }

        ps = {};
    }

    /**
     * Logs the resulting information in a log file.
     *
     * @return JSON object containing information about the physical design process.
     */
    nlohmann::json log() const override
    {
        return nlohmann::json{
            {"runtime in seconds", mockturtle::to_seconds(st.time_total)},
            {"number of gates", st.num_gates},
            {"number of wires", st.num_wires},
            {"layout", {{"x-size", st.x_size}, {"y-size", st.y_size}, {"area", st.x_size * st.y_size}}}};
    }

  private:
    /**
     * Parameters.
     */
    fiction::orthogonal_physical_design_params ps{};
    /**
     * Statistics.
     */
    fiction::orthogonal_physical_design_stats st{};
};

ALICE_ADD_COMMAND(ortho, "Physical Design")

}  // namespace alice

#endif  // FICTION_ORTHO_HPP
