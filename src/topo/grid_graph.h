//
// Created by marcel on 24.04.18.
//

#ifndef GRID_GRAPH_H
#define GRID_GRAPH_H

#include "range.h"

#include <deque>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>

#include <boost/array.hpp>
#include <boost/functional.hpp>
#include <boost/graph/astar_search.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/grid_graph.hpp>

/**
 * A template class representing a grid graph using the Graph library by Boost.
 *
 * @tparam DIMENSIONS Desired number of dimensions. (1 for a row, 2 for a grid, 3 for a cube etc.)
 */
template <size_t DIMENSIONS>
class grid_graph
{
    /**
     * Grid graph data structure. The template parameter determines its dimension while
     * uint64_t is used to address vertices and edges.
     */
    using grid_container = boost::grid_graph<DIMENSIONS>;

protected:
    /**
     * Stored grid container. All operations are performed on this object.
     */
    std::unique_ptr<grid_container> grid = nullptr;

    // ************************************************************
    // ************************* Aliases **************************
    // ************************************************************

public:
    using edge_t   = typename grid_container::edge_descriptor;
    using vertex_t = typename grid_container::vertex_descriptor;

protected:
    using num_edges_t    = typename grid_container::edges_size_type;
    using edge_index_t   = typename grid_container::edges_size_type;
    using num_vertices_t = typename grid_container::vertices_size_type;
    using vertex_index_t = typename grid_container::vertices_size_type;

    using edge_iter      = typename grid_container::edge_iterator;
    using out_edge_iter  = typename grid_container::out_edge_iterator;
    using in_edge_iter   = typename grid_container::in_edge_iterator;
    using vertex_iter    = typename grid_container::vertex_iterator;
    using adjacency_iter = typename grid_container::adjacency_iterator;

    using vertex_path    = std::deque<vertex_t>;
    using vertex_set     = boost::unordered_set<vertex_t, boost::hash<vertex_t>>;  // using boost here because std is not supported by BGL
    using filtered_grid  = typename boost::vertex_subset_complement_filter<grid_container, vertex_set>::type;

public:
    // ************************************************************
    // *********************** Constructors ***********************
    // ************************************************************

    /**
     * Initializes a grid of dimension DIMENSIONS with the sizes in each dimension as specified by lengths.
     *
     * @param lengths A DIMENSIONS-dimensional array that specifies sizes of each dimension in the grid.
     */
    explicit grid_graph(const boost::array<uint64_t, DIMENSIONS>& lengths) noexcept
            :
            grid(std::make_unique<grid_container>(lengths))
    {}
    /**
     * Resizes an already constructed grid_graph object so that only the stored grid_container is affected but
     * the overall object stays the same. The new grid_container will be initialized with the given lengths.
     *
     * @param lengths A DIMENSIONS-dimensional array that specifies sizes of each dimension in the grid.
     */
    void resize_grid(const boost::array<uint64_t, DIMENSIONS>& lengths) noexcept
    {
        grid = std::make_unique<grid_container>(lengths);
    }



    // ************************************************************
    // ************************* Getters **************************
    // ************************************************************

