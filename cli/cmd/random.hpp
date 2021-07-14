//
// Created by marcel on 08.11.19.
//

#ifndef FICTION_RANDOM_HPP
#define FICTION_RANDOM_HPP

#include <fiction/types.hpp>

#include <alice/alice.hpp>
#include <mockturtle/generators/random_logic_generator.hpp>

#include <memory>
#include <random>
#include <string>

namespace alice
{
/**
 * Generates a random logic network.
 */
class random_command : public command
{
  public:
    /**
     * Standard constructor. Adds descriptive information, options, and flags.
     *
     * @param e alice::environment that specifies stores etc.
     */
    explicit random_command(const environment::ptr& e) :
            command(e,
                    "Generates a random logic network. The random seed will be used as its name for reproducibility.")
    {
        add_flag("--aig,-a", "Create random AIG network");
        add_flag("--mig,-m", "Create random MIG network");
        add_option("--rnd_num_inp,-n", num_inp, "Number of primary inputs", true);
        add_option("--rnd_num_gates,-g", num_gates, "Number of gates (excluding inverters and fan-outs)", true);
        add_option("--rnd_seed,-s", seed, "Random seed");
    }

  protected:
    /**
     * Function to perform the synth call. Generates a logic network from a truth table.
     */
    void execute() override
    {
        if (!is_set("aig") && !is_set("mig") && !is_set("top"))
        {
            env->out() << "[e] at least one network type must be specified" << std::endl;
        }
        else
        {
            if (is_set("aig"))
            {
                generate<fiction::aig_nt, false>();
            }
            if (is_set("mig"))
            {
                generate<fiction::mig_nt, true>();
            }
            if (is_set("top"))
            {
                generate<fiction::top_nt, true>();
            }
        }

        reset_flags();
    }

  private:
    /**
     * Number of primary inputs to generate.
     */
    uint32_t num_inp = 4;
    /**
     * Number of gates to generate excluding inverters and fan-outs.
     */
    uint32_t num_gates = 8;
    /**
     * Random seed.
     */
    uint64_t seed = static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count());

    /**
     * Generates rules for the logic generator. Can in- and exclude Majority gates.
     *
     * Parts of this code are originally from:
     * https://github.com/lsils/mockturtle/blob/master/include/mockturtle/generators/random_logic_generator.hpp
     *
     * @tparam Ntk Network type to use.
     * @tparam Maj Flag to indicate that Majority gates should be used.
     * @return A random logic generator with a generated set of rules.
     */
    template <typename Ntk, bool Maj>
    mockturtle::random_logic_generator<Ntk> random_fcn_logic_generator() const noexcept
    {
        using gen_t = mockturtle::random_logic_generator<Ntk>;

        typename gen_t::rules_t gen_rules;
        if constexpr (Maj)
        {
            gen_rules.emplace_back(typename gen_t::rule{[](Ntk& ntk, std::vector<typename Ntk::signal> const& vs)
                                                        {
                                                            assert(vs.size() == 3u);
                                                            return ntk.create_maj(vs[0], vs[1], vs[2]);
                                                        },
                                                        3u});
        }
        gen_rules.emplace_back(typename gen_t::rule{[](Ntk& ntk, std::vector<typename Ntk::signal> const& vs)
                                                    {
                                                        assert(vs.size() == 2u);
                                                        return ntk.create_and(vs[0], vs[1]);
                                                    },
                                                    2u});
        gen_rules.emplace_back(typename gen_t::rule{[](Ntk& ntk, std::vector<typename Ntk::signal> const& vs)
                                                    {
                                                        assert(vs.size() == 2u);
                                                        return ntk.create_or(vs[0], vs[1]);
                                                    },
                                                    2u});

        return mockturtle::random_logic_generator<Ntk>(gen_rules);
    }
    /**
     * The actual network generator. Builds a logic_network from the generated random logic.
     *
     * @tparam Ntk Network type to generate.
     */
    template <typename Ntk, bool Maj>
    void generate() const noexcept
    {
        store<fiction::logic_network_t>().extend() =
            std::make_shared<Ntk>(mockturtle::random_logic_generator<Ntk>(random_fcn_logic_generator<Ntk, Maj>())
                                      .generate(num_inp, num_gates, seed),
                                  std::to_string(seed));
    }
    /**
     * Reset all flags, necessary for some reason... alice bug?
     */
    void reset_flags()
    {
        num_inp   = 4;
        num_gates = 8;
        seed      = static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count());
    }
};

ALICE_ADD_COMMAND(random, "Logic")

}  // namespace alice

#endif  // FICTION_RANDOM_HPP
