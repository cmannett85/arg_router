// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/parsing/parsing.hpp"
#include "arg_router/policy/required.hpp"

namespace arg_router::policy
{
/** Policy that allows a node to be ignored during the parse phase depending upon it's runtime
 * constructor argument.
 *
 * This policy allows nodes or entire modes of a parse tree to be disabled, for example a feature
 * may not be available on a particular application license type - this policy can hide the feature
 * from the user.
 *
 * This policy does not affect the arguments dispatched to appropriate router, so values associated
 * with disabled nodes come from an attached policy::default_value or a default constructed
 * instance if no policy::default_value is attached.
 */
template <typename T = void>
class runtime_enable
{
public:
    /** Policy priority. */
    constexpr static auto priority = std::size_t{800};

    /** Constructor.
     *
     * @param enable True to enable the node
     */
    explicit runtime_enable(bool enable) noexcept : enabled_{enable} {}

    /** @return Enabled state */
    [[nodiscard]] bool runtime_enabled() const noexcept { return enabled_; }

    /** Calls the enabled_type function object and skips further processing for the token parsing if
     * disabled.
     *
     * @tparam ProcessedTarget @a processed_target payload type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Currently processed tokens
     * @param processed_target Previously processed parse_target of parent node, or empty is there
     * is no non-root parent
     * @param target Pre-parse generated target
     * @param parents Parent node instances
     * @return parsing::pre_parse_action::valid_node if enabled, otherwise
     * parsing::pre_parse_action::skip_node
     */
    template <typename ProcessedTarget, typename... Parents>
    [[nodiscard]] parsing::pre_parse_result pre_parse_phase(
        [[maybe_unused]] parsing::dynamic_token_adapter& tokens,
        [[maybe_unused]] utility::compile_time_optional<ProcessedTarget> processed_target,
        [[maybe_unused]] parsing::parse_target& target,
        [[maybe_unused]] const Parents&... parents) const
    {
        static_assert(sizeof...(Parents) >= 1, "Runtime enable requires at least 1 parent");
        using node_type = boost::mp11::mp_first<std::tuple<Parents...>>;
        static_assert(!policy::is_required_v<node_type>,
                      "Runtime enable must not be used with policy::required");

        return runtime_enabled() ? parsing::pre_parse_action::valid_node :
                                   parsing::pre_parse_action::skip_node;
    }

protected:
    bool enabled_;
};

template <typename T>
struct is_policy<runtime_enable<T>> : std::true_type {
};

template <typename T>
class runtime_enable_required : public runtime_enable<T>
{
public:
    using runtime_enable<T>::pre_parse_phase;

    /** Alias of @a T. */
    using value_type = T;

    /** Constructor.
     *
     * This value is used as the parsed value when not enabled.  This is only required when the
     * owning node has a policy::required attached, as it prevents requirement failures from
     * occuring when the node is disabled.
     * @param enable True to enable the node
     * @param default_value Default parsed value when not enabled
     */
    explicit runtime_enable_required(bool enable, T default_value = {}) noexcept :
        runtime_enable<T>{enable}, default_value_{std::move(default_value)}
    {
    }

    /** Throw an error if the owning node is enabled, otherwise returns the default value.
     *
     * @tparam ValueType Parsed value type, not used in this method
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param parents Parents instances pack, not used in this method
     * @return Default value
     * @exception multi_lang_exception Thrown if the owning node is enabled
     */
    template <typename ValueType, typename... Parents>
    [[nodiscard]] ValueType missing_phase([[maybe_unused]] const Parents&... parents) const
    {
        if (this->enabled_) {
            static_assert(sizeof...(Parents) >= 1, "Runtime enable requires at least 1 parent");
            using node_type = boost::mp11::mp_first<std::tuple<Parents...>>;

            throw multi_lang_exception{error_code::missing_required_argument,
                                       parsing::node_token_type<node_type>()};
        }

        return default_value_;
    }

private:
    value_type default_value_;
};

template <typename T>
struct is_policy<runtime_enable_required<T>> : std::true_type {
};
}  // namespace arg_router::policy
