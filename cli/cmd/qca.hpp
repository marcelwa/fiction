//
// Created by marcel on 24.10.19.
//

#ifndef FICTION_QCA_HPP
#define FICTION_QCA_HPP

#include <fiction/io/write_qca_layout.hpp>
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
 * Generates a QCADesigner file for the current cell layout in store and writes it to the given path.
 *
 * QCADesigner is available at: https://waluslab.ece.ubc.ca/qcadesigner/
 */
class qca_command : public command
{
  public:
    /**
     * Standard constructor. Adds descriptive information, options, and flags.
     *
     * @param e alice::environment that specifies stores etc.
     */
    explicit qca_command(const environment::ptr& e) :
            command(e, "Generates a QCADesigner file for the current QCA cell layout in store. "
                       "QCADesigner can be used to perform physical simulations.")
    {
        add_option("filename", filename, "QCA file name");
    }

  protected:
    /**
     * Function to perform the output call. Generates a QCADesigner file.
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

        constexpr const auto is_qca = [](auto&& lyt)
        {
            using Lyt = typename std::decay_t<decltype(lyt)>::element_type;

            return std::is_same_v<typename Lyt::technology, fiction::qca_technology>;
        };

        const auto get_name = [](auto&& lyt) -> std::string { return lyt->get_layout_name(); };

        constexpr const auto get_tech_name = [](auto&& lyt)
        {
            using Lyt = typename std::decay_t<decltype(lyt)>::element_type;

            return fiction::tech_impl_name<typename Lyt::technology>;
        };

        const auto write_qca = [this](auto&& lyt) { fiction::write_qca_layout(*lyt, filename); };

        auto lyt = s.current();

        if (!std::visit(is_qca, lyt))
        {
            env->out() << fmt::format("[e] {}'s cell technology is not QCA but {}", std::visit(get_name, lyt),
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
        // add .qca file extension if necessary
        if (std::filesystem::path(filename).extension() != ".qca")
        {
            filename += ".qca";
        }

        try
        {
            std::visit(write_qca, lyt);
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
     * File name to write the QCA file into.
     */
    std::string filename;
};

ALICE_ADD_COMMAND(qca, "I/O")

}  // namespace alice

#endif  // FICTION_QCA_HPP
