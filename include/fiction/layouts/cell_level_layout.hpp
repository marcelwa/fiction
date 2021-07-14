//
// Created by marcel on 23.06.21.
//

#ifndef FICTION_CELL_LEVEL_LAYOUT_HPP
#define FICTION_CELL_LEVEL_LAYOUT_HPP

#include "../technology/cell_technologies.hpp"
#include "clocking_scheme.hpp"

#include <mockturtle/networks/detail/foreach.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace fiction
{

template <typename Technology, typename ClockedLayout>
class cell_level_layout : public ClockedLayout
{
  public:
#pragma region Types and constructors

    using cell      = typename ClockedLayout::tile;
    using cell_type = typename Technology::cell_type;
    using cell_mode = typename Technology::cell_mode;

    template <typename Cell>
    struct cell_level_layout_storage
    {
        explicit cell_level_layout_storage(std::string name, uint16_t tile_x = 1u, uint16_t tile_y = 1u) :
                layout_name{std::move(name)},
                tile_size_x{tile_x},
                tile_size_y{tile_y}
        {}

        std::string layout_name;

        const uint16_t tile_size_x;
        const uint16_t tile_size_y;

        std::unordered_map<Cell, cell_type> cell_type_map{};
        std::unordered_map<Cell, cell_mode> cell_mode_map{};

        std::unordered_map<Cell, std::string> cell_name_map{};

        std::unordered_set<Cell> inputs{}, outputs{};
    };

    using base_type = cell_level_layout;

    using technology = Technology;

    using storage = std::shared_ptr<cell_level_layout_storage<uint64_t>>;

    explicit cell_level_layout(const typename ClockedLayout::aspect_ratio& ar = {}, const std::string& name = "",
                               const uint16_t tile_size_x = 1u, const uint16_t tile_size_y = 1u) :
            ClockedLayout(ar),
            strg{std::make_shared<cell_level_layout_storage<uint64_t>>(name, tile_size_x, tile_size_y)}
    {}

    cell_level_layout(const typename ClockedLayout::aspect_ratio& ar, const clocking_scheme<cell>& scheme,
                      const std::string& name = "", const uint16_t tile_size_x = 1u, const uint16_t tile_size_y = 1u) :
            ClockedLayout(ar, scheme),
            strg{std::make_shared<cell_level_layout_storage<uint64_t>>(name, tile_size_x, tile_size_y)}
    {}

    explicit cell_level_layout(std::shared_ptr<cell_level_layout_storage<uint64_t>> s) : strg{std::move(s)} {}

#pragma endregion

#pragma region Cell types

    void assign_cell_type(const cell& c, const cell_type& ct) noexcept
    {
        if (Technology::is_empty_cell(ct))
        {
            strg->cell_type_map.erase(static_cast<uint64_t>(c));
            strg->cell_mode_map.erase(static_cast<uint64_t>(c));
            strg->inputs.erase(static_cast<uint64_t>(c));
            strg->outputs.erase(static_cast<uint64_t>(c));

            return;
        }
        else if (Technology::is_input_cell(ct))
        {
            strg->inputs.insert(static_cast<uint64_t>(c));
        }
        else if (Technology::is_output_cell(ct))
        {
            strg->outputs.insert(static_cast<uint64_t>(c));
        }

        strg->cell_type_map[static_cast<uint64_t>(c)] = ct;
    }

    [[nodiscard]] cell_type get_cell_type(const cell& c) const noexcept
    {
        if (auto it = strg->cell_type_map.find(static_cast<uint64_t>(c)); it != strg->cell_type_map.cend())
        {
            return it->second;
        }
        else
        {
            return Technology::cell_type::EMPTY;
        }
    }

    [[nodiscard]] bool is_empty_cell(const cell& c) const noexcept
    {
        return Technology::is_empty_cell(get_cell_type(c));
    }

    void assign_cell_mode(const cell& c, const cell_mode& m) noexcept
    {
        if (Technology::is_normal_cell_mode(m))
            strg->cell_mode_map.erase(static_cast<uint64_t>(c));
        else
            strg->cell_mode_map[static_cast<uint64_t>(c)] = m;
    }

    [[nodiscard]] cell_mode get_cell_mode(const cell& c) const noexcept
    {
        if (auto it = strg->cell_mode_map.find(static_cast<uint64_t>(c)); it != strg->cell_mode_map.cend())
        {
            return it->second;
        }
        else
        {
            return Technology::cell_mode::NORMAL;
        }
    }

    void assign_cell_name(const cell& c, const std::string& n) noexcept
    {
        if (n.empty())
            strg->cell_name_map.erase(static_cast<uint64_t>(c));
        else
            strg->cell_name_map[static_cast<uint64_t>(c)] = n;
    }

    [[nodiscard]] std::string get_cell_name(const cell& c) const noexcept
    {
        if (auto it = strg->cell_name_map.find(static_cast<uint64_t>(c)); it != strg->cell_name_map.cend())
        {
            return it->second;
        }
        else
        {
            return "";
        }
    }

#pragma endregion

#pragma region Properties

    [[nodiscard]] std::string get_layout_name() const noexcept
    {
        return strg->layout_name;
    }

    void set_layout_name(const std::string& name) noexcept
    {
        strg->layout_name = name;
    }

    [[nodiscard]] uint64_t num_cells() const noexcept
    {
        return static_cast<uint64_t>(strg->cell_type_map.size());
    }

    [[nodiscard]] uint32_t num_pis() const noexcept
    {
        return static_cast<uint32_t>(strg->inputs.size());
    }

    [[nodiscard]] uint32_t num_pos() const noexcept
    {
        return static_cast<uint32_t>(strg->outputs.size());
    }

    [[nodiscard]] bool is_pi(const cell& c) const noexcept
    {
        return std::find(strg->inputs.cbegin(), strg->inputs.cend(), c) != strg->inputs.cend();
    }

    [[nodiscard]] bool is_po(const cell& c) const noexcept
    {
        return std::find(strg->outputs.cbegin(), strg->outputs.cend(), c) != strg->outputs.cend();
    }

    [[nodiscard]] uint16_t get_tile_size_x() const noexcept
    {
        return strg->tile_size_x;
    }

    [[nodiscard]] uint16_t get_tile_size_y() const noexcept
    {
        return strg->tile_size_y;
    }

#pragma endregion

#pragma region Clocking

    [[nodiscard]] typename ClockedLayout::clock_number_t
    get_clock_number(const typename ClockedLayout::clock_zone& cz) const noexcept
    {
        return ClockedLayout::get_clock_number({cz.x / strg->tile_size_x, cz.y / strg->tile_size_y, cz.z});
    }

    [[maybe_unused]] bool is_incoming_clocked(const typename ClockedLayout::clock_zone& cz1,
                                              const typename ClockedLayout::clock_zone& cz2) const noexcept = delete;

    [[maybe_unused]] bool is_outgoing_clocked(const typename ClockedLayout::clock_zone& cz1,
                                              const typename ClockedLayout::clock_zone& cz2) const noexcept = delete;

#pragma endregion

#pragma region Iteration

    template <typename Fn>
    void foreach_cell(Fn&& fn) const
    {
        using IteratorType = decltype(strg->cell_type_map.cbegin());
        mockturtle::detail::foreach_element_transform<IteratorType, cell>(
            strg->cell_type_map.cbegin(), strg->cell_type_map.cend(),
            [](const auto& ct) { return static_cast<cell>(ct.first); }, fn);
    }

    template <typename Fn>
    void foreach_cell_position(Fn&& fn) const
    {
        ClockedLayout::foreach_tile(fn);
    }

#pragma endregion

  private:
    storage strg;
};

}  // namespace fiction

#endif  // FICTION_CELL_LEVEL_LAYOUT_HPP
