//
// Created by marcel on 06.05.20.
//

#ifndef FICTION_CSV_WRITER_HPP
#define FICTION_CSV_WRITER_HPP


#include <string>
#include <fstream>


class csv_writer
{
public:
    /**
     * Standard constructor. Opens a file.
     *
     * @param filename CSV file to write into.
     */
    explicit csv_writer(const std::string& filename)
            :
            file{filename, std::ios::out | std::ios::app}
    {}
    /**
     * Writes a single line of comma-separated values to the stored file. Note that no escape checks are performed.
     *
     * @tparam Ts Types of the variadic parameter pack.
     * @param args Arguments to write to the file.
     */
    template<typename... Ts>
    void write_line(Ts&&... args)
    {
        if (file.is_open())
            (file << ... << add_delimiter(std::forward<Ts>(args))) << std::endl;
    }

private:
    /**
     * CSV file to write into.
     */
    std::ofstream file;
    /**
     * The delimiter to use.
     */
    static constexpr const char* DELIMITER = ", ";
    /**
     * Writes the given argument to the stored file and returns a delimiter.
     *
     * @tparam T Type of the given parameter.
     * @param arg Argument to write to the file.
     * @return Delimiter.
     */
    template<typename T>
    const char* add_delimiter(T&& arg)
    {
        if (file.is_open())
            file << arg;

        return DELIMITER;
    }
};

#endif //FICTION_CSV_WRITER_HPP
