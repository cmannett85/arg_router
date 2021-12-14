#pragma once

#include "arg_router/parsing.hpp"
#include "arg_router/policy/multi_stage_value.hpp"
#include "arg_router/policy/no_result_value.hpp"
#include "arg_router/tree_node.hpp"

namespace arg_router
{
/** Allows the grouping of nodes to define operational modes for a program.
 *
 * If no long name policy is provided, then the node is regarded as 'anonymous',
 * and there can only be one in the parse tree.  Conversely, if any mode is
 * named, then there can only be named modes in the parse tree.
 * 
 * A mode must have at least one child node.
 * @tparam Params Policies and child node types for the mode
 */
template <typename... Params>
class mode_t : public policy::no_result_value, public tree_node<Params...>
{
    static_assert((std::tuple_size_v<typename mode_t::children_type> > 0),
                  "mode_t must have at least one child node");

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

    template <typename Tuple>
    struct phase_finder_bind;

    template <template <typename...> typename Tuple, typename... Args>
    struct phase_finder_bind<Tuple<Args...>> {
        using type = typename tree_node<Params...>::template phase_finder<
            policy::has_routing_phase_method,
            Args...>::type;
    };

public:
    using typename tree_node<Params...>::policies_type;
    using typename tree_node<Params...>::children_type;

    /** Tuple of valid children value types. */
    using value_type = boost::mp11::mp_transform<
        traits::get_value_type,
        boost::mp11::mp_remove_if<children_type, policy::has_no_result_value>>;

    /** True if this mode is anonymous. */
    constexpr static bool is_anonymous =
        !traits::has_long_name_method_v<mode_t> &&
        !traits::has_short_name_method_v<mode_t>;

    /** Constructor.
     *
     * @param params Policy and child instances
     */
    constexpr explicit mode_t(Params... params) :
        tree_node<Params...>{std::move(params)...}
    {
    }

    /** Match the token to the long form names in the children assigned to this
     * by its policies.
     *
     * @param token Command line token to match, stripped of prefix
     * @return Match result
     */
    bool match_old(const parsing::token_type& token) const
    {
        // If this mode has a long name, then match it, otherwise check if there
        // is a child that matches
        if constexpr (traits::has_long_name_method_v<mode_t>) {
            return (token.prefix == parsing::prefix_type::NONE) &&
                   (token.name == mode_t::long_name());
        } else {
            return parsing::visit_child(token,
                                        this->children(),
                                        [&](auto /*i*/, auto&& /*child*/) {});
        }
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
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Token list
     * @param parents Parents instances pack
     * @exception parse_exception Thrown if parsing failed
     */
    template <typename... Parents>
    void parse(parsing::token_list& tokens, const Parents&... parents) const
    {
        // Remove our token (if not anonymous) from the list, keep it for any
        // error messages
        auto mode_token = std::optional<parsing::token_type>{};
        if constexpr (!is_anonymous) {
            mode_token = tokens.front();
            tokens.erase(tokens.begin());
        }

        // Find any matching mode type and delegate parsing into it
        auto found_child_mode = false;
        if (!is_anonymous && !tokens.empty()) {
            tree_node<Params...>::find(
                tokens.front(),
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

        // Create an internal-use results tuple, this is made from
        // optional-wrapped children's value_types but any no_result_value
        // children are replaced with skip_tag - this makes it easier to iterate
        using results_type =
            boost::mp11::mp_transform<optional_value_type_or_skip,
                                      children_type>;
        auto results = results_type{};

        // Iterate over the tokens until consumed
        while (!tokens.empty()) {
            // Find the matching child
            const auto found = tree_node<Params...>::find(
                tokens.front(),
                [&](auto i, const auto& child) {
                    // In reality this conditional would always be true, but the
                    // compiler doesn't know that
                    if constexpr (!is_skip_tag_v<
                                      std::tuple_element_t<i, results_type>>) {
                        process_result<i.value>(tokens,
                                                results,
                                                child,
                                                *this,
                                                parents...);
                    }
                },
                results);
            if (!found) {
                throw parse_exception{"Unknown argument", tokens.front()};
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

        // Routing
        using routing_policy = typename phase_finder_bind<results_type>::type;
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
        } else {
            throw parse_exception{"Mode requires arguments", *mode_token};
        }
    }

private:
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
        static_assert(!is_skip_tag_v<optional_result_type>,
                      "Child value_type cannot be skip_tag and reach here");

        const auto first_token = tokens.front();
        auto& result = std::get<I>(results);
        auto parse_result = child.parse(tokens, parents...);

        if constexpr (policy::has_multi_stage_value_v<ChildType>) {
            child.merge(result, std::move(parse_result));
        } else {
            if (result) {
                throw parse_exception{"Argument has already been set",
                                      first_token};
            }

            result = std::move(parse_result);
        }
    }

    template <typename ValueType, typename ChildType, typename... Parents>
    void process_missing_token(std::optional<ValueType>& result,
                               const ChildType& child,
                               const Parents&... parents) const
    {
        utility::tuple_type_iterator<typename ChildType::policies_type>(
            [&](auto /*i*/, auto policy) {
                using policy_type = std::remove_pointer_t<decltype(policy)>;
                if constexpr (policy::has_missing_phase_method_v<policy_type,
                                                                 ValueType,
                                                                 Parents...>) {
                    result =
                        child.policy_type::template missing_phase<ValueType>(
                            parents...);
                }
            });
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
