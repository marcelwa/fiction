//
// Created by marcel on 14.07.21.
//

#ifndef FICTION_SQD_HPP
#define FICTION_SQD_HPP

#include <fiction/io/write_sqd_layout.hpp>
#include <fiction/types.hpp>

#include <alice/alice.hpp>

#include <filesystem>
#include <ostream>
#include <string>
#include <type_traits>
#include <variant>

namespace alice
{
/**
 * Generates a SiQAD file for the current cell layout in store and writes it to the given path.
 *
 * SiQAD is available at: https://waluslab.ece.ubc.ca/siqad/
 */
class sqd_command : public command
{
  public:
    /**
     * Standard constructor. Adds descriptive information, options, and flags.
     *
     * @param e alice::environment that specifies stores etc.
     */
    explicit sqd_command(const environment::ptr& e) :
            command(e, "Generates a SiQAD file for the current QCA or SiDB cell layout in store. "
                       "SiQAD can be used to perform physical simulations.")
    {
        add_option("filename", filename, "SQD file name");
    }

  protected:
    /**
     * Function to perform the output call. Generates a SiQAD file.
     */
    void execute() override
    {
        auto& s = store<fiction::cell_layout_t>();

        // error case: empty cell layout store
        if (s.empty())
        {
            env->out() << "[w] no cell layout in store" << std::endl;
            return;
        }

        constexpr const auto is_qca_or_sidb = [](auto&& lyt)
        {
            using Lyt = typename std::decay_t<decltype(lyt)>::element_type;

            return std::is_same_v<typename Lyt::technology, fiction::qca_technology> ||
                   std::is_same_v<typename Lyt::technology, fiction::sidb_technology>;
        };

        const auto get_name = [](auto&& lyt) -> std::string { return lyt->get_layout_name(); };

        constexpr const auto get_tech_name = [](auto&& lyt)
        {
            using Lyt = typename std::decay_t<decltype(lyt)>::element_type;

            return fiction::tech_impl_name<typename Lyt::technology>;
        };

        const auto write_sqd = [this](auto&& lyt) { fiction::write_sqd_layout(*lyt, filename); };

        auto lyt = s.current();

        if (!std::visit(is_qca_or_sidb, lyt))
        {
            env->out() << fmt::format("[e] {}'s cell technology is not QCA or SiDB but {}", std::visit(get_name, lyt),
                                      std::visit(get_tech_name, lyt))
                       << std::endl;
            return;
        }

        // error case: do not override directories
        if (std::filesystem::is_directory(filename))
        {
            env->out() << "[e] cannot override a directory" << std::endl;
            return;
        }
        // if filename was not given, use stored layout name
        if (!is_set("filename"))
        {
            filename = std::visit(get_name, lyt);
        }
        // add .sqd file extension if necessary
        if (std::filesystem::path(filename).extension() != ".sqd")
        {
            filename += ".sqd";
        }

        try
        {
            std::visit(write_sqd, lyt);
        }
        catch (const std::ofstream::failure& e)
        {
            env->out() << fmt::format("[e] {}", e.what()) << std::endl;
        }
        catch (...)
        {
            env->out() << "[e] an error occurred while the file was being written; it could be corrupted" << std::endl;
        }
    }

  private:
    /**
     * File name to write the SQD file into.
     */
    std::string filename;
};

ALICE_ADD_COMMAND(sqd, "I/O")

}  // namespace alice

#endif  // FICTION_SQD_HPP
