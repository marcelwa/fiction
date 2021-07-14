//
// Created by marcel on 18.05.18.
//

#ifndef FICTION_LOGIC_NETWORK_H
#define FICTION_LOGIC_NETWORK_H

#include "bidirectional_graph.h"
#include "operations.h"
#include "fmt/format.h"
#include "fmt/ostream.h"
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/filesystem.hpp>
#include <itertools.hpp>
#include <mockturtle/networks/mig.hpp>
#include <mockturtle/views/names_view.hpp>
#include <mockturtle/views/topo_view.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/io/aiger_reader.hpp>
#include <mockturtle/algorithms/simulation.hpp>
#include <kitty/dynamic_truth_table.hpp>

/**
 * Special graph structure with annotated vertices representing a logic_network. Each vertex holds a function from
 * operations.h. More functions can be implemented there easily. Note that this class provides topological information
 * only even though it could be extended pretty easily to also enables logic simulation.
 *
 * To iterate over all vertices of a logic_network n, use
 *  for (auto&& v : n.vertices())
 *  {
 *      ...
 *  }
 *
 * There are more functions returning ranges of vertices. They work analogously.
 *
 * logic_networks have "hidden" PI/PO vertices which are excluded from normal iteration. To iterate over them too, use
 * the respective flag in the call of vertices() and other functions.
 *
 * This class is heavily used by fcn_gate_layout to model gate and wire tiles.
 */
