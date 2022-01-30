#pragma once

#include "arg_router/parsing.hpp"
#include "arg_router/policy/multi_stage_value.hpp"
#include "arg_router/tree_node.hpp"

namespace arg_router
{
/** Represents a flag that can appear multiple times in the command line.
 *
 * A flag is a boolean indicator, it has no value assigned on the command line,
 * its presence represents a positive boolean value.  However a counting flag's
 * value is the number of times it appears on the command line.
 * 
 * Just like 'normal' flags, counting flags with shortnames can be concatenated
 * or 'collapsed' on the command line,
 * e.g.
 * @code
 * foo -a -b -c
 * foo -abc
 * @endcode
 * 
 * Create with the counting_flag(Policies...) function for consistency with
 * arg_t.
 * @tparam T Counting value type, must be explicitly convertible to std::size_t
 * @tparam Policies Pack of policies that define its behaviour
 */
template <typename T, typename... Policies>
class counting_flag_t :
    public tree_node<policy::multi_stage_value<T, bool>, Policies...>
{
    static_assert(
        policy::is_all_policies_v<std::tuple<Policies...>>,
        "Counting flags must only contain policies (not other nodes)");
    static_assert(traits::has_long_name_method_v<counting_flag_t> ||
                      traits::has_short_name_method_v<counting_flag_t>,
                  "Counting flag must have a long and/or short name policy");
    static_assert(!traits::has_display_name_method_v<counting_flag_t>,
                  "Counting flag must not have a display name policy");
    static_assert(!traits::has_none_name_method_v<counting_flag_t>,
                  "Counting flag must not have a none name policy");
    static_assert(traits::supports_static_cast_conversion_v<T, std::size_t>,
                  "T must be explicitly convertible to std::size_t");

    constexpr static bool fixed_count = []() {
        if constexpr (traits::has_minimum_count_method_v<counting_flag_t> &&
                      traits::has_maximum_count_method_v<counting_flag_t>) {
            return counting_flag_t::minimum_count() ==
                   counting_flag_t::maximum_count();
        }

        return false;
    }();
    static_assert(!fixed_count, "Counting flag cannot have a fixed count");

    using parent_type =
        tree_node<policy::multi_stage_value<T, bool>, Policies...>;

public:
    using typename parent_type::policies_type;

    /** Flag value type. */
    using value_type = T;

    /** Help data type. */
    template <bool Flatten>
    class help_data_type
    {
        constexpr static auto label_generator()
        {
            return typename parent_type::template default_leaf_help_data_type<
                       Flatten>::label{} +
                   S_(" "){} +
                   parent_type::template default_leaf_help_data_type<
                       Flatten>::count_suffix();
        }

    public:
        using label = std::decay_t<decltype(label_generator())>;
        using description =
            typename parent_type::template default_leaf_help_data_type<
                Flatten>::description;
        using children = std::tuple<>;
    };

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit counting_flag_t(Policies... policies) :
        parent_type{policy::multi_stage_value<T, bool>{merge_impl},
                    std::move(policies)...}
    {
    }

    /** Returns true and calls @a visitor if @a token matches the name of this
     * node.
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
        if (parsing::default_match<counting_flag_t>(token)) {
            visitor(*this);
            return true;
        }

        return false;
    }

    /** Parse function.
     * 
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Token list
     * @param parents Parents instances pack
     * @return Parsed result
     * @exception parse_exception Thrown if parsing failed
     */
    template <typename... Parents>
    bool parse(parsing::token_list& tokens, const Parents&... parents) const
    {
        // Remove this node's name
        tokens.mark_as_processed();

        // Pre-parse
        utility::tuple_type_iterator<policies_type>([&](auto i) {
            using policy_type = std::tuple_element_t<i, policies_type>;
            if constexpr (policy::has_pre_parse_phase_method_v<policy_type>) {
                this->policy_type::pre_parse_phase(tokens, *this, parents...);
            }
        });

        // Presence of the flag yields a constant true.  Validation is done by
        // the parent mode as it carries the final result
        const auto result = true;

        return result;
    }

private:
    static_assert(
        !parent_type::template any_phases_v<value_type,
                                            policy::has_parse_phase_method,
                                            policy::has_routing_phase_method>,
        "Counting flag does not support policies with parse or routing phases "
        "(e.g. custom_parser)");

    // Value will always be true
    static void merge_impl(std::optional<value_type>& result, bool /*value*/)
    {
        if (!result) {
            result = static_cast<value_type>(1);
            return;
        }

        *result =
            static_cast<value_type>(static_cast<std::size_t>(*result) + 1);
    }
};

/** Constructs a counting_flag_t with the given policies.
 *
 * This is used for similarity with arg_t.
 * @tparam Policies Pack of policies that define its behaviour
 * @param policies Pack of policy instances
 * @return Flag instance
 */
template <typename T, typename... Policies>
constexpr counting_flag_t<T, Policies...> counting_flag(Policies... policies)
{
    return counting_flag_t<T, std::decay_t<Policies>...>{
        std::move(policies)...};
}
}  // namespace arg_router
