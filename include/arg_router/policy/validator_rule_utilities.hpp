// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/validator.hpp"

namespace arg_router::policy::validation::utility
{
namespace detail
{
// This can't be done inside add_to_rule_types due to a g++-11 bug that prevents deduction guides
// being declared inside class scope
template <template <typename...> typename T,
          template <template <typename...> typename...>
          typename U,
          template <typename...>
          typename... V>
struct appender_t {
    using type = U<V..., T>;

    explicit appender_t([[maybe_unused]] U<V...> rule_type) {}
};

template <template <typename...> typename T,
          template <template <typename...> typename...>
          typename U,
          template <typename...>
          typename... V>
auto appender(U<V...>&& rule_type)
{
    return appender_t<T, U, V...>{std::forward<U<V...>>(rule_type)};
}
}  // namespace detail

/** Alias for the default rules type. */
using default_rules = typename decltype(default_validator)::rules_type;

/** Finds the index of the rule for @a RuleType within @a Rules.
 *
 * @tparam RuleType Type the rule is for
 * @tparam Rules Rule list
 * @return Index, or the size of @a Rules if not found
 */
template <typename RuleType, typename Rules>
constexpr std::size_t find_index_of_rule_type() noexcept
{
    return boost::mp11::mp_find_if_q<
        Rules,
        boost::mp11::mp_bind<std::is_same,
                             RuleType,
                             boost::mp11::mp_bind<boost::mp11::mp_first, boost::mp11::_1>>>::value;
}

/** Inserts @a Rule into @a Rules at position @a I.
 *
 * @tparam I Index to insert at
 * @tparam Rule New rule
 * @tparam Rules List to update
 */
template <std::size_t I, typename Rule, typename Rules>
struct insert_rule {
    static_assert(I <= std::tuple_size_v<Rules>, "I must be less than or equal Rules size");

    /** Updated rules list. */
    using type = boost::mp11::mp_insert_c<Rules, I, Rule>;
};

/** Alias helper.
 *
 * @tparam I Index to insert at
 * @tparam Rule New rule
 * @tparam Rules List to update
 */
template <std::size_t I, typename Rule, typename Rules>
using insert_rule_t = typename insert_rule<I, Rule, Rules>::type;

/** Removes the rule at index @a I from @a Rules.
 *
 * @tparam I Index to remove
 * @tparam Rules List to update
 */
template <std::size_t I, typename Rules>
struct remove_rule {
    static_assert(I < std::tuple_size_v<Rules>, "I must be less than Rules size");

    /** Updated rules list. */
    using type = boost::mp11::mp_erase_c<Rules, I, I + 1>;
};

/** Alias helper.
 *
 * @tparam I Index to remove
 * @tparam Rules List to update
 */
template <std::size_t I, typename Rules>
using remove_rule_t = typename remove_rule<I, Rules>::type;

/** Remove the rule for @a RuleType in @a Rules.
 *
 * Evaluates to @a Rules if @a RuleType cannot be found.
 * @tparam RuleType Type the rule is for
 * @tparam Rules Rule list
 */
template <typename RuleType, typename Rules>
class remove_rule_by_type
{
    constexpr static auto I = find_index_of_rule_type<RuleType, Rules>();
    constexpr static auto found = I < std::tuple_size_v<Rules>;

public:
    /** Updated rules list. */
    using type = boost::mp11::mp_eval_if_c<!found,
                                           Rules,
                                           boost::mp11::mp_erase,  // Can't use remove_rule_t here
                                           Rules,                  // due to the NTTPs...
                                           traits::integral_constant<I>,
                                           traits::integral_constant<I + 1>>;
};

/** Alias helper.
 *
 * @tparam RuleType Type the rule is for
 * @tparam Rules Rule list
 */
template <typename RuleType, typename Rules>
using remove_rule_by_type_t = typename remove_rule_by_type<RuleType, Rules>::type;

/** Replaces the entire rule at position @a I of @a Rules with @a Rule.
 *
 * @tparam I Index to update
 * @tparam Rule Updated rule
 * @tparam Rules List to update
 */
template <std::size_t I, typename Rule, typename Rules>
struct update_rule {
    static_assert(I < std::tuple_size_v<Rules>, "I must be less than Rules size");

