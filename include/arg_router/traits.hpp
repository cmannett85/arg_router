// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/tree_node_fwd.hpp"

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/bind.hpp>
#include <boost/mp11/utility.hpp>

#include <algorithm>
#include <functional>
#include <optional>
#include <ostream>
#include <tuple>
#include <type_traits>

// Surprised this is not already done somewhere in mp11, without it using
// std::tuple and mp_list in generic situations doesn't work
namespace std
{
template <typename... T>
struct tuple_size<boost::mp11::mp_list<T...>> : std::integral_constant<std::size_t, sizeof...(T)> {
};

template <std::size_t I, typename... T>
struct tuple_element<I, boost::mp11::mp_list<T...>> {
    using type = boost::mp11::mp_at_c<boost::mp11::mp_list<T...>, I>;
};
}  // namespace std

/** Namespace for all arg_router types and functions. */
namespace arg_router
{
class multi_lang_exception;

/** Namespace for type traits. */
namespace traits
{
/** Regardless of @a T, always evaluates to false.
 *
 * @tparam T 'Sink' type
 */
template <typename... T>
struct always_false : std::false_type {
};

/** Helper variable for always_false.
 *
 * @tparam T 'Sink' type
 */
template <typename... T>
constexpr bool always_false_v = always_false<T...>::value;

/** Alias for <TT>typename T::type</TT>.
 *
 * @tparam T Type holding the typedef
 */
template <typename T>
using get_type = typename T::type;

/** Alias for <TT>typename T::value_type</TT>.
 *
 * @tparam T Type holding the typedef
 */
template <typename T>
using get_value_type = typename T::value_type;

/** Determine if a type has a <TT>value_type()</TT> typedef.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_value_type {
    constexpr static bool value = boost::mp11::mp_valid<get_value_type, T>::value;
};

/** Helper variable for has_value_type.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_value_type_v = has_value_type<T>::value;

/** Same as std::underlying_type except that it acts as an identity type for non-enum inputs.
 *
 * @tparam T Type to query
 */
template <typename T, typename Enable = void>
struct underlying_type {
    using type = T;
};

template <typename T>
struct underlying_type<T, std::enable_if_t<std::is_enum_v<T>>> {
    using type = std::underlying_type_t<T>;
};

/** Helper alias for underlying_type.
 *
 * @tparam T Type to query
 */
template <typename T>
using underlying_type_t = typename underlying_type<T>::type;

/** Evaluates to true if @a T is a tuple-like type.
 *
 * A tuple-like type is one that is can be used with std::tuple_size (i.e.
 * std::pair, std::array, and std::tuple).
 * @tparam T Type to test
 */
template <typename T, typename = void>
struct is_tuple_like : std::false_type {
};

template <typename T>
struct is_tuple_like<T, std::void_t<typename std::tuple_size<T>::type>> : std::true_type {
};

/** Helper variable for is_tuple_like.
 *
 * @tparam T Type to test
 */
template <typename T>
constexpr bool is_tuple_like_v = is_tuple_like<T>::value;

/** True if @a T is a specialisation.
 *
 * @note Explicit specialisations will need adding for types with non-type
 * template parameters (e.g. std::array)
 * @tparam T Type to test
 */
template <typename T>
struct is_specialisation : std::false_type {
};

template <template <typename...> typename U, typename... Args>
struct is_specialisation<U<Args...>> : std::true_type {
};

/** Helper variable for is_specialisation.
 *
 * @tparam T Type to test
 */
template <typename T>
constexpr bool is_specialisation_v = is_specialisation<T>::value;

/** True if @a T is a specialisation of @a U.
 *
 * @code
 * is_specialisation_of<std::vector<int>, std::vector> // True
 * is_specialisation_of<std::vector<int>, std::deque>  // False
 * @endcode
 * @note Explicit specialisations will need adding for types with non-type
 * template parameters (e.g. std::array)
 * @tparam T Type to test
 * @tparam U Unspecialised type to test against
 */
template <typename T, template <typename...> typename U>
struct is_specialisation_of : std::false_type {
};

template <template <typename...> typename U, typename... Args>
struct is_specialisation_of<U<Args...>, U> : std::true_type {
};

/** Helper variable for is_specialisation_of.
 *
 * @tparam T Type to test
 * @tparam U Unspecialised type to test against
 */
template <typename T, template <typename...> typename U>
constexpr bool is_specialisation_of_v = is_specialisation_of<T, U>::value;

/** True if @a T and @a U are specialisations of the same type.
 *
 * @code
 * is_same_when_despecialised<std::vector<int>, std::vector<std::string>>::value // True
 * is_same_when_despecialised<std::vector<int>, std::vector<int>>::value         // True
 * is_same_when_despecialised<std::vector<int>, std::deque<int>>::value          // False
 * @endcode
 *
 * If any param is not a specialised type, then it evaluates to false.
 * @tparam T First type to compare
 * @tparam U Second type to compare
 */
template <typename T, typename U, typename... Args>
struct is_same_when_despecialised : std::false_type {
};

template <template <typename...> typename T, typename U, typename... Args>
struct is_same_when_despecialised<T<Args...>, U> : is_specialisation_of<U, T> {
};

/** Helper variable for is_same_when_despecialised.
 *
 * @tparam T First type to compare
 * @tparam U Second type to compare
 */
template <typename T, typename U>
constexpr bool is_same_when_despecialised_v = is_same_when_despecialised<T, U>::value;

/** CTAD wraper for std::integral_constant.
 *
 * @tparam Value Primitive value
 */
template <auto Value>
using integral_constant = std::integral_constant<decltype(Value), Value>;

/** Create a <TT>std::optional<T></TT>.
 *
 * @tparam T Type to wrap
 */
template <typename T>
struct add_optional {
    using type = std::optional<T>;
};

/** Helper alias for add_optional.
 *
 * @tparam T Type to wrap
 */
template <typename T>
using add_optional_t = typename add_optional<T>::type;

/** A struct that takes a tuple-like type, unpacks it, and derives from each of
 * the elements.
 *
 * The valid specialisation will provide a tuple constructor.
 * @tparam T Tuple-like type, whose elements to indivdually derive from
 */
template <typename T>
class unpack_and_derive
{
    static_assert(always_false_v<T>, "T must be a tuple-like type");
};

template <template <typename...> typename T, typename... Params>
class unpack_and_derive<T<Params...>> : public Params...
{
    // When are we going to get language-level tuple unpacking!?
    template <std::size_t... I>
    constexpr explicit unpack_and_derive(
        T<Params...> params,
        [[maybe_unused]] std::integer_sequence<std::size_t, I...> Is) noexcept :
        Params{std::get<I>(std::move(params))}...
    {
    }

