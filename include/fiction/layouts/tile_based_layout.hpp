//
// Created by marcel on 31.03.21.
//

#ifndef FICTION_TILE_BASED_LAYOUT_HPP
#define FICTION_TILE_BASED_LAYOUT_HPP

#include "../utils/range.hpp"
#include "coordinate.hpp"

#include <mockturtle/networks/detail/foreach.hpp>

#include <algorithm>
#include <cstdint>

namespace fiction
{

class tile_based_layout
{
  public:
#pragma region Types and constructors

    using tile         = coord_t;
    using aspect_ratio = coord_t;

    struct tile_based_layout_storage
    {
        explicit tile_based_layout_storage(const aspect_ratio& ar) noexcept : dimension{ar} {};

        aspect_ratio dimension;
    };

    using base_type = tile_based_layout;

    using storage = std::shared_ptr<tile_based_layout_storage>;

    explicit tile_based_layout(const aspect_ratio& ar = {}) : strg{std::make_shared<tile_based_layout_storage>(ar)} {}

    explicit tile_based_layout(std::shared_ptr<tile_based_layout_storage> s) : strg{std::move(s)} {}

#pragma endregion

#pragma region Structural properties

    [[nodiscard]] uint64_t x() const noexcept
    {
        return strg->dimension.x;
    }

    [[nodiscard]] uint64_t y() const noexcept
    {
        return strg->dimension.y;
    }

    [[nodiscard]] uint64_t z() const noexcept
    {
        return strg->dimension.z;
    }

    [[nodiscard]] uint64_t area() const noexcept
    {
        return x() + 1 * y() + 1;
    }

    void resize(const aspect_ratio& ar) noexcept
    {
        strg->dimension = ar;
    }

#pragma endregion

#pragma region Cardinal operations

    [[nodiscard]] constexpr tile north(const tile& t) const noexcept
    {
        if (t.y == 0ull)
            return t;

        auto nt = t;
        --nt.y;

        return nt;
    }

    [[nodiscard]] tile east(const tile& t) const noexcept
    {
        auto et = t;

        if (t.x > x())
            et.d = 1;
        else if (t.x < x())
            ++et.x;

        return et;
    }

    [[nodiscard]] tile south(const tile& t) const noexcept
    {
        auto st = t;

        if (t.y > y())
            st.d = 1;
        else if (t.y < y())
            ++st.y;

        return st;
    }

    [[nodiscard]] constexpr tile west(const tile& t) const noexcept
    {
        if (t.x == 0ull)
            return t;

        auto wt = t;
        --wt.x;

        return wt;
    }

    [[nodiscard]] tile above(const tile& t) const noexcept
    {
        auto at = t;

        if (t.z > z())
            at.d = 1;
        else if (t.z < z())
            ++at.z;

        return at;
    }

    [[nodiscard]] constexpr tile below(const tile& t) const noexcept
    {
        if (t.z == 0ull)
            return t;

        auto bt = t;
        --bt.z;

        return bt;
    }

    [[nodiscard]] constexpr bool is_north_of(const tile& t1, const tile& t2) const noexcept
    {
        return t1 != t2 && north(t1) == t2;
    }

    [[nodiscard]] bool is_east_of(const tile& t1, const tile& t2) const noexcept
    {
        return t1 != t2 && east(t1) == t2;
    }

    [[nodiscard]] bool is_south_of(const tile& t1, const tile& t2) const noexcept
    {
        return t1 != t2 && south(t1) == t2;
    }

    [[nodiscard]] constexpr bool is_west_of(const tile& t1, const tile& t2) const noexcept
    {
        return t1 != t2 && west(t1) == t2;
    }

    [[nodiscard]] bool is_adjacent_of(const tile& t1, const tile& t2) const noexcept
    {
        return is_north_of(t1, t2) || is_east_of(t1, t2) || is_south_of(t1, t2) || is_west_of(t1, t2);
    }

    [[nodiscard]] bool is_above_of(const tile& t1, const tile& t2) const noexcept
    {
        return t1 != t2 && above(t1) == t2;
    }

    [[nodiscard]] constexpr bool is_below_of(const tile& t1, const tile& t2) const noexcept
    {
        return t1 != t2 && below(t1) == t2;
    }

    [[nodiscard]] constexpr bool is_northwards_of(const tile& t1, const tile& t2) const noexcept
    {
        return (t1.z == t2.z) && (t1.y > t2.y) && (t1.x == t2.x);
    }

