//
// Created by marcel on 14.07.21.
//

#ifndef FICTION_WRITE_SQD_LAYOUT_HPP
#define FICTION_WRITE_SQD_LAYOUT_HPP

#include "../technology/cell_technologies.hpp"
#include "utils/version_info.hpp"

#include <fmt/format.h>

#include <chrono>
#include <ctime>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace fiction
{

namespace detail
{

namespace siqad
{

constexpr const char* XML_HEADER    = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
constexpr const char* OPEN_SIQAD    = "<siqad>\n";
constexpr const char* CLOSE_SIQAD   = "</siqad>\n";
constexpr const char* PROGRAM_BLOCK = "    <program>\n"
                                      "        <file_purpose>{}</file_purpose>\n"
                                      "        <created_by>{}</created_by>\n"
                                      "        <available_at>{}</available_at>\n"
                                      "        <date>{}</date>\n"
                                      "    </program>\n";

constexpr const char* GUI_BLOCK = "    <gui>\n"
                                  "        <zoom>{}</zoom>\n"
                                  "        <displayed_region x1=\"{}\" y1=\"{}\" x2=\"{}\" y2=\"{}\"/>\n"
                                  "        <scroll x=\"{}\" y=\"{}\"/>\n"
                                  "    </gui>\n";

constexpr const char* LAYERS_BLOCK = "<layers>\n"
                                     "        <layer_prop>\n"
                                     "            <name>Lattice</name>\n"
                                     "            <type>Lattice</type>\n"
                                     "            <role>Design</role>\n"
                                     "            <zoffset>0</zoffset>\n"
                                     "            <zheight>0</zheight>\n"
                                     "            <visible>1</visible>\n"
                                     "            <active>0</active>\n"
                                     "            <lat_vec>\n"
                                     "                <a1 x=\"3.84\" y=\"0\"/>\n"
                                     "                <a2 x=\"0\" y=\"7.68\"/>\n"
                                     "                <N>2</N>\n"
                                     "                <b1 x=\"0\" y=\"0\"/>\n"
                                     "                <b2 x=\"0\" y=\"2.25\"/>\n"
                                     "            </lat_vec>\n"
                                     "        </layer_prop>\n"
                                     "        <layer_prop>\n"
                                     "            <name>Screenshot Overlay</name>\n"
                                     "            <type>Misc</type>\n"
                                     "            <role>Overlay</role>\n"
                                     "            <zoffset>0</zoffset>\n"
                                     "            <zheight>0</zheight>\n"
                                     "            <visible>0</visible>\n"
                                     "            <active>0</active>\n"
                                     "        </layer_prop>\n"
                                     "        <layer_prop>\n"
                                     "            <name>Surface</name>\n"
                                     "            <type>DB</type>\n"
                                     "            <role>Design</role>\n"
                                     "            <zoffset>0</zoffset>\n"
                                     "            <zheight>0</zheight>\n"
                                     "            <visible>1</visible>\n"
                                     "            <active>0</active>\n"
                                     "        </layer_prop>\n"
                                     "        <layer_prop>\n"
                                     "            <name>Metal</name>\n"
                                     "            <type>Electrode</type>\n"
                                     "            <role>Design</role>\n"
                                     "            <zoffset>1000</zoffset>\n"
                                     "            <zheight>100</zheight>\n"
                                     "            <visible>1</visible>\n"
                                     "            <active>0</active>\n"
                                     "        </layer_prop>\n"
                                     "    </layers>\n";

constexpr const char* OPEN_DESIGN = "    <design>\n"
                                    "        <layer type=\"Lattice\"/>\n"
                                    "        <layer type=\"Misc\"/>\n"
                                    "        <layer type=\"DB\">\n";
constexpr const char* CLOSE_DESIGN = "    </design>\n";

constexpr const char* DBDOT_BLOCK = "            <dbdot>\n"
                                    "                <layer_id>2</layer_id>\n"
                                    "                <latcoord n=\"{}\" m=\"{}\" l=\"{}\"/>\n"
                                    "                <color>{}</color>\n"
                                    "            </dbdot>\n";

// color format is Alpha RBG
constexpr const char* NORMAL_COLOR = "#ffc8c8c8";
constexpr const char* INPUT_COLOR  = "#ff008dc8";
constexpr const char* OUTPUT_COLOR = "#ffe28686";
constexpr const char* CONST_COLOR  = "#ff000000";

}  // namespace siqad

template <typename Lyt>
class write_sqd_layout_impl
{
  public:
    write_sqd_layout_impl(const Lyt& src, std::ofstream& s) : lyt{src}, file{s} {}

    void run()
    {
        std::stringstream header{}, gui{}, design{};

        header << siqad::XML_HEADER << siqad::OPEN_SIQAD;

        const auto now      = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        auto       time_str = std::string{std::ctime(&now)};
        time_str.pop_back();  // remove trailing new line

        header << fmt::format(siqad::PROGRAM_BLOCK, "layout simulation", FICTION_VERSION, FICTION_REPO, time_str);

        design << siqad::LAYERS_BLOCK << siqad::OPEN_DESIGN;

        generate_db_blocks(design);

        design << siqad::CLOSE_DESIGN;

        if (!file.is_open())
            throw std::ofstream::failure("could not open file");

        file << header.str();
        file << gui.str();
        file << design.str();

        file << siqad::CLOSE_SIQAD;
    }

  private:
    Lyt lyt;

    std::ofstream& file;

    void generate_db_blocks(std::stringstream& design) noexcept
    {
        lyt.foreach_cell(
            [this, &design](const auto& c)
            {
                // generate SiDB cells
                if constexpr (std::is_same_v<typename Lyt::technology, sidb_technology>)
                {
                    design << fmt::format(siqad::DBDOT_BLOCK, c.x, c.y / 2, c.y % 2, siqad::NORMAL_COLOR);
                }
                // generate QCA cell blocks
                else if constexpr (std::is_same_v<typename Lyt::technology, qca_technology>)
                {
                    const auto type = lyt.get_cell_type(c);

                    const auto color = qca_technology::is_input_cell(type)    ? siqad::INPUT_COLOR :
                                       qca_technology::is_output_cell(type)   ? siqad::OUTPUT_COLOR :
                                       qca_technology::is_constant_cell(type) ? siqad::CONST_COLOR :
                                                                                siqad::NORMAL_COLOR;

                    if (!qca_technology::is_const_1_cell(type))
                    {
                        // top left
                        design << fmt::format(siqad::DBDOT_BLOCK, c.x * 14, c.y * 7, 0, color);
                        // bottom right
                        design << fmt::format(siqad::DBDOT_BLOCK, (c.x * 14) + 6, (c.y * 7) + 3, 0, color);
                    }
                    if (!qca_technology::is_const_0_cell(type))
                    {
                        // top right
                        design << fmt::format(siqad::DBDOT_BLOCK, (c.x * 14) + 6, c.y * 7, 0, color);
                        // bottom left
                        design << fmt::format(siqad::DBDOT_BLOCK, c.x * 14, (c.y * 7) + 3, 0, color);
                    }
                }
            });
    }
};

}  // namespace detail

template <typename Lyt>
void write_sqd_layout(const Lyt& lyt, std::ofstream& os)
{
    static_assert(std::is_same_v<typename Lyt::technology, qca_technology> ||
                      std::is_same_v<typename Lyt::technology, sidb_technology>,
                  "Lyt must be a QCA or SiDB layout");

    detail::write_sqd_layout_impl p{lyt, os};

    p.run();
}

template <typename Lyt>
void write_sqd_layout(const Lyt& lyt, const std::string& filename)
{
    std::ofstream os{filename.c_str(), std::ofstream::out};
    write_sqd_layout(lyt, os);
    os.close();
}

}  // namespace fiction

#endif  // FICTION_WRITE_SQD_LAYOUT_HPP