    // This empty tuple overload is just to prevent an
    // -Werror=unused-but-set-parameter on the params parameter
    template <std::size_t... I>
    constexpr explicit unpack_and_derive(
        [[maybe_unused]] T<Params...> t,
        [[maybe_unused]] std::integer_sequence<std::size_t> Is) noexcept
    {
    }

public:
    constexpr explicit unpack_and_derive(T<Params...> params) noexcept :
        unpack_and_derive{std::move(params), std::make_index_sequence<sizeof...(Params)>{}}
    {
    }
};

/** Determine if the static_cast conversion @a From to @a To is valid.
 *
 * @tparam From Type to convert from
 * @tparam To Type to convert to
 */
template <typename From, typename To>
struct supports_static_cast_conversion {
    template <typename F, typename T>
    using type = decltype(static_cast<T>(std::declval<F>()));

    constexpr static bool value = boost::mp11::mp_valid<type, From, To>::value;
};

/** Helper variable for supports_static_cast_conversion.
 *
 * @tparam From Type to convert from
 * @tparam To Type to convert to
 */
template <typename From, typename To>
constexpr bool supports_static_cast_conversion_v = supports_static_cast_conversion<From, To>::value;

/** Determine if a type has a <TT>long_name()</TT> static method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_long_name_method {
    template <typename U>
    using type = decltype(U::long_name());

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_long_name_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_long_name_method_v = has_long_name_method<T>::value;

/** Determine if a type has a <TT>short_name()</TT> static method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_short_name_method {
    template <typename U>
    using type = decltype(U::short_name());

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_short_name_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_short_name_method_v = has_short_name_method<T>::value;

/** Determine if a type has a <TT>display_name()</TT> static method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_display_name_method {
    template <typename U>
    using type = decltype(U::display_name());

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_display_name_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_display_name_method_v = has_display_name_method<T>::value;

/** Determine if a type has a <TT>none_name()</TT> static method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_none_name_method {
    template <typename U>
    using type = decltype(U::none_name());

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_none_name_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_none_name_method_v = has_none_name_method<T>::value;

/** Determine if a type has a <TT>error_name()</TT> static method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_error_name_method {
    template <typename U>
    using type = decltype(U::error_name());

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_error_name_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_error_name_method_v = has_error_name_method<T>::value;

/** Determine if a type has a <TT>description()</TT> static method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_description_method {
    template <typename U>
    using type = decltype(U::description());

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_description_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_description_method_v = has_description_method<T>::value;

/** Determine if a type has a <TT>value_separator()</TT> static method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_value_separator_method {
    template <typename U>
    using type = decltype(U::value_separator());

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_value_separator_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_value_separator_method_v = has_value_separator_method<T>::value;

/** Determine if a type has a <TT>token_end_marker()</TT> static method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_token_end_marker_method {
    template <typename U>
    using type = decltype(U::token_end_marker());

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_token_end_marker_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_token_end_marker_method_v = has_token_end_marker_method<T>::value;

/** Determine if a type has a <TT>maximum_count()</TT> static method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_maximum_count_method {
    template <typename U>
    using type = decltype(U::maximum_count());

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_maximum_count_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_maximum_count_method_v = has_maximum_count_method<T>::value;

/** Determine if a type has a <TT>minimum_count()</TT> static method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_minimum_count_method {
    template <typename U>
    using type = decltype(U::minimum_count());

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_minimum_count_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_minimum_count_method_v = has_minimum_count_method<T>::value;

/** Determine if a type has a <TT>minimum_value()</TT> static method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_minimum_value_method {
    template <typename U>
    using type = decltype(U::minimum_value());

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_minimum_value_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_minimum_value_method_v = has_minimum_value_method<T>::value;

/** Determine if a type has a <TT>maximum_value()</TT> static method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_maximum_value_method {
    template <typename U>
    using type = decltype(U::maximum_value());

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_maximum_value_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_maximum_value_method_v = has_maximum_value_method<T>::value;

/** Determine if a type has a <TT>push_back(typename T::value_type)</TT> method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_push_back_method {
    template <typename U>
    using type = decltype(std::declval<U&>().push_back(std::declval<typename U::value_type>()));

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_push_back_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_push_back_method_v = has_push_back_method<T>::value;

/** Determine if a type has a <TT>help_data_type</TT> nested type.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_help_data_type {
    template <typename U>
    using type = typename U::template help_data_type<false>;

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_help_data_type.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_help_data_type_v = has_help_data_type<T>::value;

/** Determine if a type has a <TT>runtime_children</TT> static method.
 *
 * @tparam T Type to query
 */
template <typename T>
class has_runtime_children_method
{
    struct fake_tree_node {
        [[nodiscard]] const std::tuple<>& children() const { return children_; }

