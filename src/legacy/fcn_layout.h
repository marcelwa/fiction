//
// Created by marcel on 25.07.18.
//

#ifndef FICTION_FCN_LAYOUT_H
#define FICTION_FCN_LAYOUT_H

#include "grid_graph.h"
#include "fcn_clocking_scheme.h"
#include <algorithm>
#include <random>
#include <set>
#include <unordered_map>
#include <itertools.hpp>
#include <boost/functional/hash.hpp>


/**
 * Alias for a coordinate.
 */
using coord_t = uint64_t;
/**
 * Alias for boost::array needed for constructor call.
 */
using fcn_dimension_xyz = boost::array<coord_t, 3>;
/**
 * Alias for boost::array needed for constructor call.
 */
using fcn_dimension_xy = boost::array<coord_t, 2>;
/**
 * Alias for a layer.
 */
using layer_t = uint8_t;
/**
 * Constant referring to ground layer.
 */
constexpr const layer_t GROUND = 0u;
/**
 * Constant referring to crossing layer.
 */
constexpr const layer_t CROSSING = 1u;
/**
 * Constant referring to x-dimension value of faces.
 */
constexpr const layer_t X = 0u;
/**
 * Constant referring to y-dimension value of faces.
 */
constexpr const layer_t Y = 1u;
/**
 * Constant referring to z-dimension value of faces.
 */
constexpr const layer_t Z = 2u;
/**
 * Returns the area covered by a size specification.
 *
 * @tparam Dimension type.
 * @param dim Layout size specification.
 * @return X * Y.
 */
template <typename T>
coord_t area(const T& dim) noexcept { return dim[X] * dim[Y]; }
/**
 * Represents general layouts for Field-coupled Nanocomputing (FCN) devices on an arbitrary abstraction level.
 * Layouts are always 3-dimensional grid-like structures consisting of faces. Each face is addressed by a 3-tuple
 * (x, y, z) where axis are named intuitively. Numbering starts in the upper left ground layer with (0, 0, 0) and ends
 * in the lower right highest layer with (x - 1, y - 1, z - 1).
 *
 * They can be accessed by operator(x, y, z) or operator[x][y][z], where the former is more performant. Special
 * constants defined above allow for direct access of X-, Y-, Z-coordinate of faces. Assume f is a face. Then f[X]
 * returns its X-coordinate. Works analogously for the other dimensions.
 *
 * Each layout must have a clocking scheme which can be regular or irregular. For regular clocking schemes, a small
 * cutout is used to extrapolate the whole floorplan. Some pre-defined clocking schemes are provided in
 * fcn_clocking_scheme.h. More can easily be implemented there. Irregular clockings are stored in a special map which
 * assigns a designated clock zone to each face.
 *
 * Besides that, special clock latches are available. See docstrings below for a detailed description of how they work.
 *
 * This class is pure-virtual so any concrete implementation can deal with what it means to be a face as well as names
 * via aliases themselves. View this class as a general framework for all kinds of layouts.
 */
