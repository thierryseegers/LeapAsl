#pragma once

#include <algorithm>
#include <memory>
#include <numeric>
#include <string>

// Lifted from https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C.2B.2B
size_t levenshtein_distance(std::string const& left, std::string const& right)
{
    size_t const llength = left.size(), rlength = right.size(), column_start = 1;
    
    auto column = std::make_unique<size_t[]>(llength + 1);
    std::iota(column.get() + column_start, column.get() + llength + 1, column_start);
    
    for(auto x = column_start; x != rlength; x++)
    {
        column[0] = x;
        auto last_diagonal = x - column_start;
        for(auto y = column_start; y <= llength; y++)
        {
            auto const old_diagonal = column[y];
            column[y] = std::min({column[y] + 1, column[y - 1] + 1, last_diagonal + (left[y - 1] == right[x - 1]? 0 : 1)});
            last_diagonal = old_diagonal;
        }
    }

    return column[llength];
}