        std::tuple<> children_;
    };

    template <typename U>
    using type =
        decltype(U::runtime_children(std::declval<const fake_tree_node&>(),
                                     std::declval<std::function<bool(const fake_tree_node&)>>()));

public:
    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_runtime_children_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_runtime_children_method_v = has_runtime_children_method<T>::value;

/** Determine if a type has a static <TT>generate_help<Node, HelpNode, Flatten>(std::ostream&)</TT>
 * method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_generate_help_method {
    template <typename U>
    using type = decltype(U::template generate_help<U, U, false>(std::declval<std::ostream&>()));

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_generate_help_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_generate_help_method_v = has_generate_help_method<T>::value;

/** Determine if a type has a static
 * <TT>generate_help<Node, HelpNode, Flatten>(std::ostream&, const runtime_help_data&)</TT> method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_runtime_generate_help_method {
    template <typename U>
    using type =
        decltype(U::template generate_help<U, U, false>(std::declval<std::ostream&>(),
                                                        std::declval<const runtime_help_data&>()));

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_runtime_generate_help_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_runtime_generate_help_method_v = has_runtime_generate_help_method<T>::value;

/** Determine if a type has a
 * <TT>has_generate_runtime_help_data_method<Flatten, Node>(const Node&)</TT> method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_generate_runtime_help_data_method {
    template <typename U>
    using type = decltype(std::declval<const T&>().template generate_runtime_help_data<true, U>(
        std::declval<const U&>()));

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_runtime_generate_help_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_generate_runtime_help_data_method_v =
    has_generate_runtime_help_data_method<T>::value;

/** Determine if a type has a <TT>runtime_enabled()</TT> method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_runtime_enabled_method {
    template <typename U>
    using type = decltype(std::declval<const U&>().runtime_enabled());

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_runtime_enabled_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_runtime_enabled_method_v = has_runtime_enabled_method<T>::value;

/** Determine if a type has a <TT>translate_exception</TT> method.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_translate_exception_method {
    template <typename U>
    using type = decltype(U::translate_exception(std::declval<const multi_lang_exception&>()));

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_translate_exception_method.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr static bool has_translate_exception_method_v = has_translate_exception_method<T>::value;

/** Determine if a type has a <TT>error_code_translations</TT> nested type.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_error_code_translations_type {
    template <typename U>
    using type = typename U::error_code_translations;

    constexpr static bool value = boost::mp11::mp_valid<type, T>::value;
};

/** Helper variable for has_error_code_translations_type.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr static bool has_error_code_translations_type_v =
    has_error_code_translations_type<T>::value;

/** Evaluates to a <TT>std::tuple</TT> of the return and argument types the
 * <TT>Callable</TT> @a T has.
 *
 * The first tuple element type is the return type.
 *
 * From https://stackoverflow.com/a/27867127/498437.
 * @note The specialisations are not complete, there are no volatile or ref-qualified versions. Does
 * @b NOT work with overloaded or templated function/methods (obviously)
 *
 * @tparam T Type to query
 */
template <typename T>
struct arg_extractor /** @cond */ : arg_extractor<decltype(&T::operator())> /** @endcond */ {
};

template <typename R, typename... Args>
struct arg_extractor<R (*)(Args...)> {
    using type = std::tuple<R, Args...>;
};
template <typename R, typename C, typename... Args>
struct arg_extractor<R (C::*)(Args...)> {
    using type = std::tuple<R, Args...>;
};
template <typename R, typename C, typename... Args>
struct arg_extractor<R (C::*)(Args...) const> {
    using type = std::tuple<R, Args...>;
};
template <typename R, typename C, typename... Args>
struct arg_extractor<R (C::*)(Args...) noexcept> {
    using type = std::tuple<R, Args...>;
};
template <typename R, typename C, typename... Args>
struct arg_extractor<R (C::*)(Args...) const noexcept> {
    using type = std::tuple<R, Args...>;
};

/** Helper alias for arg_extractor.
 *
 * @tparam T Type to query
 */
template <typename T>
using arg_extractor_t = typename arg_extractor<T>::type;

/** Evaluates to the number of arguments the <TT>Callable</TT> @a T has.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr std::size_t arity_v = std::tuple_size_v<arg_extractor_t<T>> - 1;

/** Evaluates to the argument type at index @a I in <TT>Callable</TT> @a T.
 *
 * @tparam T Type to query
 * @tparam I Argument index
 */
template <typename T, std::size_t I>
using arg_type_at_index = std::tuple_element_t<I + 1, arg_extractor_t<T>>;
}  // namespace traits
}  // namespace arg_router