    /**
     * Returns the grid container itself, i.e. the raw boost::grid_graph object.
     *
     * @return Grid container.
     */
    const std::shared_ptr<grid_container> get_grid() const noexcept
    {
        return std::move(grid);
    }
    /**
     * Returns the number of dimensions of the stored grid.
     *
     * @return Number of dimensions, i.e. value of DIMENSIONS.
     */
    uint64_t dimensions() const noexcept
    {
        return grid->dimensions();
    }
    /**
     * Returns the size of the given dimension.
     *
     * @param dimension Dimension whose size is desired.
     * @return Size of dimension.
     */
    num_vertices_t length(uint64_t dimension) const noexcept
    {
        return grid->length(dimension);
    }
    /**
     * Returns the "next" vertex to v in a dimension at a given distance i.e. step size.
     *
     * @param v Vertex whose "next" vertex is desired.
     * @param dimension Dimension in which to look for next vertex.
     * @param step Distance i.e. step size to take to reach next vertex.
     * @return Vertex found at given specifications.
     */
    vertex_t next(const vertex_t& v, const uint64_t dimension, const unsigned long step = 1u) const noexcept
    {
        return grid->next(v, dimension, step);
    }
    /**
     * Returns the "previous" vertex to v in a dimension at a given distance i.e. step size.
     *
     * @param v Vertex whose "previous" vertex is desired.
     * @param dimension Dimension in which to look for previous vertex.
     * @param step Distance i.e. step size to take to reach previous vertex.
     * @return Vertex found at given specifications.
     */
    vertex_t previous(const vertex_t& v, const uint64_t dimension, const unsigned long step = 1u) const noexcept
    {
        return grid->previous(v, dimension, step);
    }
    /**
     * Returns a vertex associated with the given vertex index.
     *
     * @param v Vertex index to dissolve.
     * @return Vertex associated with index.
     */
    vertex_t get_vertex(const vertex_index_t v) const noexcept
    {
        return vertex(v, *grid);
    }
    /**
     * Returns a vertex index associated with the given vertex.
     *
     * @param v Vertex to dissolve.
     * @return Vertex index associated with v.
     */
    vertex_index_t get_index(const vertex_t& v) const noexcept
    {
        return get(boost::vertex_index, *grid, v);
    }
    /**
     * Returns a range of all vertices of the stored grid.
     *
     * @return All vertices.
     */
    range_t<vertex_iter> get_vertices() const noexcept
    {
        return range_t<vertex_iter>{vertices(*grid)};
    }
    /**
     * Returns a range of all adjacent vertices to the given vertex v, i.e. all vertices u for which v -> u holds.
     *
     * @param v Vertex whose adjacent ones are desired.
     * @return All adjacent vertices to v.
     */
    range_t<adjacency_iter> get_adjacent_vertices(const vertex_t& v) const noexcept
    {
        return range_t<adjacency_iter>{adjacent_vertices(v, *grid)};
    }
    /**
     * Returns the edge from given vertex v1 to given vertex v2 if there exists one.
     *
     * @param v1 Source vertex.
     * @param v2 Target vertex.
     * @return Optional containing edge from v1 to v2.
     */
    std::optional<edge_t> get_edge(const vertex_t& v1, const vertex_t& v2) const noexcept
    {
        auto [e, b] = edge(v1, v2, *grid);
        if (!b)
            return std::nullopt;

        return e;
    }
    /**
     * Returns the edge from given vertex index v1 to given vertex index v2 if there exists one.
     *
     * @param v1 Source vertex.
     * @param v2 Target vertex.
     * @return Optional containing edge from v1 to v2.
     */
    std::optional<edge_t> get_edge(const vertex_index_t v1, const vertex_index_t v2) const noexcept
    {
        return get_edge(get_vertex(v1), get_vertex(v2));
    }
    /**
     * Returns an edge associated with the given edge index.
     *
     * @param e Edge index to dissolve.
     * @return Edge associated with index.
     */
    edge_t get_edge(const edge_index_t e) const noexcept
    {
        return edge_at(e, *grid);
    }
    /**
     * Returns an edge index associated with the given edge.
     *
     * @param e Edge to dissolve.
     * @return Edge index associated with e.
     */
    edge_index_t get_edge(const edge_t& e) const noexcept
    {
        return get(boost::edge_index, *grid, e);
    }
    /**
     * Returns a range containing the edges of the stored graph.
     *
     * @return Range including the graph's edges.
     */
    range_t<edge_iter> get_edges() const noexcept
    {
        return range_t<edge_iter>{edges(*grid)};
    }
    /**
     * Returns a range containing the outgoing edges of the given vertex.
     *
     * @param v Vertex whose outgoing edges are desired.
     * @return Range including the outgoing edges of vertex v.
     */
    range_t<out_edge_iter> get_out_edges(const vertex_t& v) const noexcept
    {
        return range_t<out_edge_iter>{out_edges(v, *grid)};
    }
    /**
     * Returns a range containing the ingoing edges of the given vertex.
     *
     * @param v Vertex whose ingoing edges are desired.
     * @return Range including the ingoing edges of vertex v.
     */
    range_t<in_edge_iter> get_in_edges(const vertex_t& v) const noexcept
    {
        return range_t<in_edge_iter>{in_edges(v, *grid)};
    }
    /**
     * Returns the source vertex of the given edge.
     *
     * @param e Edge whose source vertex is desired.
     * @return Source vertex of edge e.
     */
    vertex_t get_source(const edge_t& e) const noexcept
    {
        return source(e, *grid);
    }
    /**
     * Returns the target vertex of the given edge.
     *
     * @param e Edge whose target vertex is desired.
     * @return Target vertex of edge e.
     */
    vertex_t get_target(const edge_t& e) const noexcept
    {
        return target(e, *grid);
    }
    /**
     * Returns the number of edges in the graph.
     *
     * @return Number of edges.
     */
    num_edges_t get_edge_count() const noexcept
    {
        return num_edges(*grid);
    }
    /**
     * Returns the number of vertices in the graph.
     *
     * @return Number of vertices
     */
    num_vertices_t get_vertex_count() const noexcept
    {
        return num_vertices(*grid);
    }

