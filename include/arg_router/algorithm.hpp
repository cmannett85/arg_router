/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/traits.hpp"

#include <boost/mp11/bind.hpp>

#include <algorithm>
#include <string_view>

namespace arg_router
{
/** Namespace for generic algorithms. */
namespace algorithm
{
/** Evaluates to the index of the specialisation of @a T in @a Tuple.
 *
 * If @a Tuple does not contain a specialisation of @a T then the value is of the size of @a Tuple.
 * @code
 * find_specialisation<std::vector, std::tuple<int, std::deque<int>, double>>::value  // 3
 * find_specialisation<std::vector, std::tuple<int, std::vector<int>, double>>::value // 1
 * @endcode
 * 
 * @tparam T Specialisation to search for
 * @tparam Tuple Types to search (may be empty)
 */
template <template <typename...> typename T, typename Tuple>
class find_specialisation
{
    template <typename U>
    using fn = traits::is_specialisation_of<U, T>;

public:
    /** Index of specialisation of @a T in @a Tuple, or tuple size if not present.
     */
    constexpr static std::size_t value = boost::mp11::mp_find_if<Tuple, fn>::value;
};

/** Helper variable for find_specialisation.
 *
 * @tparam T Specialisation to search for
 * @tparam Tuple Types to search (may be empty)
 */
template <template <typename...> typename T, typename Tuple>
constexpr auto find_specialisation_v = find_specialisation<T, Tuple>::value;

/** Evaluates to the count of the specialisations of @a T in @a Tuple.
 *
 * @code
 * count_specialisation<std::vector, std::tuple<int, std::deque<int>, double>>::value  // 0
 * count_specialisation<std::vector, std::tuple<int, std::vector<int>, double>>::value // 1
 * count_specialisation<std::vector, std::tuple<int, std::vector<int>, std::vector<double>>>::value // 2
 * @endcode
 * 
 * @tparam T Specialisation to search for
 * @tparam Tuple Types to search (may be empty)
 */
template <template <typename...> typename T, typename Tuple>
class count_specialisation
{
    template <typename U>
    using fn = traits::is_specialisation_of<U, T>;

public:
    constexpr static std::size_t value = boost::mp11::mp_count_if<Tuple, fn>::value;
};

/** Helper variable for count_specialisation.
 *
 * @tparam T Specialisation to search for
 * @tparam Tuple Types to search (may be empty)
 */
template <template <typename...> typename T, typename Tuple>
constexpr auto count_specialisation_v = count_specialisation<T, Tuple>::value;

/** Same as count_specialisation but can accept a specialised type, effectively despecialsing it
 * before comparison.
 *
 * @tparam T Type to search for
 * @tparam Tuple Types to search (may be empty)
 */
template <typename T, typename Tuple>
class count_despecialised
{
    template <typename U>
    using fn = traits::is_same_when_despecialised<U, T>;

public:
    constexpr static std::size_t value = boost::mp11::mp_count_if<Tuple, fn>::value;
};

/** Helper variable for count_despecialised.
 *
 * @tparam T Type to search for
 * @tparam Tuple Types to search (may be empty)
 */
template <typename T, typename Tuple>
constexpr auto count_despecialised_v = count_despecialised<T, Tuple>::value;

/** Evaluates to true if @a Tuple contains a specialisation if @a T.
 *
 * @code
 * has_specialisation<std::vector, std::tuple<int, std::deque<int>, double>> // false
 * has_specialisation<std::vector, std::tuple<int, std::vector<int>, double>> // true
 * @endcode
 * 
 * @tparam T Specialisation to search for
 * @tparam Tuple Types to search (may be empty)
 */
template <template <typename...> typename T, typename Tuple>
struct has_specialisation {
    /** True if a specialisation of @a T is in @a Policies. */
    constexpr static bool value = find_specialisation_v<T, Tuple> < std::tuple_size_v<Tuple>;
};

/** Helper variable for has_specialisation.
 *
 * @tparam T Specialisation to search for
 * @tparam Tuple Types to search (may be empty)
 */
template <template <typename...> typename T, typename Tuple>
constexpr auto has_specialisation_v = has_specialisation<T, Tuple>::value;

/** Tuple zipper.
 *  
 * Zips together two equal sized tuple-like types to form another tuple where each element is the
 * pair of the equivalent elements in @a First and @a Second.
 *
 * @tparam First First tuple
 * @tparam Second Second tuple
 */
template <typename First, typename Second>
class zip
{
    static_assert((std::tuple_size_v<First> == std::tuple_size_v<Second>),
                  "First and Second tuples must contain the same number of elements");

