//
// Created by marcel on 08.06.21.
//

#ifndef FICTION_MAP_HPP
#define FICTION_MAP_HPP

#include <fiction/algorithms/name_restoration.hpp>
#include <fiction/algorithms/network_conversion.hpp>
#include <fiction/networks/topology_network.hpp>
#include <fiction/technology/technology_mapping_library.hpp>
#include <fiction/types.hpp>

#include <alice/alice.hpp>
#include <kitty/print.hpp>
#include <lorina/genlib.hpp>
#include <mockturtle/algorithms/akers_synthesis.hpp>
#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/mapper.hpp>
#include <mockturtle/io/genlib_reader.hpp>
#include <mockturtle/networks/klut.hpp>
#include <mockturtle/utils/tech_library.hpp>

#include <memory>
#include <sstream>
#include <variant>
#include <vector>

namespace alice
{
/**
 * Performs technology mapping to transform a network into another one using specific gates.
 */
class map_command : public command
{
  public:
    /**
     * Standard constructor. Adds descriptive information, options, and flags.
     *
     * @param e alice::environment that specifies stores etc.
     */
    explicit map_command(const environment::ptr& e) :
            command(e, "Performs technology mapping to transform a network into another one using specific gates.")
    {
        add_flag("--and,-a", "Enable the use of AND gates");
        add_flag("--or,-o", "Enable the use of OR gates");
        add_flag("--xor,-x", "Enable the use of XOR gates");
        add_flag("--inv,-i", "Enable the use of NOT gates");
        add_flag("--maj,-m", "Enable the use of MAJ gates");
        add_flag("--dot,-d", "Enable the use of DOT gates");

        add_flag("--and3", "Enable the use of AND3 gates");
        add_flag("--xor_and", "Enable the use of XOR-AND gates");
        add_flag("--or_and", "Enable the use of OR-AND gates");
        add_flag("--onehot", "Enable the use of ONEHOT gates");
        add_flag("--gamble", "Enable the use of GAMBLE gates");
        add_flag("--mux", "Enable the use of MUX gates");
        add_flag("--and_xor", "Enable the use of AND-XOR gates");

        add_flag("--all2", "Enable the use of all supported 2-input gates + inverters");
        add_flag("--all3", "Enable the use of all supported 3-input gates + inverters");
        add_flag("--all", "Enable the use of all supported gates");

        add_flag("--decay", "Enforce the application of at least one constant input to three-input gates");
        add_flag("--logic_sharing,-s", ps.enable_logic_sharing, "Enable logic sharing optimization");
        add_flag("--verbose,-v", ps.verbose, "Be verbose");
    }

  protected:
    /**
     * Function to perform the map call. Generates a logic network from another one.
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

        // gather library description
        std::stringstream library_stream{};

        library_stream << fiction::GATE_ZERO << fiction::GATE_ONE << fiction::GATE_BUF;

        if (is_set("and") || is_set("all2") || is_set("all"))
        {
            library_stream << fiction::GATE_AND2;
        }
        if (is_set("or") || is_set("all2") || is_set("all"))
        {
            library_stream << fiction::GATE_OR2;
        }
        if (is_set("xor") || is_set("all2") || is_set("all"))
        {
            library_stream << fiction::GATE_XOR2;
        }
        if (is_set("inv") || is_set("all2") || is_set("all3") || is_set("all"))
        {
            library_stream << fiction::GATE_INV;
        }
        if (is_set("maj") || is_set("all3") || is_set("all"))
        {
            if (!is_set("decay"))
                library_stream << fiction::GATE_MAJ3;

            library_stream << fiction::DECAY_MAJ3;
        }
        if (is_set("dot") || is_set("all3") || is_set("all"))
        {
            if (!is_set("decay"))
                library_stream << fiction::GATE_DOT;

            library_stream << fiction::DECAY_DOT;
        }
        if (is_set("and3") || is_set("all3") || is_set("all"))
        {
            if (!is_set("decay"))
                library_stream << fiction::GATE_AND3;

            library_stream << fiction::DECAY_AND3;
        }
        if (is_set("xor_and") || is_set("all3") || is_set("all"))
        {
            if (!is_set("decay"))
                library_stream << fiction::GATE_XOR_AND;

            library_stream << fiction::DECAY_XOR_AND;
        }
        if (is_set("or_and") || is_set("all3") || is_set("all"))
        {
            if (!is_set("decay"))
                library_stream << fiction::GATE_OR_AND;

            library_stream << fiction::DECAY_OR_AND;
        }
        if (is_set("onehot") || is_set("all3") || is_set("all"))
        {
            if (!is_set("decay"))
                library_stream << fiction::GATE_ONEHOT;

            library_stream << fiction::DECAY_ONEHOT;
        }
        if (is_set("gamble") || is_set("all3") || is_set("all"))
        {
            if (!is_set("decay"))
                library_stream << fiction::GATE_GAMBLE;

            library_stream << fiction::DECAY_GAMBLE;
        }
        if (is_set("mux") || is_set("all3") || is_set("all"))
        {
            if (!is_set("decay"))
                library_stream << fiction::GATE_MUX;

            library_stream << fiction::DECAY_MUX;
        }
        if (is_set("and_xor") || is_set("all3") || is_set("all"))
        {
            if (!is_set("decay"))
                library_stream << fiction::GATE_AND_XOR;

            library_stream << fiction::DECAY_AND_XOR;
        }

        // generate technology library
        std::vector<mockturtle::gate> gates{};

        auto result = lorina::read_genlib(library_stream, mockturtle::genlib_reader{gates});

        if (result == lorina::return_code::success)
        {
            if (is_set("maj") || is_set("dot") || is_set("and3") || is_set("xor_and") || is_set("or_and") ||
                is_set("onehot") || is_set("gamble") || is_set("mux") || is_set("and_xor") || is_set("all3") ||
                is_set("all"))
            {
                synthesize_and_store<3>(gates);
            }
            else if (is_set("and") || is_set("or") || is_set("xor") || is_set("all2"))
            {
                synthesize_and_store<2>(gates);
            }
            else if (is_set("inv"))
            {
                synthesize_and_store<1>(gates);
            }
            else
            {
                env->out() << "[e] no gates specified; cannot perform technology mapping" << std::endl;
                ps = {};
                return;
            }
        }
        else
        {
            env->out() << "[e] error parsing technology library" << std::endl;
        }

        ps = {};
    }

  private:
    mockturtle::map_params ps{};

    template <unsigned NumInp>
    void synthesize_and_store(const std::vector<mockturtle::gate>& gates)
    {
        auto& s = store<fiction::logic_network_t>();

        mockturtle::tech_library<NumInp> lib{gates};

        const auto mapper = [this, &s, &lib](auto&& net)
        {
            mockturtle::map_stats st{};

            const auto mapped_net = mockturtle::map(*net, lib, ps, &st);

            if (!st.mapping_error)
            {
                // convert network
                auto converted_net = fiction::convert_network<fiction::top_nt>(mapped_net);
                fiction::restore_network_name(*net, converted_net);

                s.extend() = std::make_shared<fiction::top_nt>(converted_net);
            }
        };

        std::visit(mapper, s.current());
    }
};

ALICE_ADD_COMMAND(map, "Logic")

}  // namespace alice

#endif  // FICTION_MAP_HPP
