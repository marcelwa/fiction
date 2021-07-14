//
// Created by marcel on 29.07.19.
//

#ifndef FICTION_TOPOLINANO_LIBRARY_H
#define FICTION_TOPOLINANO_LIBRARY_H

#include "fcn_gate_library.h"


/**
 * A concrete FCN gate library as used in "ToPoliNano" (https://topolinano.polito.it/) for the iNML technology. In
 * fiction, this is emulated by using vertically shifted layouts and implementing the ToPoliNano library with 4 x 4
 * magnet positions with one empty row in most tiles (except for MAJ which needs to be handled differently as this
 * library is not uniform otherwise. Theoretically, it allows for multiple wires in the same tile.
 */
class topolinano_library : public fcn_gate_library
{
public:
    /**
     * Default constructor. Initializes the library from a gate layout pointer. In the constructor, also a port_router
     * will be initialized. The port_router immediately tries to route all ports and throws an exception if it fails
     * doing so. That means, construction of a magneto_elastic_library object can fail with an exception. This is
     * intended! See commands.h for details on usage.
     */
    explicit topolinano_library(fcn_gate_layout_ptr fgl);
    /**
     * Overrides the corresponding function in fcn_gate_library. Given a tile t, this function takes all necessary
     * information from the stored grid into account to choose the correct fcn_gate representation for that tile. May it
     * be a gate or wires. Rotation and special marks like input and output, const cells etc. are computed additionally.
     *
     * @param t Tile to be realized in MagnetoElastic.
     * @return MagnetoElastic gate representation of t including I/Os, rotation, cost cells, etc.
     */
    fcn_gate set_up_gate(const fcn_gate_layout::tile& t) override;
    /**
     * Virtual destructor.
     */
    virtual ~topolinano_library() = default;

private:

    // ************************************************************
    // ************************** Gates ***************************
    // ************************************************************

    const fcn_gate conjunction =
    {{
         {'d', ' ', ' ', ' '},
         {'d', 'x', 'x', 'x'},
         {'d', ' ', ' ', ' '},
         {' ', ' ', ' ', ' '}
    }};

    const fcn_gate disjunction =
    {{
         {'u', ' ', ' ', ' '},
         {'u', 'x', 'x', 'x'},
         {'u', ' ', ' ', ' '},
         {' ', ' ', ' ', ' '}
    }};

    const fcn_gate lower_straight_inverter =
    {{
         {' ', ' ', ' ', ' '},
         {' ', ' ', ' ', ' '},
         {'n', 'n', 'n', 'n'},
         {' ', ' ', ' ', ' '}
    }};

    const fcn_gate bottom_lower_straight_inverter =
    {{
         {' ', ' ', ' ', ' '},
         {' ', ' ', ' ', ' '},
         {'n', 'n', 'n', 'n'},
         {'x', ' ', ' ', ' '}
    }};

    const fcn_gate upper_straight_inverter =
    {{
         {'n', 'n', 'n', 'n'},
         {' ', ' ', ' ', ' '},
         {' ', ' ', ' ', ' '},
         {' ', ' ', ' ', ' '}
    }};

    const fcn_gate top_down_bent_inverter =
    {{
         {'x', ' ', ' ', ' '},
         {'x', ' ', ' ', ' '},
         {'n', 'n', 'n', 'n'},
         {' ', ' ', ' ', ' '}
    }};

    const fcn_gate bottom_up_bent_inverter =
    {{
         {'n', 'n', 'n', 'n'},
         {'x', ' ', ' ', ' '},
         {'x', ' ', ' ', ' '},
         {' ', ' ', ' ', ' '}
    }};

    const fcn_gate bottom_lower_up_bent_inverter =
    {{
         {'n', 'n', 'n', 'n'},
         {'x', ' ', ' ', ' '},
         {'x', ' ', ' ', ' '},
         {'x', ' ', ' ', ' '}
    }};

    const fcn_gate majority =
    {{
         {'x', 'x', ' ', ' '},
         {' ', 'x', ' ', ' '},
         {'x', 'x', 'x', 'x'},
         {' ', 'x', ' ', ' '}
    }};

    const fcn_gate coupler =
    {{
         {'f', 'f', 'x', 'x'},
         {'f', ' ', ' ', ' '},
         {'f', 'f', 'x', 'x'},
         {' ', ' ', ' ', ' '}
    }};

    const fcn_gate crosswire =
    {{
         {'c', ' ', 'c', 'x'},
         {' ', 'c', ' ', ' '},
         {'c', ' ', 'c', 'x'},
         {' ', ' ', ' ', ' '}
    }};

    // ************************************************************
    // ************************** Wires ***************************
    // ************************************************************

    const fcn_gate lower_wire =
    {{
         {' ', ' ', ' ', ' '},
         {' ', ' ', ' ', ' '},
         {'x', 'x', 'x', 'x'},
         {' ', ' ', ' ', ' '}
     }};

