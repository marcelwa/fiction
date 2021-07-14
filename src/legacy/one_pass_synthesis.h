//
// Created by marcel on 09.04.20.
//

#ifndef FICTION_ONE_PASS_SYNTHESIS_H
#define FICTION_ONE_PASS_SYNTHESIS_H


#include "physical_design.h"
#include "onepass_pd_config.h"
#include "iter/dimension_iterator.h"
#include <pybind11/embed.h>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/print.hpp>


/**
 * An exact approach combining logic synthesis and physical design using SAT solving. This class provides the interface
 * via which the technique can be called from the CLI. The instance generation happens in its sub-class mugen_handler.
 *
 * Mugen is a framework for one-pass synthesis of FCN circuit layouts developed by Winston Haaswijk. It can be found
 * on GitHub: https://github.com/whaaswijk/mugen
 *
 * Using iterative SAT calls, an optimal synthesis & placement & routing for a given specification will be found.
 * Starting with n, each possible layout dimension in n tiles will be examined by factorization and tested for
 * realizability using the SAT solver glucose. When no upper bound is given, this approach will run until it finds a
 * solution to the synthesis & placement & routing problem instance under all given constraints. Note that there are
 * combinations of constraints for which no valid solution under the given parameters might exist.
 *
 * Since Mugen is written in Python3, fiction uses pybind11 for interoperability. This can lead to performance and
 * integration issues. Make sure to follow all steps given in the README carefully if you decide to run this code.
 *
 * This approach is still experimental and can be excluded from compilation.
 */
class one_pass_synthesis : public physical_design
{
public:
    /**
     * Standard constructor.
     *
     * @param tts List of truth tables that serve as specification.
     * @param config Configuration object storing all the bounds, flags, and so on.
     */
    one_pass_synthesis(std::vector<kitty::dynamic_truth_table>&& tts, onepass_pd_config&& config);
    /**
     * Starts the physical design process by translating the problem to a Mugen instance. The layout size will be
     * incremented after each UNSAT result until the given upper bound is reached.
     *
     * Returns a pd_result eventually.
     *
     * @return Result type containing statistical information about the process.
     */
    pd_result operator()() override;

private:
    /**
     * Specification of the Boolean functions to synthesize.
     */
    const std::vector<kitty::dynamic_truth_table> spec;
    /**
     * Arguments, flags, and options for the physical design process stored in one configuration object.
     */
    onepass_pd_config config;
    /**
     * Iterates over the possible dimensions for the fcn_gate_layout to find.
     */
    dimension_iterator dit{0};
    /**
     * A Python interpreter instance that is necessary to call Mugen, a library written in Python. This instance is
     * scoped and only need to exist. No operations are to be performed on this object. It handles creation and proper
     * destruction of all Python objects used during this session and deals with the CPython API.
     */
    static const pybind11::scoped_interpreter instance;
    /**
     * Path to the Mugen library.
     */
    const char* MUGEN_PATH = "../libs/mugen/";
    /**
     * Tests whether all needed dependencies have been installed and can be accessed via Python.
     *
     * @return true iff all dependencies are met.
     */
    bool test_dependencies() const;

    /**
     * Sub-class to handle interaction with the Python code Mugen as well as some house-keeping.
     */
    class mugen_handler
    {
    public:
        /**
         * Standard constructor.
         *
         * @param s The Boolean functions to synthesize.
         * @param fgl The gate layout pointer that is going to contain the created layout.
         * @param c The configurations to respect in the SMT instance generation process.
         */
        explicit mugen_handler(const std::vector<kitty::dynamic_truth_table>& s, fcn_gate_layout_ptr fgl, onepass_pd_config c) noexcept;
        /**
         * Evaluates a given dimension regarding the stored configurations whether it can be skipped, i.e. does not
         * need to be explored by Mugen. The better this function is, the more UNSAT instances can be skipped without
         * losing the optimality guarantee. This function should never be overly restrictive!
         *
         * @param dim Dimension to evaluate.
         * @return True if dim can safely be skipped because it is UNSAT anyways.
         */
        bool skippable(const fcn_dimension_xy& dim) const noexcept;
        /**
         * Resizes the layout.
         *
         * @param dim Current dimension to work on.
         */
        void update(const fcn_dimension_xy& dim) noexcept;
        /**
         * Sets the timeout to the given value.
         *
         * @param timeout New timeout value.
         */
        void update(const uint32_t timeout) noexcept;
        /**
         * Passes the current scheme_graph to Mugen and synthesizes it. If there is an implementation on this graph
         * realizing the specification, this function returns true.
         *
         * @return true iff the instance generated for the current configuration is SAT.
         */
        bool is_satisfiable();

    private:
        /**
         * The Boolean functions to synthesize.
         */
        const std::vector<kitty::dynamic_truth_table>& spec;
        /**
         * Number of primary inputs and primary outputs according to spec respectively.
         */
        const uint64_t num_pis;
        /**
         * The sketch that later contains the layout generated from a model.
         */
        fcn_gate_layout_ptr layout;
        /**
         * Configurations specifying layout restrictions. Used in instance generation among other places.
         */
        onepass_pd_config config;
        /**
         * The Python module named Mugen.
         */
        pybind11::module mugen;
        /**
         * Converts a vector of truth tables into a list of lists, i.e., Python data types.
         *
         * @param tts Truth tables.
         * @return Python list of lists representing the truth tables.
         */
        pybind11::list as_py_lists(const std::vector<kitty::dynamic_truth_table>& tts) const;
        /**
         * Generates a scheme_graph object that serves as a blueprint for the synthesis task. It works as an imprint of
         * the layout that is to be created by modeling the data flow connections of the used clocking scheme.
         *
         * @return A scheme_graph object representing the clocking scheme in its dimension and data flow.
         */
        pybind11::object generate_scheme_graph();
        /**
         * Extracts an fcn_gate_layout from the network synthesized by Mugen.
         *
         * @param net Synthesis result returned by Mugen.
         */
        void to_gate_layout(pybind11::handle net) const;
    };

    /**
     * Calculates the time left for solving by subtracting the time passed from the configured timeout and updates
     * Mugen's timeout accordingly.
     *
     * @param handler Handler whose timeout is to be updated.
     * @param time Time passed since beginning of the solving process.
     */
    void update_timeout(mugen_handler& handler, mockturtle::stopwatch<>::duration time) const noexcept;
};


#endif //FICTION_ONE_PASS_SYNTHESIS_H