    /** Updated rules list. */
    using type = boost::mp11::mp_replace_at_c<Rules, I, Rule>;
};

/** Alias helper.
 *
 * @tparam I Index to update
 * @tparam Rule Updated rule
 * @tparam Rules List to update
 */
template <std::size_t I, typename Rule, typename Rules>
using update_rule_t = typename update_rule<I, Rule, Rules>::type;

/** Update the rule for @a RuleType in @a Rules.
 *
 * @tparam RuleType Type the rule is for
 * @tparam Rule Updated rule
 * @tparam Rules List to update
 */
template <typename RuleType, typename Rule, typename Rules>
class update_rule_by_type
{
    constexpr static auto I = find_index_of_rule_type<RuleType, Rules>();
    static_assert(I < std::tuple_size_v<Rules>, "RuleType cannot be found");

public:
    /** Updated rules list. */
    using type = update_rule_t<I, Rule, Rules>;
};

/** Alias helper.
 *
 * @tparam RuleType Type the rule is for
 * @tparam Rule Updated rule
 * @tparam Rules List to update
 */
template <typename RuleType, typename Rule, typename Rules>
using update_rule_by_type_t = typename update_rule_by_type<RuleType, Rule, Rules>::type;

/** Adds @a T to the types the rule type at @a I applies to.
 *
 * @code
 * using namespace arg_router::policy::validation;
 * inline constexpr auto default_validator = validator<
 * ...
 *     rule_q<common_rules::despecialised_any_of_rule<arg_t>,
 *            must_not_have_policies<policy::multi_stage_value,
 *                                   policy::no_result_value,
 *                                   policy::validation::validator>>,
 * ...
 *
 * using my_rules = utility::add_to_rule_type<8, my_arg_type_t, utility::default_rules>;
 * // rule_q<common_rules::despecialised_any_of_rule<arg_t, my_arg_type_t>,
 * //        must_not_have_policies<policy::multi_stage_value,
 * //                               policy::no_result_value,
 * //                               policy::validation::validator>>
 * @endcode
 * @tparam I Index to update
 * @tparam T New type to append to the rule types list
 * @tparam Rules List to update
 */
template <std::size_t I, template <typename...> typename T, typename Rules>
class add_to_rule_types
{
    static_assert(I < std::tuple_size_v<Rules>, "I must be less than Rules size");

    using rule = std::tuple_element_t<I, Rules>;
    using rule_type = std::tuple_element_t<0, rule>;
    using new_rule_type = typename decltype(detail::appender<T>(std::declval<rule_type>()))::type;
    using new_rule = boost::mp11::mp_replace_at_c<rule, 0, new_rule_type>;

public:
    /** Updated rules list. */
    using type = boost::mp11::mp_replace_at_c<Rules, I, new_rule>;
};

/** Alias helper.
 *
 * @tparam I Index to update
 * @tparam T New type to append to the rule types list
 * @tparam Rules List to update
 */
template <std::size_t I, template <typename...> typename T, typename Rules>
using add_to_rule_types_t = typename add_to_rule_types<I, T, Rules>::type;

/** Update the rule for @a RuleType in @a Rules.
 *
 * @tparam RuleType Type the rule is for
 * @tparam T New type to append to the rule types list
 * @tparam Rules List to update
 */
template <typename RuleType, template <typename...> typename T, typename Rules>
class add_to_rule_types_by_rule
{
    constexpr static auto I = find_index_of_rule_type<RuleType, Rules>();
    static_assert(I < std::tuple_size_v<Rules>, "RuleType cannot be found");

public:
    /** Updated rules list. */
    using type = add_to_rule_types_t<I, T, Rules>;
};

/** Alias helper.
 *
 * @tparam RuleType Type the rule is for
 * @tparam T New type to append to the rule types list
 * @tparam Rules List to update
 */
template <typename RuleType, template <typename...> typename T, typename Rules>
using add_to_rule_types_by_rule_t = typename add_to_rule_types_by_rule<RuleType, T, Rules>::type;
}  // namespace arg_router::policy::validation::utility
