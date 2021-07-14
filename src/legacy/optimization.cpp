//
// Created by marcel on 14.08.19.
//

#include "fcn_cell_layout.h"


void fcn_cell_layout::clean_up_topolinano() noexcept
{
    enum class status { SEARCH, COLLECT, SKIP };
    bool improvement_found;

    auto handle = [this, &improvement_found](const auto& _hump)
    {
        // single cell collections are no humps, do not merge them.
        if (_hump.size() > 1)
        {
            // if sequence starts with a PI or there are normal cells south of first and last hump cell, this is an upper hump
            if (auto inp = get_cell_type(_hump.front()),
                     out = get_cell_type(_hump.back()),
                     fts = get_cell_type(south(_hump.front())),
                     bts = get_cell_type(south(_hump.back()));
                (inp == fcn::INPUT_CELL  || fts == fcn::NORMAL_CELL) &&
                (bts == fcn::NORMAL_CELL || out == fcn::OUTPUT_CELL || bts == fcn::inml::INVERTER_MAGNET))
            {
                // hump found, check if there is enough space below for merging
                if (std::all_of(_hump.begin() + 1, _hump.end() - 2,
                                [this](auto _hc) { return is_free_cell(south(south(_hc))); }))
                {
                    // merge it down
                    for (auto& hc : _hump)
                    {
                        const auto s = south(hc);
                        if (get_cell_type(s) != fcn::inml::INVERTER_MAGNET)
                        {
                            assign_cell_type(s, get_cell_type(hc));
                            assign_cell_mode(s, get_cell_mode(hc));
                            assign_cell_name(s, get_cell_name(hc));
                        }
                        assign_cell_type(hc, fcn::EMPTY_CELL);
                        assign_cell_mode(hc, fcn::cell_mode::NORMAL);
                        assign_cell_name(hc, "");
                    }

                    improvement_found = true;
                }
            }
            // if there are normal cells north of first and last hump cell, this is a lower hump
            else if (auto ftn = get_cell_type(north(_hump.front())),
                          btn = get_cell_type(north(_hump.back()));
                    (inp == fcn::INPUT_CELL  || ftn == fcn::NORMAL_CELL) &&
                    (btn == fcn::NORMAL_CELL || out == fcn::OUTPUT_CELL))
            {
                // hump found, check if there is enough space above for merging
                if (std::all_of(_hump.begin() + 1, _hump.end() - 2,
                                [this](auto _hc) { return is_free_cell(north(north(_hc))); }))
                {
                    // merge it up
                    for (auto& hc : _hump)
                    {
                        const auto n = north(hc);
                        assign_cell_type(n, get_cell_type(hc));
                        assign_cell_mode(n, get_cell_mode(hc));
                        assign_cell_name(n, get_cell_name(hc));
                        assign_cell_type(hc, fcn::EMPTY_CELL);
                        assign_cell_mode(hc, fcn::cell_mode::NORMAL);
                        assign_cell_name(hc, "");
                    }

                    improvement_found = true;
                }
            }
        }
    };

    do
    {
        status st = status::SEARCH;
        improvement_found = false;

        for (auto&& row : iter::range(y()))
        {
            std::vector<cell> hump{};

            for (auto&& column : iter::range(x()))
            {
                // simple state machine for identifying humps and removing them
                switch (auto c = cell{column, row, GROUND}; st)
                {
                    case status::SEARCH:
                    {
                        switch (auto t = get_cell_type(c); t)
                        {
                            // encountering a normal, input, or inverter magnet triggers collecting hump cells
                            case fcn::NORMAL_CELL:
                            case fcn::INPUT_CELL:
                            case fcn::inml::INVERTER_MAGNET:
                            {
                                st = status::COLLECT;
                                hump.push_back(c);
                                break;
                            }
                            // remain searching
                            case fcn::EMPTY_CELL:
                            {
                                break;
                            }
                            // everything else leads to skipping
                            default:
                            {
                                st = status::SKIP;
                                break;
                            }
                        }
                        break;
                    }
                    case status::COLLECT:
                    {
                        switch (auto t = get_cell_type(c); t)
                        {
                            // collect cells
                            case fcn::NORMAL_CELL:
                            case fcn::inml::INVERTER_MAGNET:
                            {
                                hump.push_back(c);
                                break;
                            }
                            // interesting branch: could be a hump
                            case fcn::EMPTY_CELL:
                            case fcn::OUTPUT_CELL:
                            {
                                handle(hump);
                                // discard hump cells and start searching again
                                hump.clear();
                                st = status::SEARCH;
                                break;
                            }
                            // encountered anything else: cannot be a hump
                            default:
                            {
                                hump.clear();
                                st = status::SKIP;
                                break;
                            }
                        }
                        break;
                    }
                    case status::SKIP:
                    {
                        if (auto t = get_cell_type(c); t == fcn::EMPTY_CELL)
                        {
                            // skipping over, return to searching
                            st = status::SEARCH;
                        }
                        break;
                    }
                    break;
                }
            }
        }
    } while (improvement_found);
}

