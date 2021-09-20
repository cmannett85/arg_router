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
/** Returns true if @a value is an alphanumeric character.
 *
 * A constexpr equivalent to <TT>std::isalnum</TT>.
 * @param value Character to test
 * @return True if alphanumeric
 */
inline constexpr bool is_alnum(char value)
{
    constexpr auto num_range = std::pair{'0', '9'};
    constexpr auto uc_range = std::pair{'A', 'Z'};
    constexpr auto lc_range = std::pair{'a', 'z'};

    return (value >= num_range.first && value <= num_range.second) ||
           (value >= uc_range.first && value <= uc_range.second) ||
           (value >= lc_range.first && value <= lc_range.second);
}

/** Returns true if @a value is a whitespace character.
 * 
 * A constexpr equivalent to <TT>std::isspace</TT>.
 * @param value Character to test
 * @return True if whitespace
 */
inline constexpr bool is_whitespace(char value)
{
    constexpr auto whitespace = std::string_view{" \f\n\r\t\v"};
    return whitespace.find(value) != std::string_view::npos;
}

/** Returns true if @a str contains whitespace.
 *
 * @param str String to test
 * @return True if contains whitespace
 */
inline constexpr bool contains_whitespace(std::string_view str)
{
    for (auto c : str) {
        if (is_whitespace(c)) {
            return true;
        }
    }

    return false;
}

/** True if every despecialised type in @a Tuple is unique.
 *
 * @code
 * is_unique_set<std::tuple<std::vector<double>, float, std::deque<double>>>::value  // True
 * is_unique_set<std::tuple<std::vector<double>, float, std::vector<double>>>::value // False
 * is_unique_set<std::tuple<std::vector<double>, float, std::vector<int>>>::value    // False
 * is_unique_set<std::tuple<std::vector<double>, float, float>>::value               // False
 * @endcode
 *
 * @tparam Tuple Types to check (may be empty)
 */
template <typename Tuple>
class is_unique_set
{
    // Split the non-specialised types from the specialised
    using non_specialised_list =
        boost::mp11::mp_remove_if<Tuple, traits::is_specialisation>;

    using specialised_list =
        boost::mp11::mp_filter<traits::is_specialisation, Tuple>;

public:
    constexpr static bool value =
        (std::tuple_size_v<
             boost::mp11::mp_unique_if<specialised_list,
                                       traits::is_same_when_despecialised>> ==
         std::tuple_size_v<
             specialised_list>)&&(std::
                                      tuple_size_v<boost::mp11::mp_unique<
                                          non_specialised_list>> ==
                                  std::tuple_size_v<non_specialised_list>);
};

/** Helper variable for is_unique_set.
 *
 * @tparam Tuple Types to check (may be empty)
 */
template <typename Tuple>
constexpr bool is_unique_set_v = is_unique_set<Tuple>::value;

/** Evaluates to the index of the specialisation of @a T in @a Tuple.
 *
 * If @a Tuple does not contain a specialisation of @a T then the value is of
 * the size of @a Tuple.
 * @code
 * find_specialisation<std::vector,
 *                     std::tuple<int, std::deque<int>, double>>::value  // 3
 * find_specialisation<std::vector,
 *                     std::tuple<int, std::vector<int>, double>>::value // 1
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
    /** Index of specialisation of @a T in @a Tuple, or tuple size if not
     * present.
     */
    constexpr static std::size_t value =
        boost::mp11::mp_find_if<Tuple, fn>::value;
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
 * count_specialisation<std::vector,
 *                      std::tuple<int, std::deque<int>, double>>::value  // 0
 * count_specialisation<std::vector,
 *                      std::tuple<int, std::vector<int>, double>>::value // 1
 * count_specialisation<std::vector,
 *                      std::tuple<int, std::vector<int>, std::vector<double>>>::value // 2
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
    constexpr static std::size_t value =
        boost::mp11::mp_count_if<Tuple, fn>::value;
};

/** Helper variable for count_specialisation.
 *
 * @tparam T Specialisation to search for
 * @tparam Tuple Types to search (may be empty)
 */
template <template <typename...> typename T, typename Tuple>
constexpr auto count_specialisation_v = count_specialisation<T, Tuple>::value;