    [[nodiscard]] constexpr bool is_eastwards_of(const tile& t1, const tile& t2) const noexcept
    {
        return (t1.z == t2.z) && (t1.y == t2.y) && (t1.x < t2.x);
    }

    [[nodiscard]] constexpr bool is_southwards_of(const tile& t1, const tile& t2) const noexcept
    {
        return (t1.z == t2.z) && (t1.y < t2.y) && (t1.x == t2.x);
    }

    [[nodiscard]] constexpr bool is_westwards_of(const tile& t1, const tile& t2) const noexcept
    {
        return (t1.z == t2.z) && (t1.y == t2.y) && (t1.x > t2.x);
    }

    [[nodiscard]] constexpr bool is_northern_border(const tile& t) const noexcept
    {
        return t.y == 0ull;
    }

    [[nodiscard]] bool is_eastern_border(const tile& t) const noexcept
    {
        return t.x == x();
    }

    [[nodiscard]] bool is_southern_border(const tile& t) const noexcept
    {
        return t.y == y();
    }

    [[nodiscard]] constexpr bool is_western_border(const tile& t) const noexcept
    {
        return t.x == 0ull;
    }

    [[nodiscard]] bool is_border(const tile& t) const noexcept
    {
        return is_northern_border(t) || is_eastern_border(t) || is_southern_border(t) || is_western_border(t);
    }

    [[nodiscard]] tile northern_border_of(const tile& t) const noexcept
    {
        return {t.x, 0ull, t.z};
    }

    [[nodiscard]] tile eastern_border_of(const tile& t) const noexcept
    {
        return {x(), t.y, t.z};
    }

    [[nodiscard]] tile southern_border_of(const tile& t) const noexcept
    {
        return {t.x, y(), t.z};
    }

    [[nodiscard]] tile western_border_of(const tile& t) const noexcept
    {
        return {0ull, t.y, t.z};
    }

    [[nodiscard]] constexpr bool is_ground_layer(const tile& t) const noexcept
    {
        return t.z == 0ull;
    }

    [[nodiscard]] constexpr bool is_crossing_layer(const tile& t) const noexcept
    {
        return t.z > 0ull;
    }

#pragma endregion

#pragma region Iteration

    [[nodiscard]] auto tiles(const tile& start = {}, const tile& stop = {}) const
    {
        return range_t{
            std::make_pair(coord_iterator{strg->dimension, start.is_dead() ? tile{0, 0} : start},
                           coord_iterator{strg->dimension, stop.is_dead() ? strg->dimension.get_dead() : stop})};
    }

    template <typename Fn>
    void foreach_tile(Fn&& fn, const tile& start = {}, const tile& stop = {}) const
    {
        mockturtle::detail::foreach_element(
            coord_iterator{strg->dimension, start.is_dead() ? tile{0, 0} : start},
            coord_iterator{strg->dimension, stop.is_dead() ? strg->dimension.get_dead() : stop}, fn);
    }

    [[nodiscard]] auto ground_tiles(const tile& start = {}, const tile& stop = {}) const
    {
        assert(start.z == 0 && stop.z == 0);

        auto ground_layer = aspect_ratio{x(), y(), 0};

        return range_t{std::make_pair(coord_iterator{ground_layer, start.is_dead() ? tile{0, 0} : start},
                                      coord_iterator{ground_layer, stop.is_dead() ? ground_layer.get_dead() : stop})};
    }

    template <typename Fn>
    void foreach_ground_tile(Fn&& fn, const tile& start = {}, const tile& stop = {}) const
    {
        assert(start.z == 0 && stop.z == 0);

        auto ground_layer = aspect_ratio{x(), y(), 0};

        mockturtle::detail::foreach_element(
            coord_iterator{ground_layer, start.is_dead() ? tile{0, 0} : start},
            coord_iterator{ground_layer, stop.is_dead() ? ground_layer.get_dead() : stop}, fn);
    }

    template <typename Container>
    Container adjacent_tiles(const tile& t) const noexcept
    {
        Container c{};

        const auto add_if_not_t = [&c, &t](const auto& cardinal)
        {
            if (cardinal != t)
                c.insert(c.end(), cardinal);
        };

        add_if_not_t(north(t));
        add_if_not_t(east(t));
        add_if_not_t(south(t));
        add_if_not_t(west(t));

        return c;
    }

#pragma endregion

  private:
    storage strg;
};

}  // namespace fiction

#endif  // FICTION_TILE_BASED_LAYOUT_HPP
