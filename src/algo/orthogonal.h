//
// Created by marcel on 13.07.17.
//

#ifndef FICTION_ORTHOGONAL_H
#define FICTION_ORTHOGONAL_H

#include "physical_design.h"

/**
 * A heuristic physical design approach based on orthogonal graph drawing. A slight modification of
 * Therese C. Biedl's improved algorithm for drawing of 3-graphs is used because the original
 * one works for undirected graphs only. Modification includes using directions of the logic network
 * directly instead of relabeling the edges according to its DFS tree, ordering the vertices
 * using jDFS instead of DFS, and adding an extra placement rule for nodes without predecessors.
 *
 * The algorithm works in linear time O(2|V| + |E|). Produced layout has a size of x * y, where
 * x + y = |V| - |PI| + 1. This is because each vertex leads to either one extra row or column
 * except for those without predecessors which create both.
 *
 * This is a proof of concept implementation for a scalable physical design approach for FCN.
 * It is not meant to be used for arranging fabricable circuits, as area is far from being optimal.
 */
class orthogonal : public physical_design
{
public:
    /**
     * Standard constructor.
     *
     * @param ln Logic network.
     * @param n Number of clock phases.
     * @param io Flag to indicate use of I/O ports.
     * @param border Flag to indicate that I/O ports should be routed to the layout's borders.
     */
    orthogonal(logic_network_ptr ln, const unsigned n, const bool io = false, const bool border = false);
    /**
     * Starts the physical design process. Computes the jdfs ordering of the stored logic network first and colors the
     * edges accordingly using an red-blue-coloring algorithm so that all incoming edges of a
     * vertex have the same color and all outgoing edges have different colors. Physical design is then
     * performed using orthogonal graph drawing where the vertices are placed in jdfs order
     * so that each blue edges leads downwards and each red edge leads rightwards.
     *
     * Information flow is from top left to bottom right. The resulting clocking scheme is diagonal.
     *
     * Returns a pd_result eventually.
     *
     * @return Result type containing statistical information about the process.
     */
    pd_result operator()() override;
private:
    /**
     * Number of clock phases to use in the diagonal clocking scheme.
     */
    const unsigned phases;
    /**
     * Flag to indicate that designated I/O ports should be routed too.
     */
    const bool io_ports;
    /**
     * Flag to indicate that designated I/O ports should be routed to the layout's borders.
     */
    const bool border_ios;
    /**
     * Colors used for a red-blue-coloring of 3-graphs.
     */
    enum class rb_color : unsigned char { WHITE = 0u, RED, BLUE };
    /**
     * Alias for a joint DFS ordering on logic networks.
     */
    using jdfs_ordering = std::vector<logic_network::vertex>;
    /**
     * Alias for a red-blue-coloring of logic edges.
     */
    using red_blue_coloring = std::unordered_map<logic_network::edge, rb_color, boost::hash<logic_network::edge>>;
    /**
     * Traverses the stored logic network in a joint DFS way and returns an ordering corresponding to the traversing.
     * Joint DFS is a self developed traversing algorithm used for making Biedl's graph drawing algorithm on 3-Graphs
     * working for directed graphs as well with respect to FCN technology constraints. (Ignoring directions of
     * graphs leads to valid placements but to contradictions in the FCN clocking as well.)
     *
     * This traversing starts at a PI and performs DFS but will not visit nodes who have undiscovered predecessors.
     * This process is then repeated for each other PI as well. Eventually all nodes will have been visited.
     *
     * @return Joint DFS ordering of the stored logic network.
     */
    jdfs_ordering jdfs_order() const;
    /**
     * Computes a red blue coloring for the stored logic network. A red blue coloring is a mapping from
     * edges to either red or blue such that all ingoing edges to a vertex v have the same color
     * and all outgoing edges from a vertex v have different colors.
     *
     * @param jdfs Joint DFS ordering of the stored logic network.
     * @return Red-blue-coloring for the logic network.
     */
    red_blue_coloring find_rb_coloring(const jdfs_ordering& jdfs) const noexcept;
    /**
     * Calculates the resulting layout dimension from the given red_blue_coloring. The layout will have size X * Y where
     * X = #RED + #WHITE + #PI, Y = #BLUE + #WHITE + #PI.
     *
     * @param rb_coloring The coloring to consider.
     * @return The resulting layout dimension calculated from the given rb_coloring.
     */
    fcn_dimension_xy determine_layout_size(const red_blue_coloring& rb_coloring) const noexcept;
    /**
     * Elongates the I/O wires so that the pins are located at the layout's borders.
     */
    void elongate_ios() const noexcept;
    /**
     * Computes a placement and a routing for the stored logic network with respect to the given red-blue-coloring.
     * It is mapped to an adequate layout which is stored within the class.
     * The improved algorithm for drawing 3-graphs by Biedl is used with a modification to work for directed graphs.
     *
     * @param rb_coloring Red-blue-coloring for the stored logic network.
     * @param jdfs Stores orderings, DFS tree, etc.
     */
    void orthogonal_embedding(red_blue_coloring& rb_coloring, const jdfs_ordering& jdfs);
};


#endif //FICTION_ORTHOGONAL_H
