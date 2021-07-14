//
// Created by marcel on 07.08.19.
//

#include "qcc_writer.h"

namespace qcc
{
    void write(fcn_cell_layout_ptr fcl, const std::string& filename, const bool comp_name)
    {
        const auto bb = fcl->determine_bounding_box();

        const auto x = [&bb](const auto& c){ return c[X] - bb.min_x; };
        const auto y = [&bb](const auto& c){ return c[Y] - bb.min_y; };

        const auto num_magnets = fcl->magcad_magnet_count();

        const auto generate_layout_id_hash = [=]
        {
            std::stringstream ss{};

            ss << fcl->get_name() << LIBRARY_NAME << fcl->get_technology() << num_magnets << bb.x_size - 1 << bb.y_size - 1;

            std::vector<std::string> pin_data{};
            auto store_pin_data = [&pin_data, &x, &y, fcl](const auto& io)
            {
                pin_data.push_back(fmt::format("{}{}{}{}", fcl->get_cell_name(io), x(io), y(io), io[Z]));
            };

            for (auto&& pi : fcl->get_pis() |
                             iter::sorted([](const auto& _c1, const auto& _c2) { return _c1[Y] < _c2[Y]; }))
            {
                store_pin_data(pi);
            }
            for (auto&& po : fcl->get_pos() |
                             iter::sorted([](const auto& _c1, const auto& _c2) { return _c1[Y] < _c2[Y]; }))
            {
                store_pin_data(po);
            }
            std::sort(pin_data.begin(), pin_data.end());
            std::for_each(pin_data.begin(), pin_data.end(), [&ss](auto&& _pdata){ ss << std::move(_pdata); });

            auto hash_fragment = static_cast<uint64_t>(std::hash<std::string>()(ss.str()));

            return fmt::format("{0:<020}13{0:<020}37{0:<020}", hash_fragment);
        };

        const auto has_io_pins = [=, &bb]() -> bool
        {
            // check if all I/Os are border cells
            for (auto&& io : iter::chain(fcl->get_pis(), fcl->get_pos()))
            {
                if (!fcl->is_border(io, bb))
                    return false;
            }

            return true;
        };

        if (fcl->get_technology() == fcn::technology::INML && !has_io_pins())
        {
            throw std::invalid_argument("the iNML layout does not fulfill all requirements to be written as a QCC "
                                        "file because it does not have designated I/O pins or they are not "
                                        "routed to the layout's borders");
        }

        std::ofstream file(filename, std::ofstream::out);

        if (!file.is_open())
            throw std::ofstream::failure("could not open file");

        file << fmt::format(VERSION_HEADER, fiction::VERSION, fiction::REPO);
        file << fmt::format(OPEN_QCA_COMPONENT, fcn::to_string(fcl->get_technology()), LIBRARY_NAME,
                comp_name ? boost::filesystem::path{filename}.stem().string() : fcl->get_name(),
                generate_layout_id_hash(), bb.x_size - 1, bb.y_size - 1, num_magnets);


        file << OPEN_ENTITY;
        for (auto&& pi : fcl->get_pis() |
                         iter::sorted([](const auto& _c1, const auto& _c2) { return _c1[Y] < _c2[Y]; }))
        {
            file << fmt::format(PIN, fcn::to_string(fcl->get_technology()), fcl->get_cell_name(pi), 0, x(pi), y(pi));
        }
        for (auto&& po : fcl->get_pos() |
                         iter::sorted([](const auto& _c1, const auto& _c2) { return _c1[Y] < _c2[Y]; }))
        {
            file << fmt::format(PIN, fcn::to_string(fcl->get_technology()), fcl->get_cell_name(po), 1, x(po), y(po));
        }
        file << CLOSE_ENTITY;


        file << OPEN_COMPONENTS;
        for (const auto& comp : components)
            file << fmt::format(COMPONENT_ITEM, fcn::to_string(fcl->get_technology()), comp);
        file << CLOSE_COMPONENTS;


        std::unordered_set<fcn_cell_layout::cell, boost::hash<fcn_cell_layout::cell>> skip{};

        file << OPEN_LAYOUT;
        for (auto row : iter::range(fcl->y()))
        {
            for (auto col : iter::range(fcl->x()))
            {
                const auto c = fcn_cell_layout::cell{col, row, GROUND};
                const auto type = fcl->get_cell_type(c);

                // skip free cells and cells marked as to be skipped as well (duh...)
                if (fcl->is_free_cell(c) || skip.count(c) > 0u)
                    continue;

                // if an AND or OR structure is encountered, the next two magnets in southern direction need to be skipped
                if (type == fcn::inml::SLANTED_EDGE_UP_MAGNET || type == fcn::inml::SLANTED_EDGE_DOWN_MAGNET)
                {
                    skip.insert({c[X], c[Y] + 1, GROUND});
                    skip.insert({c[X], c[Y] + 2, GROUND});
                }
                // if a coupler is encountered, skip all magnets relating to the fan-out structure
                else if (type == fcn::inml::FANOUT_COUPLER_MAGNET)
                {
                    skip.insert({c[X], c[Y] + 1, GROUND});
                    skip.insert({c[X], c[Y] + 2, GROUND});
                    skip.insert({c[X] + 1, c[Y], GROUND});
                    skip.insert({c[X] + 1, c[Y] + 2, GROUND});
                }
                // if a cross wire is encountered, skip all magnets relating to the crossing structure
                else if (type == fcn::inml::CROSSWIRE_MAGNET)
                {
                    skip.insert({c[X] + 2, c[Y], GROUND});
                    skip.insert({c[X], c[Y] + 2, GROUND});
                    skip.insert({c[X] + 1, c[Y] + 1, GROUND});
                    skip.insert({c[X] + 2, c[Y] + 2, GROUND});
                }
                // inverters are single structures taking up 4 magnets in the library, so skip the next 3 if encountered one
                else if (type == fcn::inml::INVERTER_MAGNET)
                {
                    skip.insert({c[X] + 1, c[Y], GROUND});
                    skip.insert({c[X] + 2, c[Y], GROUND});
                    skip.insert({c[X] + 3, c[Y], GROUND});
                }

                if (auto it = component_selector.find(type); it != component_selector.end())
                {
                    file << fmt::format(OPEN_LAYOUT_ITEM, it->second, x(c), y(c));
                }
                else
                {
                    std::cout << fmt::format("[w] cell at position {} has an unsupported type", c) << std::endl;
                }

                file << fmt::format(LAYOUT_ITEM_PROPERTY, PROPERTY_PHASE, fcl->cell_clocking(c).value_or(0));

                if (type == fcn::inml::INVERTER_MAGNET)
                {
                    file << fmt::format(LAYOUT_ITEM_PROPERTY, PROPERTY_LENGTH, 4);
                }

                file << CLOSE_LAYOUT_ITEM;
            }
        }
        file << CLOSE_LAYOUT;

        file << CLOSE_QCA_COMPONENT << std::endl;
    }
}
