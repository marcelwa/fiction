//
// Created by marcel on 26.07.18.
//

#ifndef FICTION_FCN_CELL_TYPES_H
#define FICTION_FCN_CELL_TYPES_H

#include <fcn_technology.h>

namespace fcn
{
    /**
     * Alias for a char defining different FCN cell types.
     */
    using cell_type = char;
    /**
     * Symbol used for empty FCN cells.
     */
    constexpr const cell_type EMPTY_CELL = ' ';
    /**
     * Symbol used for normal FCN cells.
     */
    constexpr const cell_type NORMAL_CELL = 'x';
    /**
     * Symbol used for input FCN cells.
     */
    constexpr const cell_type INPUT_CELL = 'i';
    /**
     * Symbol used for output FCN cells.
     */
    constexpr const cell_type OUTPUT_CELL = 'o';
    /**
     * Possible cell modes for FCN cells.
     */
    enum class cell_mode { NORMAL, ROTATED, VERTICAL, CROSSOVER };
    /**
     * Possible marks to be applied to a cell to change its type.
     */
    enum class cell_mark { INPUT = INPUT_CELL, OUTPUT = OUTPUT_CELL, EMPTY = EMPTY_CELL };

    namespace qca
    {
        /**
         * Symbol used for constant 0 input QCA cells.
         */
        constexpr const cell_type CONST_0_CELL = '0';
        /**
         * Symbol used for constant 1 input QCA cells.
         */
        constexpr const cell_type CONST_1_CELL = '1';
    }

    namespace inml
    {
        /**
         * Symbol used for upper slanted edge magnets.
         */
        constexpr const cell_type SLANTED_EDGE_UP_MAGNET = 'u';
        /**
         * Symbol used for lower slanted edge magnets.
         */
        constexpr const cell_type SLANTED_EDGE_DOWN_MAGNET = 'd';
        /**
         * Symbol used for inverter magnets.
         */
        constexpr const cell_type INVERTER_MAGNET = 'n';
        /**
         * Symbol used for cross-wire magnets.
         */
        constexpr const cell_type CROSSWIRE_MAGNET = 'c';
        /**
         * Symbol used for coupler (fan-out) magnets.
         */
        constexpr const cell_type FANOUT_COUPLER_MAGNET = 'f';
    }
}


#endif //FICTION_FCN_CELL_TYPES_H
