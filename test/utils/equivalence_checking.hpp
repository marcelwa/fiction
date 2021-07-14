//
// Created by marcel on 26.05.21.
//

#ifndef FICTION_EQUIVALENCE_CHECKING_HPP
#define FICTION_EQUIVALENCE_CHECKING_HPP

#include "catch.hpp"

#include <mockturtle/algorithms/equivalence_checking.hpp>
#include <mockturtle/algorithms/miter.hpp>
#include <mockturtle/networks/klut.hpp>

template <typename Ntk1, typename Ntk2>
void check_eq(Ntk1 ntk1, Ntk2 ntk2)
{
    auto miter = mockturtle::miter<mockturtle::klut_network>(ntk1, ntk2);

    REQUIRE(miter.has_value());

    auto eq = mockturtle::equivalence_checking(*miter);

    REQUIRE(eq.has_value());
    CHECK(*eq);
}

#endif  // FICTION_EQUIVALENCE_CHECKING_HPP
