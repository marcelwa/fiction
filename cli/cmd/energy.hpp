//
// Created by marcel on 25.11.19.
//

#ifndef FICTION_ENERGY_HPP
#define FICTION_ENERGY_HPP

#include <fiction/algorithms/energy_model.hpp>
#include <fiction/types.hpp>

#include <alice/alice.hpp>

#include <variant>

namespace alice
{
/**
 * Prints QCA energy dissipation for current gate-level layout. Calculations are based upon
 * 'An Energy-aware Model for the Logic Synthesis of Quantum-Dot Cellular Automata' by Frank Sill Torres, et al.
 * in TCAD 2018.
 */
class energy_command : public command
{
  public:
    /**
     * Standard constructor. Adds descriptive information, options, and flags.
     *
     * @param e alice::environment that specifies stores etc.
     */
    explicit energy_command(const environment::ptr& e) :
            command(e, "Prints the energy dissipation for the current gate-level layout in store that would "
                       "occur if it was compiled using the QCA-ONE library. Calculations are based upon "
                       "'An Energy-aware Model for the Logic Synthesis of Quantum-Dot Cellular Automata' "
                       "by Frank Sill Torres, et al. in TCAD 2018.")
    {}

  protected:
    /**
     * Function to perform the energy call. Prints estimated energy consumption for QCA-ONE library.
     */
    void execute() override
    {
        // reset energy values
        st = {};

        auto& s = store<fiction::gate_layout_t>();

        // error case: empty gate layout store
        if (s.empty())
        {
            env->out() << "[w] no gate layout in store" << std::endl;
            return;
        }

        const auto energy = [this](auto&& lyt) { fiction::qca_energy_dissipation(*lyt, &st); };

        std::visit(energy, s.current());

        st.report(env->out());
    }

  private:
    /**
     * Slow (25 GHz) and fast (100 GHz) energy dissipation values.
     */
    fiction::energy_dissipation_stats st{};
    /**
     * Logs the resulting information in a log file.
     *
     * @return JSON object containing details about the energy dissipation.
     */
    nlohmann::json log() const override
    {
        return {{"energy (meV, QCA)", {{"slow (25 GHz)", st.slow}, {"fast (100 GHz)", st.fast}}}};
    }
};

ALICE_ADD_COMMAND(energy, "Technology")

}  // namespace alice

#endif  // FICTION_ENERGY_HPP
