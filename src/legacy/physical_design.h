//
// Created by marcel on 12.07.17.
//

#ifndef FICTION_PHYSICAL_DESIGN_H
#define FICTION_PHYSICAL_DESIGN_H

#include "fcn_gate_layout.h"
#include "mockturtle/utils/progress_bar.hpp"
#include "mockturtle/utils/stopwatch.hpp"
#include "network_hierarchy.hpp"
#include "nlohmann/json.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/functional/hash/hash.hpp>
#include <boost/predef/os/windows.h>
#include <itertools.hpp>

/**
 * Abstract class that specifies an interface for physical design algorithms.
 */
class physical_design
{

protected:
    /**
     * Result type for physical design approaches.
     */
    struct pd_result
    {
        bool success = false;
        nlohmann::json json;
    };

public:
    /**
     * Default constructor for cases in which no logic network shall be passed.
     */
    physical_design() = default;
    /**
     * Standard constructor.
     *
     * @param ln Logic network.
     */
    explicit physical_design(std::shared_ptr<logic_network>&& ln)
            :
            network{std::make_shared<logic_network>(*ln)}
    {}
    /**
     * Starts a physical design process. This function need to be overridden by each sub-class.
     * It returns a pd_result from which one can extract statistical information about the process.
     *
     * @return Result type containing statistical information.
     */
    virtual pd_result operator()() = 0;
    /**
     * Returns the stored layout.
     *
     * @return The stored layout.
     */
    fcn_gate_layout_ptr get_layout() noexcept
    {
        return layout;
    }
    /**
     * Returns the stored logic network.
     *
     * @return The stored logic network.
     */
    logic_network_ptr get_logic_network() noexcept
    {
        return network;
    }

protected:
    /**
     * Logic network to be mapped to a layout.
     */
    logic_network_ptr network = nullptr;
    /**
     * Actual layout to which a logic network should be mapped.
     */
    fcn_gate_layout_ptr layout = nullptr;
};


#endif //FICTION_PHYSICAL_DESIGN_H
