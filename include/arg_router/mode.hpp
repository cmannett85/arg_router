#pragma once

#include "arg_router/parsing.hpp"
#include "arg_router/policy/multi_stage_value.hpp"
#include "arg_router/policy/no_result_value.hpp"
#include "arg_router/tree_node.hpp"

namespace arg_router
{
/** Allows the grouping of nodes to define operational modes for a program.
 *
 * If no none name policy is provided, then the node is regarded as 'anonymous',
 * and there can only be one in the parse tree.  Conversely, if any mode is
 * named, then there can only be named modes in the parse tree.
 * 
 * A mode must have at least one child node.
 * @tparam Params Policies and child node types for the mode
 */
template <typename... Params>
class mode_t : public tree_node<policy::no_result_value<>, Params...>
{
    using parent_type = tree_node<policy::no_result_value<>, Params...>;

    static_assert((std::tuple_size_v<typename mode_t::children_type> > 0),
                  "Mode must have at least one child node");
    static_assert(!traits::has_long_name_method_v<mode_t>,
                  "Mode must not have a long name policy");
    static_assert(!traits::has_short_name_method_v<mode_t>,
                  "Mode must not have a short name policy");
    static_assert(!traits::has_display_name_method_v<mode_t>,
                  "Mode must not have a display name policy");

    struct skip_tag {
    };

    template <typename T>
    using is_skip_tag = std::is_same<std::decay_t<T>, skip_tag>;

    template <typename T>
    constexpr static bool is_skip_tag_v = is_skip_tag<T>::value;

    template <typename T>
    using optional_value_type_or_skip = boost::mp11::mp_eval_if_c<
        policy::has_no_result_value_v<T>,
        skip_tag,
        boost::mp11::mp_bind<
            traits::add_optional_t,
            boost::mp11::mp_bind<traits::get_value_type,
                                 boost::mp11::_1>>::template fn,
        T>;

    template <typename Child>
    using is_child_mode = traits::is_specialisation_of<Child, mode_t>;

public:
    using typename parent_type::policies_type;
    using typename parent_type::children_type;

    /** Tuple of valid children value types. */
    using value_type = boost::mp11::mp_transform<
        traits::get_value_type,
        boost::mp11::mp_remove_if<children_type, policy::has_no_result_value>>;

    /** True if this mode is anonymous. */
    constexpr static bool is_anonymous =
        !traits::has_none_name_method_v<mode_t>;

    /** Constructor.
     *
     * @param params Policy and child instances
     */
    constexpr explicit mode_t(Params... params) :
        parent_type{policy::no_result_value<>{}, std::move(params)...}
    {
    }

    /** Returns true and calls @a visitor if @a token matches the name of this
     * node, unless it is anonymous in which case it will return true.
     * 
     * @a visitor needs to be equivalent to:
     * @code
     * [](const auto& node) { ... }
     * @endcode
     * <TT>node</TT> will be a reference to this node.
     * @tparam Fn Visitor type
     * @param token Command line token to match
     * @param visitor Visitor instance
     * @return Match result
     */
    template <typename Fn>
    bool match(const parsing::token_type& token, const Fn& visitor) const
    {
        if (is_anonymous || parsing::default_match<mode_t>(token)) {
            visitor(*this);
            return true;
        }

        return false;
    }

