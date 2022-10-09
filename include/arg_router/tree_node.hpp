// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/config.hpp"
#include "arg_router/list.hpp"
#include "arg_router/parsing/global_parser.hpp"
#include "arg_router/parsing/parsing.hpp"
#include "arg_router/parsing/pre_parse_data.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/policy.hpp"
#include "arg_router/utility/compile_time_string.hpp"
#include "arg_router/utility/tuple_iterator.hpp"

namespace arg_router
{
/** Base type of all nodes of a parse tree, not including policies.
 *
 * @tparam Params Parameter types
 */
template <typename... Params>
class tree_node :
    public traits::unpack_and_derive<  //
        boost::mp11::mp_filter<policy::is_policy, std::tuple<std::decay_t<Params>...>>>
{
    template <typename LHS, typename RHS>
    struct priority_order {
        template <typename T>
        struct get_priority_or_default {
            constexpr static std::size_t value = []() {
                if constexpr (policy::has_priority_v<T>) {
                    return T::priority;
                } else {
                    return 0;
                }
            }();
        };

        constexpr static auto value =
            get_priority_or_default<LHS>::value > get_priority_or_default<RHS>::value;
    };

public:
    /** The policies and child node types. */
    using parameters_type = std::tuple<std::decay_t<Params>...>;

    /** A tuple of all the policy types in parameters_type. */
    using policies_type = boost::mp11::mp_filter<policy::is_policy, parameters_type>;

    /** policies_type ordered by priority */
    using priority_ordered_policies_type = boost::mp11::mp_sort<policies_type, priority_order>;

    /** A tuple of all the child tree node types in parameters_type, with any lists expanded. */
    using children_type = boost::mp11::mp_filter<
        is_tree_node,
        std::decay_t<decltype(list_expander(std::declval<std::decay_t<Params>>()...))>>;

    static_assert(
        (std::tuple_size_v<children_type> + std::tuple_size_v<policies_type>) ==
            std::tuple_size_v<
                std::decay_t<decltype(list_expander(std::declval<std::decay_t<Params>>()...))>>,
        "tree_node constructor can only accept other tree_nodes and policies");

    /** Evaluates to true if this node has a name token that appears on the command line. */
    constexpr static auto is_named = traits::has_short_name_method_v<tree_node> ||
                                     traits::has_long_name_method_v<tree_node> ||
                                     traits::has_none_name_method_v<tree_node>;

    /** Finds a policy that the @a PolicyChecker predicate passes.
     *
     * @tparam PolicyChecker Policy predicate, use one of <TT>policy::has_*_phase_method</TT>
     * @tparam Args Auxiliary arguments to pass to @a PolicyChecker
     */
    template <template <typename...> typename PolicyChecker, typename... Args>
    struct phase_finder {
        using finder = boost::mp11::mp_bind<PolicyChecker, boost::mp11::_1, Args...>;

        /** The policy type that passes @a PolicyChecker, or void if one is not found. */
        using type =
            boost::mp11::mp_eval_if_c<boost::mp11::mp_find_if_q<policies_type, finder>::value ==
                                          std::tuple_size_v<policies_type>,
                                      void,
                                      boost::mp11::mp_at,
                                      policies_type,
                                      boost::mp11::mp_find_if_q<policies_type, finder>>;
    };

    /** Helper alias for phase_finder.
     *
     * @tparam PolicyChecker Policy predicate, use one of <TT>policy::has_*_phase_method</TT>
     * @tparam Args Auxiliary arguments to pass to @a PolicyChecker
     */
    template <template <typename...> typename PolicyChecker, typename... Args>
    using phase_finder_t = typename phase_finder<PolicyChecker, Args...>::type;

    /** Evaluates to true if any of the @a PolicyCheckers predicates pass.
     *
     * This is effectively a wrapper over phase_finder that loops over multiple policy checkers.
     *
     * Most nodes don't support certain phases, this predicate allows for <TT>static_assert</TT>
     * testing for their presence.
     * @tparam ValueType Some policy checkers require a value type template parameter, if a
     * particular policy doesn't need it then it is not used
     * @tparam PolicyCheckers Parameter pack type of policy predicates, use
     * <TT>policy::has_*_phase_method</TT>
     */
    template <typename ValueType, template <typename...> typename... PolicyCheckers>
    class any_phases
    {
        template <template <typename...> typename PolicyChecker>
        struct checker {
            constexpr static bool value = []() {
                // This is shonky but I can't think of an easy arity test for a class template
                if constexpr (std::tuple_size_v<policies_type> == 0) {
                    return false;
                } else if constexpr (boost::mp11::mp_valid<PolicyChecker,
                                                           boost::mp11::mp_first<policies_type>,
                                                           ValueType>::value) {
                    return !std::is_void_v<phase_finder_t<PolicyChecker, ValueType>>;
                } else {
                    return !std::is_void_v<phase_finder_t<PolicyChecker>>;
                }
            }();
        };

    public:
        constexpr static bool value = boost::mp11::mp_any_of<std::tuple<checker<PolicyCheckers>...>,
                                                             boost::mp11::mp_to_bool>::value;
    };

    /** Helper alias for has_phases.
     *
     * @tparam ValueType Some policy checkers require a value type template parameter, if a
     * particular policy doesn't need it then it is not used
     * @tparam PolicyCheckers Parameter pack type of policy predicates, use
     * <TT>policy::has_*_phase_method</TT>
     */
    template <typename ValueType, template <typename...> typename... PolicyCheckers>
    constexpr static bool any_phases_v = any_phases<ValueType, PolicyCheckers...>::value;

    /** Returns a reference to the children.
     *
     * @return Children
     */
    [[nodiscard]] children_type& children() noexcept { return children_; }

    /** Const overload.
     *
     * @return Children
     */
    [[nodiscard]] constexpr const children_type& children() const noexcept { return children_; }

protected:
    /** Provides a default implementation for leaf nodes (e.g. flag and arg), and some useful
     * functions/types for other types.
     *
     * @tparam Flatten True if the help output is to be flattened, unused for this
     */
    template <bool Flatten>
    class default_leaf_help_data_type
    {
        template <typename Child>
        using child_help_getter = typename Child::template help_data_type<Flatten>;

        template <typename Child>
        using child_help = boost::mp11::
            mp_eval_if_c<!traits::has_help_data_type_v<Child>, void, child_help_getter, Child>;

        constexpr static auto num_policies = std::tuple_size_v<policies_type>;

    public:
        /** @return Minimum and/or maximum value suffix string
         */
        [[nodiscard]] constexpr static auto value_suffix() noexcept
        {
            if constexpr (traits::has_minimum_value_method_v<tree_node> ||
                          traits::has_maximum_value_method_v<tree_node>) {
                constexpr auto prefix = S_("<"){};

                constexpr auto min_value = []() {
                    if constexpr (traits::has_minimum_value_method_v<tree_node>) {
                        constexpr auto value = tree_node::minimum_value();
                        return utility::convert_integral_to_cts_t<
                            static_cast<traits::underlying_type_t<decltype(value)>>(value)>{};
                    } else {
                        // If we have got this far, then we must a maximum value, we can use that
                        // type to determine if the lowest bound is 0 or -N depending on whether or
                        // not the type is signed
                        using max_value_type =
                            traits::underlying_type_t<decltype(tree_node::maximum_value())>;
                        if constexpr (std::is_unsigned_v<max_value_type>) {
                            return S_("0"){};
                        } else {
                            return S_("-N"){};
                        }
                    }
                }();

                constexpr auto max_value = []() {
                    if constexpr (traits::has_maximum_value_method_v<tree_node>) {
                        constexpr auto value = tree_node::maximum_value();
                        if constexpr (std::is_enum_v<std::decay_t<decltype(value)>>) {
                            return utility::convert_integral_to_cts_t<
                                static_cast<std::underlying_type_t<decltype(value)>>(value)>{};
                        } else {
                            return utility::convert_integral_to_cts_t<value>{};
                        }
                    } else {
                        return S_("N"){};
                    }
                }();

                return prefix + min_value + S_("-"){} + max_value + S_(">"){};
            } else {
                return S_(""){};
            }
        }

        /** @return Textual representation of a value suffix suitable for help output, or an empty
         *  string if no value separator policy is attached to the node
         */
        [[nodiscard]] constexpr static auto value_separator_suffix()
        {
            [[maybe_unused]] constexpr bool fixed_count_of_one = []() {
                if constexpr (traits::has_minimum_count_method_v<tree_node> &&
                              traits::has_maximum_count_method_v<tree_node>) {
                    return (tree_node::minimum_count() == tree_node::maximum_count()) &&
                           (tree_node::minimum_count() == 1);
                }

                return false;
            }();

            [[maybe_unused]] constexpr auto value_str = []() {
                constexpr auto min_max_str = value_suffix();
                if constexpr (std::is_same_v<std::decay_t<decltype(min_max_str)>, S_("")>) {
                    return S_("<Value>"){};
                } else {
                    return min_max_str;
                }
            }();

            [[maybe_unused]] constexpr auto separator_index =
                boost::mp11::mp_find_if<policies_type, traits::has_value_separator_method>::value;

            if constexpr (separator_index != num_policies) {
                constexpr auto value_separator =
                    std::tuple_element_t<separator_index, policies_type>::value_separator();

                return S_(value_separator){} + value_str;
            } else if constexpr (fixed_count_of_one) {
                return S_(" "){} + value_str;
            } else {
                return S_(""){};
            }
        }

        /** @return Long and short name label(s) for node with value suffix if present, or empty
         *  string if not present
         */
        [[nodiscard]] constexpr static auto label_generator() noexcept
        {
            [[maybe_unused]] constexpr auto long_name_index =
                boost::mp11::mp_find_if<policies_type, traits::has_long_name_method>::value;
            [[maybe_unused]] constexpr auto short_name_index =
                boost::mp11::mp_find_if<policies_type, traits::has_short_name_method>::value;

            if constexpr ((long_name_index != num_policies) && (short_name_index != num_policies)) {
                constexpr auto long_name =
                    std::tuple_element_t<long_name_index, policies_type>::long_name();
                constexpr auto short_name =
                    std::tuple_element_t<short_name_index, policies_type>::short_name();

                return S_(config::long_prefix){} + S_(long_name){} + S_(","){} +  //
                       S_(config::short_prefix){} + S_(short_name){} + value_separator_suffix();
            } else if constexpr (long_name_index != num_policies) {
                constexpr auto long_name =
                    std::tuple_element_t<long_name_index, policies_type>::long_name();

                return S_(config::long_prefix){} + S_(long_name){} + value_separator_suffix();
            } else if constexpr (short_name_index != num_policies) {
                constexpr auto short_name =
                    std::tuple_element_t<short_name_index, policies_type>::short_name();

                return S_(config::short_prefix){} + S_(short_name){} + value_separator_suffix();
            } else {
                return S_(""){};
            }
        }

        /** @return Description text for string, or empty string if providing policy not present
         */
        [[nodiscard]] constexpr static auto description_generator() noexcept
        {
            constexpr auto description_index =
                boost::mp11::mp_find_if<policies_type, traits::has_description_method>::value;

            if constexpr (description_index != num_policies) {
                return S_(
                    (std::tuple_element_t<description_index, policies_type>::description())){};
            } else {
                return S_(""){};
            }
        }

        /** @return Minimum and/or maximum count suffix string
         */
        [[nodiscard]] constexpr static auto count_suffix() noexcept
        {
            constexpr bool fixed_count = []() {
                if constexpr (traits::has_minimum_count_method_v<tree_node> &&
                              traits::has_maximum_count_method_v<tree_node>) {
                    return tree_node::minimum_count() == tree_node::maximum_count();
                }

                return false;
            }();

            constexpr auto prefix = S_("["){};

            if constexpr (fixed_count) {
                return prefix + utility::convert_integral_to_cts_t<tree_node::minimum_count()>{} +
                       S_("]"){};
            } else {
                constexpr auto min_count = []() {
                    if constexpr (traits::has_minimum_count_method_v<tree_node>) {
                        return utility::convert_integral_to_cts_t<tree_node::minimum_count()>{};
                    } else {
                        return S_("0"){};
                    }
                }();

                constexpr auto max_count = []() {
                    if constexpr (traits::has_maximum_count_method_v<tree_node>) {
                        if constexpr (tree_node::maximum_count() ==
                                      decltype(policy::min_count<0>)::maximum_count()) {
                            return S_("N"){};
                        } else {
                            return utility::convert_integral_to_cts_t<tree_node::maximum_count()>{};
                        }
                    } else {
                        return S_("N"){};
                    }
                }();

                return prefix + min_count + S_(","){} + max_count + S_("]"){};
            }
        }

        /** Collects the help data from all the children that have a help_data_type. */
        using all_children_help =
            boost::mp11::mp_remove_if<boost::mp11::mp_transform<child_help, children_type>,
                                      std::is_void>;

        /** Label compile time string.
         *
         * Evaluates to a string of the form "<short name>,<long_name>", or only of them if only one
         * available, or an empty string if none are available.
         */
        using label = std::decay_t<decltype(label_generator())>;

        /** Description compile time string.
         *
         * Evaluates to a the description time, or an empty string if none is available.
         */
        using description = std::decay_t<decltype(description_generator())>;

        /** Children help data tuple.
         *
         * Empty as this is the default for leaf types.
         */
        using children = std::tuple<>;
    };

    /** Constructor.
     *
     * @param params Policy and child instances
     */
    constexpr explicit tree_node(Params... params) noexcept :
        traits::unpack_and_derive<policies_type>{common_filter<policy::is_policy>(params...)},
        children_(common_filter_tuple<is_tree_node>(list_expander(std::move(params)...)))
    {
    }

    /** Default pre-parse implementation.
     *
     * This implementation simply iterates over the pre-parse phase implementing policies, and uses
     * the results to update @a args and generate a parse_target.
     *
     * The @a validator is called just before @a args is updated, and allows the caller to run a
     * custom verification the on the @a node and @a parents. If the validator returns true then the
     * result is kept, otherwise an empty result is returned from this method.
     *
     * @note The implementation does not prepend <TT>*this</TT> into @a parents, so derived types
     * are expected to reimplement this so @ node is the correct type (remember there are no virtual
     * methods in this library)
     * @tparam Validator Validator type
     * @tparam HasTarget True if @a pre_parse_data contains the parent's parse_target
     * @tparam Node This node's derived type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param pre_parse_data Pre-parse data aggregate
     * @param node This node instance
     * @param parents Parent node instances
     * @return Non-empty if the leading tokens in @a args are consumable by this node
     * @exception parse_exception Thrown if any of the policies' pre-parse implementations have
     * returned an exception
     */
    template <typename Validator, bool HasTarget, typename Node, typename... Parents>
    [[nodiscard]] std::optional<parsing::parse_target> pre_parse(
        parsing::pre_parse_data<Validator, HasTarget> pre_parse_data,
        const Node& node,
        const Parents&... parents) const
    {
        auto result = vector<parsing::token_type>{};
        auto tmp_args = pre_parse_data.args();
        auto adapter = parsing::dynamic_token_adapter{result, tmp_args};

        // At this stage, the target is only for collecting sub-targets
        auto target = parsing::parse_target{node, parents...};

        auto match = parsing::pre_parse_result{parsing::pre_parse_action::valid_node};
        utility::tuple_type_iterator<priority_ordered_policies_type>([&](auto i) {
            using policy_type = std::tuple_element_t<i, priority_ordered_policies_type>;
            if constexpr (policy::has_pre_parse_phase_method_v<policy_type>) {
                if (match == parsing::pre_parse_action::skip_node) {
                    return;
                }

                // Keep the skip_node_but_use_sub_targets state if later policies return valid_node
                const auto use_subs =
                    match == parsing::pre_parse_action::skip_node_but_use_sub_targets;
                match = this->policy_type::pre_parse_phase(adapter,
                                                           extract_parent_target(pre_parse_data),
                                                           target,
                                                           node,
                                                           parents...);
                if (use_subs && (match == parsing::pre_parse_action::valid_node)) {
                    match = parsing::pre_parse_action::skip_node_but_use_sub_targets;
                }
            }
        });

        // If we have a result and it is false, then exit early.  We need to wait until after name
        // checking is (possibly) performed to throw an exception, otherwise a parse-stopping
        // exception could be thrown when the target of the token isn't even this node
        if (match == parsing::pre_parse_action::skip_node) {
            return {};
        }

        if constexpr (is_named) {
            // Check the first token against the names (if the node is named of course)
            if (match != parsing::pre_parse_action::skip_node_but_use_sub_targets) {
                // If the node is named but there are no tokens, then take the first from args.
                // If args is empty, then return false
                if (result.empty()) {
                    if (tmp_args.empty()) {
                        return {};
                    }
                    result.push_back(tmp_args.front());
                    tmp_args.erase(tmp_args.begin());
                }

                // The first token may not have been processed, so convert
                auto& first_token = result.front();
                if (first_token.prefix == parsing::prefix_type::none) {
                    first_token = parsing::get_token_type(first_token.name);
                }

                // And then test it is correct unless we are skipping
                if (!parsing::match<tree_node>(first_token)) {
                    return {};
                }
            }
        }

        // If the policy checking returned an exception, now is the time to throw it
        match.throw_exception();

        // If there are no processed tokens and no sub-targets then this node cannot do anything and
        // therefore should not be used
        if (result.empty() && target.sub_targets().empty()) {
            return {};
        }

        if (!pre_parse_data.validator()(node, parents...)) {
            return {};
        }

        // Update the unprocessed args
        pre_parse_data.args() = tmp_args;

        // Update the target with the pre-parsed tokens.  Remove the label token if present
        if (is_named && !result.empty()) {
            result.erase(result.begin());
        }
        target.tokens(std::move(result));

        return target;
    }

    /** Generic parse call, uses a policy that supports the parse phase if present, or the global
     * parser.
     *
     * @tparam ValueType Parsed type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param token Token to parse
     * @param parents Parents instances pack
     * @return Parsed result
     * @exception parse_exception Thrown if parsing failed
     */
    template <typename ValueType, typename... Parents>
    [[nodiscard]] auto parse(std::string_view token, const Parents&... parents) const
    {
        using finder_type = phase_finder<policy::has_parse_phase_method, ValueType>;
        using policy_type = typename finder_type::type;

        static_assert((boost::mp11::mp_count_if_q<policies_type,
                                                  typename finder_type::finder>::value  //
                       <= 1),
                      "Only zero or one policies supporting a parse phase is supported");
        static_assert(
            (boost::mp11::mp_count_if_q<policies_type,
                                        typename phase_finder<policy::has_missing_phase_method,
                                                              ValueType>::finder>::value <= 1),
            "Only zero or one policies supporting a missing phase is "
            "supported");

        if constexpr (std::is_void_v<policy_type>) {
            return parser<ValueType>::parse(token);
        } else {
            return this->policy_type::template parse_phase<ValueType>(token, parents...);
        }
    }

private:
    static_assert((boost::mp11::mp_count_if_q<
                       policies_type,
                       typename phase_finder<policy::has_routing_phase_method>::finder>::value <=
                   1),
                  "Only zero or one policies supporting a routing phase is supported");

    template <template <typename...> typename Fn, typename... ExpandedParams>
    [[nodiscard]] constexpr static auto common_filter(ExpandedParams&... expanded_params) noexcept
    {
        // Send references to the filter method so we don't copy anything unnecessarily
        using ref_tuple = std::tuple<std::reference_wrapper<ExpandedParams>...>;

        // We have to wrap Fn because we're now giving it a std::reference_wrapper
        using ref_fn =
            boost::mp11::mp_bind<Fn, boost::mp11::mp_bind<traits::get_type, boost::mp11::_1>>;

        // We have our result tuple, but it only contains references so we now have to move
        // construct them into the 'real' types
        auto ref_result = algorithm::tuple_filter_and_construct<ref_fn::template fn>(
            ref_tuple{expanded_params...});

        using ref_result_t = std::decay_t<decltype(ref_result)>;
        using result_t = boost::mp11::mp_transform<traits::get_type, ref_result_t>;

        // Converting move-constructor of std::tuple
        return result_t{std::move(ref_result)};
    }

    template <template <typename...> typename Fn, typename Tuple>
    [[nodiscard]] constexpr static auto common_filter_tuple(Tuple&& tuple_params) noexcept
    {
        // std::apply does not like templates, so we have to wrap in a lambda
        return std::apply(
            [](auto&... args) { return common_filter<Fn>(std::forward<decltype(args)>(args)...); },
            tuple_params);
    }

    template <typename Validator, bool HasTarget>
    [[nodiscard]] constexpr auto extract_parent_target(
        [[maybe_unused]] parsing::pre_parse_data<Validator, HasTarget> pre_parse_data)
        const noexcept
    {
        if constexpr (HasTarget) {
            return utility::compile_time_optional{std::cref(pre_parse_data.target())};
        } else {
            return utility::compile_time_optional{};
        }
    }

    children_type children_;
};
}  // namespace arg_router
