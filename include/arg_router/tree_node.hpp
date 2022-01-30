#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/config.hpp"
#include "arg_router/global_parser.hpp"
#include "arg_router/list.hpp"
#include "arg_router/policy/policy.hpp"
#include "arg_router/utility/compile_time_string.hpp"
#include "arg_router/utility/tuple_iterator.hpp"

#include <limits>

namespace arg_router
{
/** Base type of all nodes of a parse tree, not including policies.
 *
 * @tparam Params Parameter types
 */
template <typename... Params>
class tree_node :
    public traits::unpack_and_derive<  //
        boost::mp11::mp_filter<policy::is_policy, std::tuple<Params...>>>
{
public:
    /** The policies and child node types. */
    using parameters_type = std::tuple<Params...>;

    /** A tuple of all the policy types in parameters_type. */
    using policies_type =
        boost::mp11::mp_filter<policy::is_policy, parameters_type>;

    /** A tuple of all the child tree node types in parameters_type,
     * with any lists expanded
     */
    using children_type = boost::mp11::mp_filter<
        is_tree_node,
        std::decay_t<decltype(list_expander(std::declval<Params>()...))>>;

    /** Finds a policy that the @a PolicyChecker predicate passes.
     *
     * @tparam PolicyChecker Policy predicate, use one of
     * <TT>policy::has_*_phase_method</TT>
     * @tparam Args Auxiliary arguments to pass to @a PolicyChecker
     */
    template <template <typename...> typename PolicyChecker, typename... Args>
    struct phase_finder {
        using finder =
            boost::mp11::mp_bind<PolicyChecker, boost::mp11::_1, Args...>;

        /** The policy type that passes @a PolicyChecker, or void if one is not
         * found.
         */
        using type = boost::mp11::mp_eval_if_c<
            boost::mp11::mp_find_if_q<policies_type, finder>::value ==
                std::tuple_size_v<policies_type>,
            void,
            boost::mp11::mp_at,
            policies_type,
            boost::mp11::mp_find_if_q<policies_type, finder>>;
    };

    /** Helper alias for phase_finder.
     *
     * @tparam PolicyChecker Policy predicate, use one of
     * <TT>policy::has_*_phase_method</TT>
     * @tparam Args Auxiliary arguments to pass to @a PolicyChecker
     */
    template <template <typename...> typename PolicyChecker, typename... Args>
    using phase_finder_t = typename phase_finder<PolicyChecker, Args...>::type;

    /** Evaluates to true if any of the @a PolicyCheckers predicates pass.
     *
     * This is effectively a wrapper over phase_finder that loops over multiple
     * policy checkers.
     * 
     * Most nodes don't support certain phases, this predicate allows for
     * static_assert testing for their presence.
     * @tparam ValueType Some policy checkers require a value type template
     * parameter, if a particular policy doesn't need it then it is not used
     * @tparam PolicyCheckers Parameter pack type of policy predicates, use
     * <TT>policy::has_*_phase_method</TT>
     */
    template <typename ValueType,
              template <typename...>
              typename... PolicyCheckers>
    class any_phases
    {
        template <template <typename...> typename PolicyChecker>
        struct checker {
            constexpr static bool value = []() {
                // This is shonky but I can't think of an easy arity test for a
                // class template
                if constexpr (std::tuple_size_v<policies_type> == 0) {
                    return false;
                } else if constexpr (boost::mp11::mp_valid<
                                         PolicyChecker,
                                         boost::mp11::mp_first<policies_type>,
                                         ValueType>::value) {
                    return !std::is_void_v<
                        phase_finder_t<PolicyChecker, ValueType>>;
                } else {
                    return !std::is_void_v<phase_finder_t<PolicyChecker>>;
                }
            }();
        };

    public:
        constexpr static bool value =
            boost::mp11::mp_any_of<std::tuple<checker<PolicyCheckers>...>,
                                   boost::mp11::mp_to_bool>::value;
    };

    /** Helper alias for has_phases.
     *
     * @tparam ValueType Some policy checkers require a value type template
     * parameter, if a particular policy doesn't need it then it is not used
     * @tparam PolicyCheckers Parameter pack type of policy predicates, use
     * <TT>policy::has_*_phase_method</TT>
     */
    template <typename ValueType,
              template <typename...>
              typename... PolicyCheckers>
    constexpr static bool any_phases_v =
        any_phases<ValueType, PolicyCheckers...>::value;

    /** Returns a reference to the children.
     *
     * @return Children
     */
    [[nodiscard]] const children_type& children() const noexcept
    {
        return children_;
    }

    /** Finds the child matching token (if present) and calls @a visitor with a
     * reference to it.
     *
     * @a visitor needs to be equivalent to:
     * @code
     * [](auto i, const auto& child) { ... }
     * @endcode
     * <TT>i</TT> is the index of <TT>child</TT> in the owner's child list.
     * 
     * This calls the match(..) const method on each child until it returns
     * true.
     * @tparam Fn Visitor type
     * @param token Command line token to match
     * @param visitor Visitor instance
     * @param results_tuple Parent results tuple, used to pass the current value
     * (if any) to the found child's match(..) const method.  Leave unset if not
     * required
     * @return True if a child was found
     */
    template <typename Fn, typename ResultsTuple = std::tuple<>>
    constexpr bool find(const parsing::token_type& token,
                        const Fn& visitor,
                        const ResultsTuple& results_tuple = {}) const
    {
        auto result = false;
        utility::tuple_iterator(
            [&](auto i, const auto& node) {
                using node_type = std::decay_t<decltype(node)>;
                constexpr auto match_arity =
                    traits::arity_v<decltype(&node_type::template match<Fn>)>;

                if (result) {
                    return;
                }

                // Wrap the caller's visitor with one that forwards the child
                // index
                const auto wrapped_visitor = [&](const auto& child) {
                    visitor(i, child);
                };

                if constexpr ((i <= std::tuple_size_v<ResultsTuple>)&&  //
                              (match_arity == 3)) {
                    if (node.match(token,
                                   wrapped_visitor,
                                   std::get<i.value>(results_tuple))) {
                        result = true;
                    }
                } else if constexpr (match_arity == 2) {
                    if (node.match(token, wrapped_visitor)) {
                        result = true;
                    }
                }
            },
            children());

        return result;
    }

protected:
    /** Provides a default implementation for leaf nodes (e.g. flag and arg),
     * and some useful functions/types for other types.
     *
     * @tparam Flatten True if the help output is to be flattened, unused for
     * this
     */
    template <bool Flatten>
    class default_leaf_help_data_type
    {
        template <typename Child>
        using child_help_getter =
            typename Child::template help_data_type<Flatten>;

        template <typename Child>
        using child_help =
            boost::mp11::mp_eval_if_c<!traits::has_help_data_type_v<Child>,
                                      void,
                                      child_help_getter,
                                      Child>;

        constexpr static auto num_policies = std::tuple_size_v<policies_type>;

    public:
        [[nodiscard]] constexpr static auto label_generator() noexcept
        {
            constexpr auto long_name_index =
                boost::mp11::mp_find_if<policies_type,
                                        traits::has_long_name_method>::value;
            constexpr auto short_name_index =
                boost::mp11::mp_find_if<policies_type,
                                        traits::has_short_name_method>::value;
            constexpr auto none_name_index =
                boost::mp11::mp_find_if<policies_type,
                                        traits::has_none_name_method>::value;

            if constexpr ((long_name_index != num_policies) &&
                          (short_name_index != num_policies)) {
                constexpr auto long_name =
                    std::tuple_element_t<long_name_index,
                                         policies_type>::long_name();
                constexpr auto short_name =
                    std::tuple_element_t<short_name_index,
                                         policies_type>::short_name();

                return S_(config::long_prefix){} + S_(long_name){} +
                       S_(","){} +  //
                       S_(config::short_prefix){} + S_(short_name){};
            } else if constexpr (long_name_index != num_policies) {
                constexpr auto long_name =
                    std::tuple_element_t<long_name_index,
                                         policies_type>::long_name();

                return S_(config::long_prefix){} + S_(long_name){};
            } else if constexpr (short_name_index != num_policies) {
                constexpr auto short_name =
                    std::tuple_element_t<short_name_index,
                                         policies_type>::short_name();

                return S_(config::short_prefix){} + S_(short_name){};
            } else if constexpr (none_name_index != num_policies) {
                constexpr auto none_name =
                    std::tuple_element_t<none_name_index,
                                         policies_type>::none_name();

                return S_(none_name){};
            } else {
                return S_(""){};
            }
        }

        [[nodiscard]] constexpr static auto description_generator() noexcept
        {
            constexpr auto description_index =
                boost::mp11::mp_find_if<policies_type,
                                        traits::has_description_method>::value;

            if constexpr (description_index != num_policies) {
                return S_(
                    (std::tuple_element_t<description_index,
                                          policies_type>::description())){};
            } else {
                return S_(""){};
            }
        }

        [[nodiscard]] constexpr static auto count_suffix() noexcept
        {
            constexpr bool fixed_count = []() {
                if constexpr (traits::has_minimum_count_method_v<tree_node> &&
                              traits::has_maximum_count_method_v<tree_node>) {
                    return tree_node::minimum_count() ==
                           tree_node::maximum_count();
                }

                return false;
            }();

            constexpr auto prefix = S_("["){};

            if constexpr (fixed_count) {
                return prefix +
                       utility::convert_integral_to_cts_t<
                           tree_node::minimum_count()>{} +
                       S_("]"){};
            } else {
                constexpr auto min_count = []() {
                    if constexpr (traits::has_minimum_count_method_v<
                                      tree_node>) {
                        return utility::convert_integral_to_cts_t<
                            tree_node::minimum_count()>{};
                    } else {
                        return S_("0"){};
                    }
                }();

                constexpr auto max_count = []() {
                    if constexpr (traits::has_maximum_count_method_v<
                                      tree_node>) {
                        constexpr auto max_value = std::numeric_limits<
                            decltype(tree_node::maximum_count())>::max();
                        if constexpr (tree_node::maximum_count() == max_value) {
                            return S_("N"){};
                        } else {
                            return utility::convert_integral_to_cts_t<
                                tree_node::maximum_count()>{};
                        }
                    } else {
                        return S_("N"){};
                    }
                }();

                return prefix + min_count + S_(","){} + max_count + S_("]"){};
            }
        }

        /** Collects the help data from all the children that have a
         * help_data_type.
         */
        using all_children_help = boost::mp11::mp_remove_if<
            boost::mp11::mp_transform<child_help, children_type>,
            std::is_void>;

        /** Label compile time string.
         *
         * Evaluates to a string of the form "<short name>,<long_name>", or
         * only of them if only one available, or an empty string if none are
         * available.
         */
        using label = std::decay_t<decltype(label_generator())>;

        /** Description compile time string.
         *
         * Evaluates to a the description time, or an empty string if none is
         * available.
         */
        using description = std::decay_t<decltype(description_generator())>;

        /** Children help data tuple.
         *
         * Empty as this is the default for leaf types.
         */
        using children = std::tuple<>;
    };

    /** Generic parse call, uses a policy that supports the parse phase if
     * present, or the global parser.
     *
     * @tparam ValueType Parsed type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param token Token to parse
     * @param parents Parents instances pack
     * @return Parsed result
     * @exception parse_exception Thrown if parsing failed
     */
    template <typename ValueType, typename... Parents>
    [[nodiscard]] auto parse(std::string_view token,
                             const Parents&... parents) const
    {
        using finder_type =
            phase_finder<policy::has_parse_phase_method, ValueType>;
        using policy_type = typename finder_type::type;

        static_assert(
            (boost::mp11::mp_count_if_q<policies_type,
                                        typename finder_type::finder>::value  //
             <= 1),
            "Only zero or one policies supporting a parse phase is supported");

        if constexpr (std::is_void_v<policy_type>) {
            return parser<ValueType>::parse(token);
        } else {
            return this->policy_type::template parse_phase<ValueType>(
                token,
                parents...);
        }
    }

protected:
    /** Constructor.
     *
     * @param params Policy and child instances
     */
    constexpr explicit tree_node(Params... params) noexcept :
        traits::unpack_and_derive<policies_type>{
            common_filter<policy::is_policy>(params...)},
        children_(common_filter_tuple<is_tree_node>(
            list_expander(std::move(params)...)))
    {
    }

private:
    static_assert(
        (boost::mp11::mp_count_if_q<
             policies_type,
             typename phase_finder<policy::has_routing_phase_method>::finder>::
             value <= 1),
        "Only zero or one policies supporting a routing phase is supported");

    template <template <typename...> typename Fn, typename... ExpandedParams>
    [[nodiscard]] constexpr static auto common_filter(
        ExpandedParams&... expanded_params) noexcept
    {
        // Send references to the filter method so we don't copy anything
        // unnecessarily
        using ref_tuple = std::tuple<std::reference_wrapper<ExpandedParams>...>;

        // We have to wrap Fn because we're now giving it a
        // std::reference_wrapper
        using ref_fn = boost::mp11::mp_bind<
            Fn,
            boost::mp11::mp_bind<traits::get_type, boost::mp11::_1>>;

        // We have our result tuple, but it only contains references so we now
        // have to move construct them into the 'real' types
        auto ref_result =
            algorithm::tuple_filter_and_construct<ref_fn::template fn>(
                ref_tuple{expanded_params...});

        using ref_result_t = std::decay_t<decltype(ref_result)>;
        using result_t =
            boost::mp11::mp_transform<traits::get_type, ref_result_t>;

        // Converting move-constructor of std::tuple
        return result_t{std::move(ref_result)};
    }

    template <template <typename...> typename Fn, typename Tuple>
    [[nodiscard]] constexpr static auto common_filter_tuple(
        Tuple&& tuple_params) noexcept
    {
        // std::apply does not like templates, so we have to wrap in a lambda
        return std::apply(
            [](auto&... args) {
                return common_filter<Fn>(std::forward<decltype(args)>(args)...);
            },
            tuple_params);
    }

    children_type children_;
};
}  // namespace arg_router
