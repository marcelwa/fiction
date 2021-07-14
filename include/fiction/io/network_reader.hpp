//
// Created by marcel on 01.09.20.
//

#ifndef FICTION_NETWORK_READER_HPP
#define FICTION_NETWORK_READER_HPP

#include <lorina/aiger.hpp>
#include <lorina/common.hpp>
#include <lorina/diagnostics.hpp>
#include <lorina/verilog.hpp>
#include <mockturtle/io/aiger_reader.hpp>
#include <mockturtle/io/verilog_reader.hpp>

#include <algorithm>
#include <array>
#include <filesystem>
#include <ostream>
#include <string>
#include <vector>

namespace fiction
{

template <typename NtkPtr>
class network_reader
{
  public:
    /**
     * Standard constructor. Reads and constructs logic networks. May throw.
     *
     * @param filename Path to the file or folder of files to read.
     * @param out Output stream to write status updates into.
     */
    network_reader(const std::string& filename, std::ostream& o) : out{o}
    {
        constexpr const char* VERILOG_EXT = ".v";
        constexpr const char* AIG_EXT     = ".aig";

        // checks for extension validity
        auto is_valid_extension = [&](const auto& p) -> bool
        {
            std::array<const char*, 2> extensions{{VERILOG_EXT, AIG_EXT}};
            return std::any_of(extensions.cbegin(), extensions.cend(),
                               [&p](const auto& valid) { return std::filesystem::path(p).extension() == valid; });
        };

        std::vector<std::string> paths{};

        // check for for given file's properties
        if (std::filesystem::exists(filename))
        {
            if (std::filesystem::is_regular_file(filename))
            {
                // collect valid files only
                if (is_valid_extension(filename))
                    paths.push_back(filename);
            }

            else if (std::filesystem::is_directory(filename))
            {
                // iterate over each file in the directory
                for (auto& file : std::filesystem::directory_iterator(filename))
                {
                    if (std::filesystem::is_regular_file(file))
                    {
                        // collect valid files only
                        if (is_valid_extension(file))
                            paths.push_back(file.path().string());
                    }
                }
            }
            else  // existing file but a weird one
                out << "[e] given file name does not point to a regular file" << std::endl;
        }
        else  // path leads nowhere
            out << "[e] given file name does not exist" << std::endl;

        // handle collected files
        for (const auto& p : paths)
        {
            // parse Verilog
            if (std::filesystem::path(p).extension() == VERILOG_EXT)
            {
                read<mockturtle::verilog_reader<Ntk>,
                     lorina::return_code(const std::string&, const lorina::verilog_reader&,
                                         lorina::diagnostic_engine*)>(p, lorina::read_verilog);
            }
            // parse Aiger
            else if (std::filesystem::path(p).extension() == AIG_EXT)
            {
                read<mockturtle::aiger_reader<Ntk>,
                     lorina::return_code(const std::string&, const lorina::aiger_reader&, lorina::diagnostic_engine*)>(
                    p, lorina::read_aiger);
            }
            // parse ...
            // else if (std::filesystem::path(p).extension() == ...)
        }
    }
    /**
     * Getter for networks generated from given files.
     *
     * @param sorted Flag to indicate that the networks are to be returned in ascending order of their vertex count.
     * @return Vector of read and constructed logic networks.
     */
    const std::vector<NtkPtr>& get_networks(bool sorted = false)
    {
        // sort by network size to make the small ones go first
        if (sorted)
        {
            std::sort(networks.begin(), networks.end(),
                      [](auto n1, auto n2) { return n1->num_gates() >= n2->num_gates(); });
        }

        return networks;
    }

  private:
    /**
     * Output stream to which status updates are to be written.
     */
    std::ostream& out;
    /**
     * Vector of logic networks read from files given at construction.
     */
    std::vector<NtkPtr> networks;

    using Ntk = typename NtkPtr::element_type;
    /**
     * Actual read function that constructs the logic network from a file.
     *
     * @tparam Reader The functor type which constructs the network, i.e. handles the callbacks.
     * @tparam ReadFun The file parser type.
     * @param file The file name.
     * @param rfun The actual parsing function.
     */
    template <class Reader, class ReadFun>
    void read(const std::string& file, ReadFun rfun) noexcept
    {
        Ntk ntk{};

        try
        {
            lorina::text_diagnostics client{};
            if (lorina::diagnostic_engine diag{&client}; rfun(file, Reader{ntk}, &diag) == lorina::return_code::success)
            {
                const auto name = std::filesystem::path{file}.stem().string();
                networks.push_back(std::make_shared<Ntk>(ntk, name));
            }
            else
            {
                out << "[e] parsing error in " << file << std::endl;
            }
        }
        catch (...)
        {
            out << "[e] " << file << " contains unsupported features" << std::endl;
        }
    }
};

}  // namespace fiction

#endif  // FICTION_NETWORK_READER_HPP
