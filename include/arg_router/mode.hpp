/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/policy/multi_stage_value.hpp"
#include "arg_router/policy/no_result_value.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/tree_recursor.hpp"

#include <bitset>

namespace arg_router
{
/** Allows the grouping of nodes to define operational modes for a program.
 *
 * If no none name policy is provided, then the node is regarded as 'anonymous', and there can only
 * be one in the parse tree.  Conversely, if any mode is named, then there can only be named modes
 * in the parse tree.
 *
 * A mode must have at least one child node.
 * @tparam Params Policies and child node types for the mode
 */
template <typename... Params>
class mode_t : public tree_node<policy::no_result_value<>, std::decay_t<Params>...>
{
    using parent_type = tree_node<policy::no_result_value<>, std::decay_t<Params>...>;

    static_assert((std::tuple_size_v<typename mode_t::children_type> > 0),
                  "Mode must have at least one child node");
    static_assert(!traits::has_long_name_method_v<mode_t>, "Mode must not have a long name policy");
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
            boost::mp11::mp_bind<traits::get_value_type, boost::mp11::_1>>::template fn,
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
    constexpr static bool is_anonymous = !traits::has_none_name_method_v<mode_t>;

    /** Help data type. */
    template <bool Flatten>
    class help_data_type
    {
    public:
        using label = std::conditional_t<
            is_anonymous,
            S_(""),
            typename parent_type::template default_leaf_help_data_type<Flatten>::label>;

        using description = std::conditional_t<
            is_anonymous,
            S_(""),
            typename parent_type::template default_leaf_help_data_type<Flatten>::description>;

        using children = std::conditional_t<
            is_anonymous || Flatten,
            typename parent_type::template default_leaf_help_data_type<Flatten>::all_children_help,
            std::tuple<>>;
    };

    /** Constructor.
     *
     * @param params Policy and child instances
     */
    constexpr explicit mode_t(Params... params) noexcept :
        parent_type{policy::no_result_value<>{}, std::move(params)...}
    {
    }

    /** Mode pre-parse implementation.
     *
     * Delegates the pre-parsing to any matching child modes, otherwise iterates over the tokens
     * dispatching to any matching children until the tokens are consumed or all of the children
     * have been matched.
     * @tparam Validator Validator type
     * @tparam HasTarget True if @a pre_parse_data contains the parent's parse_target.  It is a
     * static_assert failure is this true for a mode
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param pre_parse_data Pre-parse data aggregate
     * @param parents Parent node instances
     * @return Non-empty if the leading tokens in @a args are consumable by this node
     * @exception parse_exception Thrown if a child node cannot be found, or a delegated child
     * pre-parse policy throws
     */
    template <typename Validator, bool HasTarget, typename... Parents>
    [[nodiscard]] std::optional<parsing::parse_target> pre_parse(
        parsing::pre_parse_data<Validator, HasTarget> pre_parse_data,
        const Parents&... parents) const
    {
        using namespace utility::string_view_ops;
        using namespace std::string_literals;

        static_assert(!HasTarget,
                      "Modes cannot receive pre_parse_data containing parent "
                      "parse_targets");
        static_assert(!is_anonymous || (sizeof...(Parents) <= 1),
                      "Anonymous modes can only exist under the root");

        auto& args = pre_parse_data.args();

        // If we're not anonymous then check the leading token is a match.  We can just delegate to
        // the default implementation for this
        if constexpr (!is_anonymous) {
            auto match = parent_type::pre_parse(pre_parse_data, *this, parents...);
            if (!match) {
                return match;
            }

            // Check if the next token (if any) matches a child mode.  If so, delegate to that
            if (!args.empty()) {
                match.reset();
                utility::tuple_iterator(
                    [&](auto /*i*/, const auto& child) {
                        using child_type = std::decay_t<decltype(child)>;
                        if constexpr (traits::is_specialisation_of_v<child_type, mode_t>) {
                            if (!match) {
                                match = child.pre_parse(pre_parse_data, *this, parents...);
                            }
                        }
                    },
                    this->children());
                if (match) {
                    return match;
                }
            }
        }

        if (!pre_parse_data.validator()(*this, parents...)) {
            return {};
        }

        auto target = parsing::parse_target{*this, parents...};

        // Iterate over the tokens until consumed, skipping children already processed that cannot
        // be repeated on the command line
        auto matched = std::bitset<std::tuple_size_v<children_type>>{};
        while (!args.empty()) {
            // Take a copy of the front token for the error messages
            const auto front_token = args.front();

            auto match = std::optional<parsing::parse_target>{};
            utility::tuple_iterator(
                [&]([[maybe_unused]] auto i, const auto& child) {
                    using child_type = std::decay_t<decltype(child)>;

                    // The token(s) have been processed so skip over any remaining children
                    if (match) {
                        return;
                    }

                    // Skip past modes, as they're handled earlier
                    if constexpr (!traits::is_specialisation_of_v<child_type, mode_t>) {
                        match = child.pre_parse(
                            parsing::pre_parse_data{
                                args,
                                target,
                                [&](const auto& real_child, const auto&...) {
                                    using real_child_type = std::decay_t<decltype(real_child)>;
                                    return verify_match<real_child_type>(matched[i], front_token);
                                }},
                            *this,
                            parents...);

                        // Update the matched bitset
                        if (match) {
                            matched.set(i);
                        }
                    }
                },
                this->children());

            if (!match) {
                if (matched.all()) {
                    throw parse_exception{"Unhandled arguments", args};
                }

                throw parse_exception{"Unknown argument", front_token};
            }

            // Flatten out nested sub-targets
            if (match->sub_targets().empty()) {
                target.add_sub_target(std::move(*match));
            } else {
                for (auto& sub_target : match->sub_targets()) {
                    target.add_sub_target(std::move(sub_target));
                }
            }
        }

        return target;
    }

