//
// Created by marcel on 01.05.21.
//

#ifndef FICTION_COORDINATE_HPP
#define FICTION_COORDINATE_HPP

#include <fmt/format.h>

#include <cstdint>
#include <iostream>

namespace fiction
{
struct coord_t
{
    uint64_t d : 1;  // MSB as dead indicator
    uint64_t z : 1;
    uint64_t y : 31;
    uint64_t x : 31;

    constexpr coord_t() :
            d{static_cast<uint64_t>(1ull)},  // default-constructed coord_ts are dead
            z{static_cast<uint64_t>(0ull)},
            y{static_cast<uint64_t>(0ull)},
            x{static_cast<uint64_t>(0ull)}
    {}

    template <class X, class Y, class Z>
    constexpr coord_t(X x, Y y, Z z) :
            d{static_cast<uint64_t>(0ull)},
            z{static_cast<uint64_t>(z)},
            y{static_cast<uint64_t>(y)},
            x{static_cast<uint64_t>(x)}
    {}

    template <class X, class Y>
    constexpr coord_t(X x, Y y) :
            d{static_cast<uint64_t>(0ull)},
            z{static_cast<uint64_t>(0ull)},
            y{static_cast<uint64_t>(y)},
            x{static_cast<uint64_t>(x)}
    {}

    constexpr explicit coord_t(const uint64_t t) :
            d{static_cast<uint64_t>(t >> 63)},
            z{static_cast<uint64_t>((t << 1) >> 63)},
            y{static_cast<uint64_t>((t << 2) >> 33)},
            x{static_cast<uint64_t>((t << 33) >> 33)}
    {}

    explicit constexpr operator uint64_t() const
    {
        return (((((((0ull | d) << 1) | z) << 31) | y) << 31) | x);
    }

    [[nodiscard]] constexpr bool is_dead() const
    {
        return static_cast<bool>(d);
    }

    [[nodiscard]] constexpr coord_t get_dead() const
    {
        return coord_t{static_cast<uint64_t>(*this) | static_cast<uint64_t>(coord_t{})};
    }

    constexpr bool operator==(const coord_t& other) const
    {
        return d == other.d && z == other.z && y == other.y && x == other.x;
    }

    constexpr bool operator==(const uint64_t& other) const
    {
        return static_cast<uint64_t>(*this) == other;
    }

    constexpr bool operator!=(const coord_t& other) const
    {
        return !(*this == other);
    }

    constexpr bool operator<(const coord_t& other) const
    {
        if (z < other.z)
            return true;

        if (z == other.z)
        {
            if (y < other.y)
                return true;

            if (y == other.y)
            {
                return x < other.x;
            }
        }

        return false;
    }

    constexpr bool operator>(const coord_t& other) const
    {
        return other < *this;
    }

    constexpr bool operator<=(const coord_t& other) const
    {
        return !(*this > other);
    }

    constexpr bool operator>=(const coord_t& other) const
    {
        return !(*this < other);
    }
};

std::ostream& operator<<(std::ostream& os, const coord_t& t)
{
    os << fmt::format("({},{},{})", t.x, t.y, t.z);
    return os;
}

class coord_iterator
{
  public:
    using value_type = coord_t;

    constexpr explicit coord_iterator(const coord_t& dimension, const coord_t& start) noexcept :
            dimension{dimension},
            coord{start}
    {}

    constexpr coord_iterator& operator++() noexcept
    {
        if (coord != dimension)
        {
            ++coord.x;

            if (coord.x > dimension.x)
            {
                coord.x = 0;

                ++coord.y;
                if (coord.y > dimension.y)
                {
                    coord.y = 0;

                    ++coord.z;
                }
            }
        }
        else
        {
            coord = coord.get_dead();
        }

        return *this;
    }

    constexpr const coord_iterator operator++(int) noexcept
    {
        const auto result{*this};

        ++(*this);

        return result;
    }

    constexpr coord_t operator*() const
    {
        return coord;
    }

    constexpr bool operator==(const coord_iterator& other) const
    {
        return (coord == other.coord);
    }

    constexpr bool operator!=(const coord_iterator& other) const
    {
        return !(*this == other);
    }

    constexpr bool operator<(const coord_iterator& other) const
    {
        return (coord < other.coord);
    }

    constexpr bool operator<=(const coord_iterator& other) const
    {
        return (coord <= other.coord);
    }

  private:
    const coord_t dimension;

    coord_t coord;
};
}  // namespace fiction

namespace std
{
// make coord_iterator compatible with STL iterator categories
template <>
struct iterator_traits<fiction::coord_iterator>
{
    using iterator_category = std::forward_iterator_tag;
    using value_type        = fiction::coord_t;
};
}  // namespace std

#endif  // FICTION_COORDINATE_HPP
