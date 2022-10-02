/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/dependency/detail.hpp"
#include "arg_router/policy/multi_stage_value.hpp"

/** The dependency namespace carries nodes and policies that define dependency relationships between
 * other nodes.
 */
namespace arg_router::dependency
{
/** Groups child nodes so they all become aliases of a single output.
 *
 * policy::alias_t defines @em input aliases, it works by duplicating the input tokens for the node
 * for each of the aliased nodes.  An implication of this is that all the aliased nodes need to have
 * the same count and be able to parse the same input tokens, and each aliased node has an entry
 * (but not the node the alias policy is attached to!) in the router arguments.
 *
 * alias_group_t is almost the opposite of this; it defines output aliases where each child of the
 * group independently parses the tokens it matches to (in the same way one_of_t does).  However
 * unlike policy::alias_t, there is only a single entry in the router arguments for the whole group,
 * as such there is a compile-time check that all <TT>value_type</TT> types are the same (or ignored
 * if policy::no_result_value is used).  In other words, all the children of the group represent the
 * same output.
 *
 * You can think of policy::alias_t as defining a one-to-many alias, whilst alias_group_t is a
 * many-to-one.
 * @tparam Params Policies and child node types for the mode
 */
template <typename... Params>
class alias_group_t : public detail::basic_one_of_t<S_("Alias Group: "), Params...>
{
    using parent_type = detail::basic_one_of_t<S_("Alias Group: "), Params...>;

public:
    using typename parent_type::children_type;
    using typename parent_type::policies_type;

    /** The common output type of the all the children that support it. */
    using value_type = boost::mp11::mp_first<typename parent_type::basic_value_type>;

    static_assert(
        std::tuple_size_v<boost::mp11::mp_unique<typename parent_type::basic_value_type>> == 1,
        "All children of alias_group must have the same value_type, or use "
        "policy::no_result_value");

private:
    template <typename Child>
    struct multi_stage_with_result_and_validation {
        constexpr static auto value =
            policy::has_multi_stage_value_v<Child> && !policy::has_no_result_value_v<Child> &&
            Child::template any_phases_v<value_type, policy::has_validation_phase_method>;
    };

public:
    // If any of the children are multi_stage_values, not no_result_value, and have a validation
    // phase; then fail as the policy that implements needs to be moved into this node otherwise
    // they won't be called
    static_assert(
        boost::mp11::mp_none_of<children_type, multi_stage_with_result_and_validation>::value,
        "Multi-stage value supporting alias_group children (e.g. "
        "counting_flag) cannot have a validation phase as they won't be "
        "executed, move the implementing policies into the alias_group");

    /** Help data type. */
    template <bool Flatten>
    class help_data_type
    {
    public:
        using label =
            decltype(S_("Alias Group: "){} +
                     parent_type::template default_leaf_help_data_type<Flatten>::value_suffix());
        using description = S_("");
        using children = typename parent_type::template  //
            children_help_data_type<Flatten>::children;
    };

    /** Constructor.
     *
     * @param params Policy and child instances
     */
    constexpr explicit alias_group_t(Params... params) noexcept : parent_type{std::move(params)...}
    {
    }

    /** Propagates the pre-parse phase to the child, returns on a positive return from one of them.
     *
     * @tparam Validator Validator type\
     * @tparam HasTarget True if @a pre_parse_data contains the parent's
     * parse_target
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param pre_parse_data Pre-parse data aggregate
     * @param parents Parent node instances
     * @return Non-empty if the leading tokens in @a args are consumable by this
     * node
     * @exception parse_exception Thrown if any of the child pre-parse
     * implementations have returned an exception
     */
    template <typename Validator, bool HasTarget, typename... Parents>
    [[nodiscard]] std::optional<parsing::parse_target> pre_parse(
        parsing::pre_parse_data<Validator, HasTarget> pre_parse_data,
        const Parents&... parents) const
    {
        auto found = std::optional<parsing::parse_target>{};
        utility::tuple_iterator(
            [&](auto /*i*/, const auto& child) {
                if (!found) {
                    found = child.pre_parse(pre_parse_data, parents...);
                }
            },
            this->children());

        return found;
    }
};

/** Constructs a alias_group_t with the given policies and children.
 *
 * This is used for similarity with arg_t.
 * @tparam Params Policies and child node types for the mode
 * @param params Pack of policy and child node instances
 * @return Instance
 */
template <typename... Params>
[[nodiscard]] constexpr auto alias_group(Params... params) noexcept
{
    return alias_group_t{std::move(params)...};
}
}  // namespace arg_router::dependency
