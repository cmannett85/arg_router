/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

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
class positional_arg_t : public tree_node<std::decay_t<Policies>...>
{
    static_assert(
        policy::is_all_policies_v<std::tuple<std::decay_t<Policies>...>>,
        "Positional args must only contain policies (not other nodes)");

    using parent_type = tree_node<std::decay_t<Policies>...>;

    template <std::size_t N>
    constexpr static bool has_fixed_count = []() {
        if constexpr (traits::has_minimum_count_method_v<parent_type> &&
                      traits::has_maximum_count_method_v<parent_type>) {
            return (parent_type::minimum_count() == N) &&
                   (parent_type::maximum_count() == N);
        }
        return false;
    }();

    static_assert(!has_fixed_count<0>, "Cannot have a fixed count of zero");
    static_assert(has_fixed_count<1> || traits::has_push_back_method_v<T>,
                  "value_type must have a push_back() method");

    static_assert(traits::has_display_name_method_v<positional_arg_t>,
                  "Positional arg must have a display name policy");
    static_assert(!traits::has_long_name_method_v<positional_arg_t>,
                  "Positional arg must not have a long name policy");
    static_assert(!traits::has_short_name_method_v<positional_arg_t>,
                  "Positional arg must not have a short name policy");
    static_assert(!traits::has_none_name_method_v<positional_arg_t>,
                  "Positional arg must not have a none name policy");
    static_assert(!traits::has_value_separator_method_v<positional_arg_t>,
                  "Positional arg must not have a value separator policy");

public:
    using typename parent_type::policies_type;

    /** Argument value type. */
    using value_type = T;

    /** Help data type. */
    template <bool Flatten>
    class help_data_type
    {
        [[nodiscard]] constexpr static auto label_generator() noexcept
        {
            constexpr auto name_index =
                boost::mp11::mp_find_if<policies_type,
                                        traits::has_display_name_method>::value;
            constexpr auto name =
                std::tuple_element_t<name_index, policies_type>::display_name();

            return S_("<"){} + S_(name){} + S_("> "){} +
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
    constexpr explicit positional_arg_t(Policies... policies) noexcept :
        parent_type{std::move(policies)...}
    {
    }

    /** Called during initial token processing to handle the value tokens.
     *
     * This is only used if the node doesn't have a policy that implements a
     * <TT>maximum_count()</TT> method, which means that all tokens in @a args
     * will be transferred to @a result with a prefix type of 
     * parsing::prefix_type::NONE.
     * @param args Command line arguments, remove the tokens that would be
     * handled by this node
     * @param result The token_list result, add tokens removed from @a args into
     * this
     * @return void
     */
    constexpr static void process_value_tokens(span<const char*>& args,
                                               parsing::token_list& result)
    {
        result.reserve(result.pending_view().size() + args.size());
        for (auto arg : args) {
            result.emplace_pending(parsing::prefix_type::NONE, arg);
        }
        args = {};
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
    constexpr bool match([[maybe_unused]] const parsing::token_type& token,
                         const Fn& visitor,
                         const std::optional<T>& result = {}) const
    {
        if constexpr (traits::has_push_back_method_v<T> &&
                      traits::has_maximum_count_method_v<parent_type>) {
            if (result &&
                (std::size(*result) >= parent_type::maximum_count())) {
                return false;
            }
        } else {
            // If we're here then we must have a fixed size of 1
            if (result) {
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
        // Pre-parse
        utility::tuple_type_iterator<policies_type>([&](auto i) {
            using policy_type = std::tuple_element_t<i, policies_type>;
            if constexpr (policy::has_pre_parse_phase_method_v<policy_type>) {
                this->policy_type::pre_parse_phase(tokens, *this, parents...);
            }
        });

        auto view = tokens.pending_view();
        if constexpr (traits::has_maximum_count_method_v<positional_arg_t>) {
            view = view.subspan(0, positional_arg_t::maximum_count());
        }

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
        tokens.mark_as_processed(view.size());

        // Validation
        utility::tuple_type_iterator<policies_type>([&](auto i) {
            using policy_type = std::tuple_element_t<i, policies_type>;
            if constexpr (policy::has_validation_phase_method_v<policy_type,
                                                                value_type>) {
                this->policy_type::validation_phase(result, *this, parents...);
            }
        });

        // No routing phase, a positional_arg cannot be used as a top-level node

        return result;
    }

private:
    static_assert(
        !parent_type::template any_phases_v<value_type,
                                            policy::has_routing_phase_method>,
        "Positional arg does not support policies with routing phases "
        "(e.g. router)");
};

/** Constructs an positional_arg_t with the given policies and value type.
 *
 * This is necessary due to CTAD being required for all template parameters or
 * none, and unfortunately in our case we need @a T to be explicitly set by the
 * user whilst @a Policies should be deduced.
 * @tparam T Argument value type, must have a <TT>push_back(..)</TT> method
 * @tparam Policies Pack of policies that define its behaviour
 * @param policies Pack of policy instances
 * @return Argument instance
 */
template <typename T, typename... Policies>
[[nodiscard]] constexpr auto positional_arg(Policies... policies) noexcept
{
    return positional_arg_t<T, std::decay_t<Policies>...>{
        std::move(policies)...};
}
}  // namespace arg_router
