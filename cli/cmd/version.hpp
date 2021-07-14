//
// Created by marcel on 24.10.19.
//

#ifndef FICTION_VERSION_HPP
#define FICTION_VERSION_HPP

#include "utils/version_info.hpp"

#include <alice/alice.hpp>
#include <fmt/format.h>

namespace alice
{
/**
 * Outputs version and build information.
 */
class version_command : public command
{
  public:
    /**
     * Standard constructor. Adds descriptive information, options, and flags.
     *
     * @param e alice::environment that specifies stores etc.
     */
    explicit version_command(const environment::ptr& e) :
            command(e, "Outputs the version string as well as build time and date.")
    {}

  protected:
    /**
     * Function to perform the version print call.
     */
    void execute() override
    {
        env->out() << fmt::format("[i] {} - compiled on {} at {}\n", fiction::FICTION_VERSION, fiction::COMPILED_DATE,
                                  fiction::COMPILED_TIME)
                   << fmt::format("[i] {}", fiction::FICTION_REPO) << std::endl;
    }
};

ALICE_ADD_COMMAND(version, "General")

}  // namespace alice

#endif  // FICTION_VERSION_HPP
