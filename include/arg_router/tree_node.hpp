// Copyright (C) 2022-2023 by Camden Mannett.
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
     * @exception multi_lang_exception Thrown if any of the policies' pre-parse implementations have
     * returned an exception
     */
    template <typename Validator, bool HasTarget, typename Node, typename... Parents>
    [[nodiscard]] std::optional<parsing::parse_target> pre_parse(
        parsing::pre_parse_data<Validator, HasTarget> pre_parse_data,
        const Node& node,
        const Parents&... parents) const
    {
        return std::apply(
            [&](auto&&... ancestors) { return pre_parse_impl(pre_parse_data, ancestors.get()...); },
            parsing::clean_node_ancestry_list(node, parents...));
    }

    /** Generic parse call, uses a policy that supports the parse phase if present, or the global
     * parser.
     *
     * @tparam ValueType Parsed type
     * @tparam Node This node's derived type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param token Token to parse
     * @param node This node instance
     * @param parents Parents instances pack
     * @return Parsed result
     * @exception multi_lang_exception Thrown if parsing failed
     */
    template <typename ValueType, typename Node, typename... Parents>
    [[nodiscard]] auto parse(std::string_view token,
                             const Node& node,
                             const Parents&... parents) const
    {
        return std::apply(
            [&](auto&&... ancestors) { return parse_impl<ValueType>(token, ancestors.get()...); },
            parsing::clean_node_ancestry_list(node, parents...));
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

    template <typename Validator, bool HasTarget, typename Node, typename... Parents>
    [[nodiscard]] std::optional<parsing::parse_target> pre_parse_impl(
        parsing::pre_parse_data<Validator, HasTarget> pre_parse_data,
        const Node& node,
        const Parents&... parents) const
    {
        auto result = std::vector<parsing::token_type>{};
        auto tmp_args = pre_parse_data.args();
        auto adapter = parsing::dynamic_token_adapter{result, tmp_args};

        // At this stage, the target is only for collecting sub-targets
        auto target = parsing::parse_target{node, parents...};

        auto match = parsing::pre_parse_result{parsing::pre_parse_action::valid_node};
        utility::tuple_type_iterator<priority_ordered_policies_type>([&](auto i) {
            using policy_type = std::tuple_element_t<i, priority_ordered_policies_type>;
            if constexpr (policy::has_pre_parse_phase_method_v<policy_type>) {
                if (!match || (match == parsing::pre_parse_action::skip_node)) {
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
                    first_token = parsing::get_token_type(*this, first_token.name);
                }

                // And then test it is correct unless we are skipping
                if (!parsing::match<tree_node>(first_token)) {
                    return {};
                }
            }
        }

        // Exit early if the caller doesn't want this node
        if (!pre_parse_data.validator()(node, parents...)) {
            return {};
        }

        // If the policy checking returned an exception, now is the time to throw it
        match.throw_exception();

        // Update the unprocessed args
        pre_parse_data.args() = tmp_args;

        // Update the target with the pre-parsed tokens.  Remove the label token if present
        if constexpr (is_named) {
            if (!result.empty()) {
                result.erase(result.begin());
            }
        }
        target.tokens(std::move(result));

        return target;
    }

    template <typename ValueType, typename... Parents>
    [[nodiscard]] auto parse_impl(std::string_view token, const Parents&... parents) const
    {
        using finder_type = phase_finder<policy::has_parse_phase_method, ValueType>;
        using policy_type = typename finder_type::type;

#ifdef MSVC_1936_WORKAROUND
        static_assert(
            (boost::mp11::mp_count_if<policies_type, finder_type::finder::fn>::value <= 1),
            "Only zero or one policies supporting a parse phase is supported");
        static_assert(
            (boost::mp11::mp_count_if<
                 policies_type,
                 phase_finder<policy::has_missing_phase_method, ValueType>::finder::fn>::value <=
             1),
            "Only zero or one policies supporting a missing phase is "
            "supported");
#else
        static_assert(
            (boost::mp11::mp_count_if_q<policies_type, typename finder_type::finder>::value <= 1),
            "Only zero or one policies supporting a parse phase is supported");
        static_assert(
            (boost::mp11::mp_count_if_q<policies_type,
                                        typename phase_finder<policy::has_missing_phase_method,
                                                              ValueType>::finder>::value <= 1),
            "Only zero or one policies supporting a missing phase is "
            "supported");
#endif

        if constexpr (std::is_void_v<policy_type>) {
            return parser<ValueType>::parse(token);
        } else {
            return this->policy_type::template parse_phase<ValueType>(token, parents...);
        }
    }

    children_type children_;
};
}  // namespace arg_router
