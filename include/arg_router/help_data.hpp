// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/config.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/traits.hpp"
#include "arg_router/utility/always_true.hpp"
#include "arg_router/utility/compile_time_string.hpp"
#include "arg_router/utility/dynamic_string_view.hpp"

#include <vector>

namespace arg_router
{
/** Namespace for help data generation. */
namespace help_data
{
/** Generic help data type. */
struct type {
    utility::dynamic_string_view label;
    utility::dynamic_string_view description;
    std::vector<type> children;

    /** Equality operator.
     *
     * @param other Instance to compare against
     * @return True if equal
     */
    [[nodiscard]] bool operator==(const type& other) const noexcept
    {
        return label == other.label && description == other.description &&
               children == other.children;
    }

    /** Inequality operator.
     *
     * @param other Instance to compare against
     * @return True if not equal
     */
    [[nodiscard]] bool operator!=(const type& other) const noexcept { return !(*this == other); }
};
}  // namespace help_data

namespace traits
{
/** Determine if a type has a static
 * <TT>generate_help<Node, HelpNode, Flatten>(std::ostream&, const runtime_help_data&)</TT> method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_generate_help_method {
    template <typename U>
    using type = decltype(U::template generate_help<U>(std::declval<std::ostream&>(),
                                                       std::declval<const help_data::type&>()));

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_generate_help_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_generate_help_method_v = has_generate_help_method<T>::value;

/** Determine if a type has a <TT>generate_help_data<Flatten, FilterFn>(FilterFn&&)</TT>
 * method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_generate_help_data_method {
    template <typename U>
    using type =
        decltype(std::declval<const U&>().template generate_help_data<false, utility::always_true>(
            std::declval<const utility::always_true&>()));

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_generate_help_data_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_generate_help_data_method_v = has_generate_help_data_method<T>::value;
}  // namespace traits

namespace help_data
{
/** @tparam Node Node type to generate the data for
 *  @return Minimum and/or maximum value suffix string, or an empty string if a min/max policy is
 *  not used
 */
template <typename Node>
[[nodiscard]] constexpr auto value_suffix() noexcept
{
    constexpr auto has_min_value = traits::has_minimum_value_method_v<Node>;
    constexpr auto has_max_value = traits::has_maximum_value_method_v<Node>;

    if constexpr (has_min_value || has_max_value) {
        constexpr auto min_value = []() {
            if constexpr (has_min_value) {
                constexpr auto value = Node::minimum_value();
                return utility::convert_integral_to_cts_t<
                    static_cast<traits::underlying_type_t<std::decay_t<decltype(value)>>>(value)>{};
            } else {
                // If we have got this far, then we must have a maximum value, we can use
                // that type to determine if the lowest bound is 0 or -N depending on
                // whether or not the type is signed
                using max_value_type =
                    traits::underlying_type_t<std::decay_t<decltype(Node::maximum_value())>>;
                if constexpr (std::is_unsigned_v<max_value_type>) {
                    return str<"0">{};
                } else {
                    return str<"-N">{};
                }
            }
        }();

        constexpr auto max_value = []() {
            if constexpr (has_max_value) {
                constexpr auto value = Node::maximum_value();
                return utility::convert_integral_to_cts_t<
                    static_cast<traits::underlying_type_t<std::decay_t<decltype(value)>>>(value)>{};
            } else {
                return str<"N">{};
            }
        }();

        return str<"<">{} + min_value + str<"-">{} + max_value + str<">">{};
    } else {
        return str<"">{};
    }
}

/** @tparam Node Node type to generate the data for
 *  @return Value string for help output when displaying data for a parameter that uses a value\
 *  separator, or an empty string if a value separator policy is not used
 */
template <typename Node>
[[nodiscard]] constexpr auto value_separator_suffix() noexcept
{
    [[maybe_unused]] constexpr bool fixed_count_of_one = []() {
        if constexpr (traits::has_minimum_count_method_v<Node> &&
                      traits::has_maximum_count_method_v<Node>) {
            return (Node::minimum_count() == Node::maximum_count()) && (Node::minimum_count() == 1);
        }

        return false;
    }();

    [[maybe_unused]] constexpr auto value_str = []() {
        constexpr auto min_max_str = value_suffix<Node>();
        if constexpr (std::is_same_v<std::decay_t<decltype(min_max_str)>, str<"">>) {
            return str<"<Value>">{};
        } else {
            return min_max_str;
        }
    }();

    [[maybe_unused]] constexpr auto has_separator =
        boost::mp11::mp_find_if<typename Node::policies_type,
                                traits::has_value_separator_method>::value !=
        std::tuple_size_v<typename Node::policies_type>;

    if constexpr (has_separator) {
        return AR_STRING_SV(Node::value_separator()){} + value_str;
    } else if constexpr (fixed_count_of_one) {
        return str<" ">{} + value_str;
    } else {
        return str<"">{};
    }
}

/** @tparam Node Node type to generate the data for
 *  @return Long and short name label(s) for node with value suffix if present, or empty
 *  string if not present
 */
template <typename Node>
[[nodiscard]] constexpr auto label_generator() noexcept
{
    using policies_type = typename Node::policies_type;
    constexpr auto num_policies = std::tuple_size_v<typename Node::policies_type>;

    [[maybe_unused]] constexpr auto has_long_name =
        boost::mp11::mp_find_if<policies_type, traits::has_long_name_method>::value != num_policies;
    [[maybe_unused]] constexpr auto has_short_name =
        boost::mp11::mp_find_if<policies_type, traits::has_short_name_method>::value !=
        num_policies;
    [[maybe_unused]] constexpr auto has_none_name =
        boost::mp11::mp_find_if<policies_type, traits::has_none_name_method>::value != num_policies;

    if constexpr (has_long_name && has_short_name) {
        return AR_STRING_SV(config::long_prefix){} + AR_STRING_SV(Node::long_name()){} +
               str<",">{} + AR_STRING_SV(config::short_prefix){} +
               AR_STRING_SV(Node::short_name()){} + value_separator_suffix<Node>();
    } else if constexpr (has_long_name) {
        return AR_STRING_SV(config::long_prefix){} + AR_STRING_SV(Node::long_name()){} +
               value_separator_suffix<Node>();
    } else if constexpr (has_short_name) {
        return AR_STRING_SV(config::short_prefix){} + AR_STRING_SV(Node::short_name()){} +
               value_separator_suffix<Node>();
    } else if constexpr (has_none_name) {
        return AR_STRING_SV(Node::none_name()){} + value_separator_suffix<Node>();
    } else {
        return str<"">{};
    }
}

/** @tparam Node Node type to generate the data for
 *  @return Description text for string, or empty string if providing policy not present
 */
template <typename Node>
[[nodiscard]] constexpr auto description_generator() noexcept
{
    [[maybe_unused]] constexpr auto has_description =
        boost::mp11::mp_find_if<typename Node::policies_type,
                                traits::has_description_method>::value !=
        std::tuple_size_v<typename Node::policies_type>;

    if constexpr (has_description) {
        return AR_STRING_SV(Node::description()){};
    } else {
        return str<"">{};
    }
}

/** @tparam Node Node type to generate the data for
 *  @return Minimum and/or maximum count suffix string
 */
template <typename Node>
[[nodiscard]] constexpr auto count_suffix() noexcept
{
    [[maybe_unused]] constexpr bool fixed_count = []() {
        if constexpr (traits::has_minimum_count_method_v<Node> &&
                      traits::has_maximum_count_method_v<Node>) {
            return Node::minimum_count() == Node::maximum_count();
        }

        return false;
    }();

    constexpr auto prefix = str<"[">{};

    if constexpr (fixed_count) {
        return prefix + utility::convert_integral_to_cts_t<Node::minimum_count()>{} + str<"]">{};
    } else {
        constexpr auto min_count = []() {
            if constexpr (traits::has_minimum_count_method_v<Node>) {
                return utility::convert_integral_to_cts_t<Node::minimum_count()>{};
            } else {
                return str<"0">{};
            }
        }();

        constexpr auto max_count = []() {
            if constexpr (traits::has_maximum_count_method_v<Node>) {
                if constexpr (Node::maximum_count() ==
                              decltype(policy::min_count<0>)::maximum_count()) {
                    return str<"N">{};
                } else {
                    return utility::convert_integral_to_cts_t<Node::maximum_count()>{};
                }
            } else {
                return str<"N">{};
            }
        }();

        return prefix + min_count + str<",">{} + max_count + str<"]">{};
    }
}

/** Generates a help_data::type instance representing @a owner.
 *
 * Children can be filtered using @a f that must have the equivalent signature to:
 * @code
 * template <typename Child>
 * bool operator()(const Child&) const;
 * @endcode
 * Only if the return is true is the child added to the return list.
 *
 * @note Flatten is not used by this function, it is just propagated to types that have a
 * <TT>generate_help_data<bool, typename FilterFn>(const FilterFn&)</TT> method, that may or may not
 * use it
 *
 * @tparam Flatten True if the help output is to be flattened, unused for this
 * @tparam Node Node type to generate the data for
 * @tparam FilterFn Child filter type
 * @param node Node type to generate the data for
 * @param f A filter function that determines if the child should be shown.  Defaults to always
 * passing
 * @return runtime_help_data instances representing the unfiltered children
 */
template <bool Flatten, typename Node, typename FilterFn = utility::always_true>
[[nodiscard]] type generate(const Node& node, const FilterFn& f = utility::always_true{})
{
    auto result = type{label_generator<Node>().get(), description_generator<Node>().get(), {}};
    if constexpr (traits::has_generate_help_data_method_v<Node>) {
        result = node.template generate_help_data<Flatten>(f);
    } else {
        utility::tuple_iterator(
            [&](auto, auto& child) {
                if (f(child)) {
                    result.children.push_back(generate<Flatten>(child, f));
                }
            },
            node.children());
    }

    return result;
}
}  // namespace help_data
}  // namespace arg_router
