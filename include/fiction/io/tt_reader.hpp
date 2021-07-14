//
// Created by marcel on 05.05.20.
//

#ifndef FICTION_TT_READER_HPP
#define FICTION_TT_READER_HPP


#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <fstream>
#include <istream>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <vector>


/**
 * Simple reader to parse truth tables from files. The files must be organized as follows: Each line consists of a
 * truth table in hexadecimal form plus its corresponding Boolean expression. The two are separated by a space.
 *
 * This format is used by, e.g., Alan Mishchenko for his DSD functions: https://people.eecs.berkeley.edu/~alanmi/temp5/
 */
class tt_reader
{
public:
    /**
     * Standard constructor. Reads the given file in a vector of lines.
     *
     * @param filename File to parse.
     */
    explicit tt_reader(const std::string& filename);
    /**
     * Returns the next truth table parsed from the file or std::nullopt if no further truth tables are available.
     *
     * @return Next truth table from the file.
     */
    std::optional<kitty::dynamic_truth_table> next();

private:
    /**
     * Contains the hexadecimal truth tables and their Boolean expressions in string representation. The two are
     * alternating.
     */
    std::vector<std::string> elements;
    /**
     * Current position in the vector.
     */
    std::size_t pos = 0ul;
};


#endif //FICTION_TT_READER_HPP
