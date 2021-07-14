//
// Created by marcel on 24.10.19.
//

#ifndef FICTION_QCC_HPP
#define FICTION_QCC_HPP

#include "fcn_cell_layout.h"
#include "qcc_writer.h"

#include <alice/alice.hpp>

#include <boost/filesystem.hpp>

namespace alice
{
/**
 * Generates a ToPoliNano/MagCAD component file for the current cell layout in store and writes it to the given path.
 *
 * ToPoliNano and MagCAD are available at: https://topolinano.polito.it/
 */
class qcc_command : public command
{
  public:
    /**
     * Standard constructor. Adds descriptive information, options, and flags.
     *
     * @param e alice::environment that specifies stores etc.
     */
    explicit qcc_command(const environment::ptr& e) :
            command(e, "Generates a ToPoliNano/MagCAD component file for the current iNML cell layout in store. "
                       "ToPoliNano can reuse the component in hierarchical layouts while MagCAD is able to "
                       "perform physical simulations.")
    {
        add_option("filename", filename, "QCC file name");
        add_flag("-c,--component_name", component_name, "Use given file name as the component's identifier");
    }

  protected:
    /**
     * Function to perform the output call. Generates a ToPoliNano/MagCAD component file.
     */
    void execute() override
    {
        auto& s = store<fcn_cell_layout_ptr>();

        // error case: empty cell layout store
        if (s.empty())
        {
            env->out() << "[w] no cell layout in store" << std::endl;
            reset_flags();
            return;
        }

        auto fcl = s.current();
        if (auto tech = fcl->get_technology(); tech != fcn::technology::INML)
        {
            env->out() << "[w] " << fcl->get_name() << "'s cell technology is not iNML but " << tech << std::endl;
            reset_flags();
            return;
        }

        // error case: do not override directories
        if (boost::filesystem::is_directory(filename))
        {
            env->out() << "[e] cannot override a directory" << std::endl;
            reset_flags();
            return;
        }
        // if filename was empty or not given, use stored layout name
        if (filename.empty())
            filename = fcl->get_name();
        // add .qca file extension if necessary
        if (boost::filesystem::extension(filename) != ".qcc")
            filename += ".qcc";

        try
        {
            qcc::write(std::move(fcl), filename, component_name);
        }
        catch (const std::invalid_argument& e)
        {
            env->out() << "[e] " << e.what() << std::endl;
        }
        catch (...)
        {
            env->out() << "[e] an error occurredopd while the file was being written; it could be corrupted"
                       << std::endl;
        }

        reset_flags();
    }

  private:
    /**
     * File name to write the QCC file into.
     */
    std::string filename;
    /**
     * Flag to indicate that the component name should be the filename.
     */
    bool component_name = false;

    /**
     * Reset all flags. Necessary for some reason... alice bug?
     */
    void reset_flags()
    {
        filename       = "";
        component_name = false;
    }
};

ALICE_ADD_COMMAND(qcc, "I/O")
}  // namespace alice

#endif  // FICTION_QCC_HPP
