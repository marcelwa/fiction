//
// Created by marcel on 25.07.18.
//

#ifndef FICTION_FCN_CELL_LAYOUT_H
#define FICTION_FCN_CELL_LAYOUT_H


#include "fcn_layout.h"
#include "fcn_gate_library.h"
#include "fcn_gate_layout.h"
#include "port_router.h"


/**
 * Represents layouts of field coupled nanocomputing (FCN) devices on a cell level abstraction. Inherits from fcn_layout
 * so it is a 3-dimensional grid-like structure as well. Faces are called cells in a cell layout. Cells can have special
 * types which correlate with their physical behavior. This abstraction level is close to physics. It incorporates FCN
 * technologies from which the gate layout abstracts. fcn_cell_layouts can be created as QCA, NML, DB, etc.
 * implementations for which necessary technologies and cell types can be defined in fcn_technologies.h and
 * fcn_cell_types.h.
 *
 * To iterate over all cells of a fcn_cell_layout l, use
 *  for (auto&& c : l.cells())
 *  {
 *      ...
 *  }
 *
 * fcn_cell_layouts can be constructed in a classical way by assigning types to cells or from a already existing
 * fcn_gate_layout. This can be achieved by constructing a fcn_gate_library, which incorporates the desired layout
 * together with a technology to which the gate layout should be mapped, and passing it to the constructor of
 * fcn_cell_layout. Depending on the chosen tile size and technology, this may fail and throw an exception. For correct
 * use of this technique, see commands.h.
 */