    /** Parse function.
     *
     * This function will recurse into child nodes to find matching tokens, a mode must have a
     * routing phase policy which is why this method does not return the parsed tuple.
     *
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param target Parse target
     * @param parents Parents instances pack
     * @exception parse_exception Thrown if parsing failed
     */
    template <typename... Parents>
    void parse(parsing::parse_target target, const Parents&... parents) const
    {
        // Create an internal-use results tuple, this is made from optional-wrapped children's
        // value_types but any no_result_value children are replaced with skip_tag - this makes it
        // easier to iterate
        using results_type = boost::mp11::mp_transform<optional_value_type_or_skip, children_type>;
        auto results = results_type{};

        for (auto& sub_target : target.sub_targets()) {
            const auto node_hash = sub_target.node_type();
            auto result = sub_target();

            if (result.has_value()) {
                // We need to find the matching node to the sub_target.  We do this by searching
                // each child's subtree for a match - this gives us the index into the results tuple
                // and the sub node type for the match
                auto found = false;
                utility::tuple_iterator(
                    [&](auto i, const auto& child) {
                        if (found) {
                            return;
                        }

                        match_child(child, node_hash, [&](const auto& sub_child) {
                            found = true;
                            process_result<i>(results, result, sub_child);
                        });
                    },
                    this->children());
            }
        }

        // Handle missing tokens
        utility::tuple_iterator(
            [&]([[maybe_unused]] auto i, auto& result) {
                if constexpr (!is_skip_tag_v<std::decay_t<decltype(result)>>) {
                    if (!result) {
                        const auto& child = std::get<i>(this->children());
                        process_missing_token(result, child, *this, parents...);
                    }
                }
            },
            results);

        // Handle multi-stage value validation.  Multi-stage value nodes cannot be validated during
        // processing, as they will likely fail validation when partially processed.  So the
        // multi-stage nodes do not perform validation themselves, but have their owner (i.e. modes)
        // do it at the end of processing - including after any default values have been generated
        multi_stage_validation(results, *this, parents...);

        // Routing
        using routing_policy =
            typename parent_type::template phase_finder_t<policy::has_routing_phase_method>;
        if constexpr (!std::is_void_v<routing_policy>) {
            // Strip out the skipped results
            auto stripped_results = algorithm::tuple_filter_and_construct<
                boost::mp11::mp_not_fn<is_skip_tag>::template fn>(std::move(results));

            std::apply(
                [&](auto&&... args) {
                    this->routing_policy::routing_phase(
                        std::forward<std::decay_t<decltype(*args)>>(*args)...);
                },
                std::move(stripped_results));
        } else if constexpr (is_anonymous) {
            static_assert(traits::always_false_v<Params...>, "Anonymous modes must have routing");
        } else if constexpr (!boost::mp11::mp_all_of<children_type, is_child_mode>::value) {
            static_assert(traits::always_false_v<Params...>,
                          "Mode must have a router or all its children are modes");
        } else {
            // Mode guaranteed to be named here
            throw parse_exception{"Mode requires arguments", parsing::node_token_type<mode_t>()};
        }
    }

private:
    static_assert(!is_anonymous || boost::mp11::mp_none_of<children_type, is_child_mode>::value,
                  "Anonymous mode cannot have a child mode");

    static_assert(!parent_type::template any_phases_v<value_type,
                                                      policy::has_pre_parse_phase_method,
                                                      policy::has_parse_phase_method,
                                                      policy::has_validation_phase_method,
                                                      policy::has_missing_phase_method>,
                  "Mode does not support policies with pre-parse, parse, validation, "
                  "or missing phases; as it delegates those to its children");

    template <typename Child>
    struct child_has_routing_phase {
        using type = typename Child::template phase_finder_t<policy::has_routing_phase_method>;

