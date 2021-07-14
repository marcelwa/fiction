//
// Created by marcel on 05.10.18.
//

#ifndef FICTION_QCA_WRITER_H
#define FICTION_QCA_WRITER_H

#include "fcn_cell_layout.h"
#include <itertools.hpp>
#include <iostream>
#include <string>
#include "fmt/format.h"
#include "fmt/ostream.h"

namespace qca
{
    constexpr const char* VERSION_2_HEADER = "[VERSION]\nqcadesigner_version=2.000000\n[#VERSION]\n"
                                             "[TYPE:DESIGN]\n[TYPE:QCADLayer]\ntype=3\nstatus=1\n"
                                             "pszDescription=Drawing Layer\n[#TYPE:QCADLayer]\n"
                                             "[TYPE:QCADLayer]\ntype=0\nstatus=1\npszDescription=Substrate\n"
                                             "[TYPE:QCADSubstrate]\n[TYPE:QCADStretchyObject]\n"
                                             "[TYPE:QCADDesignObject]\nx=3000.000000\ny=1500.000000\nbSelected=FALSE\n"
                                             "clr.red=65535\nclr.green=65535\nclr.blue=65535\n"
                                             "bounding_box.xWorld=0.000000\nbounding_box.yWorld=0.000000\n"
                                             "bounding_box.cxWorld=6000.000000\nbounding_box.cyWorld=3000.000000\n"
                                             "[#TYPE:QCADDesignObject]\n[#TYPE:QCADStretchyObject]\n"
                                             "grid_spacing=20.000000\n[#TYPE:QCADSubstrate]\n[#TYPE:QCADLayer]\n";

    constexpr const char* CLOSE_DESIGN = "[#TYPE:DESIGN]";  // no new line to use std::endl to flush stream
    constexpr const char* OPEN_QCAD_LAYER = "[TYPE:QCADLayer]\n";
    constexpr const char* CLOSE_QCAD_LAYER = "[#TYPE:QCADLayer]\n";
    constexpr const char* OPEN_QCAD_CELL = "[TYPE:QCADCell]\n";
    constexpr const char* CLOSE_QCAD_CELL = "[#TYPE:QCADCell]\n";
    constexpr const char* OPEN_QCAD_LABEL = "[TYPE:QCADLabel]\n";
    constexpr const char* CLOSE_QCAD_LABEL = "[#TYPE:QCADLabel]\n";
    constexpr const char* OPEN_QCAD_DESIGN_OBJECT = "[TYPE:QCADDesignObject]\n";
    constexpr const char* CLOSE_QCAD_DESIGN_OBJECT = "[#TYPE:QCADDesignObject]\n";
    constexpr const char* OPEN_QCAD_STRETCHY_OBJECT = "[TYPE:QCADStretchyObject]\n";
    constexpr const char* CLOSE_QCAD_STRETCHY_OBJECT = "[#TYPE:QCADStretchyObject]\n";
    constexpr const char* OPEN_CELL_DOT = "[TYPE:CELL_DOT]\n";
    constexpr const char* CLOSE_CELL_DOT = "[#TYPE:CELL_DOT]\n";

    constexpr const char* TYPE = "type=";
    constexpr const char* STATUS = "status=";
    constexpr const char* PSZ_DESCRIPTION = "pszDescription=";
    constexpr const char* PSZ = "psz=";
    constexpr const char* B_SELECTED = "bSelected=";

    constexpr const char* SELECTED_TRUE = "TRUE";
    constexpr const char* SELECTED_FALSE = "FALSE";

    constexpr const char* COLOR = "clr.red={}\nclr.green={}\nclr.blue={}\n";

    constexpr const char* BOUNDING_BOX_X = "bounding_box.xWorld=";
    constexpr const char* BOUNDING_BOX_Y = "bounding_box.yWorld=";
    constexpr const char* BOUNDING_BOX_CX = "bounding_box.cxWorld=";
    constexpr const char* BOUNDING_BOX_CY = "bounding_box.cyWorld=";

    constexpr const char* CELL_OPTIONS_CX = "cell_options.cxCell=";
    constexpr const char* CELL_OPTIONS_CY = "cell_options.cyCell=";
    constexpr const char* CELL_OPTIONS_DOT_DIAMETER = "cell_options.dot_diameter=";
    constexpr const char* CELL_OPTIONS_CLOCK = "cell_options.clock=";
    constexpr const char* CELL_OPTIONS_MODE = "cell_options.mode=";
    constexpr const char* CELL_FUNCTION = "cell_function=";
    constexpr const char* NUMBER_OF_DOTS_4 = "number_of_dots=4\n";

    constexpr const char* CELL_MODE_NORMAL = "QCAD_CELL_MODE_NORMAL";
    constexpr const char* CELL_MODE_VERTICAL = "QCAD_CELL_MODE_VERTICAL";
    constexpr const char* CELL_MODE_CROSSOVER = "QCAD_CELL_MODE_CROSSOVER";
    constexpr const char* CELL_MODE_ROTATED = "QCAD_CELL_MODE_ROTATED";
    constexpr const char* CELL_FUNCTION_NORMAL = "QCAD_CELL_NORMAL";
    constexpr const char* CELL_FUNCTION_FIXED = "QCAD_CELL_FIXED";
    constexpr const char* CELL_FUNCTION_INPUT = "QCAD_CELL_INPUT";
    constexpr const char* CELL_FUNCTION_OUTPUT = "QCAD_CELL_OUTPUT";

    constexpr const char* X_POS = "x=";
    constexpr const char* Y_POS = "y=";
    constexpr const char* DIAMETER = "diameter=";
    constexpr const char* CHARGE = "charge=";
    constexpr const char* SPIN = "spin=";
    constexpr const char* POTENTIAL = "potential=";

    constexpr const char* CHARGE_0 = "0.000000e+00";
    constexpr const char* CHARGE_1 = "1.602176e-19";
    constexpr const char* CHARGE_8 = "8.010882e-20";
    constexpr const char* NEGATIVE_SPIN = "-319472243083548083355648.000000";

    constexpr const unsigned COLOR_MAX = 65535u;
    constexpr const unsigned COLOR_HALF = 32768u;
    constexpr const unsigned COLOR_MIN = 0u;
    constexpr const unsigned CELL_DISTANCE = 20u;
    constexpr const unsigned X_Y_OFFSET = 100u;
    constexpr const float CELL_SIZE = 18.0f;
    constexpr const float DOT_SIZE = 5.0f;
    constexpr const float LABEL_Y_OFFSET = 21.5f;
    constexpr const float BB_X_OFFSET = 9.0f;
    constexpr const float BB_Y_OFFSET = 33.0f;
    constexpr const float BB_CX_OFFSET = 4.0f;
    constexpr const float BB_CY_OFFSET = 23.0f;
    constexpr const float CHARACTER_WIDTH = 10.0f;

    /**
     * Writes an fcn_cell_layout to a file readable by the QCADesigner (https://waluslab.ece.ubc.ca/qcadesigner/).
     *
     * @param fcl Cell layout to be written.
     * @param filename Desired file name of file to write fcl into. Should end with ".qca" (without quotes).
     */
    void write(fcn_cell_layout_ptr fcl, const std::string& filename);
}


#endif //FICTION_QCA_WRITER_H
