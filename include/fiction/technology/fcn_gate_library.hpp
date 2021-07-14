//
// Created by marcel on 26.07.18.
//

#ifndef FICTION_FCN_GATE_LIBRARY_H
#define FICTION_FCN_GATE_LIBRARY_H

#include "../layouts/coordinate.hpp"
#include "../utils/array.hpp"
#include "cell_technologies.hpp"

#include <array>
#include <initializer_list>
#include <memory>

namespace fiction
{

class unsupported_gate_type_exception : public std::exception
{
  public:
    explicit unsupported_gate_type_exception(const coord_t& c) noexcept : std::exception(), coord{c} {}

    [[nodiscard]] coord_t where() const noexcept
    {
        return coord;
    }

  private:
    const coord_t coord;
};

class unsupported_gate_orientation_exception : public std::exception
{
  public:
    unsupported_gate_orientation_exception(const coord_t& c, const port_list& p) noexcept :
            std::exception(),
            coord{c},
            ports{p}
    {}

    [[nodiscard]] coord_t where() const noexcept
    {
        return coord;
    }

    [[nodiscard]] port_list which_ports() const noexcept
    {
        return ports;
    }

  private:
    const coord_t   coord;
    const port_list ports;
};

/**
 * This class represents a base class for various FCN libraries used to map gate tiles to cell level.
 */
template <typename Technology, uint16_t GateSizeX, uint16_t GateSizeY>
class fcn_gate_library
{
  public:
    using technology = Technology;

    template <typename T>
    using cell_list = std::array<std::array<T, GateSizeX>, GateSizeY>;

    using fcn_gate = cell_list<typename Technology::cell_type>;

    explicit fcn_gate_library() = delete;

    template <typename T>
    static constexpr fcn_gate cell_list_to_gate(const cell_list<T>& c) noexcept
    {
        return convert_array_of_arrays<typename Technology::cell_type>(c);
    }
    /**
     * Rotates the given fcn_gate by 90° clockwise.
     *
     * @param g fcn_gate to rotate.
     * @return Rotated fcn_gate.
     */
    static constexpr fcn_gate rotate_90(const fcn_gate& g) noexcept
    {
        return reverse_columns(transpose(g));
    }
    /**
     * Rotates the given fcn_gate by 180°.
     *
     * @param g fcn_gate to rotate.
     * @return Rotated fcn_gate.
     */
    static constexpr fcn_gate rotate_180(const fcn_gate& g) noexcept
    {
        return reverse_columns(reverse_rows(g));
    }
    /**
     * Rotates the given fcn_gate by 270° clockwise.
     *
     * @param g fcn_gate to rotate.
     * @return Rotated fcn_gate.
     */
    static constexpr fcn_gate rotate_270(const fcn_gate& g) noexcept
    {
        return transpose(reverse_columns(g));
    }
    /**
     * Merges multiple fcn_gates into one. This is intended to be used for wires. Unexpected behavior can be caused, if
     * more than one fcn_gate has a cell at the same position.
     *
     * @param gates Vector of gates to be merged.
     * @return Merged fcn_gate.
     */
    static constexpr fcn_gate merge(const std::vector<fcn_gate>& gates) noexcept
    {
        auto merged = empty_gate;

        for (auto x = 0ul; x < GateSizeX; ++x)
        {
            for (auto y = 0ul; y < GateSizeY; ++y)
            {
                for (const auto& g : gates)
                {
                    if (!Technology::is_empty_cell(g[x][y]))
                        merged[x][y] = g[x][y];
                }
            }
        }

        return merged;
    }
    /**
     * Applies given mark to given fcn_gate g at given port p.
     *
     * @param g Gate to apply mark to.
     * @param p Port specifying where to apply the mark.
     * @param mark Mark to be applied
     * @return Marked fcn_gate.
     */
    template <typename Port>
    static constexpr fcn_gate mark_cell(const fcn_gate& g, const Port& p,
                                        const typename Technology::cell_mark& mark) noexcept
    {
        auto marked_gate = g;

        marked_gate[p.y][p.x] = static_cast<typename Technology::cell_type>(mark);

        return marked_gate;
    }
    /**
     * Returns horizontal size of gate blocks.
     *
     * @return GateSizeX.
     */
    [[nodiscard]] static constexpr uint16_t gate_x_size() noexcept
    {
        return GateSizeX;
    }
    /**
     * Returns vertical size of gate blocks.
     *
     * @return GateSizeY.
     */
    [[nodiscard]] static constexpr uint16_t gate_y_size() noexcept
    {
        return GateSizeY;
    }

  protected:
    /**
     * Transposes the given fcn_gate.
     *
     * @param g fcn_gate to transpose.
     * @return Transposed fcn_gate.
     */
    static constexpr fcn_gate transpose(const fcn_gate& g) noexcept
    {
        auto trans = empty_gate;

        for (auto x = 0ul; x < GateSizeX; ++x)
        {
            for (auto y = 0ul; y < GateSizeY; ++y) { trans[y][x] = g[x][y]; }
        }

        return trans;
    }
    /**
     * Reverses the columns of the given fcn_gate.
     *
     * @param g fcn_gate whose columns are to be reversed.
     * @return fcn_gate with reversed columns.
     */
    static constexpr fcn_gate reverse_columns(const fcn_gate& g) noexcept
    {
        fcn_gate rev_cols = g;

        std::for_each(std::begin(rev_cols), std::end(rev_cols),
                      [](auto& i) { std::reverse(std::begin(i), std::end(i)); });

        return rev_cols;
    }
    /**
     * Reverses the rows of the given fcn_gate.
     *
     * @param g fcn_gate whose rows are to be reversed.
     * @return fcn_gate with reversed rows.
     */
    static constexpr fcn_gate reverse_rows(const fcn_gate& g) noexcept
    {
        fcn_gate rev_rows = g;

        std::reverse(std::begin(rev_rows), std::end(rev_rows));

        return rev_rows;
    }
    /**
     * Single empty gate in given technology and tile_size. Used as a blue print to create new ones in merge and
     * transpose for example.
     */
    static constexpr const fcn_gate empty_gate =
        fiction::create_array<GateSizeY>(fiction::create_array<GateSizeX>(Technology::cell_type::EMPTY));
};

}  // namespace fiction

#endif  // FICTION_FCN_GATE_LIBRARY_H