    /** Parse function.
     * 
     * This function will recurse into child nodes to find matching tokens, a
     * mode must have a routing phase policy which is why this method does not
     * return the parsed tuple.
     * 
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Token list
     * @param parents Parents instances pack
     * @exception parse_exception Thrown if parsing failed
     */
    template <typename... Parents>
    void parse(parsing::token_list& tokens, const Parents&... parents) const
    {
        static_assert(!is_anonymous || (sizeof...(Parents) <= 1),
                      "Anonymous modes can only exist under the root");

        // Remove our token (if not anonymous) from the list, keep it for any
        // error messages
        auto mode_token = std::optional<parsing::token_type>{};
        if constexpr (!is_anonymous) {
            mode_token = tokens.pending_view().front();
            tokens.mark_as_processed();
        }

        // Find any matching mode type and delegate parsing into it
        if constexpr (!is_anonymous) {
            if (!tokens.pending_view().empty()) {
                auto found_child_mode = false;
                parent_type::find(
                    tokens.pending_view().front(),
                    [&](auto /*i*/, const auto& child) {
                        using child_type = std::decay_t<decltype(child)>;
                        if constexpr (traits::is_specialisation_of_v<child_type,
                                                                     mode_t>) {
                            found_child_mode = true;
                            child.parse(tokens, *this, parents...);
                        }
                    });
                if (found_child_mode) {
                    return;
                }
            }
        }

        // Create an internal-use results tuple, this is made from
        // optional-wrapped children's value_types but any no_result_value
        // children are replaced with skip_tag - this makes it easier to iterate
        using results_type =
            boost::mp11::mp_transform<optional_value_type_or_skip,
                                      children_type>;
        auto results = results_type{};

        // Iterate over the tokens until consumed
        while (!tokens.pending_view().empty()) {
            // Find the matching child
            const auto found = parent_type::find(
                tokens.pending_view().front(),
                [&](auto i, const auto& child) {
                    process_result<i.value>(tokens,
                                            results,
                                            child,
                                            *this,
                                            parents...);
                },
                results);
            if (!found) {
                throw parse_exception{"Unknown argument",
                                      tokens.pending_view().front()};
            }
        }

        // Handle missing tokens
        utility::tuple_iterator(
            [&](auto i, auto& result) {
                if constexpr (!is_skip_tag_v<std::decay_t<decltype(result)>>) {
                    if (!result) {
                        const auto& child = std::get<i>(this->children());
                        process_missing_token(result, child, *this, parents...);
                    }
                }
            },
            results);

        // Handle multi-stage value validation.  Multi-stage value nodes cannot
        // be validated during processing, as they will likely fail validation
        // when partially processed.  So the multi-stage nodes do not validation
        // themselves, but have their owner (i.e. modes) do it at the end of
        // processing - including after any default values have been generated
        multi_stage_validation(results, *this, parents...);

        // Routing
        using routing_policy = typename parent_type::template phase_finder_t<
            policy::has_routing_phase_method>;
        if constexpr (!std::is_void_v<routing_policy>) {
            // Strip out the skipped results
            auto stripped_results = algorithm::tuple_filter_and_construct<
                boost::mp11::mp_not_fn<is_skip_tag>::template fn>(
                std::move(results));

            std::apply(
                [&](auto&&... args) {
                    this->routing_policy::routing_phase(
                        tokens,  // Will always be empty
                        std::forward<std::decay_t<decltype(*args)>>(*args)...);
                },
                std::move(stripped_results));
        } else if constexpr (is_anonymous) {
            static_assert(traits::always_false_v<Params...>,
                          "Anonymous modes must have routing");
        } else if constexpr (!boost::mp11::mp_all_of<children_type,
                                                     is_child_mode>::value) {
            static_assert(
                traits::always_false_v<Params...>,
                "Mode must have a router or all its children are modes");
        } else {
            // mode_token guaranteed to be valid here, because it the mode must
            // not be anonymous to reach here
            throw parse_exception{"Mode requires arguments", *mode_token};
        }
    }

private:
    static_assert(
        !is_anonymous ||
            boost::mp11::mp_none_of<children_type, is_child_mode>::value,
        "Anonymous mode cannot have a child mode");

    static_assert(
        !parent_type::template any_phases_v<value_type,
                                            policy::has_pre_parse_phase_method,
                                            policy::has_parse_phase_method,
                                            policy::has_validation_phase_method,
                                            policy::has_missing_phase_method>,
        "Mode does not support policies with pre-parse, parse, validation, "
        "or missing phases; as it delegates those to its children");

    template <typename Child>
    struct child_has_routing_phase {
        using type = typename Child::template phase_finder_t<
            policy::has_routing_phase_method>;

