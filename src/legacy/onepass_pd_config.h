//
// Created by marcel on 22.04.20.
//

#ifndef ONEPASS_PD_CONFIGURATION_H
#define ONEPASS_PD_CONFIGURATION_H

#include <string>
#include <memory>
#include <limits>
#include "fcn_clocking_scheme.h"


/**
 * Configuration struct to set up onepass physical design calls.
 */
struct onepass_pd_config
{
    /**
     * Number of tiles to use.
     */
    std::size_t upper_bound = std::numeric_limits<unsigned>::max();
    /**
     * Use just one fixed tile size.
     */
    std::size_t fixed_size = 0ul;
    /**
     * Number of threads to use for exploring the possible dimensions.
     */
    std::size_t num_threads = 1ul;
    /**
     * Enable the use of wire elements.
     */
    bool enable_wires = false;
    /**
     * Enable the use of NOT gates.
     */
    bool enable_not = false;
    /**
     * Enable the use of AND gates.
     */
    bool enable_and = false;
    /**
     * Enable the use of OR gates.
     */
    bool enable_or = false;
    /**
     * Enable the use of MAJ gates.
     */
    bool enable_maj = false;
    /**
     * Flag to indicate that crossings should be used.
     */
    bool crossings = false;
    /**
     * Flag to indicate that designated wires should be routed to balance I/O port paths.
     */
    bool io_ports = false;
    /**
     * Sets a timeout in seconds for the solving process, where 0 allows for unlimited time.
     */
    uint32_t timeout = 0;
    /**
     * Clocking scheme to be used.
     */
    std::shared_ptr<fcn_clocking_scheme> scheme = nullptr;
    /**
     * Name of the resulting network.
     */
    std::string name = "";
};


#endif //ONEPASS_PD_CONFIGURATION_H