class fcn_layout : protected grid_graph<3>
{
public:
    /**
     * Vertices of FCN layouts are called faces.
     */
    using face = vertex_t;
    /**
     * Same for face_indices.
     */
    using face_index = vertex_index_t;
    /**
     * Alias for a face in ground layer. Since clocking is identical in all layers, third dimension can be neglected.
     */
    using ground = boost::array<coord_t, 2u>;
    /**
     * Alias for a number that represents delay of a latch face in clock phases (1/n clock cycles).
     */
    using latch_delay = uint8_t;
    /**
     * An offset shifts the layout's faces either vertically or horizontally. While in a non-shifted layout,
     * face {0,1,0}'s adjacent face in eastern direction would be {1,1,0} for instance, in a vertically shifted
     * layout, every odd row is moved downwards by half a face, meaning that {0,1,0} has the two western adjacent faces
     * {1,0,0} and {1,1,0}. This works analogously for horizontally shifted layouts.
     */
    enum class offset { NONE, VERTICAL /*, HORIZONTAL */ };
    /**
     * Standard constructor. Creates an FCN layout by the means of an array determining its size
     * as well as a clocking scheme defining its data flow possibilities.
     *
     * @param lengths 3-dimensional array defining sizes of each dimension (x, y, z) where z - 1 determines
     * the number of crossing layers.
     * NOTE: Due to a bug in the BGL, every dimension should have a minimum size of 2 to prevent SEGFAULTs.
     * See https://svn.boost.org/trac10/ticket/11735 for details.
     * @param clocking Clocking scheme defining possible data flow.
     * @param o Offset for face shift in vertical or horizontal direction.
     */
    fcn_layout(const fcn_dimension_xyz& lengths, fcn_clocking_scheme&& clocking, offset o = offset::NONE) noexcept;
    /**
     * Standard constructor. Creates an FCN layout by the means of an array determining its size
     * as well as a clocking scheme defining its data flow possibilities.
     *
     * @param lengths 2-dimensional array defining sizes of dimensions (x, y) where no crossing layer is given.
     * NOTE: Due to a bug in the BGL, every dimension should have a minimum size of 2 to prevent SEGFAULTs.
     * See https://svn.boost.org/trac10/ticket/11735 for details.
     * @param clocking Clocking scheme defining possible data flow.
     * @param o Offset for face shift in vertical or horizontal direction.
     */
    fcn_layout(const fcn_dimension_xy& lengths, fcn_clocking_scheme&& clocking, offset o = offset::NONE) noexcept;
    /**
     * Standard constructor. Creates a FCN layout of size 2 x 2 x 2 with the given clocking scheme.
     * NOTE: Due to a bug in the BGL, every dimension should have a minimum size of 2 to prevent SEGFAULTs.
     * See https://svn.boost.org/trac10/ticket/11735 for details.
     *
     * @param clocking Clocking scheme defining possible data flow.
     * @param o Offset for face shift in vertical or horizontal direction.
     */
    explicit fcn_layout(fcn_clocking_scheme&& clocking, offset o = offset::NONE) noexcept;
    /**
     * Standard constructor. Creates a FCN layout of size 2 x 2 x 2 with an open clocking scheme.
     * NOTE: Due to a bug in the BGL, every dimension should have a minimum size of 2 to prevent SEGFAULTs.
     * See https://svn.boost.org/trac10/ticket/11735 for details.
     */
    fcn_layout() noexcept;
    /**
     * Pure virtual destructor for polymorphism.
     */
    virtual ~fcn_layout() = 0;
    /**
     * Returns true iff the layout has a vertical offset.
     *
     * @return True iff the layout has a vertical shift..
     */
    bool is_vertically_shifted() const noexcept;
    /**
     * Resizes the layout to the dimensions given. The fcn_layout object will not be re-constructed by this
     * function! All maps and other attributes stay untouched.
     *
     * NOTE, that face_index variables might refer to different faces after resizing and that mappings could run out of
     * scope i.e. the faces to which attributes were assigned might not exist in the layout anymore after resizing.
     * Since mappings are done via face positions and not via objects, they can still be accessed but could lead to
     * unwanted behavior.
     *
     * @param lengths 3-dimensional array defining sizes of each dimension (x, y, z) where z - 1 determines the number
     * of crossing layers.
     */
    void resize(const fcn_dimension_xyz& lengths) noexcept;
    /**
     * Resizes the layout to the dimensions given. The fcn_layout object will not be re-constructed by this
     * function! All maps and other attributes stay untouched.
     *
     * NOTE, that face_index variables might refer to different faces after resizing and that mappings could run out of
     * scope i.e. the faces to which attributes were assigned might not exist in the layout anymore after resizing.
     * Since mappings are done via face positions and not via objects, they can still be accessed but could lead to
     * unwanted behavior.
     *
     * @param lengths 2-dimensional array defining sizes of dimensions (x, y) where no crossing layer is given.
     */
    void resize(const fcn_dimension_xy& lengths) noexcept;
    /**
     * Returns true iff f is located in an even numbered row.
     *
     * @param f Face whose row number is to check.
     * @return True if f[Y] is even.
     */
    bool is_even_row(const face& f) const noexcept;
    /**
     * Returns true iff f is located in an odd numbered row.
     *
     * @param f Face whose row number is to check.
     * @return True if f[Y] is odd.
     */
    bool is_odd_row(const face& f) const noexcept;
    /**
     * Returns true iff f is located in an even numbered column.
     *
     * @param f Face whose column number is to check.
     * @return True if f[X] is even.
     */
    bool is_even_column(const face& f) const noexcept;
    /**
     * Returns true iff f is located in an odd numbered column.
     *
     * @param f Face whose column number is to check.
     * @return True if f[X] is odd.
     */
    bool is_odd_column(const face& f) const noexcept;
    /**
     * Returns the face north of a given face f i.e. the face whose y-dimension is lower by 1.
     * If f's y-dimension is already at minimum, f is returned instead.
     *
     * @param f Face whose north counterpart is desired.
     * @return Face north of f.
     */
    face north(const face& f) const noexcept;
    /**
     * Returns the face east of a given face f i.e. the face whose x-dimension is higher by 1.
     * If f's x-dimension is already at maximum, f is returned instead.
     *
     * @param f Face whose east counterpart is desired.
     * @return Face east of f.
     */
    face east(const face& f) const noexcept;
    /**
     * Returns the face south of a given face f i.e. the face whose y-dimension is higher by 1.
     * If f's y-dimension is already at maximum, f is returned instead.
     *
     * @param f Face whose south counterpart is desired.
     * @return Face south of f.
     */
    face south(const face& f) const noexcept;
    /**
     * Returns the face west of a given face f i.e. the face whose x-dimension is lower by 1.
     * If f's x-dimension is already at minimum, f is returned instead.
     *
     * @param f Face whose west counterpart is desired.
     * @return Face west of f.
     */
    face west(const face& f) const noexcept;
    /**
     * Returns the face above a given face f i.e. the face whose z-dimension is higher by 1.
     * If f's z-dimension is already at maximum, f is returned instead.
     *
     * @param f Face whose above counterpart is desired.
     * @return Face above f.
     */
    face above(const face& f) const noexcept;
    /**
     * Returns the face below a given face f i.e. the face whose z-dimension is lower by 1.
     * If f's z-dimension is already at minimum, f is returned instead.
     *
     * @param f Face whose below counterpart is desired.
     * @return Face below f.
     */
    face below(const face& f) const noexcept;
    /**
     * Returns the north-eastern face of the given face f if there is one. This takes shifted layouts into account and
     * therefore returns std::nullopt if no such face exists.
     *
     * @param f Face whose north-eastern adjacent face is desired.
     * @return North-eastern face of f or std::nullopt if such a face does not exist.
     */
    std::optional<face> north_east(const face& f) const noexcept;
    /**
     * Returns the north-western face of the given face f if there is one. This takes shifted layouts into account and
     * therefore returns std::nullopt if no such face exists.
     *
     * @param f Face whose north-western adjacent face is desired.
     * @return North-western face of f or std::nullopt if such a face does not exist.
     */
    std::optional<face> north_west(const face& f) const noexcept;
    /**
     * Returns the south-eastern face of the given face f if there is one. This takes shifted layouts into account and
     * therefore returns std::nullopt if no such face exists.
     *
     * @param f Face whose south-eastern adjacent face is desired.
     * @return South-eastern face of f or std::nullopt if such a face does not exist.
     */
    std::optional<face> south_east(const face& f) const noexcept;
    /**
     * Returns the south-western face of the given face f if there is one. This takes shifted layouts into account and
     * therefore returns std::nullopt if no such face exists.
     *
     * @param f Face whose south-western adjacent face is desired.
     * @return South-western face of f or std::nullopt if such a face does not exist.
     */
    std::optional<face> south_west(const face& f) const noexcept;
    /**
     * Returns the face in ground layer below the given one f regardless of its z-dimension.
     *
     * @param f Face whose ground layer counter part is desired.
     * @return Face in ground layer below f.
     */
    ground get_ground(const face& f) const noexcept;
    /**
     * Returns the layout's x-dimension.
     *
     * @return x-dimension.
     */
    coord_t x() const noexcept;
    /**
     * Returns the layout's y-dimension.
     *
     * @return y-dimension.
     */
    coord_t y() const noexcept;
    /**
     * Returns the layout's z-dimension.
     *
     * @return z-dimension.
     */
    coord_t z() const noexcept;
    /**
     * Function alias for get_vertex using perfect forwarding and the name get_by_index to fit naming in fcn_layout.
     *
     * @tparam ARGS Stack of argument types.
     * @param args Stack of arguments.
     * @return get_vertex(args).
     */
    template <typename... ARGS>
    auto get_by_index(ARGS&&... args) const noexcept
    {
        return get_vertex(std::forward<ARGS>(args)...);
    }
    /**
     * Function alias for get_index using perfect forwarding and the name index to fit naming in fcn_layout.
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
     * Returns a range of all surrounding faces to f including the ones in higher/lower levels directly above/below f.
     *
     * @param f Face whose surrounding counterparts are desired.
     * @return All faces adjacent to f in all directions.
     */
    auto surrounding_3d(const face& f) const noexcept
    {
        std::vector<face> surrounding;
        if (is_vertically_shifted())
        {
            if (is_even_column(f))
            {
                if (auto ne = north_east(f); ne)
                    surrounding.push_back(*ne);
                if (auto nw = north_west(f); nw)
                    surrounding.push_back(*nw);
            }
            else  // odd row
            {
                if (auto se = south_east(f); se)
                    surrounding.push_back(*se);
                if (auto sw = south_west(f); sw)
                    surrounding.push_back(*sw);
            }
        }
//        else if (shift == offset::HORIZONTAL)
//        {
//
//        }

        auto av = get_adjacent_vertices(f);
        std::copy(av.begin(), av.end(), std::back_inserter(surrounding));
        return surrounding;
    }
    /**
     * Returns a range of all surrounding faces to f in that very layer. For layouts with z == 1, this function
     * is equivalent to surrounding_3d.
     *
     * @param f Face whose surrounding counterparts are desired.
     * @return All faces adjacent to f in the same layer as f.
     */
    auto surrounding_2d(const face& f) const noexcept
    {
        return surrounding_3d(f) | iter::filter([f = f](const face& _f){return f[Z] == _f[Z];});
    }
    /**
     * Returns a range of all faces in ground layer. For layouts with z == 1, this function is equivalent to get_faces.
     *
     * @return All faces in ground layer, i.e. where z-value == 0.
     */
    auto ground_layer() const noexcept
    {
        return get_vertices() | iter::filter([](const face& _f){return _f[Z] == GROUND;});
    }
    /**
     * Returns a range of all faces except for those in ground layer. For layouts with z == 1, this function returns an
     * empty range.
     *
     * @return All faces except for those in ground layer, i.e. where z-value > 0.
     */
    auto crossing_layers() const noexcept
    {
        return get_vertices() | iter::filterfalse([](const face& _f){return _f[Z] == GROUND;});
    }
    /**
     * Returns a range of all faces in nth layer. For layouts with z <= n, this function returns an empty range.
     *
     * @return All faces in nth layer, i.e. where z-value == n.
     */
    auto layer_n(const layer_t n) const noexcept
    {
        return get_vertices() | iter::filter([n = n](const face& _f){return _f[Z] == n;});
    }
    /**
     * Samples a random face from the layout.
     *
     * @return Randomly sampled face.
     */
    face random_face() const noexcept;
    /**
     * Samples a random face from given layout's layer.
     *
     * @param n Layer from which the face should be sampled.
     * @return Randomly sampled face from layer n.
     */
    face random_face(const layer_t n) const noexcept;
    /**
     * Function alias for distance with euclidean metric using perfect forwarding and the name euclidean_distance to fit
     * naming in fcn_layout.
     *
     * @tparam ARGS Stack of argument types.
     * @param args Stack of arguments.
     * @return distance<euclidean_metric, double>(args).
     */
    template <typename... ARGS>
    auto euclidean_distance(ARGS&&... args) const noexcept
    {
        return distance<euclidean_metric, double>(std::forward<ARGS>(args)...);
    }
    /**
     * Function alias for distance with manhattan metric using perfect forwarding and the name manhattan_distance to fit
     * naming in fcn_layout.
     *
     * @tparam ARGS Stack of argument types.
     * @param args Stack of arguments.
     * @return distance<manhattan_metric, int>(args).
     */
    template <typename... ARGS>
    auto manhattan_distance(ARGS&&... args) const noexcept
    {
        return distance<manhattan_metric, int>(std::forward<ARGS>(args)...);
    }
    /**
     * Returns area of layout as x * y many faces.
     *
     * @return Area of layout.
     */
    coord_t area() const noexcept;
    /**
     * Returns the number of clock zones in this layout.
     *
     * @return Number of clock zones given as template argument.
     */
    fcn_clock::number num_clocks() const noexcept;
    /**
     * Returns true iff stored clocking scheme is regular.
     *
     * @return clocking.regular
     */
    bool is_regularly_clocked() const noexcept;
    /**
     * Returns true iff stored clocking's name string is equal to given string.
     *
     * @param name Name of clocking scheme to test for.
     * @return name == clocking.name.
     */
    bool is_clocking(std::string&& name) const noexcept;
    /**
     * Assigns a clock number c to the given face f. The clock number will only be assigned if the stored clocking
     * scheme is irregular and if c <= clocking.num_clocks hold. Otherwise, this function does nothing.
     *
     * @param f Face to which clock number c should be assigned.
     * @param c Clock number to assign to face f.
     */
    void assign_clocking(const face& f, const fcn_clock::number c) noexcept;
    /**
     * Assigns a clock number c to the given index' face f. The clock number will only be assigned if the stored
     * clocking scheme is irregular and if c <= clocking.num_clocks hold. Otherwise, this function does nothing.
     *
     * @param f Face to which clock number c should be assigned.
     * @param c Clock number to assign to face f.
     */
    void assign_clocking(const face_index f, const fcn_clock::number c) noexcept;
    /**
     * Returns whether or not given face f is a PI port.
     *
     * @param f Face whose assigned PI port should be checked.
     * @return true iff f is a PI port.
     */
    bool is_pi(const face& f) const noexcept;
    /**
     * Returns a range of all faces flagged as primary input in the layout.
     *
     * @return range_t of all primary input faces.
     */
    auto get_pis() const noexcept
    {
        return range_t<primary_set::const_iterator>{{pi_set.cbegin(), pi_set.cend()}};
    }
    /**
     * Returns whether or not given face f is a PO port.
     *
     * @param f Face whose assigned PO port should be checked.
     * @return true iff f is a PO port.
     */
    bool is_po(const face& f) const noexcept;
    /**
     * Returns a range of all faces flagged as primary output in the layout.
     *
     * @return range_t of all primary output faces.
     */
    auto get_pos() const noexcept
    {
        return range_t<primary_set::const_iterator>{{po_set.cbegin(), po_set.cend()}};
    }
    /**
     * Assigns a latch delay to a given face in the layout. The delay is given in clock phases (i.e. fractions of clock
     * cycles). The latch is virtually added on the clock number (modulo num_clocks()) for information propagation
     * purposes. For every full clock cycle added this way, the signal on this face is stalled and can therefore
     * compensate synchronisation issues. For more information on these kinds of artificial clock latches see
     * "Synchronization of Clocked Field-Coupled Circuits" by Frank Sill Torres, Marcel Walter, Robert Wille, Daniel
     * Große, and Rolf Drechsler in IEEE-NANO 2018.
     *
     * Note that latch delays are bound to the underlying clocking and therefore are valid for all faces with the same
     * Z-dimension value. The function handles this behavior already. That is, a face (x, y, z) is given, it applies the
     * latch to all faces with that x-dimension and y-dimension values without overhead.
     *
     * @param f Face (stack) to which an artificial latch delay should be assigned.
     * @param l Latch in clock phases to be assigned to face (stack) f.
     */
    void assign_latch(const face& f, const latch_delay l) noexcept;
    /**
     * Returns a previously assigned latch delay of the given face f. If no latch delay was previously assigned, 0 is
     * returned as a default value which corresponds to no artificial delay is assumed.
     *
     * @param f Face whose assigned latch delay is desired.
     * @return Assigned latch delay of face f or 0 if no delay was previously assigned.
     */
    latch_delay get_latch(const face& f) const noexcept;
    /**
     * Returns the number of latches in the layout.
     *
     * @return Number of assigned latches.
     */
    auto latch_count() const noexcept
    {
        return l_map.size();
    }
    /**
     * Returns a vector of string representations of the assigned latches. The representation looks similar to
     * "l@{1,2}=2".
     *
     * @return Vector of string representations of the assigned latches.
     */
    std::vector<std::string> latch_str_reprs() const noexcept;
    /**
     * Represents a bounding box surrounding the elements assigned to the layout's faces. Helps to determine the "real"
     * size of a layout.
     */
    struct bounding_box
    {
        /**
         * Standard constructor. Defines corner points and calculates size.
         */
        bounding_box(coord_t min_x, coord_t min_y, coord_t max_x, coord_t max_y)
                :
                min_x{min_x},
                min_y{min_y},
                max_x{max_x},
                max_y{max_y},
                x_size{max_x == 0u && min_x == 0u ? 0u : (max_x - min_x) + 1u},
                y_size{max_y == 0u && min_y == 0u ? 0u : (max_y - min_y) + 1u}
        {}

