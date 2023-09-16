// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/utility/utf8/levenshtein_distance.hpp"

namespace arg_router::utility::utf8
{
std::size_t levenshtein_distance(std::string_view a, std::string_view b) noexcept
{
    if (a.empty()) {
        return count(b);
    }
    if (b.empty()) {
        return count(a);
    }

    const auto n = count(b);

    auto costs = std::vector<std::size_t>(n + 1);
    std::iota(costs.begin(), costs.end(), 0);

    auto i = std::size_t{0};
    for (auto c1 : iterator::range(a)) {
        costs[0] = i + 1;
        auto corner = i;

        auto j = std::size_t{0};
        for (auto c2 : iterator::range(b)) {
            const auto upper = costs[j + 1];
            if (c1 == c2) {
                costs[j + 1] = corner;
            } else {
                auto t = std::min(upper, corner);
                costs[j + 1] = std::min(costs[j], t) + 1;
            }

            corner = upper;
            ++j;
        }
        ++i;
    }

    return costs[n];
}
}  // namespace arg_router::utility::utf8