/** Same as count_specialisation but can accept a specialised type, effectively
 * despecialsing it before comparison.
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
    constexpr static std::size_t value =
        boost::mp11::mp_count_if<Tuple, fn>::value;
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
 * has_specialisation<std::vector,
 *                    std::tuple<int, std::deque<int>, double>> // false
 * has_specialisation<std::vector,
 *                    std::tuple<int, std::vector<int>, double>> // true
 * @endcode
 * 
 * @tparam T Specialisation to search for
 * @tparam Tuple Types to search (may be empty)
 */
template <template <typename...> typename T, typename Tuple>
struct has_specialisation {
    /** True if a specialisation of @a T is in @a Policies. */
    constexpr static bool value =
        find_specialisation_v<T, Tuple> < std::tuple_size_v<Tuple>;
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
 * Zips together two equal sized tuple-like types to form another tuple where
 * each element is the pair of the equivalent elements in @a First and
 * @a Second.
 *
 * @tparam First First tuple
 * @tparam Second Second tuple
 */
template <typename First, typename Second>
class zip
{
    static_assert(
        (std::tuple_size_v<First> == std::tuple_size_v<Second>),
        "First and Second tuples must contain the same number of elements");

    template <std::size_t... N>
    constexpr static auto zip_impl_t(std::integer_sequence<std::size_t, N...>)
        -> std::tuple<std::pair<std::tuple_element_t<N, First>,
                                std::tuple_element_t<N, Second>>...>;

public:
    /** Zipped tuple type. */
    using type = decltype(zip_impl_t(
        std::declval<std::make_index_sequence<std::tuple_size_v<First>>>()));
};

/** Helper alias for zip.
 *
 * @tparam First First tuple
 * @tparam Second Second tuple
 */
template <typename First, typename Second>
using zip_t = typename zip<First, Second>::type;

/** Unzips the zipped tuple @a T (i.e. a tuple of pairs) into its two
 * constituent tuples.
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
constexpr auto tuple_filter_and_construct_impl(U&& input, std::tuple<I...>)
{
    return std::tuple{std::get<I::value>(input)...};
}

template <template <typename...> typename Tuple,
          typename Insert,
          typename... Args,
          std::size_t... I>
constexpr auto tuple_push_back_impl(Tuple<Args...> tuple,
                                    Insert insert,
                                    std::integer_sequence<std::size_t, I...>)
{
    return Tuple{std::move(std::get<I>(tuple))..., std::move(insert)};
}

// Empty tuple specialisation
template <template <typename...> typename Tuple, typename Insert>
constexpr auto tuple_push_back_impl(Tuple<>,
                                    Insert insert,
                                    std::integer_sequence<std::size_t>)
{
    return Tuple{std::move(insert)};
}
}  // namespace detail

/** Moves (or copies if unable to) elements from @a input if their type passes
 * @a Fn, and constructs a tuple instance from them.
 * 
 * The key point of this function is that this operation is done in a single
 * expression so it works with tuples with elements that are @em not default
 * constructible.
 * @tparam Fn Metafunction type whose value is true for types in @a U wanted in
 * the return
 * @tparam U Tuple-like input type
 * @param input Tuple-like input
 * @return Result tuple
 */
template <template <typename...> typename Fn, typename U>
constexpr auto tuple_filter_and_construct(U&& input)
{
    // Zip together an index-sequence of U and the elements of it
    using zipped = zip_t<boost::mp11::mp_iota_c<std::tuple_size_v<U>>, U>;

    // Filter out types not approved by Fn, we need to wrap Fn so that it only
    // operates on the second type in the pair (the type from U)
    using wrapped_fn = boost::mp11::mp_bind<
        Fn,
        boost::mp11::mp_bind<boost::mp11::mp_second, boost::mp11::_1>>;
    using filtered = boost::mp11::mp_filter<wrapped_fn::template fn, zipped>;

    // Fold construct the return value using the zip indices as accessors to
    // input
    return detail::tuple_filter_and_construct_impl(
        input,
        typename unzip<filtered>::first_type{});
}

/** Appends @a insert to @a tuple.
 *
 * @tparam Tuple A tuple-like type
 * @tparam Insert The type to append
 * @param tuple Tuple instance
 * @param insert Instance to append
 * @return A Tuple with @a insert appended
 */
template <typename Tuple, typename Insert, std::size_t... I>
constexpr auto tuple_push_back(Tuple tuple, Insert insert)
{
    return detail::tuple_push_back_impl(
        std::move(tuple),
        std::move(insert),
        std::make_index_sequence<std::tuple_size_v<Tuple>>{});
}
}  // namespace algorithm
}  // namespace arg_router
