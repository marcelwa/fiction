//
// Created by marcel on 24.10.19.
//

#ifndef FICTION_BALANCE_HPP
#define FICTION_BALANCE_HPP

#include <fiction/algorithms/network_balancing.hpp>
#include <fiction/types.hpp>

#include <alice/alice.hpp>

#include <variant>

namespace alice
{
/**
 * Creates a new topology network of the current logic network in store and performs a path balancing.
 */
class balance_command : public command
{
  public:
    /**
     * Standard constructor. Adds descriptive information, options, and flags.
     *
     * @param e alice::environment that specifies stores etc.
     */
    explicit balance_command(const environment::ptr& e) :
            command(
                e,
                "Creates a new topology network from the current logic network in store. All paths in the new network "
                "that are leading to any common node will have the same length. This is realized by inserting buffer "
                "nodes. This does not respect possible crossings in the network.")
    {
        add_flag("--unify_outputs,-u", ps.unify_outputs,
                 "Additionally, push all primary outputs down to the same level");
    }

  protected:
    /**
     * Function to perform the balancing call. Inserts buffer nodes to equalize path lengths.
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

        const auto perform_balancing = [this](auto&& net)
        { return std::make_shared<fiction::top_nt>(fiction::network_balancing<fiction::top_nt>(*net, ps)); };

        s.extend() = std::visit(perform_balancing, s.current());

        ps = {};
    }

  private:
    fiction::network_balancing_params ps{};
};

ALICE_ADD_COMMAND(balance, "Logic")

}  // namespace alice

#endif  // FICTION_BALANCE_HPP
