//
// Created by marcel on 24.10.19.
//

#ifndef FICTION_CLEAR_HPP
#define FICTION_CLEAR_HPP

#include <fiction/types.hpp>

#include <alice/alice.hpp>

namespace alice
{
/**
 * Clears all stores by removing all their entries.
 */
class clear_command : public command
{
  public:
    /**
     * Standard constructor. Adds descriptive information, options, and flags.
     *
     * @param e alice::environment that specifies stores etc.
     */
    explicit clear_command(const environment::ptr& e) :
            command(e, "Removes all elements from the respectively flagged stores. If no store is specified, all "
                       "stores will be cleared.")
    {
        //            add_flag("--cell_layout,-c", cell,
        //                     "Clear cell layout store");
        add_flag("--gate_layout,-g", gate, "Clear gate layout store");
        add_flag("--network,-n", network, "Clear logic network store");
        add_flag("--truth_table,-t", table, "Clear truth table store");
    }

  protected:
    /**
     * Function to perform the clear call. Calls alice's clear function for all stores.
     */
    void execute() override
    {
        // if no store was specified, clear all.
        if (!cell && !gate && !network && !table)
        {
            cell    = true;
            gate    = true;
            network = true;
            table   = true;
        }

        //            if (cell)
        //                store<fiction::cell_layout_t>().clear();
        if (gate)
            store<fiction::gate_layout_t>().clear();
        if (network)
            store<fiction::logic_network_t>().clear();
        if (table)
            store<fiction::truth_table_t>().clear();

        // reset flags, necessary for some reason... alice bug?
        cell    = false;
        gate    = false;
        network = false;
        table   = false;
    }

  private:
    /**
     * Stores to clear.
     */
    bool cell = false, gate = false, network = false, table = false;
};

ALICE_ADD_COMMAND(clear, "General")

}  // namespace alice

#endif  // FICTION_CLEAR_HPP
