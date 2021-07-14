//
// Created by marcel on 24.10.19.
//

#ifndef FICTION_TT_HPP
#define FICTION_TT_HPP

#include <fiction/types.hpp>

#include <alice/alice.hpp>
#include <kitty/constructors.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <memory>
#include <string>

namespace alice
{
/**
 * Command to create a truth table either from a binary string, a hex string, or an expression.
 *
 * Parts of this code are originally from: https://github.com/msoeken/cirkit/blob/cirkit3/cli/algorithms/tt.hpp.
 */
class tt_command : public command
{
  public:
    /**
     * Standard constructor. Adds descriptive information, options, and flags.
     *
     * @param e alice::environment that specifies stores etc.
     */
    explicit tt_command(const environment::ptr& e) :
            command(e, "Creates a truth table from the given hex or binary string or from an expression. "
                       "An expression E is a constant 0 or 1, or a variable a, b, ..., p, the negation of an "
                       "expression !E, the conjunction of multiple expressions (E...E), the disjunction of "
                       "multiple expressions {E...E}, the exclusive OR of multiple expressions [E...E], or the "
                       "majority of three expressions <EEE>. Examples are [(ab)(!ac)] to describe if-then-else, "
                       "or !{!a!b} to describe the application of De Morgan’s law to (ab). The size of the truth "
                       "table must fit the largest variable in the expression, e.g., if c is the largest "
                       "variable, then the truth table have at least three variables.")
    {
        add_option("table,--table", table, "Truth table (prefix with 0x to parse as hexadecimal)");
        add_option("--expression,-e", expression, "Creates truth table from expression");
        add_option("--random,-r", random_vars, "Creates a random truth table over <INPUT> variables");
    }

  protected:
    /**
     * Function to perform the read call. Reads Verilog and creates a logic_network.
     */
    void execute() override
    {
        if (is_set("table") && is_set("expression"))
        {
            env->out() << "[w] 'table' and 'expression' cannot be set at the same time" << std::endl;
            reset_flags();
            return;
        }
        if (is_set("table") && is_set("random"))
        {
            env->out() << "[w] 'table' and 'random' cannot be set at the same time" << std::endl;
            reset_flags();
            return;
        }
        if (is_set("expression") && is_set("random"))
        {
            env->out() << "[w] 'expression' and 'random' cannot be set at the same time" << std::endl;
            reset_flags();
            return;
        }

        auto& s = store<fiction::truth_table_t>();

        if (is_set("table"))
        {
            if (auto size = table.size(); size > 2u && table[0] == '0' && table[1] == 'x')
            {
                table = table.substr(2u);
                if (is_hex_string(table))
                {
                    auto num_vars = static_cast<uint32_t>(std::log2(table.size() << 2ul));

                    fiction::tt tt(num_vars);
                    kitty::create_from_hex_string(tt, table);

                    s.extend() = std::make_shared<fiction::tt>(tt);
                }
                else
                {
                    env->out() << "[e] " << table << " is not a hex string" << std::endl;
                }
            }
            else if ((size & (size - 1)) == 0)  // size is power of 2
            {
                if (is_bin_string(table))
                {
                    auto num_vars = static_cast<uint32_t>(std::log2(table.size()));

                    fiction::tt tt(num_vars);
                    kitty::create_from_binary_string(tt, table);

                    s.extend() = std::make_shared<fiction::tt>(tt);
                }
                else
                {
                    env->out() << "[e] " << table << " is not a binary string" << std::endl;
                }
            }
            else
            {
                env->out() << "[e] number of bits is not a power of 2" << std::endl;
            }
        }
        else if (is_set("expression"))
        {
            /* find max var */
            uint32_t num_vars{0u};
            for (auto c : expression)
            {
                if (c >= 'a' && c <= 'p')
                {
                    num_vars = std::max<uint32_t>(num_vars, static_cast<uint32_t>(c - 'a') + 1u);
                }
            }

            fiction::tt tt(num_vars);
            if (kitty::create_from_expression(tt, expression))
            {
                s.extend() = std::make_shared<fiction::tt>(tt);
            }
        }
        else if (is_set("random"))
        {
            fiction::tt tt{random_vars};
            kitty::create_random(tt);

            s.extend() = std::make_shared<fiction::tt>(tt);
        }
        else
        {
            env->out() << "[w] no truth table generation approach specified" << std::endl;
        }

        reset_flags();
    }

  private:
    /**
     * String representing the truth table.
     */
    std::string table;
    /**
     * String representing the expression.
     */
    std::string expression;
    /**
     * Number of variables to create a random string for.
     */
    unsigned random_vars = 0;

    /**
     * Reset all flags, necessary for some reason... alice bug?
     */
    void reset_flags()
    {
        table       = "";
        expression  = "";
        random_vars = 0;
    }
    /**
     * Checks whether a passed string contains hex symbols only.
     *
     * @param s String to check.
     * @return True iff s contains hex symbols only.
     */
    static bool is_hex_string(const std::string& s) noexcept
    {
        return std::all_of(s.begin(), s.end(), [](const auto c) { return std::isxdigit(c); });
    }
    /**
     * Checks whether a passed string contains binary symbols only.
     *
     * @param s String to check.
     * @return True iff s contains binary symbols only.
     */
    static bool is_bin_string(const std::string& s) noexcept
    {
        return std::all_of(s.begin(), s.end(), [](const auto c) { return c == '0' || c == '1'; });
    }
};

ALICE_ADD_COMMAND(tt, "I/O")

}  // namespace alice

#endif  // FICTION_TT_HPP
