//
// Created by marcel on 24.10.19.
//

#ifndef FICTION_FANOUTS_HPP
#define FICTION_FANOUTS_HPP

#include <fiction/algorithms/fanout_substitution.hpp>
#include <fiction/types.hpp>

#include <alice/alice.hpp>

namespace alice
{
/**
 * Substitutes multi-outputs of gate vertices and replaces them with fan-out ones.
 */
class fanouts_command : public command
{
  public:
    /**
     * Standard constructor. Adds descriptive information, options, and flags.
     *
     * @param e alice::environment that specifies stores etc.
     */
    explicit fanouts_command(const environment::ptr& e) :
            command(e, "Substitutes high-degree outputs of gates of the current logic network in store "
                       "and replaces them with fan-out nodes. Some algorithms require fan-out nodes explicitly and "
                       "will break down networks if needed. Others might be able to handle both.")
    {
        add_option("--degree,-d", ps.degree, "Maximum number of outputs a fan-out node can have", true)
            ->set_type_name("{2, 3}");
        add_option("--strategy,-s", ps.strategy,
                   "Chain fan-outs in a balanced tree (breadth) or a DFS tree (depth) fashion", true)
            ->set_type_name("{breadth=0, depth=1}");
        add_option("--threshold,-t", ps.threshold,
                   "Maximum number of outputs any gate can have before substitution applies", true);
    }

  protected:
    /**
     * Function to perform the fanouts call. Substitutes high-degree outputs and replaces them by fan-out vertices.
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

        if (ps.degree < 2 || ps.degree > 3)
        {
            env->out() << "[w] " << ps.degree << " outputs per fan-out are not supported" << std::endl;
            ps = {};
            return;
        }

        if (ps.strategy != 0 && ps.strategy != 1)
        {
            env->out() << "[w] " << ps.strategy << " does not refer to a valid strategy" << std::endl;
            ps = {};
            return;
        }

        const auto perform_substitution = [this](auto&& net)
        { return std::make_shared<fiction::top_nt>(fiction::fanout_substitution<fiction::top_nt>(*net, ps)); };

        s.extend() = std::visit(perform_substitution, s.current());

        ps = {};
    }

  private:
    fiction::fanout_substitution_params ps{};
};

ALICE_ADD_COMMAND(fanouts, "Logic")
}  // namespace alice

#endif  // FICTION_FANOUTS_HPP
