//
// Created by marcel on 25.11.19.
//

#ifndef FICTION_AREA_DEFAULTS_HPP
#define FICTION_AREA_DEFAULTS_HPP

#include <cstddef>


/**
 * Defines some default area values for technologies. All values are in nm.
 */
namespace area_defaults
{
    namespace qca
    {
        /**
         * Default width of a QCA cell in QCADesigner (https://waluslab.ece.ubc.ca/qcadesigner/).
         */
        constexpr uint64_t width = 18ul;
        /**
         * Default height of a QCA cell in QCADesigner.
         */
        constexpr uint64_t height = 18ul;
        /**
         * Default horizontal spacing between two QCA cells in QCADesigner.
         */
        constexpr uint64_t hspace = 2ul;
        /**
         * Default vertical spacing between two QCA cells in QCADesigner.
         */
        constexpr uint64_t vspace = 2ul;
    }

    namespace inml
    {
        /**
         * Default width of a iNML cell in NMLSim.
         */
        constexpr uint64_t width = 50ul;
        /**
         * Default height of a iNML cell in NMLSim.
         */
        constexpr uint64_t height = 100ul;
        /**
         * Default horizontal spacing between two iNML cells in NMLSim.
         */
        constexpr uint64_t hspace = 10ul;
        /**
         * Default vertical spacing between two iNML cells in NMLSim.
         */
        constexpr uint64_t vspace = 25ul;
    }
}

#endif //FICTION_AREA_DEFAULTS_HPP
