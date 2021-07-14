//
// Created by marcel on 28.06.21.
//

#ifndef FICTION_HASH_HPP
#define FICTION_HASH_HPP

#include <functional>
#include <set>

namespace fiction
{

/**
 * From https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
 * @tparam T
 * @tparam Rest
 * @param seed
 * @param v
 * @param rest
 */
template <typename T, typename... Rest>
void hash_combine(std::size_t& seed, const T& v, const Rest&... rest)
{
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hash_combine(seed, rest), ...);
}

}  // namespace fiction

namespace std
{

template <typename T>
struct hash<std::set<T>>
{
    std::size_t operator()(const std::set<T>& s) const noexcept
    {
        std::size_t h = 0;
        for (const auto& e : s) fiction::hash_combine(h, e);

        return h;
    }
};

}  // namespace std

#endif  // FICTION_HASH_HPP
