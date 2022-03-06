/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/dependency/detail.hpp"
#include "arg_router/policy/multi_stage_value.hpp"

namespace arg_router
{
/** The dependency namespace carries nodes and policies that define dependency
 * relationships between other nodes.
 */
namespace dependency
{
/** Groups child nodes such that any @em one can be used on the command line.
 *
 * The value_type is a variant of all the policies' value_types that are not
 * derived from policy::no_result_value.  If that list is only a single type,
 * then it collapses into just that type (i.e. it won't be a variant containing
 * a single type).
 * @tparam Params Policies and child node types for the mode
 */
template <typename... Params>
class one_of_t : public detail::basic_one_of_t<S_("One of: "), Params...>
{
    using parent_type = detail::basic_one_of_t<S_("One of: "), Params...>;

    static_assert(boost::mp11::mp_none_of<typename parent_type::children_type,
                                          policy::has_multi_stage_value>::value,
                  "one_of children must not use a multi_stage_value policy");

    using variant_type =
        boost::mp11::mp_rename<typename parent_type::basic_value_type,
                               std::variant>;

    static_assert(!parent_type::template any_phases_v<
                      variant_type,
                      policy::has_validation_phase_method>,
                  "one_of does not support policies with validation phases; as "
                  "it delegates those to its children");

public:
    using typename parent_type::children_type;
    using typename parent_type::policies_type;

    /** A variant of the child value_types, or just the value_type if there is
     * only a single child.
     */
    using value_type =
        std::conditional_t<(std::variant_size_v<variant_type> == 1),
                           std::variant_alternative_t<0, variant_type>,
                           variant_type>;

    /** Help data type. */
    template <bool Flatten>
    class help_data_type
    {
    public:
        using label = S_("One of:");
        using description = S_("");
        using children = typename parent_type::template  //
            children_help_data_type<Flatten>::children;
    };

    /** Constructor.
     *
     * @param params Policy and child instances
     */
    constexpr explicit one_of_t(Params... params) noexcept :
        parent_type{std::move(params)...}
    {
    }

    /** Returns true and calls @a visitor if any of the children's match methods
     * return true.
     * 
     * @a visitor needs to be equivalent to:
     * @code
     * [](const auto& node) { ... }
     * @endcode
     * <TT>node</TT> will be a reference to the first matching child.
     * @tparam Fn Visitor type
     * @param token Command line token to match
     * @param visitor Visitor instance
     * @return Match result
     */
    template <typename Fn>
    constexpr bool match(const parsing::token_type& token,
                         const Fn& visitor) const
    {
        auto found = false;
        utility::tuple_iterator(
            [&](auto /*i*/, const auto& child) {
                if (!found) {
                    found = child.match(token, visitor);
                }
            },
            this->children());

        return found;
    }
};

/** Constructs a one_of_t with the given policies and children.
 *
 * This is used for similarity with arg_t.
 * @tparam Params Policies and child node types for the mode
 * @param params Pack of policy and child node instances
 * @return Instance
 */
template <typename... Params>
[[nodiscard]] constexpr auto one_of(Params... params) noexcept
{
    return one_of_t{std::move(params)...};
}
}  // namespace dependency
}  // namespace arg_router
