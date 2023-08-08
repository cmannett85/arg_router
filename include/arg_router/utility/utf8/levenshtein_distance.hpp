// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/parsing/token_type.hpp"
#include "arg_router/utility/tree_recursor.hpp"

#include <numeric>

namespace arg_router::utility::utf8
{
/** Calculates the Levenshtein distance between @a a and @a b.
 *
 * <a href="https://en.wikipedia.org/wiki/Levenshtein_distance">Levenshtein distance</a> gives a
 * measure of similarity between two strings.
 * @param a First string
 * @param b Second string
 * @return 'Distance' metric as an integer
 */
[[nodiscard]] inline std::size_t levenshtein_distance(std::string_view a, std::string_view b)
{
    if (a.size() == 0) {
        return count(b);
    } else if (b.size() == 0) {
        return count(a);
    }

    const auto n = count(b);

    auto costs = vector<std::size_t>(n + 1);
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

/** Uses the Levenshtein distance algorithm to find the closest matching child node to the given
 * token, and it's parents (if any).
 *
 * @note This function requires @a Node to have at least one child
 * @tparam Node Parent node type
 * @param node Parent node instance
 * @param token Token being queried, the prefix type is considered during the distance calculation
 * @return Closest matching child node token_type and any parents, or an empty vector if all
 * available children are runtime disabled
 */
template <typename Node>
[[nodiscard]] vector<parsing::token_type> closest_matching_child_node(const Node& node,
                                                                      parsing::token_type token)
{
    static_assert(std::tuple_size_v<typename Node::children_type> > 0,
                  "Node must have at least one child");

    auto best_token = vector<parsing::token_type>{};
    auto best_score = std::numeric_limits<std::size_t>::max();

    // The token may not have been processed yet, so do a type conversion to be sure
    if (token.prefix == parsing::prefix_type::none) {
        token = parsing::get_token_type(token.name);
    }

    utility::tree_recursor(
        [&](const auto& child, const auto&... parents) {
            // Skip if child is the starting node
            if constexpr (sizeof...(parents) > 0) {
                using child_type = std::decay_t<decltype(child)>;
                using parents_type_without_root =
                    boost::mp11::mp_pop_back<std::tuple<std::decay_t<decltype(parents)>...>>;

                // This monstrosity is because we want to keep the vector initialisation from a
                // fold-expression, whilst removing the root entry which requires converting the
                // pack to a tuple first
                [[maybe_unused]] const auto append_parents = [](parsing::token_type child_token) {
                    return std::apply(
                        [&](auto... parents_without_root) {
                            return vector<parsing::token_type>{
                                child_token,
                                parsing::node_token_type<std::decay_t<
                                    std::remove_pointer_t<decltype(parents_without_root)>>>()...};
                        },
                        boost::mp11::mp_transform<std::add_pointer_t, parents_type_without_root>{});
                };

                // Skip runtime disabled nodes
                if (parsing::is_runtime_disabled(child, parents...)) {
                    return;
                }

                // We have to check if the tested token is not the same as the input token, because
                // sometimes the input token is valid e.g. a value separator is required but one
                // wasn't on the command line.  We only test names for this as the unknown args are
                // always none-named
                if constexpr (traits::has_long_name_method_v<child_type>) {
                    const auto score =
                        utility::utf8::levenshtein_distance(token.name, child.long_name());
                    if (score < best_score) {
                        best_token =
                            append_parents({parsing::prefix_type::long_, child.long_name()});
                        best_score = score;
                    }
                }
                if constexpr (traits::has_short_name_method_v<child_type>) {
                    const auto score =
                        utility::utf8::levenshtein_distance(token.name, child.short_name());
                    if (score < best_score) {
                        best_token =
                            append_parents({parsing::prefix_type::short_, child.short_name()});
                        best_score = score;
                    }
                }
                if constexpr (traits::has_none_name_method_v<child_type>) {
                    const auto score =
                        utility::utf8::levenshtein_distance(token.name, child.none_name());
                    if (score < best_score) {
                        best_token =
                            append_parents({parsing::prefix_type::none, child.none_name()});
                        best_score = score;
                    }
                }
            }
        },
        node);

    return best_token;
}
}  // namespace arg_router::utility::utf8
