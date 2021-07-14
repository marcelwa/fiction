//
// Created by marcel on 26.07.18.
//

#ifndef FICTION_FCN_GATE_LIBRARY_H
#define FICTION_FCN_GATE_LIBRARY_H

#include "fcn_gate_layout.h"
#include "fcn_cell_types.h"
#include "fcn_technology.h"
#include "port_router.h"
#include <vector>
#include <memory>
#include <itertools.hpp>


/**
 * Alias for a 2D cell_type matrix specifying FCN gates on a cell level.
 */
using fcn_gate = std::vector<std::vector<fcn::cell_type>>;

/**
 * This class represents a pure virtual base class for all kinds of FCN libraries used to map gate tiles to cell level.
 * Concrete libraries should extend this class and implement set_up_gate.
 */
class fcn_gate_library
{
public:
    /**
     * Alias for gate size.
     */
    using gate_size_t = uint8_t;
    /**
     * Default constructor. Needs a gate layout to map from, a technology specifying cells to map and information about
     * tile sizes in that gate library.
     *
     * @param name Name of the gate library.
     * @param fgl Layout to map from.
     * @param tech Technology to use for cell types.
     * @param size Size of a single gate.
     */
    fcn_gate_library(std::string&& name, fcn_gate_layout_ptr&& fgl,
                     const fcn::technology tech, const fcn::tile_size size) noexcept;
    /**
     * Pure virtual function. Need to be implemented by inheriting gate library. Returns a gate fulfilling all
     * requirements with respect to rotation and I/O marks in the given tile t. This function should be the only one
     * necessary to be called by a user.
     *
     * @param t Tile to be realized as a gate in this library.
     * @return FCN gate realizing t.
     */
    virtual fcn_gate set_up_gate(const fcn_gate_layout::tile& t) = 0;
    /**
     * Rotates the given fcn_gate by 90° clockwise.
     *
     * @param g fcn_gate to rotate.
     * @return Rotated fcn_gate.
     */
    fcn_gate rotate_90(const fcn_gate& g) const noexcept;
    /**
     * Rotates the given fcn_gate by 180°.
     *
     * @param g fcn_gate to rotate.
     * @return Rotated fcn_gate.
     */
    fcn_gate rotate_180(const fcn_gate& g) const noexcept;
    /**
     * Rotates the given fcn_gate by 270° clockwise.
     *
     * @param g fcn_gate to rotate.
     * @return Rotated fcn_gate.
     */
    fcn_gate rotate_270(const fcn_gate& g) const noexcept;
    /**
     * Merges multiple fcn_gates into one. This is intended to be used for wires. Unexpected behavior can be caused, if
     * more than one fcn_gate has a cell at the same position.
     *
     * @param gates Vector of gates to be merged.
     * @return Merged fcn_gate.
     */
    fcn_gate merge(const std::vector<fcn_gate>& gates) const noexcept;
    /**
     * Returns library's name.
     *
     * @return name.
     */
    std::string get_name() const noexcept;
    /**
     * Returns pointer to the stored layout.
     *
     * @return layout.
     */
    fcn_gate_layout_ptr get_layout() const noexcept;
    /**
     * Returns the assigned FCN technology.
     *
     * @return technology.
     */
    fcn::technology get_technology() const noexcept;
    /**
     * Returns the assigned tile size.
     *
     * @return tile_size.
     */
    fcn::tile_size get_tile_size() const noexcept;
    /**
     * Returns horizontal size of gate blocks.
     *
     * @return x_size.
     */
    gate_size_t gate_x_size() const noexcept;
    /**
     * Returns vertical size of gate blocks.
     *
     * @return y_size.
     */
    gate_size_t gate_y_size() const noexcept;
    /**
     * Returns pointer the port router.
     *
     * @return p_router.
     */
    port_router_ptr get_port_router() const noexcept;

protected:
    /**
     * Shortcut for a single port.
     */
    using port = port_router::port;
    /**
     * Shortcut for a list of ports.
     */
    using ports = port_router::port_list;
    /**
     * Alias for a map of ports to fcn_gates. Used to identify rotation quickly.
     */
    using port_gate_map = std::unordered_map<ports, fcn_gate, boost::hash<ports>>;
    /**
     * Transposes the given fcn_gate.
     *
     * @param g fcn_gate to transpose.
     * @return Transposed fcn_gate.
     */
    fcn_gate transpose(const fcn_gate& g) const noexcept;
    /**
     * Reverses the columns of the given fcn_gate.
     *
     * @param g fcn_gate whose columns are to be reversed.
     * @return fcn_gate with reversed columns.
     */
    fcn_gate reverse_columns(const fcn_gate& g) const noexcept;
    /**
     * Reverses the rows of the given fcn_gate.
     *
     * @param g fcn_gate whose rows are to be reversed.
     * @return fcn_gate with reversed rows.
     */
    fcn_gate reverse_rows(const fcn_gate& g) const noexcept;
    /**
     * Applies given mark to given fcn_gate g at given port p.
     *
     * @param g Gate to apply mark to.
     * @param p Port specifying where to apply the mark.
     * @param mark Mark to be applied
     * @return Marked fcn_gate.
     */
    fcn_gate mark_cell(const fcn_gate& g, const port& p, const fcn::cell_mark mark) const noexcept;
    /**
     * Returns the opposite of a port. Only border ports are accepted.
     *
     * @param p Port whose opposite counterpart is desired.
     * @return Opposite port to p.
     */
    port opposite(const port& p) const;
    /**
     * Name of this library.
     */
    const std::string name;
    /**
     * Pointer to fcn_gate_layout to map from.
     */
    fcn_gate_layout_ptr layout;
    /**
     * Technology to be used.
     */
    const fcn::technology technology;
    /**
     * Size of a single gate block.
     */
    const fcn::tile_size tile_size;
    /**
     * X-size of gates.
     */
    const gate_size_t x_size;
    /**
     * Y-size of gates.
     */
    const gate_size_t y_size;
    /**
     * Single empty gate in given technology and tile_size. Used as a blue print to create new ones in merge and
     * transpose for example.
     */
    const fcn_gate empty_gate;
    /**
     * Pointer to port router to determine ports within tiles.
     */
    port_router_ptr p_router;
};

/**
 * Overloaded stream operator for fcn_gate objects. Useful for debugging because it allows printing cell-level gates to
 * to an output stream.
 *
 * @param os Output stream to write a textual representation of g into.
 * @param g Cell-level gate to write into os.
 * @return os extended by a textual representation of g.
 */
std::ostream& operator<<(std::ostream& os, const fcn_gate& g);

using fcn_gate_library_ptr = std::shared_ptr<fcn_gate_library>;

#endif //FICTION_FCN_GATE_LIBRARY_H