    /**
     * Functor to measure distances within the grid using euclidean.
     */
    class euclidean_metric : public boost::astar_heuristic<filtered_grid, double>
    {
    public:
        explicit euclidean_metric(vertex_t target) : target(target) {};

        double operator()(vertex_t v)
        {
            double sum = 0;
            for (auto i = 0u; i < DIMENSIONS; ++i)
                sum += std::pow(static_cast<double>(target[i]) - v[i], 2);

            return std::sqrt(sum);
        }
    private:
        vertex_t target;
    };
    /**
     * Functor to measure distances within the grid using manhattan.
     */
    class manhattan_metric : public boost::astar_heuristic<filtered_grid, int>
    {
    public:
        explicit manhattan_metric(vertex_t target) : target(target) {};

        double operator()(vertex_t v)
        {
            double sum = 0;
            for (auto i = 0u; i < DIMENSIONS; ++i)
                sum += std::abs(static_cast<double>(target[i]) - v[i]);

            return sum;
        }
    private:
        vertex_t target;
    };
    /**
     * Measure distance between two vertices in the grid using a metric.
     *
     * @tparam metric A class that uses the distance type within a boost::default_astar_visitor to perform a metric
     *                function like euclidean or manhattan.
     * @tparam dist Integral type to measure distance (double, int, ...)
     * @param source Source vertex.
     * @param target Target vertex.
     * @return Distance between source and target as a dist type.
     */
    template <class metric, typename dist>
    dist distance(const vertex_t& source, const vertex_t& target) const noexcept
    {
        return metric(target)(source);
    }
};

/**
 * Cheated overload for the vertex_t type to actually print grid elements nicely.
 *
 * @tparam DIMENSIONS Template parameter determining grid's dimension.
 * @param v Vertex to create a string representation for.
 * @return String representation of v.
 */
template <uint64_t DIMENSIONS>
std::string to_string(const boost::array<uint64_t, DIMENSIONS>& v)
{
    std::stringstream repr{};
    repr << "{";
    for (auto i = 0u; i < DIMENSIONS; ++i)
    {
        repr << v[i];
        if (i != DIMENSIONS - 1)
            repr << ",";
    }
    repr << "}";

    return repr.str();
}
/**
 * Cheated overload for the vertex_t type to actually print grid elements nicely.
 *
 * @tparam DIMENSIONS Template parameter determining grid's dimension.
 * @param os Stream into which the ASCII representation should be written.
 * @param v Vertex to write into os.
 * @return os after appending v.
 */
template <uint64_t DIMENSIONS>
std::ostream& operator<<(std::ostream& os, const boost::array<uint64_t, DIMENSIONS>& v)
{
    os << "{";
    for (auto i = 0u; i < DIMENSIONS; ++i)
    {
        os << v[i];
        if (i != DIMENSIONS - 1)
            os << ",";
    }
    os << "}";

    return os;
}
/**
 * Cheated overload for the edge_t type to actually print grid elements nicely.
 *
 * @tparam DIMENSIONS Template parameter determining grid's dimension.
 * @param os Stream into which the ASCII representation should be written.
 * @param e Edge to write into os.
 * @return os after appending e.
 */
template <uint64_t DIMENSIONS>
std::ostream& operator<<(std::ostream& os, const std::pair<boost::array<uint64_t, DIMENSIONS>,
                                                           boost::array<uint64_t, DIMENSIONS>>& e)
{
    os << e.first << " -> " << e.second;
    return os;
}


#endif //GRID_GRAPH_H
