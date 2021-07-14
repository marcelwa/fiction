//
// Created by marcel on 10.04.17.
//

#ifndef EXACT_PD_CONFIGURATION_H
#define EXACT_PD_CONFIGURATION_H

#include <string>
#include <memory>
#include <limits>
#include "fcn_clocking_scheme.h"

/**
 * Default timeout defined by Z3.
 */
constexpr const unsigned DEFAULT_TIMEOUT = 4294967u;

/**
 * Configuration struct to set up exact physical design calls.
 */
struct exact_pd_config
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
     * Flag to indicate that a vertically shifted layout should be created. See fcn_layout::offset.
     */
    bool vertical_offset = false;
    /**
     * Flag to indicate that crossings should be used.
     */
    bool crossings = false;
    /**
     * Flag to indicate that the number of used crossing tiles should be minimized.
     */
    bool minimize_wires = false;
    /**
     * Flag to indicate that the number of used crossing tiles should be minimized.
     */
    bool minimize_crossings = false;
    /**
     * Flag to indicate that designated wires should be routed to balance I/O port paths.
     */
    bool io_ports = false;
    /**
     * Flag to indicate that I/Os should be placed at the grid's border.
     */
    bool border_io = false;
    /**
     * Flag to indicate that a discrepancy in fan-in path lengths was allowed.
     */
    bool desynchronize = false;
    /**
     * Flag to indicate that artificial clock latch delays should be used to balance paths.
     */
    bool clock_latches = false;
    /**
     * Flag to indicate that only straight inverters should be used (instead of bend ones).
     */
    bool straight_inverters = false;
    /**
     * Clocking scheme to be used.
     */
    std::shared_ptr<fcn_clocking_scheme> scheme = nullptr;
    /**
     * Flag to indicate that a ToPoliNano clocking is used.
     */
    bool topolinano = false;
    /**
     * Flag to indicate that a 2DDWave clocking is used.
     */
    bool twoddwave = false;
    /**
     * Sets a timeout in ms for the solving process. Standard is 4294967 seconds as defined by Z3.
     */
    unsigned timeout = DEFAULT_TIMEOUT;
};


#endif //EXACT_PD_CONFIGURATION_H