void fcn_cell_layout::cut_optimization() noexcept
{
    auto find_cut_wire = [this](auto c, bool exit_on_occupied) -> std::optional<cell>
    {
        auto counter = 0u;
        // iterate from start position left to right over layout cells
        for (; c[X] != this->x() - 1; c = east(c))
        {
            auto t = get_cell_type(c);

            if (t == fcn::NORMAL_CELL)
                ++counter;
            else if (is_free_cell(c))
            {
                if (exit_on_occupied)
                    break;
            }
            // cell occupied but not a normal cell
            if (t != fcn::NORMAL_CELL && !is_free_cell(c))
            {
                if (exit_on_occupied)
                    break;
            }

            // cut wire found, return end position
            if (counter == library->gate_x_size())
                return c;
        }

        // no cut wire in this row
        return std::nullopt;
    };

    auto is_first_cut = [this](auto c) -> bool
    {
        for (auto n = north(c); is_free_cell(n); n = north(n))
        {
            if (n[Y] == 0)
                return true;
        }

        return false;
    };

    auto cut_if_valid = [this](auto c) -> bool
    {
        std::vector<cell> cut{};
        for (auto x : iter::range(c[X] - (library->gate_x_size() - 1), c[X] + 1))
        {
            for (auto y : iter::range(c[Y], y()))
            {
                switch (auto trace_cell = cell{x, y, GROUND}; get_cell_type(trace_cell))
                {
                    case fcn::NORMAL_CELL:
                    {
                        if (auto pre_cell = cell{x, y - 1, GROUND}; get_cell_type(pre_cell) == fcn::EMPTY_CELL)
                        {
                            cut.push_back(trace_cell);
                            continue;
                        }
                        else
                            return false;
                    }
                    case fcn::EMPTY_CELL:
                    {
                        continue;
                    }
                    default:
                    {
                        return false;
                    }
                }
            }
        }

        for (const auto& _c : cut)
        {
            assign_cell_type(_c, fcn::EMPTY_CELL);
            assign_cell_mode(_c, fcn::cell_mode::NORMAL);
            assign_cell_name(_c, "");
        }

        return true;
    };

    auto merge_cut_pieces = [this](auto c) -> void
    {
        for (auto y : iter::range(y()))
        {
            for (auto x : iter::range(c[X] + 1, x()))
            {
                if (auto source = cell{x, y, GROUND}; !is_free_cell(source))
                {
                    auto target = cell{x - library->gate_x_size(), y, GROUND};
                    // copy cell to new position
                    assign_cell_type(target, get_cell_type(source));
                    assign_cell_mode(target, get_cell_mode(source));
                    assign_cell_name(target, get_cell_name(source));
                    // remove cell from old position
                    assign_cell_type(source, fcn::EMPTY_CELL);
                    assign_cell_mode(source, fcn::cell_mode::NORMAL);
                    assign_cell_name(source, "");
                }
            }
        }
    };

    bool cut_found;
    do
    {
        cut_found = false;
        for (auto&& y : iter::range(y()))
        {
            for (coord_t x = 0u; x < this->x(); ++x)
            {
                if (auto c = find_cut_wire(cell{x, y, GROUND}, false); c)
                {
                    // update x iterator
                    x = (*c)[X];

                    if (is_first_cut(*c) && cut_if_valid(*c))
                    {
                        cut_found = true;
                        merge_cut_pieces(*c);
                        break;
                    }
                }
                else
                {
                    // no need to look more than once if there is no cut wire found
                    break;
                }
            }
        }
    } while (cut_found);
}
