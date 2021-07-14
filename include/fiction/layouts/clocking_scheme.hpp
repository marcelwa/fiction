//
// Created by marcel on 12.05.21.
//

#ifndef FICTION_CLOCKING_SCHEME_HPP
#define FICTION_CLOCKING_SCHEME_HPP

#include "coordinate.hpp"

#include <array>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace fiction
{
template <typename ClockZone>
class clocking_scheme
{
  public:
    using clock_zone     = ClockZone;
    using clock_number   = uint8_t;
    using clock_function = std::function<clock_number(ClockZone)>;

    explicit clocking_scheme(const std::string& n, clock_function f, const clock_number cn = 4,
                             const bool r = true) noexcept :
            name{n},
            num_clocks{cn},
            regular{r},
            fn{std::move(f)}
    {}

    clock_number operator()(ClockZone cz) const noexcept
    {
        if (regular)
            return fn(cz);

        if (auto it = override.find(static_cast<uint64_t>(cz)); it != override.end())
        {
            return it->second;
        }

        return fn(cz);
    }

    bool operator==(const std::string& n) const noexcept
    {
        return name == n;
    }

    [[nodiscard]] bool is_regular() const noexcept
    {
        return regular;
    }

    void override_clock_number(const ClockZone& cz, const clock_number cn) noexcept
    {
        regular = false;

        override[static_cast<uint64_t>(cz)] = cn % num_clocks;
    }
    /**
     * Name of the clocking scheme.
     */
    const std::string name;
    /**
     * Number of different clocks in this scheme.
     */
    const clock_number num_clocks;

  private:
    /**
     * Defines the clocking as regular and well-defined by the scheme.
     */
    bool regular;
    /**
     * A function that determines clock numbers for given zones.
     */
    const clock_function fn;
    /**
     * Alias for a hash map that overrides clock zones.
     */
    using clocking_map = std::unordered_map<uint64_t, clock_number>;
    /**
     * Stores mappings ClockZone -> clock_number to override clock zones.
     * ClockZone must be convertible to uint64_t.
     */
    clocking_map override{};
};

namespace clock_function
{
/**
 * Representing an irregular clocking that always returns the standard clock. It is intended to be overridden.
 */
const clocking_scheme<coord_t>::clock_function open = []([[maybe_unused]] coord_t cz) noexcept
{ return clocking_scheme<coord_t>::clock_number{}; };
/**
 * Representing a 3-phase adoption of the 2DDWave clocking as defined in "Clocking and Cell Placement for QCA" by
 * V. Vankamamidi, M. Ottavi, and F. Lombardi in IEEE Conference on Nanotechnology 2006.
 */
const clocking_scheme<coord_t>::clock_function twoddwave_3 = [](coord_t cz) noexcept
{
    constexpr std::array<std::array<clocking_scheme<coord_t>::clock_number, 3u>, 3u> cutout{
        {{{0, 1, 2}}, {{1, 2, 0}}, {{2, 0, 1}}}};

    return cutout[cz.y % 3ul][cz.x % 3ul];
};
/**
 * Representing the original 2DDWave clocking as defined in "Clocking and Cell Placement for QCA" by V. Vankamamidi,
 * M. Ottavi, and F. Lombardi in IEEE Conference on Nanotechnology 2006.
 */
const clocking_scheme<coord_t>::clock_function twoddwave_4 = [](coord_t cz) noexcept
{
    constexpr std::array<std::array<clocking_scheme<coord_t>::clock_number, 4u>, 4u> cutout{
        {{{0, 1, 2, 3}}, {{1, 2, 3, 0}}, {{2, 3, 0, 1}}, {{3, 0, 1, 2}}}};

    return cutout[cz.y % 4ul][cz.x % 4ul];
};
/**
 * Representing the USE clocking as defined in "USE: A Universal, Scalable, and Efficient Clocking Scheme for QCA"
 * by Caio Araujo T. Campos, Abner L. Marciano, Omar P. Vilela Neto, and Frank Sill Torres in TCAD 2015.
 */
const clocking_scheme<coord_t>::clock_function use_4 = [](coord_t s) noexcept
{
    constexpr std::array<std::array<clocking_scheme<coord_t>::clock_number, 4u>, 4u> cutout{
        {{{0, 1, 2, 3}}, {{3, 2, 1, 0}}, {{2, 3, 0, 1}}, {{1, 0, 3, 2}}}};

    return cutout[s.y % 4ul][s.x % 4ul];
};
/**
 * Representing the RES clocking as defined in "An efficient clocking scheme for quantum-dot cellular automata" by
 * Mrinal Goswami, Anindan Mondal, Mahabub Hasan Mahalat, Bibhash Sen, and Biplab K. Sikdar in International Journal
 * of Electronics Letters 2019.
 */
const clocking_scheme<coord_t>::clock_function res_4 = [](coord_t cz) noexcept
{
    constexpr std::array<std::array<clocking_scheme<coord_t>::clock_number, 4u>, 4u> cutout{
        {{{3, 0, 1, 2}}, {{0, 1, 0, 3}}, {{1, 2, 3, 0}}, {{0, 3, 2, 1}}}};

    return cutout[cz.y % 4ul][cz.x % 4ul];
};
/**
 * Representing the BANCS clocking as defined in "BANCS: Bidirectional Alternating Nanomagnetic Clocking Scheme" by
 * Ruan Evangelista Formigoni, Omar P. Vilela Neto, and Jose Augusto M. Nacif in SBCCI 2018.
 */
const clocking_scheme<coord_t>::clock_function bancs_3 = [](coord_t cz) noexcept
{
    constexpr std::array<std::array<clocking_scheme<coord_t>::clock_number, 3u>, 6u> cutout{
        {{{0, 1, 2}}, {{2, 1, 0}}, {{2, 0, 1}}, {{1, 0, 2}}, {{1, 2, 0}}, {{0, 2, 1}}}};

    return cutout[cz.y % 6ul][cz.x % 3ul];
};
/**
 * Representing a 3-phase adaption of the clocking originally introduced in "A device architecture for computing
 * with quantum dots" by C. S. Lent and P. D. Tougaw in the Proceedings of the IEEE 1997. As it is used in
 * "ToPoliNano" (https://topolinano.polito.it/), it is referred to by that name in fiction to differentiate it
 * from 2DDWave.
 */
const clocking_scheme<coord_t>::clock_function topolinano_3 = [](coord_t cz) noexcept
{
    constexpr std::array<std::array<clocking_scheme<coord_t>::clock_number, 3u>, 3u> cutout{
        {{{0, 1, 2}}, {{0, 1, 2}}, {{0, 1, 2}}}};

    return cutout[cz.y % 3ul][cz.x % 3ul];
};
/**
 * Representing a linear 4-phase 1D clocking as originally introduced in "A device architecture for computing with
 * quantum dots" by C. S. Lent and P. D. Tougaw in the Proceedings of the IEEE 1997. As it is used in "ToPoliNano"
 * (https://topolinano.polito.it/), it is referred to by that name in fiction to differentiate it from 2DDWave.
 */
const clocking_scheme<coord_t>::clock_function topolinano_4 = [](coord_t cz) noexcept
{
    constexpr std::array<std::array<clocking_scheme<coord_t>::clock_number, 4u>, 4u> cutout{
        {{{0, 1, 2, 3}}, {{0, 1, 2, 3}}, {{0, 1, 2, 3}}, {{0, 1, 2, 3}}}};

    return cutout[cz.y % 4ul][cz.x % 4ul];
};
}  // namespace clock_function

namespace clock_name
{
constexpr const char* open3       = "OPEN3";
constexpr const char* open4       = "OPEN4";
constexpr const char* twoddwave3  = "2DDWAVE3";
constexpr const char* twoddwave4  = "2DDWAVE4";
constexpr const char* use         = "USE";
constexpr const char* res         = "RES";
constexpr const char* bancs       = "BANCS";
constexpr const char* topolinano3 = "TOPOLINANO3";
constexpr const char* topolinano4 = "TOPOLINANO4";
}  // namespace clock_name

/**
 * Pre-defined open clocking with 3 clock phases.
 */
static const clocking_scheme<coord_t> open_3_clocking{clock_name::open3, clock_function::open, 3u, false};
/**
 * Pre-defined open clocking with 4 clock phases.
 */
static const clocking_scheme<coord_t> open_4_clocking{clock_name::open4, clock_function::open, 4u, false};
/**
 * Pre-defined 3-phase 2DDWave clocking instance.
 */
static const clocking_scheme<coord_t> twoddwave_3_clocking{clock_name::twoddwave3, clock_function::twoddwave_3, 3u,
                                                           true};
/**
 * Pre-defined 4-phase 2DDWave clocking instance.
 */
static const clocking_scheme<coord_t> twoddwave_4_clocking{clock_name::twoddwave4, clock_function::twoddwave_4, 4u,
                                                           true};
/**
 * Pre-defined 4-phase USE clocking instance.
 */
static const clocking_scheme<coord_t> use_4_clocking{clock_name::use, clock_function::use_4, 4u, true};
/**
 * Pre-defined 4-phase RES clocking instance.
 */
static const clocking_scheme<coord_t> res_4_clocking{clock_name::res, clock_function::res_4, 4u, true};
/**
 * Pre-defined 3-phase BANCS clocking instance.
 */
static const clocking_scheme<coord_t> bancs_3_clocking{clock_name::bancs, clock_function::bancs_3, 3u, true};
/**
 * Pre-defined 3-phase ToPoliNano clocking instance.
 */
static const clocking_scheme<coord_t> topolinano_3_clocking{clock_name::topolinano3, clock_function::topolinano_3, 3u,
                                                            true};
/**
 * Pre-defined 4-phase ToPoliNano clocking instance.
 */
static const clocking_scheme<coord_t> topolinano_4_clocking{clock_name::topolinano4, clock_function::topolinano_4, 4u,
                                                            true};

template <typename ClockZone>
std::optional<clocking_scheme<ClockZone>> get_clocking_scheme(const std::string& name) noexcept
{
    if constexpr (std::is_same_v<ClockZone, coord_t>)
    {
        static const std::unordered_map<std::string, clocking_scheme<coord_t>> scheme_lookup{
            {clock_name::open3, open_3_clocking},
            {clock_name::open4, open_4_clocking},
            {"OPEN", open_4_clocking},
            {clock_name::twoddwave3, twoddwave_3_clocking},
            {"DIAG3", twoddwave_3_clocking},
            {clock_name::twoddwave4, twoddwave_4_clocking},
            {"DIAG4", twoddwave_4_clocking},
            {"2DDWAVE", twoddwave_4_clocking},
            {clock_name::use, use_4_clocking},
            {clock_name::res, res_4_clocking},
            {clock_name::bancs, bancs_3_clocking},
            {clock_name::topolinano3, topolinano_3_clocking},
            {"TOPOLINANO", topolinano_3_clocking},
            {clock_name::topolinano4, topolinano_4_clocking}};

        auto upper_name = name;
        std::transform(upper_name.begin(), upper_name.end(), upper_name.begin(), ::toupper);

        if (auto it = scheme_lookup.find(upper_name); it != scheme_lookup.end())
        {
            return it->second;
        }
        else
        {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

}  // namespace fiction

#endif  // FICTION_CLOCKING_SCHEME_HPP
