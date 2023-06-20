// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/dependency/detail.hpp"
#include "arg_router/policy/multi_stage_value.hpp"

namespace arg_router::dependency
{
/** Groups child nodes such that any @em one can be used on the command line, but only one.
 *
 * The value_type is a variant of all the policies' value_types that are not derived from
 * policy::no_result_value.  If that list is only a single type, then it collapses into just that
 * type (i.e. it won't be a variant containing a single type).
 * @tparam Params Policies and child node types for the mode
 */
template <typename... Params>
class one_of_t : public detail::basic_one_of_t<AR_STRING("One of: "), Params...>
{
    using parent_type = detail::basic_one_of_t<AR_STRING("One of: "), Params...>;

    static_assert(boost::mp11::mp_none_of<typename parent_type::children_type,
                                          policy::has_multi_stage_value>::value,
                  "one_of children must not use a multi_stage_value policy");

    using variant_type =
        boost::mp11::mp_rename<typename parent_type::basic_value_type, std::variant>;

    static_assert(
        !parent_type::template any_phases_v<variant_type, policy::has_validation_phase_method>,
        "one_of does not support policies with validation phases; as it delegates those to its "
        "children");

public:
    using typename parent_type::children_type;
    using typename parent_type::policies_type;

    /** A variant of the child value_types, or just the value_type if there is only a single child.
     */
    using value_type = std::conditional_t<(std::variant_size_v<variant_type> == 1),
                                          std::variant_alternative_t<0, variant_type>,
                                          variant_type>;

    /** Help data type. */
    template <bool Flatten>
    class help_data_type
    {
    public:
        using label = AR_STRING_SV(parent_type::display_name());
        using description = AR_STRING("");
        using children = typename parent_type::template children_help_data_type<Flatten>::children;

        template <typename OwnerNode, typename FilterFn>
        [[nodiscard]] static vector<runtime_help_data> runtime_children(const OwnerNode& owner,
                                                                        FilterFn&& f)
        {
            return parent_type::template children_help_data_type<Flatten>::runtime_children(
                owner,
                std::forward<FilterFn>(f));
        }
    };

    /** Constructor.
     *
     * @param params Policy and child instances
     */
    constexpr explicit one_of_t(Params... params) noexcept : parent_type{std::move(params)...} {}

    /** Propagates the pre-parse phase to the child, returns on a positive return from one of them.
     *
     * @tparam Validator Validator type
     * @tparam HasTarget True if @a pre_parse_data contains the parent's parse_target
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param pre_parse_data Pre-parse data aggregate
     * @param parents Parent node instances
     * @return Non-empty if the leading tokens in @a args are consumable by this node
     * @exception multi_lang_exception Thrown if any of the child pre-parse implementations have
     * returned an exception, or if called more than once resolving to different children (i.e.
     * a "one of" violation)
     */
    template <typename Validator, bool HasTarget, typename... Parents>
    [[nodiscard]] std::optional<parsing::parse_target> pre_parse(
        parsing::pre_parse_data<Validator, HasTarget> pre_parse_data,
        const Parents&... parents) const
    {
        // We need to wrap the pre_parse_data validator so that we can catch child type mismatches
        // before the caller's validation, otherwise the user can get misleading error messages
        const auto validator = [&](const auto& child, const auto&... parents) {
            using child_type = std::decay_t<decltype(child)>;

            auto type_hash = utility::type_hash<child_type>();
            if (last_typeid_) {
                if (type_hash != *last_typeid_) {
                    throw multi_lang_exception{error_code::one_of_selected_type_mismatch,
                                               parsing::node_token_type<one_of_t>()};
                }
            } else {
                last_typeid_ = type_hash;
            }

            return pre_parse_data.validator()(child, parents...);
        };
        auto wrapper =
            std::optional<parsing::pre_parse_data<std::decay_t<decltype(validator)>, HasTarget>>{};
        if constexpr (pre_parse_data.has_target) {
            wrapper =
                parsing::pre_parse_data{pre_parse_data.args(), pre_parse_data.target(), validator};
        } else {
            wrapper = parsing::pre_parse_data{pre_parse_data.args(), validator};
        }

        auto found = std::optional<parsing::parse_target>{};
        utility::tuple_iterator(
            [&](auto /*i*/, const auto& child) {
                if (!found) {
                    found = child.pre_parse(*wrapper, parents...);
                    if (found) {
                        // There is no requirement to use pre_parse_data validation, so we also
                        // need to manually check here
                        validator(child, parents...);
                    }
                }
            },
            this->children());

        return found;
    }

private:
    mutable std::optional<std::size_t> last_typeid_;
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
}  // namespace arg_router::dependency
