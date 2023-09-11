// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/parsing/token_type.hpp"
#include "arg_router/utility/compile_time_string.hpp"
#include "arg_router/utility/string_view_ops.hpp"
#include "arg_router/utility/tuple_iterator.hpp"

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/list.hpp>

namespace arg_router::utility
{
/** An incredibly simple and dumb formatter specifically for translated exception messages.
 *
 * Only two placeholders are supported:
 * @code
 * const auto str = exception_formatter<str<"Hello {}!">>::format({
 *     {prefix_type::long_, "world"}
 * });
 * // str == "Hello --world!"
 * @endcode
 * An empty bracket pair in the placeholder for a single token type.
 * @code
 * const auto str1 = exception_formatter<str<"Hello {}, {, }, d">>::format({
 *     {prefix_type::short_, "a"},
 *     {prefix_type::long_, "b"},
 *     {prefix_type::none, "c"}
 * });
 * // str1 == "Hello -a, --b, c, d"
 *
 * const auto str2 = exception_formatter<str<"Hello {/}">>::format({
 *     {prefix_type::short_, "a"},
 *     {prefix_type::long_, "b"},
 *     {prefix_type::none, "c"}
 * });
 * // str2 == "Hello -a/--b/c"
 * @endcode
 * Whilst a bracket pair with at least one character in will greedily consume all remaining tokens,
 * usng the string in the brace pair as the joining string for the remaining tokens.  The brackets
 * are checked at compile-time so that there is a maximum of only one greedy placeholder
 * per string, and that it comes @em after the single token placeholders (if any).
 *
 * If there are more tokens than placeholders, and none of the placeholders are greedy, then the
 * remaining tokens are ignored.  If there are less tokens, then the placeholders are replaced with
 * empty strings.
 *
 * @tparam S Compile-time string type
 */
template <typename S>
class exception_formatter
{
    template <std::size_t Start, typename J>
    struct placeholder {
        static constexpr std::size_t start = Start;
        using joining = J;
    };

    template <typename Str, typename PHs, std::size_t Current>
    [[nodiscard]] static consteval auto placeholders_impl() noexcept
    {
        constexpr auto start = Str::get().find('{', Current);
        constexpr auto end = Str::get().find('}', Current + 1);

        if constexpr ((start != std::string_view::npos) && (end != std::string_view::npos)) {
            return placeholders_impl<
                Str,
                boost::mp11::mp_push_back<
                    PHs,
                    placeholder<start, AR_STR_SV(Str::get().substr(start + 1, end - start - 1))>>,
                end + 1>();
        } else {
            return PHs{};
        }
    }

    template <typename PH>
    struct is_greedy {
        static constexpr bool value = PH::joining::size() > 0;
    };

    template <typename Str>
    using generate_placeholders = std::decay_t<decltype(placeholders_impl<Str, std::tuple<>, 0>())>;

    using initial_placeholders = generate_placeholders<S>;

public:
    /** Format the string using @a tokens and rules in the description.
     *
     * @param tokens Tokens to use
     * @return Formatted string
     */
    [[nodiscard]] static std::string format(const std::vector<parsing::token_type>& tokens)
    {
        if constexpr ((std::tuple_size_v<initial_placeholders>) > 0) {
            return fmt<S>(tokens);
        } else if (!tokens.empty()) {
            using greedy_appended = typename S::template append_t<str<": {, }">>;
            return fmt<greedy_appended>(tokens);
        }

        return std::string{S::get()};
    }

private:
    // Make sure there's only one greedy_token_placeholder in the string at most, and it's at the
    // end
    [[nodiscard]] static consteval bool placeholder_check() noexcept
    {
        if constexpr ((std::tuple_size_v<initial_placeholders>) > 1) {
            constexpr auto greedy_count =
                boost::mp11::mp_count_if<initial_placeholders, is_greedy>::value;
            static_assert(greedy_count <= 1,
                          "Can only be one greedy entry in the formatted string");

            if constexpr (greedy_count == 1) {
                return is_greedy<boost::mp11::mp_back<initial_placeholders>>::value;
            }
        }

        return true;
    }
    static_assert(placeholder_check(), "Greedy entry must be last in the formatted string");

    template <typename Str>
    [[nodiscard]] static std::string fmt(const std::vector<parsing::token_type>& tokens)
    {
        using std::to_string;
        using placeholders = generate_placeholders<Str>;
        using namespace utility::string_view_ops;

        constexpr auto bracket_width = std::size_t{2};

        auto str = std::string{Str::get()};

        // The placeholders positions need to take into account that previous placeholder's tokens
        // may be different with than the placeholders, so the start position needs to be shifted
        // by the rolling sum of the previous token widths (minus placeholder brackets width)
        auto offset = std::size_t{0};

        auto it = tokens.begin();
        utility::tuple_type_iterator<placeholders>([&](auto i) {
            using PH = std::tuple_element_t<i, placeholders>;

            // Wouldn't normally create a local from static data, but this is necessary due to
            // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=92654
            constexpr auto joining_str = PH::joining::get();
            if (it != tokens.end()) {
                auto next_str = to_string(*it);
                auto pos = PH::start + offset;
                str.replace(pos, joining_str.size() + bracket_width, next_str);
                ++it;

                offset += next_str.size() - bracket_width;

                if constexpr (!joining_str.empty()) {
                    // Greedily consume the remaining tokens
                    pos += next_str.size();
                    for (; it != std::end(tokens); ++it) {
                        next_str = joining_str + to_string(*it);
                        str.insert(pos, next_str);
                        pos += next_str.size();
                    }
                }
            } else {
                // Replace any remaining placeholders with empty strings
                str.replace(PH::start + offset, joining_str.size() + bracket_width, "");
                offset -= bracket_width;
            }
        });

        return str;
    }
};
}  // namespace arg_router::utility
