#pragma once

#include "arg_router/tree_node.hpp"

namespace arg_router
{
/** Represents a positional argument on the command line that has potentially
 * multiple values that need parsing.
 *
 * @tparam T Argument value type, must have a <TT>push_back(..)</TT> method
 * @tparam Policies Pack of policies that define its behaviour
 */
template <typename T, typename... Policies>
class positional_arg_t : public tree_node<Policies...>
{
    static_assert(
        policy::is_all_policies_v<std::tuple<Policies...>>,
        "Positional args must only contain policies (not other nodes)");

    using parent_type = tree_node<Policies...>;

    template <std::size_t N>
    constexpr static bool has_fixed_count = []() {
        if constexpr (traits::has_minimum_count_method_v<parent_type> &&
                      traits::has_maximum_count_method_v<parent_type>) {
            return (parent_type::minimum_count() == N) &&
                   (parent_type::maximum_count() == N);
        }
        return false;
    }();

    constexpr static bool valid_counts = []() {
        if constexpr (traits::has_minimum_count_method_v<parent_type> &&
                      traits::has_maximum_count_method_v<parent_type>) {
            return parent_type::minimum_count() <= parent_type::maximum_count();
        }
        return true;
    }();
    static_assert(valid_counts,
                  "Minimum count must be less than or equal to maximum count");

    static_assert(!has_fixed_count<0>, "Cannot have a fixed count of zero");
    static_assert(has_fixed_count<1> || traits::has_push_back_method_v<T>,
                  "value_type must have a push_back() method");

    static_assert(traits::has_display_name_method_v<positional_arg_t>,
                  "Positional arg must have a display name policy");
    static_assert(!traits::has_long_name_method_v<positional_arg_t>,
                  "Positional arg must not have a long name policy");
    static_assert(!traits::has_short_name_method_v<positional_arg_t>,
                  "Positional arg must not have a short name policy");

public:
    using typename parent_type::policies_type;

    /** Argument value type. */
    using value_type = T;

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit positional_arg_t(Policies... policies) :
        parent_type{std::move(policies)...}
    {
    }

    /** Always returns true as positional arguments do not have a token to
     * match, @em unless the maximum count has been reached (if present).
     *
     * @a visitor needs to be equivalent to:
     * @code
     * [](const auto& node) { ... }
     * @endcode
     * <TT>node</TT> will be a reference to this node.
     * @tparam Fn Visitor type
     * @param token Command line token to match
     * @param visitor Visitor instance
     * @param result Current result from parent (if any), used to determine if
     * the maximum number of results has been reached
     * @return Match result, always true
     */
    template <typename Fn>
    bool match(const parsing::token_type& token,
               const Fn& visitor,
               const std::optional<T>& result) const
    {
        boost::ignore_unused(token);
        if constexpr (traits::has_push_back_method_v<T> &&
                      traits::has_maximum_count_method_v<parent_type>) {
            if (result &&
                (std::size(*result) >= parent_type::maximum_count())) {
                return false;
            }
        }

        visitor(*this);
        return true;
    }

    /** Parse function.
     * 
     * This will consume up to maximum_count() (if available) or all of the
     * tokens.
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Token list
     * @param parents Parents instances pack
     * @return Parsed result
     * @exception parse_exception Thrown if parsing failed
     */
    template <typename... Parents>
    value_type parse(parsing::token_list& tokens,
                     const Parents&... parents) const
    {
        auto view = utility::span<const parsing::token_type>{tokens};

        // Pre-parse
        utility::tuple_type_iterator<policies_type>([&](auto /*i*/, auto ptr) {
            using policy_type = std::remove_pointer_t<decltype(ptr)>;
            if constexpr (policy::has_pre_parse_phase_method_v<policy_type,
                                                               positional_arg_t,
                                                               Parents...>) {
                this->policy_type::pre_parse_phase(tokens,
                                                   view,
                                                   *this,
                                                   parents...);
            }
        });

        auto result = value_type{};
        if constexpr (traits::has_push_back_method_v<value_type>) {
            for (auto token : view) {
                result.push_back(
                    parent_type::template parse<value_type>(token.name,
                                                            *this,
                                                            parents...));
            }
        } else if (!view.empty()) {
            result = parent_type::template parse<value_type>(view.front().name,
                                                             *this,
                                                             parents...);
        }

        // Pop the tokens, we don't need them anymore
        tokens.pop_front(view.size());

        // Validation
        utility::tuple_type_iterator<policies_type>([&](auto /*i*/, auto ptr) {
            using policy_type = std::remove_pointer_t<decltype(ptr)>;
            if constexpr (policy::has_validation_phase_method_v<
                              policy_type,
                              value_type,
                              positional_arg_t,
                              Parents...>) {
                this->policy_type::validation_phase(result, *this, parents...);
            }
        });

        // Routing
        using routing_policy = typename parent_type::template phase_finder<
            policy::has_routing_phase_method,
            value_type>::type;
        if constexpr (!std::is_void_v<routing_policy>) {
            this->routing_policy::routing_phase(tokens, std::move(result));
        }

        return result;
    }
};

/** Constructs an positional_arg_t with the given policies and value type.
 *
 * This is necessary due to CTAD being required for all template parameters or
 * none, and unfortunately in our case we need @a T to be explicitly set by the
 * user whilst @a Policies need to be deduced.
 * @tparam T Argument value type, must have a <TT>push_back(..)</TT> method
 * @tparam Policies Pack of policies that define its behaviour
 * @param policies Pack of policy instances
 * @return Argument instance
 */
template <typename T, typename... Policies>
constexpr positional_arg_t<T, Policies...> positional_arg(Policies... policies)
{
    return positional_arg_t<T, std::decay_t<Policies>...>{
        std::move(policies)...};
}
}  // namespace arg_router