    template <std::size_t... N>
    [[nodiscard]] constexpr static auto zip_impl_t(
        std::integer_sequence<std::size_t, N...>) noexcept
        -> std::tuple<
            std::pair<std::tuple_element_t<N, First>, std::tuple_element_t<N, Second>>...>;

public:
    /** Zipped tuple type. */
    using type =
        decltype(zip_impl_t(std::declval<std::make_index_sequence<std::tuple_size_v<First>>>()));
};

/** Helper alias for zip.
 *
 * @tparam First First tuple
 * @tparam Second Second tuple
 */
template <typename First, typename Second>
using zip_t = typename zip<First, Second>::type;

/** Unzips the zipped tuple @a T (i.e. a tuple of pairs) into its two constituent tuples.
 *
 * @tparam T Zipped tuple type
 */
template <typename T>
struct unzip {
    /** First element of each zipped pair. */
    using first_type = boost::mp11::mp_transform<boost::mp11::mp_first, T>;

    /** Second element of each zipped pair. */
    using second_type = boost::mp11::mp_transform<boost::mp11::mp_second, T>;
};

namespace detail
{
template <typename U, typename... I>
[[nodiscard]] constexpr auto tuple_filter_and_construct_impl(U&& input, std::tuple<I...>) noexcept
{
    return std::tuple{std::get<I::value>(std::forward<U>(input))...};
}
}  // namespace detail

/** Moves (or copies if unable to) elements from @a input if their type passes @a Fn, and constructs
 * a tuple instance from them.
 * 
 * The key point of this function is that this operation is done in a single expression so it works
 * with tuples with elements that are @em not default constructible.
 * @tparam Fn Metafunction type whose value is true for types in @a U wanted in the return
 * @tparam U Tuple-like input type
 * @param input Tuple-like input
 * @return Result tuple
 */
template <template <typename...> typename Fn, typename U>
[[nodiscard]] constexpr auto tuple_filter_and_construct(U&& input) noexcept
{
    // Zip together an index-sequence of U and the elements of it
    using zipped =
        zip_t<boost::mp11::mp_iota_c<std::tuple_size_v<std::decay_t<U>>>, std::decay_t<U>>;

    // Filter out types not approved by Fn, we need to wrap Fn so that it only operates on the
    // second type in the pair (the type from U)
    using wrapped_fn =
        boost::mp11::mp_bind<Fn, boost::mp11::mp_bind<boost::mp11::mp_second, boost::mp11::_1>>;
    using filtered = boost::mp11::mp_filter<wrapped_fn::template fn, zipped>;

    // Fold construct the return value using the zip indices as accessors to input
    return detail::tuple_filter_and_construct_impl(input, typename unzip<filtered>::first_type{});
}

namespace detail
{
template <template <typename...> typename Tuple,
          typename Insert,
          typename... Args,
          std::size_t... I>
[[nodiscard]] constexpr auto tuple_push_back_impl(Tuple<Args...> tuple,
                                                  Insert insert,
                                                  std::integer_sequence<std::size_t, I...>) noexcept
{
    return Tuple{std::move(std::get<I>(tuple))..., std::move(insert)};
}

// Empty tuple specialisation
template <template <typename...> typename Tuple, typename Insert>
[[nodiscard]] constexpr auto tuple_push_back_impl(Tuple<>,
                                                  Insert insert,
                                                  std::integer_sequence<std::size_t>) noexcept
{
    return Tuple{std::move(insert)};
}
}  // namespace detail

/** Appends @a insert to @a tuple.
 *
 * @tparam Tuple A tuple-like type
 * @tparam Insert The type to append
 * @param tuple Tuple instance
 * @param insert Instance to append
 * @return A Tuple with @a insert appended
 */
template <typename Tuple, typename Insert, std::size_t... I>
[[nodiscard]] constexpr auto tuple_push_back(Tuple tuple, Insert insert) noexcept
{
    return detail::tuple_push_back_impl(std::move(tuple),
                                        std::move(insert),
                                        std::make_index_sequence<std::tuple_size_v<Tuple>>{});
}
}  // namespace algorithm
}  // namespace arg_router