        constexpr static bool value = !std::is_void_v<type>;
    };
    static_assert(boost::mp11::mp_none_of<
                      boost::mp11::mp_remove_if<children_type, is_child_mode>,
                      child_has_routing_phase>::value,
                  "Non-mode children cannot have routing");

    template <std::size_t I,
              typename ResultsType,
              typename ChildType,
              typename... Parents>
    void process_result(parsing::token_list& tokens,
                        ResultsType& results,
                        const ChildType& child,
                        const Parents&... parents) const
    {
        using optional_result_type = std::tuple_element_t<I, ResultsType>;

        if constexpr (!is_skip_tag_v<optional_result_type>) {
            // Make a copy of the token in case we need it for the error message
            // later (parsing consumes the tokens)
            const auto first_token = tokens.pending_view().front();

            auto parse_result = child.parse(tokens, parents...);
            auto& result = std::get<I>(results);

            if constexpr (policy::has_multi_stage_value_v<ChildType>) {
                child.merge(result, std::move(parse_result));
            } else {
                if (result) {
                    throw parse_exception{"Argument has already been set",
                                          first_token};
                }

                result = std::move(parse_result);
            }
        } else {
            // Even if the skip tag is set, we still need to process the token
            // as it may have side effects that affect other token processing
            // (e.g. aliases)
            child.parse(tokens, parents...);
        }
    }

    template <typename ValueType, typename ChildType, typename... Parents>
    void process_missing_token(std::optional<ValueType>& result,
                               const ChildType& child,
                               const Parents&... parents) const
    {
        utility::tuple_type_iterator<typename ChildType::policies_type>(
            [&](auto i) {
                using policy_type =
                    std::tuple_element_t<i, typename ChildType::policies_type>;
                if constexpr (policy::has_missing_phase_method_v<policy_type,
                                                                 ValueType>) {
                    result =
                        child.policy_type::template missing_phase<ValueType>(
                            child,
                            parents...);
                }
            });

        // If no missing_phase methods were found that made the result valid,
        // then it still needs to be valid - just default initialise
        if (!result) {
            result = ValueType{};
        }

        // Irritatingly, we have to run the validation phase on the new value
        utility::tuple_type_iterator<
            typename ChildType::policies_type>([&](auto i) {
            using policy_type =
                std::tuple_element_t<i, typename ChildType::policies_type>;
            if constexpr (policy::has_validation_phase_method_v<policy_type,
                                                                ValueType>) {
                child.policy_type::template validation_phase(*result,
                                                             child,
                                                             parents...);
            }
        });
    }

    template <typename ResultsType, typename... Parents>
    void multi_stage_validation(const ResultsType& results,
                                const Parents&... parents) const
    {
        // For all the children that use multi-stage values, invoke any policy
        // that supports a validation phase
        utility::tuple_iterator(
            [&](auto i, const auto& child) {
                using child_type = std::decay_t<decltype(child)>;
                using child_policies_type = typename child_type::policies_type;
                using optional_result_type =
                    std::tuple_element_t<i, ResultsType>;

                if constexpr (!is_skip_tag_v<optional_result_type> &&
                              policy::has_multi_stage_value_v<child_type>) {
                    using result_type =
                        typename optional_result_type::value_type;
                    const auto& result = *std::get<i>(results);

                    utility::tuple_type_iterator<child_policies_type>(
                        [&](auto j) {
                            using policy_type =
                                std::tuple_element_t<j, child_policies_type>;
                            if constexpr (policy::has_validation_phase_method_v<
                                              policy_type,
                                              result_type>) {
                                child.policy_type::template validation_phase(
                                    result,
                                    child,
                                    parents...);
                            }
                        });
                }
            },
            this->children());
    }
};

/** Constructs a mode_t with the given policies.
 *
 * This is used for similarity with arg_t.
 * @tparam Params Policies and child node types for the mode
 * @param params Pack of policy and child node instances
 * @return Mode instance
 */
template <typename... Params>
constexpr mode_t<Params...> mode(Params... params)
{
    return mode_t{std::move(params)...};
}
}  // namespace arg_router
