//
// Created by marcel on 14.05.21.
//

#ifndef FICTION_CLOCKED_LAYOUT_HPP
#define FICTION_CLOCKED_LAYOUT_HPP

#include "clocking_scheme.hpp"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <set>

namespace fiction
{

template <typename TileBasedLayout>
class clocked_layout : public TileBasedLayout
{
  public:
#pragma region Types and constructors

    using clock_zone = typename TileBasedLayout::tile;

    using clocking_scheme_t = clocking_scheme<clock_zone>;
    using clock_number_t    = typename clocking_scheme_t::clock_number;

    using degree_t = uint8_t;

    struct clocked_layout_storage
    {
        explicit clocked_layout_storage(const clocking_scheme_t& scheme) noexcept :
                clocking{std::make_shared<clocking_scheme_t>(scheme)}
        {}

        std::shared_ptr<clocking_scheme_t> clocking;
    };

    using base_type = clocked_layout;

    using storage = std::shared_ptr<clocked_layout_storage>;

    explicit clocked_layout(const typename TileBasedLayout::aspect_ratio& aspect_ratio) :
            TileBasedLayout(aspect_ratio),
            strg{std::make_shared<clocked_layout_storage>(open_4_clocking)}
    {}

    clocked_layout(const typename TileBasedLayout::aspect_ratio& aspect_ratio, const clocking_scheme_t& scheme) :
            TileBasedLayout(aspect_ratio),
            strg{std::make_shared<clocked_layout_storage>(scheme)}
    {}

    explicit clocked_layout(std::shared_ptr<clocked_layout_storage> storage) : strg{std::move(storage)} {}

#pragma endregion

#pragma region Clocking

    void assign_clock_number(const clock_zone& cz, const clock_number_t cn) noexcept
    {
        strg->clocking->override_clock_number(cz, cn);
    }

    [[nodiscard]] clock_number_t get_clock_number(const clock_zone& cz) const noexcept
    {
        return (*strg->clocking)(cz);
    }

    [[nodiscard]] clock_number_t num_clocks() const noexcept
    {
        return strg->clocking->num_clocks;
    }

    [[nodiscard]] bool is_regularly_clocked() const noexcept
    {
        return strg->clocking->is_regular();
    }

    [[nodiscard]] bool is_clocking_scheme(std::string&& name) const noexcept
    {
        return *strg->clocking == name;
    }

    [[nodiscard]] bool is_incoming_clocked(const clock_zone& cz1, const clock_zone& cz2) const noexcept
    {
        if (cz1 == cz2)
            return false;

        return static_cast<clock_number_t>((get_clock_number(cz2) + static_cast<clock_number_t>(1)) % num_clocks()) ==
               get_clock_number(cz1);
    }

    [[nodiscard]] bool is_outgoing_clocked(const clock_zone& cz1, const clock_zone& cz2) const noexcept
    {
        if (cz1 == cz2)
            return false;

        return static_cast<clock_number_t>((get_clock_number(cz1) + static_cast<clock_number_t>(1)) % num_clocks()) ==
               get_clock_number(cz2);
    }

#pragma endregion

#pragma region Iteration

    template <typename Container>
    [[nodiscard]] Container incoming_clocked_zones(const clock_zone& cz) const noexcept
    {
        auto adj = TileBasedLayout::template adjacent_tiles<Container>(cz);

        Container incoming{};

        std::copy_if(std::cbegin(adj), std::cend(adj), std::inserter(incoming, std::cend(incoming)),
                     [this, &cz](const auto& ct) { return is_incoming_clocked(cz, ct); });

        return incoming;
    }

    template <typename Container>
    [[nodiscard]] Container outgoing_clocked_zones(const clock_zone& cz) const noexcept
    {
        auto adj = TileBasedLayout::template adjacent_tiles<Container>(cz);

        Container outgoing{};

        std::copy_if(std::cbegin(adj), std::cend(adj), std::inserter(outgoing, std::cend(outgoing)),
                     [this, &cz](const auto& ct) { return is_outgoing_clocked(cz, ct); });

        return outgoing;
    }

#pragma endregion

#pragma region Structural properties

    degree_t in_degree(const clock_zone& cz) const noexcept
    {
        return static_cast<degree_t>(incoming_clocked_zones<std::set<clock_zone>>(cz).size());
    }

    degree_t out_degree(const clock_zone& cz) const noexcept
    {
        return static_cast<degree_t>(outgoing_clocked_zones<std::set<clock_zone>>(cz).size());
    }

    degree_t degree(const clock_zone& cz) const noexcept
    {
        return static_cast<degree_t>(in_degree(cz) + out_degree(cz));
    }

#pragma endregion

  private:
    storage strg;
};

}  // namespace fiction

#endif  // FICTION_CLOCKED_LAYOUT_HPP