    const fcn_gate upper_wire =
    {{
         {'x', 'x', 'x', 'x'},
         {' ', ' ', ' ', ' '},
         {' ', ' ', ' ', ' '},
         {' ', ' ', ' ', ' '}
    }};

    const fcn_gate top_down_bent_wire =
    {{
         {'x', ' ', ' ', ' '},
         {'x', 'x', 'x', ' '},
         {' ', ' ', 'x', 'x'},
         {' ', ' ', ' ', ' '}
    }};

    const fcn_gate bottom_up_bent_wire =
    {{
         {' ', ' ', ' ', 'x'},
         {'x', 'x', 'x', 'x'},
         {'x', ' ', ' ', ' '},
         {' ', ' ', ' ', ' '}
    }};

    const fcn_gate top_down_staircase_wire =
    {{
         {'x', ' ', ' ', ' '},
         {'x', 'x', 'x', ' '},
         {' ', ' ', 'x', ' '},
         {' ', ' ', 'x', 'x'}
    }};

    const fcn_gate bottom_up_staircase_wire =
    {{
         {' ', ' ', 'x', 'x'},
         {' ', ' ', 'x', ' '},
         {'x', 'x', 'x', ' '},
         {'x', ' ', ' ', ' '}
    }};

    const fcn_gate bottom_down_bent_wire =
    {{
         {' ', ' ', ' ', ' '},
         {' ', ' ', ' ', ' '},
         {'x', 'x', 'x', ' '},
         {' ', ' ', 'x', 'x'}
    }};

    const fcn_gate majority_wire =
    {{
         {'x', 'x', ' ', ' '},
         {' ', ' ', ' ', ' '},
         {' ', ' ', ' ', ' '},
         {' ', ' ', ' ', ' '}
    }};

    const fcn_gate coupler_wire =
    {{
         {' ', ' ', ' ', ' '},
         {' ', ' ', ' ', ' '},
         {'x', 'x', 'x', 'x'},
         {'x', ' ', ' ', ' '}
    }};
    /**
     * Lookup table for wire rotations. Maps ports to corresponding wires.
     */
    port_gate_map wire_map =
    {
        // straight wires
        {{{port(0, 2)}, {port(3, 2)}}, lower_wire},
        {{{}, {port(3, 2)}}, lower_wire},
        {{{port(0, 2)}, {}}, lower_wire},
        {{{port(0, 0)}, {port(3, 0)}}, upper_wire},
        {{{}, {port(3, 0)}}, upper_wire},
        {{{port(0, 0)}, {}}, upper_wire},
        {{{port(0, 3)}, {port(3, 3)}}, rotate_180(upper_wire)},
        {{{}, {port(3, 3)}}, rotate_180(upper_wire)},
        {{{port(0, 3)}, {}}, rotate_180(upper_wire)},
        // bent wires
        {{{port(0, 0)}, {port(3, 2)}}, top_down_bent_wire},
        {{{port(0, 2)}, {port(3, 0)}}, bottom_up_bent_wire},
        {{{port(0, 2)}, {port(3, 3)}}, bottom_down_bent_wire},
        // staircase wires
        {{{port(0, 0)}, {port(3, 3)}}, top_down_staircase_wire},
        {{{port(0, 3)}, {port(3, 0)}}, bottom_up_staircase_wire},
        // special wires
        {{{port(0, 0)}, {port(1, 0)}}, majority_wire},
        {{{port(0, 3)}, {port(3, 2)}}, coupler_wire}
        // TODO more wires go here!
    };
    /**
     * Lookup table for inverter rotations. Maps ports to corresponding inverters.
     */
    port_gate_map inverter_map =
    {
        // straight inverters
        {{{port(0, 2)}, {port(3, 2)}}, lower_straight_inverter},
        {{{port(0, 3)}, {port(3, 2)}}, bottom_lower_straight_inverter},
        {{{port(0, 0)}, {port(3, 0)}}, upper_straight_inverter},
            // without outputs
        {{{port(0, 2)}, {}}, lower_straight_inverter},
        {{{port(0, 0)}, {}}, upper_straight_inverter},
        // without inputs
        {{{}, {port(3, 2)}}, lower_straight_inverter},
        {{{}, {port(3, 0)}}, upper_straight_inverter},
        // bent inverters
        {{{port(0, 0)}, {port(3, 2)}}, top_down_bent_inverter},
        {{{port(0, 2)}, {port(3, 0)}}, bottom_up_bent_inverter},
        {{{port(0, 3)}, {port(3, 0)}}, bottom_lower_up_bent_inverter}
    };
    /**
     * Returns the port equivalent of the given direction.
     *
     * @param d Direction whose port is desired.
     * @return Port of direction d.
     */
    port dir_to_port(const layout::directions d) const;
};


#endif //FICTION_TOPOLINANO_LIBRARY_H