class fcn_cell_layout : public fcn_layout
{
public:
    /**
     * Vertices of FCN cell layouts are called cells.
     */
    using cell = vertex_t;
    /**
     * Same for cell_indices.
     */
    using cell_index = vertex_index_t;
    /**
     * Standard constructor. Creates an FCN cell layout by the means of an array determining its size
     * as well as a clocking scheme defining its data flow possibilities. A gate library specifies gate structures
     * made of cells.
     *
     * @param lengths 3-dimensional array defining sizes of each dimension (x, y, z) where z - 1 determines
     * the number of crossing layers.
     * NOTE: Due to a bug in the BGL, every dimension should have a minimum size of 2 to prevent SEGFAULTs.
     * See https://svn.boost.org/trac10/ticket/11735 for details.
     * @param clocking Clocking scheme defining possible data flow.
     * @param tech Technology which should be used for this layout.
     * @param name Layout's name.
     */
    fcn_cell_layout(const fcn_dimension_xyz& lengths, fcn_clocking_scheme clocking,
                    fcn::technology tech, const std::string& name) noexcept;
    /**
     * Standard constructor. Creates an FCN cell layout by the means of an array determining its size
     * as well as a clocking scheme defining its data flow possibilities.
     *
     * @param lengths 2-dimensional array defining sizes of dimensions (x, y) where no crossing layer is given.
     * NOTE: Due to a bug in the BGL, every dimension should have a minimum size of 2 to prevent SEGFAULTs.
     * See https://svn.boost.org/trac10/ticket/11735 for details.
     * @param clocking Clocking scheme defining possible data flow.
     * @param tech Technology which should be used for this layout.
     * @param Layout's name.
     */
    fcn_cell_layout(const fcn_dimension_xy& lengths, fcn_clocking_scheme clocking,
                    fcn::technology tech, const std::string& name) noexcept;
    /**
     * Standard constructor. Creates an FCN cell layout from a gate library that is associated with a fcn_gate_layout.
     * All tiles from the stored gate layout will be mapped to cells using the library.
     *
     * @param lib FCN gate library to use for mapping operations.
     */
    explicit fcn_cell_layout(fcn_gate_library_ptr&& lib);
    /**
     * Function alias for get_vertices using perfect forwarding and the name cells to fit naming in fcn_cell_layout.
     *
     * @tparam ARGS Stack of argument types.
     * @param args Stack of arguments.
     * @return get_vertices(args).
     */
    template <typename... ARGS>
    auto cells(ARGS&&... args) const noexcept
    {
        return get_vertices(std::forward<ARGS>(args)...);
    }
    /**
     * Function alias for random_face using perfect forwarding and the name random_cell to fit naming in fcn_cell_layout.
     *
     * @tparam ARGS Stack of argument types.
     * @param args Stack of arguments.
     * @return random_face(args).
     */
    template<typename... ARGS>
    auto random_cell(ARGS&&... args) const noexcept
    {
        return random_face(std::forward<ARGS>(args)...);
    }
    /**
     * Assigns a cell c with type t to the layout.
     *
     * @param c Location of newly to insert cell type.
     * @param t Type of new cell.
     */
    void assign_cell_type(const cell& c, const fcn::cell_type t) noexcept;
    /**
     * Returns the type of cell c. If no cell type was stored before, fcn::EMPTY_CELL is returned.
     *
     * @param c Location of cell to check.
     * @return Cell type of c if there is one, fcn::EMPTY_CELL otherwise.
     */
    fcn::cell_type get_cell_type(const cell& c) const noexcept;
    /**
     * Returns true iff cell c has an empty or none cell_type assigned.
     *
     * @param c Cell to consider.
     * @return True iff cell c is empty.
     */
    bool is_free_cell(const cell& c) const noexcept;
    /**
     * Assigns a cell c mode m.
     *
     * @param c Location of cell mode.
     * @param t Mode of cell c.
     */
    void assign_cell_mode(const cell& c, const fcn::cell_mode m) noexcept;
    /**
     * Returns the mode of cell c. If no cell mode was stored before, fcn::cell_mode::NORMAL is returned.
     *
     * @param c Location of cell to check.
     * @return Cell mode of c if there is one, fcn::cell_mode::NORMAL otherwise.
     */
    fcn::cell_mode get_cell_mode(const cell& c) const noexcept;
    /**
     * Assigns a cell c name n.
     *
     * @param c Location of cell name.
     * @param n Name of cell c.
     */
    void assign_cell_name(const cell& c, const std::string& n) noexcept;
    /**
     * Returns the name of cell c. If no cell name was stored before, "" is returned.
     *
     * @param c Location of cell to check.
     * @return Cell name of c if there is one, "" otherwise.
     */
    std::string get_cell_name(const cell& c) const noexcept;
    /**
     * Prints the assigned cell type of the given cell c to the given channel os.
     *
     * @param c Cell whose type is to be printed.
     * @param os Channel to write c's type into.
     */
    void print_cell(const cell& c, std::ostream& os = std::cout) const noexcept;
    /**
     * Returns the number of assigned cells in the layout. Cell types do not matter.
     *
     * @return Size of cell_map.
     */
    std::size_t cell_count() const noexcept;
    /**
     * Determines the given cell's clock number by consulting the stored clocking cutout in case of regular clockings,
     * while taking library's gate size into account, and looks up in the clocking map if clocking is irregular. If no
     * entry has been stored for irregular clockings yet, std::nullopt is returned.
     *
     * @param c Cell whose clock number is desired.
     * @return Clock number of c.
     */
    std::optional<fcn_clock::zone> cell_clocking(const cell& c) const noexcept;
    /**
     * Assigns an FCN gate to a region beginning with cell c in the top left corner (lowest x, y position). The gate
     * then spans to the bottom right corner (highest x, y position).
     *
     * @param c Location of top left corner cell of gate g.
     * @param g FCN gate to be assigned to a region starting in the top left corner at cell c.
     * @param l A latch delay in phases.
     * @param inp_names Names for PI ports. Each input cell occurring in g will take the next name from this vector.
     * @param out_names Names for PO ports. Each output cell occurring in g will take the next name from this vector.
     */
    void assign_gate(const cell& c, const fcn_gate& g, const latch_delay l = 0,
                     const std::vector<std::string>& inp_names = {}, const std::vector<std::string>& out_names = {});
    /**
     * Returns library pointer.
     *
     * @return Pointer to gate library.
     */
    fcn_gate_library_ptr get_library() const noexcept;
    /**
     * Returns technology.
     *
     * @return Technology in use.
     */
    fcn::technology get_technology() const noexcept;
    /**
     * Returns layout name.
     *
     * @return Name of the layout.
     */
    std::string get_name() const noexcept;
    /**
     * Calculates the number of magnets for an iNML layout like MagCAD (https://topolinano.polito.it/) would do it.
     * If the layout is not an iNML one, 0 is returned.
     *
     * @return Number of magnets as counted by MagCAD if layout is an iNML one, 0 otherwise.
     */
    std::size_t magcad_magnet_count() const noexcept;
    /**
     * Determines the layout's bounding box i.e. the area in which cells are placed. Helps to determine the
     * "real" size of a layout.
     *
     * @return Bounding box.
     */
    bounding_box determine_bounding_box() const noexcept override;
    /**
     * Prints the assigned cell types to the given std::ostream channel. A textual representation is used for
     * visualization. Currently only one crossing layer can be represented correctly. This is more of a debug function
     * and unsuitable for large layouts.
     *
     * @param os An std::ostream channel to write the textual representation of this layout into.
     * @param io_color Flag to indicate features like PI/PO should be printed with color escape.
     * @param clk_color Flag to indicate that clock zones should be printed with color escape. Can look weird.
     */
    void write_layout(std::ostream& os = std::cout, const bool io_color = true, const bool clk_color = false) const noexcept;

private:
    /**
     * Gate library associated with the layout. Determines tile sizes and enables for adding gates instead of cells.
     */
    fcn_gate_library_ptr library = nullptr;
    /**
     * Technology which is used for the cells in this layout.
     */
    fcn::technology technology;
    /**
     * Layout name.
     */
    std::string name;
    /*
     * Alias for a hash map that assigns cell types to cells.
     */
    using cell_type_map = std::unordered_map<cell, fcn::cell_type, boost::hash<cell>>;
    /**
     * Stores mapping cell -> cell_type. Helper functions for access save memory.
     */
    cell_type_map type_map{};
    /**
     * Alias for a hash map that assigns cell modes to cells.
     */
    using cell_mode_map = std::unordered_map<cell, fcn::cell_mode, boost::hash<cell>>;
    /**
     * Stores a mapping cell -> cell_mode. Helper functions for access save memory. Crossover mode does not need to be
     * saved because it is inherently given by the X-coordinate.
     */
    cell_mode_map mode_map{};
    /**
     * Alias for a hash map that assigns names to cells.
     */
    using cell_name_map = std::unordered_map<cell, std::string, boost::hash<cell>>;
    /**
     * Stores a mapping cell -> std::string. Helper functions for access save memory.
     */
    cell_name_map name_map{};
    /**
     * Maps a non-regular clocking taken from a fcn_gate_layout to the cell-level. Therefore, the library's tile size
     * has to be taken into account.
     */
    void map_irregular_clocking() noexcept;
    /**
     * Determines via cells in the current layout's crossing layer and marks them accordingly. This also includes
     * creating via cells in bottom layer to transport information correctly. A middle (via) layer is not needed and
     * automatically generated by output functions if needed (like for QCA).
     */
    void assign_vias() noexcept;
    /**
     * Removes bumps and straightens input and output wires.
     * This optimization function is designed to be used with iNML layouts clocked with one of the ToPoliNano clocking
     * schemes. Due to the shifted tile layouts, which emulate row-wise clocking in a tile-based fashion, wire bumps can
     * occur when mapping to a cell level layout. This function removes them and thereby creates a more pleasing looking
     * layout that even saves a few magnets.
     *
     * Implemented in optimization.cpp.
     */
    void clean_up_topolinano() noexcept;
    /**
     * Tries to find cuts in a layout to optimize depth.
     * A cut is defined as a set of horizontal cell sequences (wires) in different y-positions that can be removed to
     * seperate the layout into two non-connected blocks that can be merged together seamlessly.
     */
    void cut_optimization() noexcept;
    /**
     * Optimizes the structure of the cell layout if needed. This can include cosmetics or cost reductions that cannot
     * be done on gate level. This function will be called by map_layout when mapping from an fcn_gate_layout.
     * This function needs to respect the technology and library of the layout.
     */
    void optimize() noexcept;
    /**
     * Maps all operations of the given fcn_gate_layout to cell level using the stored library.
     */
    void map_layout();
};

using fcn_cell_layout_ptr = std::shared_ptr<fcn_cell_layout>;


#endif //FICTION_FCN_CELL_LAYOUT_H
