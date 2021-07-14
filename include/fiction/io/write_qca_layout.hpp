//
// Created by marcel on 05.10.18.
//

#ifndef FICTION_WRITE_QCA_LAYOUT_HPP
#define FICTION_WRITE_QCA_LAYOUT_HPP

#include "../technology/cell_technologies.hpp"

#include <fmt/format.h>

#include <ostream>
#include <string>
#include <type_traits>
#include <vector>

namespace fiction
{

namespace detail
{

namespace qcad
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

constexpr const char* CLOSE_DESIGN               = "[#TYPE:DESIGN]";  // no new line to use std::endl to flush stream
constexpr const char* OPEN_QCAD_LAYER            = "[TYPE:QCADLayer]\n";
constexpr const char* CLOSE_QCAD_LAYER           = "[#TYPE:QCADLayer]\n";
constexpr const char* OPEN_QCAD_CELL             = "[TYPE:QCADCell]\n";
constexpr const char* CLOSE_QCAD_CELL            = "[#TYPE:QCADCell]\n";
constexpr const char* OPEN_QCAD_LABEL            = "[TYPE:QCADLabel]\n";
constexpr const char* CLOSE_QCAD_LABEL           = "[#TYPE:QCADLabel]\n";
constexpr const char* OPEN_QCAD_DESIGN_OBJECT    = "[TYPE:QCADDesignObject]\n";
constexpr const char* CLOSE_QCAD_DESIGN_OBJECT   = "[#TYPE:QCADDesignObject]\n";
constexpr const char* OPEN_QCAD_STRETCHY_OBJECT  = "[TYPE:QCADStretchyObject]\n";
constexpr const char* CLOSE_QCAD_STRETCHY_OBJECT = "[#TYPE:QCADStretchyObject]\n";
constexpr const char* OPEN_CELL_DOT              = "[TYPE:CELL_DOT]\n";
constexpr const char* CLOSE_CELL_DOT             = "[#TYPE:CELL_DOT]\n";

constexpr const char* TYPE            = "type=";
constexpr const char* STATUS          = "status=";
constexpr const char* PSZ_DESCRIPTION = "pszDescription=";
constexpr const char* PSZ             = "psz=";
constexpr const char* B_SELECTED      = "bSelected=";

constexpr const char* SELECTED_TRUE  = "TRUE";
constexpr const char* SELECTED_FALSE = "FALSE";

constexpr const char* COLOR = "clr.red={}\nclr.green={}\nclr.blue={}\n";

constexpr const char* BOUNDING_BOX_X  = "bounding_box.xWorld=";
constexpr const char* BOUNDING_BOX_Y  = "bounding_box.yWorld=";
constexpr const char* BOUNDING_BOX_CX = "bounding_box.cxWorld=";
constexpr const char* BOUNDING_BOX_CY = "bounding_box.cyWorld=";

constexpr const char* CELL_OPTIONS_CX           = "cell_options.cxCell=";
constexpr const char* CELL_OPTIONS_CY           = "cell_options.cyCell=";
constexpr const char* CELL_OPTIONS_DOT_DIAMETER = "cell_options.dot_diameter=";
constexpr const char* CELL_OPTIONS_CLOCK        = "cell_options.clock=";
constexpr const char* CELL_OPTIONS_MODE         = "cell_options.mode=";
constexpr const char* CELL_FUNCTION             = "cell_function=";
constexpr const char* NUMBER_OF_DOTS_4          = "number_of_dots=4\n";

constexpr const char* CELL_MODE_NORMAL     = "QCAD_CELL_MODE_NORMAL";
constexpr const char* CELL_MODE_VERTICAL   = "QCAD_CELL_MODE_VERTICAL";
constexpr const char* CELL_MODE_CROSSOVER  = "QCAD_CELL_MODE_CROSSOVER";
constexpr const char* CELL_MODE_ROTATED    = "QCAD_CELL_MODE_ROTATED";
constexpr const char* CELL_FUNCTION_NORMAL = "QCAD_CELL_NORMAL";
constexpr const char* CELL_FUNCTION_FIXED  = "QCAD_CELL_FIXED";
constexpr const char* CELL_FUNCTION_INPUT  = "QCAD_CELL_INPUT";
constexpr const char* CELL_FUNCTION_OUTPUT = "QCAD_CELL_OUTPUT";

constexpr const char* X_POS     = "x=";
constexpr const char* Y_POS     = "y=";
constexpr const char* DIAMETER  = "diameter=";
constexpr const char* CHARGE    = "charge=";
constexpr const char* SPIN      = "spin=";
constexpr const char* POTENTIAL = "potential=";

constexpr const char* CHARGE_0      = "0.000000e+00";
constexpr const char* CHARGE_1      = "1.602176e-19";
constexpr const char* CHARGE_8      = "8.010882e-20";
constexpr const char* NEGATIVE_SPIN = "-319472243083548083355648.000000";

constexpr const unsigned COLOR_MAX       = 65535u;
constexpr const unsigned COLOR_HALF      = 32768u;
constexpr const unsigned COLOR_MIN       = 0u;
constexpr const unsigned CELL_DISTANCE   = 20u;
constexpr const unsigned X_Y_OFFSET      = 100u;
constexpr const float    CELL_SIZE       = 18.0f;
constexpr const float    DOT_SIZE        = 5.0f;
constexpr const float    LABEL_Y_OFFSET  = 21.5f;
constexpr const float    BB_X_OFFSET     = 9.0f;
constexpr const float    BB_Y_OFFSET     = 33.0f;
constexpr const float    BB_CX_OFFSET    = 4.0f;
constexpr const float    BB_CY_OFFSET    = 23.0f;
constexpr const float    CHARACTER_WIDTH = 10.0f;

}  // namespace qcad

template <typename Lyt>
class write_qca_layout_impl
{
  public:
    write_qca_layout_impl(const Lyt& src, std::ofstream& s) : lyt{src}, file{s} {}