        /**
         * Corners of bounding box.
         */
        const coord_t min_x, min_y, max_x, max_y;
        /**
         * Size in x- and y-dimension.
         */
        const coord_t x_size, y_size;

        /**
         * Returns area of bounding box.
         *
         * @return x-size * y-size.
         */
        coord_t area() const noexcept
        {
            return x_size * y_size;
        }
    };
    /**
     * Determines the layout's bounding box i.e. the area in which elements are placed. Helps to determine the
     * "real" size of a layout.
     *
     * @return Bounding box.
     */
    virtual bounding_box determine_bounding_box() const noexcept = 0;
    /**
     * Returns whether the given face f is located at the layout's northern border where y is minimal.
     * A bounding box object can be given to indicate that instead the check should be made based on that one.
     *
     * @param f Face to check for border location.
     * @param bb Bonding box to use instead of layout dimensions.
     * @return True iff f is located at the layout's or bb's northern border, false otherwise.
     */
    bool is_northern_border(const face& f, const std::optional<bounding_box>& bb = std::nullopt) const noexcept;
    /**
     * Returns whether the given face f is located at the layout's eastern border where x is maximal.
     * A bounding box object can be given to indicate that instead the check should be made based on that one.
     *
     * @param f Face to check for border location.
     * @param bb Bonding box to use instead of layout dimensions.
     * @return True iff f is located at the layout's or bb's eastern border, false otherwise.
     */
    bool is_eastern_border(const face& f, const std::optional<bounding_box>& bb = std::nullopt) const noexcept;
    /**
     * Returns whether the given face f is located at the layout's southern border where y is maximal.
     * A bounding box object can be given to indicate that instead the check should be made based on that one.
     *
     * @param f Face to check for border location.
     * @param bb Bonding box to use instead of layout dimensions.
     * @return True iff f is located at the layout's or bb's southern border, false otherwise.
     */
    bool is_southern_border(const face& f, const std::optional<bounding_box>& bb = std::nullopt) const noexcept;
    /**
     * Returns whether the given face f is located at the layout's western border where x is minimal.
     * A bounding box object can be given to indicate that instead the check should be made based on that one.
     *
     * @param f Face to check for border location.
     * @param bb Bonding box to use instead of layout dimensions.
     * @return True iff f is located at the layout's or bb's western border, false otherwise.
     */
    bool is_western_border(const face& f, const std::optional<bounding_box>& bb = std::nullopt) const noexcept;
    /**
     * Returns whether the given face f is located at the layout borders where x or y are either minimal or maximal.
     * A bounding box object can be given to indicate that instead checks should be made based on that one.
     *
     * @param f Face to check for border location.
     * @param bb Bonding box to use instead of layout dimensions.
     * @return True iff f is located at one of layout's or bb's borders, false otherwise.
     */
    bool is_border(const face& f, const std::optional<bounding_box>& bb = std::nullopt) const noexcept;
    /**
     * Resizes the layout so that all empty rows and columns on the right and bottom side are removed respectively.
     * No rows and columns on the left and top are affected because that would change indices of all tiles and therefore
     * all maps would not work properly anymore.
     */
    void shrink_to_fit() noexcept;
    /**
     * Clears all maps and sets stored in the layout.
     */
    void clear_layout() noexcept;

protected:
    /**
     * Clocking scheme representing possible data flow.
     */
    fcn_clocking_scheme clocking;
    /**
     * Determines layout's row/column face shift.
     */
    const offset shift = offset::NONE;
    /**
     * Alias for a hash map that assigns clock zones to faces. Used for irregular clocking schemes only.
     */
    using clocking_map = std::unordered_map<ground, fcn_clock::zone, boost::hash<ground>>;
    /**
     * Stores a mapping face -> clock number for irregular clocking schemes. Helper functions for access save memory.
     */
    clocking_map c_map{};
    /**
     * Alias for a set that holds faces to represent PI/PO ports.
     */
    using primary_set = std::set<face>;
    /**
     * Stores faces that are marked as PIs/POs.
     */
    primary_set pi_set{}, po_set{};
    /**
     * Alias for a hash map that assigns a certain delay to a face.
     */
    using latch_map = std::unordered_map<ground, latch_delay, boost::hash<ground>>;
    /**
     * Stores a mapping face -> latch_delay. Helper functions for access save memory.
     * The idea of latch elements is based on the paper "Synchronization of Clocked Field-Coupled Circuits" by
     * Frank Sill Torres, Marcel Walter, Robert Wille, Daniel Große, and Rolf Drechsler in IEEE-NANO 2018.
     * A latch in this case represents a delay of a single face in l clock phases and thereby holds data.
     * Let l == 0 represent the standard case of no artificial delay and therefore normal signal propagation. A face
     * with clock number of 2 and l == 1 for instance, accepts data from faces with clock number of 1 as usual but holds
     * data for 1 phase longer so that it feeds information to faces with clock number 4 instead of 3. It is doubtful
     * that it is technically realizable to hold data for only 1 to 3 phases longer. What is possible however is to hold
     * data for 1 full cycle plus l phases longer. If one want to realize a hold of 2 extra phases, it can be achieved
     * by adding a latch with l == 6 (4 + 2) to hold data for one cycle and 2 phases.
     */
    latch_map l_map{};
};


#endif //FICTION_FCN_LAYOUT_H
