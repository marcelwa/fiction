//
// Created by marcel on 29.09.18.
//

#ifndef FICTION_PORT_ROUTER_H
#define FICTION_PORT_ROUTER_H

#include "fcn_gate_layout.h"
#include "fcn_technology.h"
#include "fcn_cell_types.h"
#include "logic_network.h"
#include <boost/functional/hash.hpp>
#include <string>
#include <vector>
#include <unordered_map>

/**
 * Module to route ports of an fcn_gate_layout concretely within the tile when the layout should be mapped to a specific
 * technology. Since the data structure fcn_gate_layout abstracts from detailed routing, it has to be computed later if
 * a technology mapping should happen. This class provides functionality for that.
 */
class port_router
{
public:
    /**
     * Default constructor.
     */
    port_router(fcn_gate_layout_ptr fgl, const fcn::technology t, const fcn::tile_size s) noexcept;
    /**
     * Computes the I/O ports for all gate and wire tiles in the stored layout.
     * Throws exceptions if routing with the given technology is impossible.
     */
    void compute_ports();
    /**
     * A port is a position of a cell relative to i.e. within a tile.
     */
    struct port
    {
        /**
         * Standard constructor.
         */
        port(coord_t x, coord_t y) : x{x}, y{y} {}
        /**
         * Positions.
         */
        coord_t x, y;
        /**
         * Comparator for set insertion.
         *
         * @param p Port to compare to.
         * @return True iff this port goes before p in set.
         */
        bool operator<(const port& p) const
        {
            return (this->x < p.x) || ((this->x == p.x) && (this->y < p.y));
        }
        /**
         * Comparator for equality tests.
         *
         * @param p Port to compare to.
         * @return True iff this port is equal to given port p.
         */
        bool operator==(const port& p) const
        {
            return (this->x == p.x) && (this->y == p.y);
        }
        /**
         * Hash function for unordered_set/map insertion.
         *
         * @param p Port to hash.
         * @return Hash value of p based on x and y.
         */
        friend std::size_t hash_value(const port& p)
        {
            std::size_t seed = p.x ^ p.y;
            boost::hash_combine(seed, p.x);
            boost::hash_combine(seed, p.y);

            return seed;
        }
    };
    /**
     * Port lists are collections of input and output identifiers.
     */
    struct port_list
    {
        /**
         * Default constructor.
         */
        port_list() = default;
        /**
         * Standard constructor.
         */
        port_list(std::set<port> inp, std::set<port> out) : inp{std::move(inp)}, out{std::move(out)} {}
        /**
         * Input and output positions.
         */
        std::set<port> inp, out;
        /**
         * Comparator for unordered_set/map.
         *
         * @param p Ports to compare to.
         * @return True iff these ports are equal to p.
         */
        bool operator==(const port_list& p) const
        {
            return hash_value(*this) == hash_value(p);
        }
        /**
         * Hash function for unordered_set/map insertion.
         *
         * @param p Ports to hash.
         * @return Hash value of p based on inp and out.
         */
        friend std::size_t hash_value(const port_list& p)
        {
            std::size_t seed = 0;
            boost::hash_combine(seed, p.inp);
            boost::hash_combine(seed, p.out);

            return seed;
        }
    };
    /**
     * Returns the ports for given gate tile specified as tile t and logic vertex v. Throws std::out_of_range if v was
     * not assigned to t.
     *
     * @param t Tile to consider.
     * @param v Logic vertex assigned as gate to t.
     * @return Ports for gate tile specified by t and v.
     */
    port_list get_ports(const fcn_gate_layout::tile t, const logic_network::vertex v) noexcept;
    /**
     * Returns the ports for given wire specified by logic edge e on tile t. Throws std::out_of_range if e was not
     * assigned to t.
     *
     * @param t Tile to consider.
     * @param e Logic edge assigned as wire to t.
     * @return Ports for wire specified by t and e.
     */
    port_list get_ports(const fcn_gate_layout::tile t, const logic_network::edge& e) noexcept;
private:
    /**
     * Layout whose ports should be routed.
     */
    const fcn_gate_layout_ptr layout;
    /**
     * Technology for which ports should be routed.
     */
    const fcn::technology tech;
    /**
     * Available tile size to route ports within.
     */
    const fcn::tile_size size;
    /**
     * Alias for a hash map of gate tiles to connector ports.
     */
    using gate_port_map = std::unordered_map<std::pair<fcn_gate_layout::tile, logic_network::vertex>, port_list,
            boost::hash<std::pair<fcn_gate_layout::tile, logic_network::vertex>>>;
    /**
     * Maps gates to their respective ports. They can be viewed as connectors to adjacent tiles. These are no PI/POs.
     */
    gate_port_map g_ports{};
    /**
     * Alias for a hash map of wire tiles to connector ports.
     */
    using wire_port_map = std::unordered_map<std::pair<fcn_gate_layout::tile, logic_network::edge>, port_list,
            boost::hash<std::pair<fcn_gate_layout::tile, logic_network::edge>>>;
    /**
     * Maps wires to their respective ports. They can be viewed as connectors to adjacent tiles. These are no PI/POs.
     */
    wire_port_map w_ports{};
    /**
     * Computes ports for QCA technology with tile size of 5 x 5.
     */
    void compute_qca_5x5_ports();
    /**
     * Computes ports for QCA technology with tile size of 4 x 4.
     */
    void compute_inml_4x4_ports();
};

using port_router_ptr = std::shared_ptr<port_router>;


#endif //FICTION_PORT_ROUTER_H