        constexpr static bool value = !std::is_void_v<type>;
    };
    static_assert(boost::mp11::mp_none_of<boost::mp11::mp_remove_if<children_type, is_child_mode>,
                                          child_has_routing_phase>::value,
                  "Non-mode children cannot have routing");

    template <typename Child, typename Handler>
    static void match_child(const Child& child, std::size_t hash, Handler handler)
    {
        auto found = false;
        utility::tree_recursor(
            [&](const auto& node, const auto&...) {
                using node_type = std::decay_t<decltype(node)>;

                // Skip modes and nodes without a parse function.  In reality the runtime code will
                // never allow that, but let's help the compiler out...
                if constexpr (!traits::is_specialisation_of_v<node_type, mode_t> &&
                              traits::has_parse_method_v<node_type>) {
                    if (!found && (utility::type_hash<node_type>() == hash)) {
                        found = true;
                        handler(node);
                    }
                }
            },
            child);
    }

    template <typename Child>
    static bool verify_match(bool already_matched, [[maybe_unused]] parsing::token_type token)
    {
        if constexpr (!Child::is_named && !policy::has_multi_stage_value_v<Child>) {
            // Child is not named and can only appear on the command line once, so only perform the
            // pre-parse if it hasn't been matched already
            return !already_matched;
        } else if constexpr (Child::is_named && !policy::has_multi_stage_value_v<Child>) {
            // Child is named, but can only appear once on the command line, so perform the
            // pre-parse and if there is a match check it isn't already matched
            if (already_matched) {
                throw parse_exception{"Argument has already been set", token};
            }
        }

        return true;
    }

    template <std::size_t I, typename ResultsType, typename ChildType>
    void process_result(ResultsType& results,
                        utility::unsafe_any parse_result,
                        const ChildType& child) const
    {
        using optional_result_type = std::tuple_element_t<I, ResultsType>;

        // The mode check is just for the compiler, the runtime code prevents it reaching here
        if constexpr (!is_skip_tag_v<optional_result_type>) {
            auto& result = std::get<I>(results);

            if constexpr (policy::has_multi_stage_value_v<ChildType>) {
                using result_type = traits::arg_type_at_index<decltype(&ChildType::merge), 1>;
                child.merge(result, std::move(parse_result.get<result_type>()));
            } else {
                if (result) {
                    throw parse_exception{"Argument has already been set",
                                          parsing::node_token_type<ChildType>()};
                }

                using result_type = decltype(std::declval<ChildType>().parse(
                    std::declval<parsing::parse_target>()));
                result = std::move(parse_result.get<result_type>());
            }
        }
    }

    template <typename ValueType, typename ChildType, typename... Parents>
    void process_missing_token(std::optional<ValueType>& result,
                               const ChildType& child,
                               const Parents&... parents) const
    {
        utility::tuple_type_iterator<typename ChildType::policies_type>([&](auto i) {
            using policy_type = std::tuple_element_t<i, typename ChildType::policies_type>;
            if constexpr (policy::has_missing_phase_method_v<policy_type, ValueType>) {
                result = child.policy_type::template missing_phase<ValueType>(child, parents...);
            }
        });

        // If no missing_phase methods were found that made the result valid, then it still needs to
        // be valid - just default initialise
        if (!result) {
            result = ValueType{};
        }

        // Irritatingly, we have to run the validation phase on the new value
        utility::tuple_type_iterator<typename ChildType::policies_type>([&](auto i) {
            using policy_type = std::tuple_element_t<i, typename ChildType::policies_type>;
            if constexpr (policy::has_validation_phase_method_v<policy_type, ValueType>) {
                child.policy_type::template validation_phase(*result, child, parents...);
            }
        });
    }

    template <typename ResultsType, typename... Parents>
    void multi_stage_validation(const ResultsType& results, const Parents&... parents) const
    {
        // For all the children that use multi-stage values, invoke any policy
        // that supports a validation phase
        utility::tuple_iterator(
            [&](auto i, const auto& child) {
                using child_type = std::decay_t<decltype(child)>;
                using child_policies_type = typename child_type::policies_type;
                using optional_result_type = std::tuple_element_t<i, ResultsType>;

                constexpr auto msv = policy::has_multi_stage_value_v<child_type> ||
                                     boost::mp11::mp_any_of<typename child_type::children_type,
                                                            policy::has_multi_stage_value>::value;

                if constexpr (!is_skip_tag_v<optional_result_type> && msv) {
                    using result_type = typename optional_result_type::value_type;
                    const auto& result = *std::get<i>(results);

                    utility::tuple_type_iterator<child_policies_type>([&](auto j) {
                        using policy_type = std::tuple_element_t<j, child_policies_type>;
                        if constexpr (policy::has_validation_phase_method_v<policy_type,
                                                                            result_type>) {
                            child.policy_type::template validation_phase(result, child, parents...);
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
constexpr auto mode(Params... params)
{
    return mode_t{std::move(params)...};
}
}  // namespace arg_router
