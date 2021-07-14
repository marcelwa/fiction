//
// Created by marcel on 25.11.19.
//

#ifndef FICTION_AREA_HPP
#define FICTION_AREA_HPP

#include "area_defaults.h"
#include "fcn_cell_layout.h"

#include <alice/alice.hpp>

namespace alice
{
/**
 * Prints the area usage of the current cell layout in nm². Uses standard values or given ones.
 */
class area_command : public command
{
  public:
    /**
     * Standard constructor. Adds descriptive information, options, and flags.
     *
     * @param e alice::environment that specifies stores etc.
     */
    explicit area_command(const environment::ptr& e) :
            command(e, "Prints the area usage (in nm²) of the current cell layout in store. Dimensions for each "
                       "cell use default values from QCADesigner (QCA) or NMLSim (iNML) if no value is provided.")
    {
        add_option("--width,-x", width, "Cell width in nm");
        add_option("--height,-y", height, "Cell height in nm");
        add_option("--hspace", hspace, "Horizontal cell spacing in nm");
        add_option("--vspace", vspace, "Vertical cell spacing in nm");
    }

  protected:
    /**
     * Function to perform the energy call. Prints estimated energy consumption for QCA-ONE library.
     */
    void execute() override
    {
        // reset area
        area = 0ul;

        auto& s = store<fcn_cell_layout_ptr>();

        // error case: empty cell layout store
        if (s.empty())
        {
            env->out() << "[w] no cell layout in store" << std::endl;
            return;
        }

        auto fcl = s.current();

        if (!is_set("width"))
        {
            width =
                fcl->get_technology() == fcn::technology::QCA ? area_defaults::qca::width : area_defaults::inml::width;
        }
        if (!is_set("height"))
        {
            height = fcl->get_technology() == fcn::technology::QCA ? area_defaults::qca::height :
                                                                     area_defaults::inml::height;
        }
        if (!is_set("hspace"))
        {
            hspace = fcl->get_technology() == fcn::technology::QCA ? area_defaults::qca::hspace :
                                                                     area_defaults::inml::hspace;
        }
        if (!is_set("vspace"))
        {
            vspace = fcl->get_technology() == fcn::technology::QCA ? area_defaults::qca::vspace :
                                                                     area_defaults::inml::vspace;
        }

        const auto bb = fcl->determine_bounding_box();

        area = (bb.x_size * width + (bb.x_size - 1) * hspace) * (bb.y_size * height + (bb.y_size - 1) * vspace);

        env->out() << fmt::format("[i] {} nm²", area) << std::endl;
    }

  private:
    /**
     * Slow (25 GHz) and fast (100 GHz) energy dissipation values.
     */
    uint64_t area;
    /**
     * Width and height of each cell.
     */
    uint64_t width, height;
    /**
     * Horizontal and vertical spacing between cells.
     */
    uint64_t hspace, vspace;
    /**
     * Logs the resulting information in a log file.
     *
     * @return JSON object containing details about the area usage.
     */
    nlohmann::json log() const override
    {
        return {{"area (nm²)", area}};
    }
};

ALICE_ADD_COMMAND(area, "Technology")
}  // namespace alice

#endif  // FICTION_AREA_HP