    void run()
    {
        if (!file.is_open())
            throw std::ofstream::failure("could not open file");

        std::vector<typename Lyt::cell> via_layer_cells{};

        auto write_cell = [&, this](const auto& cell) -> void
        {
            auto cell_type = lyt.get_cell_type(cell);

            // open cell
            file << qcad::OPEN_QCAD_CELL;
            // open design object
            file << qcad::OPEN_QCAD_DESIGN_OBJECT;

            const auto x_pos = static_cast<float>(cell.x * qcad::CELL_DISTANCE + qcad::X_Y_OFFSET);
            const auto y_pos = static_cast<float>(cell.y * qcad::CELL_DISTANCE + qcad::X_Y_OFFSET);
            file << qcad::X_POS << x_pos << '\n';
            file << qcad::Y_POS << y_pos << '\n';
            file << qcad::B_SELECTED << qcad::SELECTED_FALSE << '\n';

            // supports colors for 4 clocks only
            unsigned red, green, blue;
            if (qca_technology::is_input_cell(cell_type))
            {
                red   = qcad::COLOR_MIN;
                green = qcad::COLOR_MIN;
                blue  = qcad::COLOR_MAX;
            }
            else if (qca_technology::is_output_cell(cell_type))
            {
                red   = qcad::COLOR_MAX;
                green = qcad::COLOR_MAX;
                blue  = qcad::COLOR_MIN;
            }
            else if (qca_technology::is_constant_cell(cell_type))
            {
                red   = qcad::COLOR_MAX;
                green = qcad::COLOR_HALF;
                blue  = qcad::COLOR_MIN;
            }
            else
            {
                switch (lyt.get_clock_number(cell))
                {
                    case 0:
                    {
                        red   = qcad::COLOR_MIN;
                        green = qcad::COLOR_MAX;
                        blue  = qcad::COLOR_MIN;
                        break;
                    }
                    case 1:
                    {
                        red   = qcad::COLOR_MAX;
                        green = qcad::COLOR_MIN;
                        blue  = qcad::COLOR_MAX;
                        break;
                    }
                    case 2:
                    {
                        red   = qcad::COLOR_MIN;
                        green = qcad::COLOR_MAX;
                        blue  = qcad::COLOR_MAX;
                        break;
                    }
                    case 3:
                    {
                        red   = qcad::COLOR_MAX;
                        green = qcad::COLOR_MAX;
                        blue  = qcad::COLOR_MAX;
                        break;
                    }
                    default: break;
                }
            }
            file << fmt::format(qcad::COLOR, red, green, blue);

            file << qcad::BOUNDING_BOX_X << x_pos - qcad::CELL_SIZE / 2.0f << '\n';
            file << qcad::BOUNDING_BOX_Y << y_pos - qcad::CELL_SIZE / 2.0f << '\n';
            file << qcad::BOUNDING_BOX_CX << qcad::CELL_SIZE << '\n';
            file << qcad::BOUNDING_BOX_CY << qcad::CELL_SIZE << '\n';

            // close design object
            file << qcad::CLOSE_QCAD_DESIGN_OBJECT;

            file << qcad::CELL_OPTIONS_CX << qcad::CELL_SIZE << '\n';
            file << qcad::CELL_OPTIONS_CY << qcad::CELL_SIZE << '\n';
            file << qcad::CELL_OPTIONS_DOT_DIAMETER << qcad::DOT_SIZE << '\n';
            file << qcad::CELL_OPTIONS_CLOCK << std::to_string(lyt.get_clock_number(cell)) << '\n';

            // handle cell mode
            file << qcad::CELL_OPTIONS_MODE;
            if (const auto mode = lyt.get_cell_mode(cell); qca_technology::is_vertical_cell_mode(mode))
            {
                file << qcad::CELL_MODE_VERTICAL;
                via_layer_cells.push_back(cell);  // save via cell for inter-layer
            }
            else if (lyt.is_crossing_layer(cell))
            {
                file << qcad::CELL_MODE_CROSSOVER;
            }
            else if (qca_technology::is_rotated_cell_mode(mode))
            {
                file << qcad::CELL_MODE_ROTATED;
            }
            else
            {
                file << qcad::CELL_MODE_NORMAL;
            }

            file << '\n';

            // handle cell function
            file << qcad::CELL_FUNCTION;

            if (qca_technology::is_normal_cell(cell_type))
            {
                file << qcad::CELL_FUNCTION_NORMAL;
            }
            else if (qca_technology::is_constant_cell(cell_type))
            {
                file << qcad::CELL_FUNCTION_FIXED;
            }
            else if (qca_technology::is_input_cell(cell_type))
            {
                file << qcad::CELL_FUNCTION_INPUT;
            }
            else if (qca_technology::is_output_cell(cell_type))
            {
                file << qcad::CELL_FUNCTION_OUTPUT;
            }

            file << '\n' << qcad::NUMBER_OF_DOTS_4;

            // create quantum dots
            for (int i = 1; i > -2; i -= 2)
            {
                for (int j2 = 1; j2 > -2; j2 -= 2)
                {
                    int j = i == 1 ? -j2 : j2;

                    // open dot
                    file << qcad::OPEN_CELL_DOT;

                    file << qcad::X_POS << x_pos + (qcad::CELL_SIZE / 4.0f) * static_cast<float>(i) << '\n';
                    file << qcad::Y_POS << y_pos + (qcad::CELL_SIZE / 4.0f) * static_cast<float>(j) << '\n';
                    file << qcad::DIAMETER << qcad::DOT_SIZE << '\n';

                    // determine charge
                    file << qcad::CHARGE;
                    if (!qca_technology::is_constant_cell(cell_type))
                    {
                        file << qcad::CHARGE_8;
                    }
                    else if ((qca_technology::is_const_0_cell(cell_type) && std::abs(i + j) == 2) ||
                             (qca_technology::is_const_1_cell(cell_type) && std::abs(i + j) == 0))
                    {
                        file << qcad::CHARGE_1;
                    }
                    else if ((qca_technology::is_const_0_cell(cell_type) && std::abs(i + j) == 0) ||
                             (qca_technology::is_const_1_cell(cell_type) && std::abs(i + j) == 2))
                    {
                        file << qcad::CHARGE_0;
                    }
                    file << '\n';

                    // determine spin
                    file << qcad::SPIN;
                    if (qca_technology::is_input_cell(cell_type) || qca_technology::is_output_cell(cell_type))
                    {
                        file << qcad::NEGATIVE_SPIN;
                    }
                    else
                    {
                        file << 0.0f;
                    }
                    file << '\n';

                    file << qcad::POTENTIAL << 0.0f << '\n';

                    // close dot
                    file << qcad::CLOSE_CELL_DOT;
                }
            }

            // override cell_name if cell is constant; if cell has a name
            if (auto cell_name = qca_technology::is_const_0_cell(cell_type) ? "-1.00" :
                                 qca_technology::is_const_1_cell(cell_type) ? "1.00" :
                                                                              lyt.get_cell_name(cell);
                !cell_name.empty())
            {
                // open label
                file << qcad::OPEN_QCAD_LABEL;
                file << qcad::OPEN_QCAD_STRETCHY_OBJECT;
                file << qcad::OPEN_QCAD_DESIGN_OBJECT;

                file << qcad::X_POS << x_pos << '\n';
                file << qcad::Y_POS << y_pos - qcad::LABEL_Y_OFFSET << '\n';
                file << qcad::B_SELECTED << qcad::SELECTED_FALSE << '\n';
                file << fmt::format(qcad::COLOR, red, green, blue);
                file << qcad::BOUNDING_BOX_X << x_pos - qcad::BB_X_OFFSET << '\n';
                file << qcad::BOUNDING_BOX_Y << y_pos - qcad::BB_Y_OFFSET << '\n';
                file << qcad::BOUNDING_BOX_CX
                     << static_cast<float>(cell_name.size()) * qcad::CHARACTER_WIDTH + qcad::BB_CX_OFFSET << '\n';
                file << qcad::BOUNDING_BOX_CY << qcad::BB_CY_OFFSET << '\n';

                file << qcad::CLOSE_QCAD_DESIGN_OBJECT;
                file << qcad::CLOSE_QCAD_STRETCHY_OBJECT;

                file << qcad::PSZ << cell_name << '\n';

                // close label
                file << qcad::CLOSE_QCAD_LABEL;
            }

            // close cell
            file << qcad::CLOSE_QCAD_CELL;
        };

        auto via_counter     = 1u;
        auto write_via_cells = [&](std::vector<typename Lyt::cell>& vias) -> void
        {
            if (vias.empty())
                return;

            // open via layer
            file << qcad::OPEN_QCAD_LAYER;

            file << qcad::TYPE << 1 << '\n';
            file << qcad::STATUS << 0 << '\n';
            file << qcad::PSZ_DESCRIPTION << "Via Layer " << std::to_string(via_counter++) << '\n';

            for (auto& v : vias) write_cell(v);

            // close design layer
            file << qcad::CLOSE_QCAD_LAYER;

            vias.clear();
        };

        // write version header
        file << qcad::VERSION_2_HEADER;

        // for each layer
        for (auto layer = 0u; layer <= lyt.z(); ++layer)
        {
            write_via_cells(via_layer_cells);

            // open design layer
            file << qcad::OPEN_QCAD_LAYER;

            file << qcad::TYPE << 1 << '\n';
            file << qcad::STATUS << 0 << '\n';
            file << qcad::PSZ_DESCRIPTION
                 << ((layer == 0) ? "Ground Layer" : ("Crossing Layer " + std::to_string(layer))) << '\n';

            // for all cells in that layer
            lyt.foreach_cell(
                [&write_cell, &layer](const auto& cell)
                {
                    // skip cells not belonging to current layer
                    if (cell.z == layer)
                    {
                        write_cell(cell);
                    }
                });

            // close design layer
            file << qcad::CLOSE_QCAD_LAYER;
        }

        // close design block
        file << qcad::CLOSE_DESIGN << std::endl;
    }

  private:
    Lyt lyt;

    std::ofstream& file;
};

}  // namespace detail

template <typename Lyt>
void write_qca_layout(const Lyt& lyt, std::ofstream& os)
{
    static_assert(std::is_same_v<typename Lyt::technology, qca_technology>, "Lyt must be a QCA layout");

    detail::write_qca_layout_impl p{lyt, os};

    p.run();
}

template <typename Lyt>
void write_qca_layout(const Lyt& lyt, const std::string& filename)
{
    std::ofstream os{filename.c_str(), std::ofstream::out};
    write_qca_layout(lyt, os);
    os.close();
}

}  // namespace fiction

#endif  // FICTION_WRITE_QCA_LAYOUT_HPP
