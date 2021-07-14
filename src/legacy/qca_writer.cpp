//
// Created by marcel on 05.10.18.
//

#include "qca_writer.h"

namespace qca
{
    void write(fcn_cell_layout_ptr fcl, const std::string& filename)
    {
        std::ofstream file(filename, std::ios::out);

        if (!file.is_open())
            throw std::ofstream::failure("could not open file");

        std::vector<fcn_cell_layout::cell> via_layer_cells{};

        auto write_cell = [&](const fcn_cell_layout::cell cell) -> void
        {
            auto cell_type = fcl->get_cell_type(cell);

            // open cell
            file << OPEN_QCAD_CELL;
            // open design object
            file << OPEN_QCAD_DESIGN_OBJECT;

            auto x_pos = cell[X] * CELL_DISTANCE + X_Y_OFFSET;
            auto y_pos = cell[Y] * CELL_DISTANCE + X_Y_OFFSET;
            file << X_POS << x_pos << '\n';
            file << Y_POS << y_pos << '\n';
            file << B_SELECTED << SELECTED_FALSE << '\n';

            // supports colors for 4 clocks only
            unsigned red, green, blue;
            if (cell_type == fcn::INPUT_CELL)
            { red = COLOR_MIN; green = COLOR_MIN; blue = COLOR_MAX; }
            else if (cell_type == fcn::OUTPUT_CELL)
            { red = COLOR_MAX; green = COLOR_MAX; blue = COLOR_MIN; }
            else if (cell_type == fcn::qca::CONST_0_CELL || cell_type == fcn::qca::CONST_1_CELL)
            { red = COLOR_MAX; green = COLOR_HALF; blue = COLOR_MIN; }
            else
            {
                switch (*fcl->cell_clocking(cell))
                {
                    case 0: { red = COLOR_MIN; green = COLOR_MAX; blue = COLOR_MIN; break; }
                    case 1: { red = COLOR_MAX; green = COLOR_MIN; blue = COLOR_MAX; break; }
                    case 2: { red = COLOR_MIN; green = COLOR_MAX; blue = COLOR_MAX; break; }
                    case 3: { red = COLOR_MAX; green = COLOR_MAX; blue = COLOR_MAX; break; }
                    default: break;
                }
            }
            file << fmt::format(COLOR, red, green, blue);


            file << BOUNDING_BOX_X << x_pos - CELL_SIZE / 2.0f << '\n';
            file << BOUNDING_BOX_Y << y_pos - CELL_SIZE / 2.0f << '\n';
            file << BOUNDING_BOX_CX << CELL_SIZE << '\n';
            file << BOUNDING_BOX_CY << CELL_SIZE << '\n';

            // close design object
            file << CLOSE_QCAD_DESIGN_OBJECT;

            file << CELL_OPTIONS_CX << CELL_SIZE << '\n';
            file << CELL_OPTIONS_CY << CELL_SIZE << '\n';
            file << CELL_OPTIONS_DOT_DIAMETER << DOT_SIZE << '\n';
            file << CELL_OPTIONS_CLOCK << *fcl->cell_clocking(cell) << '\n';

            // handle cell mode
            file << CELL_OPTIONS_MODE;
            if (fcl->get_cell_mode(cell) == fcn::cell_mode::VERTICAL)
            {
                file << CELL_MODE_VERTICAL;
                via_layer_cells.push_back(cell);  // save via cell for inter-layer
            }
            else if (cell[Z] != GROUND)
                file << CELL_MODE_CROSSOVER;
            else if (fcl->get_cell_mode(cell) == fcn::cell_mode::ROTATED)
                file << CELL_MODE_ROTATED;
            else
                file << CELL_MODE_NORMAL;

            file << '\n';

            // handle cell function
            file << CELL_FUNCTION;
            switch (cell_type)
            {
                case fcn::NORMAL_CELL:
                {
                    file << CELL_FUNCTION_NORMAL;
                    break;
                }
                case fcn::qca::CONST_0_CELL:
                case fcn::qca::CONST_1_CELL:
                {
                    file << CELL_FUNCTION_FIXED;
                    break;
                }
                case fcn::INPUT_CELL:
                {
                    file << CELL_FUNCTION_INPUT;
                    break;
                }
                case fcn::OUTPUT_CELL:
                {
                    file << CELL_FUNCTION_OUTPUT;
                    break;
                }
                default:
                {
                    break;
                }
            }

            file << '\n' << NUMBER_OF_DOTS_4;

            // create quantum dots
            for (int i = 1; i > -2; i -= 2)
            {
                for (int j2 = 1; j2 > -2; j2 -= 2)
                {
                    int j = i == 1 ? -j2 : j2;

                    // open dot
                    file << OPEN_CELL_DOT;

                    file << X_POS << x_pos + (CELL_SIZE / 4.0f) * i << '\n';
                    file << Y_POS << y_pos + (CELL_SIZE / 4.0f) * j << '\n';
                    file << DIAMETER << DOT_SIZE << '\n';

                    // determine charge
                    file << CHARGE;
                    if (cell_type != fcn::qca::CONST_0_CELL && cell_type != fcn::qca::CONST_1_CELL)
                        file << CHARGE_8;
                    else if ((cell_type == fcn::qca::CONST_0_CELL && std::abs(i + j) == 2) ||
                             (cell_type == fcn::qca::CONST_1_CELL && std::abs(i + j) == 0))
                        file << CHARGE_1;
                    else if ((cell_type == fcn::qca::CONST_0_CELL && std::abs(i + j) == 0) ||
                             (cell_type == fcn::qca::CONST_1_CELL && std::abs(i + j) == 2))
                        file << CHARGE_0;
                    file << '\n';

                    // determine spin
                    file << SPIN;
                    if (cell_type == fcn::INPUT_CELL || cell_type == fcn::OUTPUT_CELL)
                        file << NEGATIVE_SPIN;
                    else
                        file << 0.0f;
                    file << '\n';

                    file << POTENTIAL << 0.0f << '\n';

                    // close dot
                    file << CLOSE_CELL_DOT;
                }
            }

            // override cell_name if cell is constant; if cell has a name
            if (auto cell_name = cell_type == fcn::qca::CONST_0_CELL ? "-1.00" :
                                 cell_type == fcn::qca::CONST_1_CELL ?  "1.00" : fcl->get_cell_name(cell);
                !cell_name.empty())
            {
                // open label
                file << OPEN_QCAD_LABEL;
                file << OPEN_QCAD_STRETCHY_OBJECT;
                file << OPEN_QCAD_DESIGN_OBJECT;

                file << X_POS << x_pos << '\n';
                file << Y_POS << y_pos - LABEL_Y_OFFSET << '\n';
                file << B_SELECTED << SELECTED_FALSE << '\n';
                file << fmt::format(COLOR, red, green, blue);
                file << BOUNDING_BOX_X << x_pos - BB_X_OFFSET << '\n';
                file << BOUNDING_BOX_Y << y_pos - BB_Y_OFFSET << '\n';
                file << BOUNDING_BOX_CX << cell_name.size() * CHARACTER_WIDTH + BB_CX_OFFSET << '\n';
                file << BOUNDING_BOX_CY << BB_CY_OFFSET << '\n';

                file << CLOSE_QCAD_DESIGN_OBJECT;
                file << CLOSE_QCAD_STRETCHY_OBJECT;

                file << PSZ << cell_name << '\n';

                // close label
                file << CLOSE_QCAD_LABEL;
            }

            // close cell
            file << CLOSE_QCAD_CELL;
        };

        auto via_counter = 1u;
        auto write_via_cells = [&](std::vector<fcn_cell_layout::cell>& vias) -> void
        {
            if (vias.empty())
                return;

            // open via layer
            file << OPEN_QCAD_LAYER;

            file << TYPE << 1 << '\n';
            file << STATUS << 0 << '\n';
            file << PSZ_DESCRIPTION << "Via Layer " << std::to_string(via_counter++) << '\n';

            for (auto& v : vias)
                write_cell(v);

            // close design layer
            file << CLOSE_QCAD_LAYER;

            vias.clear();
        };

        // write version header
        file << VERSION_2_HEADER;

        // for each layer
        for (auto layer : iter::range(fcl->z()))
        {
            write_via_cells(via_layer_cells);

            // open design layer
            file << OPEN_QCAD_LAYER;

            file << TYPE << 1 << '\n';
            file << STATUS << 0 << '\n';
            file << PSZ_DESCRIPTION << ((layer == 0) ? "Ground Layer" : ("Crossing Layer " + std::to_string(layer))) << '\n';

            // for all cells in that layer
            for (auto&& cell : fcl->layer_n(layer))
            {
                if (fcl->get_cell_type(cell) == fcn::EMPTY_CELL)
                    continue;

                write_cell(cell);
            }

            // close design layer
            file << CLOSE_QCAD_LAYER;
        }

        // close design block
        file << CLOSE_DESIGN << std::endl;
    }
}
