//
// Created by marcel on 05.05.20.
//

#include "tt_reader.h"


tt_reader::tt_reader(const std::string& filename)
{
    std::ifstream file{filename, std::ios::in};
    if (file.is_open())
    {
        // read the file in a vector of strings
        elements = std::vector<std::string>{std::istream_iterator<std::string>{file},
                                            std::istream_iterator<std::string>{}};
        // TODO fill the vector with an std::copy_if instead where you increase a counter and only copy if counter is even
    }
}

std::optional<kitty::dynamic_truth_table> tt_reader::next()
{
    if (pos >= elements.size() - 1)
        return std::nullopt;

    // remove the 0x prefix
    auto table = elements[pos].substr(2u);
    // determine number of truth table variables
    auto num_vars = static_cast<uint32_t>(std::log2(table.size() << 2ul));
    // create truth table
    kitty::dynamic_truth_table tt{num_vars};
    kitty::create_from_hex_string(tt, table);

    // TODO change pos to an iterator
    pos += 2;

    return tt;
}
