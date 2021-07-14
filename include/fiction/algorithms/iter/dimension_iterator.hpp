//
// Created by marcel on 06.01.20.
//

#ifndef FICTION_DIMENSION_ITERATOR_HPP
#define FICTION_DIMENSION_ITERATOR_HPP

#include "fcn_layout.h"
#include <vector>


class dimension_iterator
{
public:
    /**
     * Standard constructor. Takes a starting value and computes an initial factorization.
     *
     * @param n Starting value of the dimension iteration.
     */
    explicit dimension_iterator(uint64_t n) noexcept
            :
            num{n ? n - 1 : 0}
    {
        next();
    }
    /**
     * Lets the iterator point to the next dimension of the current factorization. If there are no next factors,
     * num is incremented and the next factors are computed.
     *
     * Prefix version.
     *
     * @return Reference to this.
     */
    dimension_iterator& operator++() noexcept
    {
        ++it;

        // end of factors: compute next ones
        if (it == factors.end())
        {
            next();
        }

        return *this;
    }
    /**
     * Creates a new iterator that points to the next dimension of the current factorization. If there are no next
     * factors, num is incremented and the next factors are computed.
     *
     * Postfix version. Less performance than the prefix version due to copy construction.
     *
     * @return Resulting iterator.
     */
    dimension_iterator operator++(int) noexcept
    {
        auto result{*this};

        ++(*this);

        return result;
    }

    fcn_dimension_xy operator*() const
    {
        return *it;
    }

    bool operator==(const uint64_t m) const noexcept
    {
        return num == m;
    }

    bool operator==(const dimension_iterator& other) const
    {
        return (num == other.num) && (*it == *(other.it));
    }

    bool operator!=(const uint64_t m) const noexcept
    {
        return num != m;
    }

    bool operator!=(const dimension_iterator& other) const
    {
        return !(*this == other);
    }

    bool operator<(const uint64_t m) const noexcept
    {
        return num < m;
    }

    bool operator<(const dimension_iterator& other) const
    {
        return (num < other.num) || (num == other.num && *it < *(other.it));
    }

    bool operator<=(const uint64_t m) const noexcept
    {
        return num <= m;
    }

    bool operator<=(const dimension_iterator& other) const
    {
        return (num <= other.num) || (num == other.num && *it <= *(other.it));
    }

private:
    /**
     * Number to factorize into dimensions.
     */
    uint64_t num;
    /**
     * Factors of num.
     */
    std::vector<fcn_dimension_xy> factors;
    /**
     * Pointer to current factor.
     */
    std::vector<fcn_dimension_xy>::iterator it;

    /**
     * Factorizes the current num into all possible factors (x, y) with x * y = num, x, y >= 2. The result is returned
     * as a vector of fcn_dimension_xy objects.
     *
     * NOTE: Due to a bug in the BGL, every dimension should have a minimum size of 2 to prevent SEGFAULTs.
     * See https://svn.boost.org/trac10/ticket/11735 for details.
     *
     * @return All possible layout dimensions with num faces.
     */
    void factorize() noexcept
    {
        factors.clear();

        for (auto i = 1u; i <= std::sqrt(num); ++i)
        {
            if (num % i == 0)
            {
                auto x = i;
                auto y = num / i;

                if (x > 1 && y > 1) // workaround for BGL bug
                {
                    factors.push_back(fcn_dimension_xy{x, y});
                    if (x != y)
                    {
                        factors.push_back(fcn_dimension_xy{y, x});
                    }
                }
            }
        }

        // let it point to the first element of the sequence
        it = factors.begin();
    }
    /**
     * Computes the next possible num where a factorization (x, y) with x * y = num, x, y >= 2 exists.
     */
    void next() noexcept
    {
        do
        {
            ++num;
            factorize();
        } while (factors.empty());
    }
};

// make dimension_iterator compatible with STL iterator categories
namespace std
{
    template<>
    struct iterator_traits<dimension_iterator>
    {
        using iterator_category = std::forward_iterator_tag;
        using value_type = fcn_dimension_xy;
    };
}

#endif //FICTION_DIMENSION_ITERATOR_HPP