class logic_network : protected bidirectional_graph<operation, boost::no_property, boost::no_property>
{
public:
    /**
     * vertex_t of logic_networks are called vertices for convenience.
     */
    using vertex = vertex_t;
    /**
     * edge_t of logic_networks are called edges for convenience.
     */
    using edge = edge_t;
    /**
     * A sequence of edges can be used as a path.
     */
    using edge_path = edges_t;
    /**
     * Alias for an MIG network wrapped in a view for port names. Used as a logic description.
     */
    using mig_nt = mockturtle::names_view<mockturtle::mig_network>;
    /**
     * Granting equivalence_checker access to private data members.
     */
    friend class equivalence_checker;
    /**
     * Standard constructor. Creates an empty logic network.
     */
    explicit logic_network() noexcept;
    /**
     * Standard constructor. Creates a named empty logic network.
     *
     * @param name Path name from which the logic network was created.
     */
    explicit logic_network(std::string&& name) noexcept;
    /**
     * Standard constructor. Creates a named logic network from an MIG.
     *
     * @param mig MIG network representing a Boolean function.
     * @param name Path name from which the logic network was created.
     */
    logic_network(mig_nt&& mig, std::string&& name) noexcept;
    /**
     * Function alias for add_edge using perfect forwarding and the name create_edge to fit naming in logic_network.
     *
     * @tparam ARGS Stack of argument types.
     * @param args Stack of arguments.
     * @return get_target(args).
     */
    template <typename... ARGS>
    auto create_edge(ARGS&&... args) noexcept
    {
        return add_edge(std::forward<ARGS>(args)...);
    }
    /**
     * Function alias for get_target using perfect forwarding and the name target to fit naming in logic_network.
     *
     * @tparam ARGS Stack of argument types.
     * @param args Stack of arguments.
     * @return get_target(args).
     */
    template <typename... ARGS>
    auto target(ARGS&&... args) const noexcept
    {
        return get_target(std::forward<ARGS>(args)...);
    }
    /**
     * Function alias for get_source using perfect forwarding and the name source to fit naming in
     * logic_network.
     *
     * @tparam ARGS Stack of argument types.
     * @param args Stack of arguments.
     * @return get_source(args).
     */
    template <typename... ARGS>
    auto source(ARGS&&... args) const noexcept
    {
        return get_source(std::forward<ARGS>(args)...);
    }
    /**
     * Function alias for get_index using perfect forwarding and the name index to fit naming in logic_network.
     *
     * @tparam ARGS Stack of argument types.
     * @param args Stack of arguments.
     * @return get_index(args).
     */
    template <typename... ARGS>
    auto index(ARGS&&... args) const noexcept
    {
        return get_index(std::forward<ARGS>(args)...);
    }
    /**
     * Function alias for get_edge using perfect forwarding and the name get_edge to fit naming in
     * logic_network.
     *
     * @tparam ARGS Stack of argument types.
     * @param args Stack of arguments.
     * @return get_edge(args).
     */
    template <typename... ARGS>
    auto get_edge(ARGS&&... args) const noexcept
    {
        return bidirectional_graph::get_edge(std::forward<ARGS>(args)...);
    }
    /**
     * Adds a vertex with the given operation type to the network.
     *
     * @param o Operation type.
     * @return Created vertex.
     */
    vertex create_logic_vertex(const operation o) noexcept;
    /**
     * Removes the given vertex from the network.
     *
     * NOTE that this might invalidate mappings as all node IDs will be recomputed.
     *
     * @param v Vertex to remove.
     */
    void remove_logic_vertex(const vertex v) noexcept;
    /**
     * Creates a primary input vertex in the network.
     *
     * @param name Name associated with the input port.
     * @return Created vertex.
     */
    vertex create_pi(const std::string& name = "") noexcept;
    /**
     * Creates a primary output vertex in the network.
     *
     * @param name Name associated with the output port.
     */
    vertex create_po(const std::string& name = "") noexcept;
    /**
     * Creates a primary output vertex in the network and connects it to the given input signal.
     *
     * @param a Input signal to the newly created PO.
     * @param name Name associated with the output port.
     */
    vertex create_po(const vertex a, const std::string& name = "") noexcept;
    /**
     * Creates a NOT gate in the network with the given vertex as input signal.
     *
     * @param a Input signal to the newly created inverter.
     * @return The created inverter vertex.
     */
    vertex create_not(const vertex a) noexcept;
    /**
     * Creates a AND gate in the network with the given vertices as input signals.
     *
     * @param a Input signal 1 to the newly created conjunction.
     * @param b Input signal 2 to the newly created conjunction.
     * @return The created conjunction vertex.
     */
    vertex create_and(const vertex a, const vertex b) noexcept;
    /**
     * Creates an OR gate in the network with the given vertices as input signals.
     *
     * @param a Input signal 1 to the newly created disjunction.
     * @param b Input signal 2 to the newly created disjunction.
     * @return The created disjunction vertex.
     */
    vertex create_or(const vertex a, const vertex b) noexcept;
    /**
     * Creates a MAJ gate in the network with the given vertices as input signals.
     *
     * @param a Input signal 1 to the newly created majority.
     * @param b Input signal 2 to the newly created majority.
     * @param c Input signal 3 to the newly created majority.
     * @return The created majority vertex.
     */
    vertex create_maj(const vertex a, const vertex b, const vertex c) noexcept;
    /**
     * Replaces the given edge e (s->e->t) with a balance vertex b of operation W and two new edges (s->e'->b->e''->t).
     *
     * @param e Edge to substitute.
     * @return Inserted balance vertex.
     */
    vertex create_balance_vertex(const edge& e) noexcept;
    /**
     * Returns a range of vertices in the network. The range can be parameterized to specify whether I/Os should be
     * included.
     *
     * @param ios Flag to indicate that I/Os should be included in the range.
     * @return Range of vertices that might include I/Os depending on parameters.
     */
    auto vertices(const bool ios = false) const noexcept
    {
        auto logic_selector = [this, ios = ios](const vertex _v) -> bool
        {
            return !ios && is_io(_v);
        };

        return get_vertices() | iter::filterfalse(logic_selector);
    }
    /**
     * Returns a range of vertices adjacent to the given vertex v. The range can be parameterized to specify whether
     * I/Os should be included.
     *
     * @param v Vertex whose adjacent vertices are desired.
     * @param ios Flag to indicate that I/Os should be included in the range.
     * @return Range of vertices adjacent to v that might include I/Os depending on parameters.
     */
    auto adjacent_vertices(const vertex v, const bool ios = false) const noexcept
    {
        auto logic_selector = [this, ios = ios](const vertex _v) -> bool
        {
            return !ios && is_io(_v);
        };

        return get_adjacent_vertices(v) | iter::filterfalse(logic_selector);
    }
    /**
     * Returns a range of vertices inversely adjacent to the given vertex v. The range can be parameterized to specify
     * whether I/Os should be included.
     *
     * @param v Vertex whose inversely adjacent vertices are desired.
     * @param ios Flag to indicate that I/Os should be included in the range.
     * @return Range of vertices inversely adjacent to v that might include I/Os depending on parameters.
     */
    auto inv_adjacent_vertices(const vertex v, const bool ios = false) const noexcept
    {
        auto logic_selector = [this, ios = ios](const vertex _v) -> bool
        {
            return !ios && is_io(_v);
        };

        return get_inv_adjacent_vertices(v) | iter::filterfalse(logic_selector);
    }
    /**
     * Returns the number of vertices in the network. The function can be parameterized to specify whether I/Os should
     * be included.
     *
     * Calculation is performed as #V [- #I - #O] (brackets denote options).
     *
     * @param ios Flag to indicate that I/Os should be counted as vertices.
     * @return Number of vertices with respect to the flags.
     */
    num_vertices_t vertex_count(const bool ios = false) const noexcept;
    /**
     * Returns a range of edges in the network. The range can be parameterized to specify whether edges leading towards
     * or from I/Os should be included.
     *
     * @param ios Flag to indicate that edges leading towards or from I/Os should be included in the range.
     * @return Range of edges that might include such leading towards or from I/Os depending on parameters.
     */
    auto edges(const bool ios = false) const noexcept
    {
        auto logic_selector = [this, ios = ios](const edge& _e) -> bool
        {
            return !ios && (is_io(source(_e)) || is_io(target(_e)));
        };

        return get_edges() | iter::filterfalse(logic_selector);
    }
    /**
     * Returns a range of edges outgoing from the given vertex v. The range can be parameterized to specify whether
     * edges leading towards I/Os should be included.
     *
     * @param v Vertex whose outgoing edges are desired.
     * @param ios Flag to indicate that edges leading towards I/Os should be included in the range.
     * @return Range of edges outgoing from v that might include edges towards I/Os depending on parameters.
     */
    auto out_edges(const vertex v, const bool ios = false) const noexcept
    {
        auto logic_selector = [this, ios = ios](const edge& _e) -> bool
        {
            return !ios && is_io(target(_e));
        };

        return get_out_edges(v) | iter::filterfalse(logic_selector);
    }
    /**
     * Returns a range of edges incoming to the given vertex v. The range can be parameterized to specify whether
     * edges coming from I/Os should be included.
     *
     * @param v Vertex whose incoming edges are desired.
     * @param ios Flag to indicate that edges coming from I/Os should be included in the range.
     * @return Range of edges incoming to v that might include edges from I/Os depending on parameters.
     */
    auto in_edges(const vertex v, const bool ios = false) const noexcept
    {
        auto logic_selector = [this, ios = ios](const edge& _e) -> bool
        {
            return !ios && is_io(source(_e));
        };

        return get_in_edges(v) | iter::filterfalse(logic_selector);
    }
    /**
     * Returns the number of edges in the network. The function can be parameterized to specify whether edges connecting
     * I/Os should be included.
     *
     * Calculation is performed as #E [- #I - #O] (brackets denote options).
     *
     * @param ios Flag to indicate that I/Os should be counted as edges.
     * @return Number of edges with respect to the flags.
     */
    num_edges_t edge_count(const bool ios = false) const noexcept;
    /**
     * Returns the number of outgoing edges of the given vertex v. The function can be parameterized to specify whether
     * edges to I/Os should be included.
     *
     * @param v Vertex whose out degree is desired.
     * @param ios Flag to indicate that edges to I/Os should be counted.
     * @return Number of v's outgoing edges with respect to the flags.
     */
    degree_t out_degree(const vertex v, const bool ios = false) const noexcept;
    /**
     * Returns the number of incoming edges of the given vertex v. The function can be parameterized to specify whether
     * edges from I/Os should be included.
     *
     * @param v Vertex whose in degree is desired.
     * @param ios Flag to indicate that edges from I/Os should be counted.
     * @return Number of v's incoming edges with respect to the flags.
     */
    degree_t in_degree(const vertex v, const bool ios = false) const noexcept;
    /**
     * Assigns vertex v with operation o.
     *
     * @param v vertex which should be associated with operation o.
     * @param o Operation which should be assigned to vertex v.
     */
    void assign_op(const vertex v, const operation o) noexcept;
    /**
     * Returns the operation assigned to vertex v.
     *
     * @param v vertex whose assigned operation is desired.
     * @return Operation associated with v.
     */
    operation get_op(const vertex v) const noexcept;
    /**
     * Returns whether or not given vertex v is a PI port as by set entry.
     *
     * @param v vertex whose assigned PI port should be checked.
     * @return true iff v is a PI port.
     */
    bool is_pi(const vertex v) const noexcept;
    /**
     * Returns whether or not given vertex v has predecessors which are PIs as by set entry.
     *
     * @param v vertex whose predecessors should be checked for PI status.
     * @return True if v has PI predecessors.
     */
    bool pre_pi(const vertex v) const noexcept;
    /**
     * Returns the number of primary input flagged vertices in the network.
     *
     * @return Number of PIs as an unsigned integral type.
     */
    auto num_pis() const noexcept
    {
        return pi_set.size();
    }
    /**
     * Returns a range of all vertices flagged as primary input in the network.
     *
     * @return range_t of all primary input vertices.
     */
    auto get_pis() const noexcept
    {
        return range_t<primary_set::const_iterator>{std::make_pair(pi_set.begin(), pi_set.end())};
    }
    /**
     * Returns whether or not given vertex v is a PO port as by set entry.
     *
     * @param v vertex whose assigned PO port should be checked.
     * @return true iff v is a PO port.
     */
    bool is_po(const vertex v) const noexcept;
    /**
     * Returns whether or not given vertex v has successors which are POs as by set entry.
     *
     * @param v vertex whose successors should be checked for PO status.
     * @return True if v has PO successors.
     */
    bool post_po(const vertex v) const noexcept;
    /**
     * Returns the number of primary output flagged vertices in the network.
     *
     * @return Number of POs as an unsigned integral type.
     */
    auto num_pos() const noexcept
    {
        return po_set.size();
    }
    /**
     * Returns a range of all vertices flagged as primary output in the network.
     *
     * @return range_t of all primary output vertices.
     */
    auto get_pos() const noexcept
    {
        return range_t<primary_set::const_iterator>{std::make_pair(po_set.begin(), po_set.end())};
    }
    /**
     * Returns true iff given vertex v is an explicit PI or PO node.
     *
     * @param v Vertex to test for I/O-ness.
     * @return True iff v is I/O.
     */
    bool is_io(const vertex v) const noexcept;
    /**
     * Returns the number of operations of given type in the network.
     *
     * @param o Type of operation whose count is desired.
     * @return Number of operations of type o in the network.
     */
    uint64_t operation_count(const operation o) const noexcept;
    /**
     * Checks whether the network only contains MAJ and NOT logic vertices. As non-logic vertices,
     * fan-outs, and auxiliary wire vertices are accepted as well.
     *
     * @return True iff network is an Majority-Inverter-Graph.
     */
    bool is_MIG() const noexcept;
    /**
     * Checks whether the network only contains AND and NOT logic vertices. As non-logic vertices,
     * fan-outs, and auxiliary wire vertices are accepted as well.
     *
     * @return True iff network is an AND-Inverter-Graph.
     */
    bool is_AIG() const noexcept;
    /**
     * Checks whether the network only contains OR and NOT logic vertices. As non-logic vertices,
     * fan-outs, and auxiliary wire vertices are accepted as well.
     *
     * @return True iff network is an OR-Inverter-Graph.
     */
    bool is_OIG() const noexcept;
    /**
     * Checks whether the network only contains AND, OR, and NOT logic vertices. As non-logic vertices,
     * fan-outs, and auxiliary wire vertices are accepted as well.
     *
     * @return True iff network is an AND-OR-Inverter-Graph.
     */
    bool is_AOIG() const noexcept;
    /**
     * Checks whether the network only contains MAJ, AND, OR, and NOT logic vertices. As non-logic vertices,
     * fan-outs, and auxiliary wire vertices are accepted as well.
     *
     * @return True iff network is an Majority-AND-OR-Inverter-Graph.
     */
    bool is_MAOIG() const noexcept;
    /**
     * Returns the stored I/O port name of given vertex v. If no port name was stored before, an empty string is
     * returned.
     *
     * @param v Vertex whose port name is desired.
     * @return Port name of v if there is one, "" otherwise.
     */
    std::string get_port_name(const vertex v) const noexcept;
    /**
     * Returns the stored file path name.
     *
     * @return The stored file path name.
     */
    std::string get_name() const noexcept;
    /**
     * Returns a vector of all possible paths to reach the given vertex within the logic_network. Function can be
     * parameterized to define whether I/Os should be considered as well.
     *
     * Each vertex without predecessors is considered a terminal and a path is defined as a sequence of edges.
     *
     * @param v Vertex to which all paths should lead.
     * @param ios Flag to indicate that I/Os should be considered as path elements.
     * @return A vector of all possible edge paths leading from terminals to v.
     */
    std::vector<edge_path> get_all_paths(const vertex v, const bool ios = false) noexcept;
    /**
     * Computes truth tables for each primary output in the network.
     *
     * @return Vector of truth tables.
     */
    std::vector<kitty::dynamic_truth_table> simulate() const;
    /**
     * Strategies for breaking down gates in a depth first or breadth first way. Depth creates deeper networks, while
     * breadth creates wider ranks.
     */
    enum substitution_strategy { BREADTH, DEPTH };
    /**
     * Substitutes multi-outputs of gate vertices and replaces them with fan-out ones. That is, connections become
     * "gates". Some algorithms require fan-out vertices explicitly and will break down networks if needed. Others might
     * be able to handle both.
     *
     * @param degree Maximum number of outputs a fan-out vertex is allowed to have. Currently, there are different
     *               operation types for them, F1O2 and F1O3, which means, only 2 and 3 are supported.
     * @param stgy Strategy to align fan-outs. They can form a balanced tree (BREADTH) or a DFS tree (DEPTH).
     * @param threshold Maximum number of outputs an AND/OR/MAJ gate can have before substitution applies.
     */
    void substitute_fan_outs(const std::size_t degree = 2u, const substitution_strategy stgy = substitution_strategy::BREADTH,
                             const std::size_t threshold = 1u) noexcept;
    /**
     * Writes a Graphviz (https://www.graphviz.org/) dot representation of the logic_network to the given ostream.
     * Incorporates the logic function names.
     *
     * @param os Channel into which the Graphviz representation should be written.
     */
    void write_dot(std::ostream& os = std::cout) const noexcept;
    /**
     * Writes a less detailed Graphviz (https://www.graphviz.org/) dot representation of the logic_network to the given
     * ostream. Incorporates the logic function names.
     *
     * @param os Channel into which the Graphviz representation should be written.
     */
    void write_simple_dot(std::ostream& os = std::cout) const noexcept;
    /**
     * Re-initializes an empty logic network.
     */
    void clear_network() noexcept;
private:
    /**
     * Path name from which the logic_network was created.
     */
    const std::string name;
    /**
     * Boolean function representation as an MIG network.
     */
    mig_nt mig;
    /**
     * Alias for a set that holds vertices to represent PI/PO ports.
     */
    using primary_set = std::set<vertex>;
    /**
     * Stores vertices that are marked as PIs/POs. Helper functions for access save memory.
     */
    primary_set pi_set{}, po_set{};
    /**
     * Alias for a bidirectional hash map of vertices to names specifying port nodes in the network.
     */
    using port_map = boost::bimap<boost::bimaps::unordered_set_of<vertex, boost::hash<vertex>>,
                                  boost::bimaps::unordered_set_of<std::string, boost::hash<std::string>>>;
    /**
     * Stores I/O port names.
     */
    port_map io_port_map{};
    /**
     * Counts the operations the network is composed of.
     */
    std::vector<uint64_t> operation_counter;
    /**
     * Increases the operation counter of the given operation by 1.
     *
     * @param o Operation type to increment.
     */
    void increment_op_counter(const operation o) noexcept;
    /**
     * Decrements the operation counter of the given operation by 1.
     *
     * @param o Operation type to decrement.
     */
    void decrement_op_counter(const operation o) noexcept;
    /**
     * Uses the stored logic description to (re-)initialize the network structure.
     */
    void initialize_network_from_logic() noexcept;
};

using logic_network_ptr = std::shared_ptr<logic_network>;


#endif //FICTION_LOGIC_NETWORK_H
