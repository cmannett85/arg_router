#pragma once

#include <boost/mp11/algorithm.hpp>

#include <algorithm>
#include <cstddef>
#include <new>
#include <string_view>
#include <type_traits>

// Surprised this is not already done somewhere in mp11, without it using
// std::tuple and mp_list in generic situations doesn't work
namespace std
{
template <typename... T>
struct tuple_size<boost::mp11::mp_list<T...>> :
    std::integral_constant<std::size_t, sizeof...(T)> {
};

template <std::size_t I, typename... T>
struct tuple_element<I, boost::mp11::mp_list<T...>> {
    using type = boost::mp11::mp_at_c<boost::mp11::mp_list<T...>, I>;
};
}  // namespace std

/** Namespace for all arg_router types and functions. */
namespace arg_router
{
/** Types and functions relating to the properties of types. */
namespace traits
{
/** Regardless of @a T, always evaluates to false.
 *
 * @tparam T 'Sink' type
 */
template <typename T>
struct always_false : std::false_type {
};

/** Helper variable for always_false.
 *
 * @tparam T 'Sink' type
 */
template <typename T>
constexpr bool always_false_v = always_false<T>::value;

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
struct is_tuple_like<T, std::void_t<typename std::tuple_size<T>::type>> :
    std::true_type {
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

template <template <typename...> class U, typename... Args>
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
template <typename T, template <typename...> class U>
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
constexpr bool is_same_when_despecialised_v =
    is_same_when_despecialised<T, U>::value;

/** CTAD wraper for std::integral_constant.
 *
 * @tparam Value Primitive value
 */
template <auto Value>
using integral_constant = std::integral_constant<decltype(Value), Value>;

namespace detail
{
template <template <typename...> class T, typename... Args>
constexpr auto arg_extractor_impl(const T<Args...>&) -> std::tuple<Args...>{};

template <template <typename...> class T, typename R, typename... Args>
constexpr auto arg_extractor_impl(const T<R(Args...)>&)
    -> std::tuple<R, Args...>{};

template <typename T>
constexpr auto arg_extractor_impl(const T&) -> std::tuple<>
{
    return {};
};
}  // namespace detail

/** Alias which is a tuple of the template parameters of @a T.
 *
 * @code
 * std::is_same_v<arg_extractor<std::vector<int>>, std::tuple<int, std::allocator<int>>>;
 * std::is_same_v<arg_extractor<std::variant<int, double>>, std::tuple<int, double>>;
 * std::is_same_v<arg_extractor<double>, std::tuple<>>;
 * std::is_same_v<arg_extractor<std::function<void (int, double)>>, std::tuple<void, int, double>>;
 * @endcode
 * @tparam T Type to test
 * @tparam U Unspecialised type, deducted
 */
template <typename T>
using arg_extractor =
    std::decay_t<decltype(detail::arg_extractor_impl(std::declval<T>()))>;

namespace detail
{
template <template <class...> typename Trait,
          typename AlwaysVoid,
          typename... Args>
struct is_detected_impl : std::false_type {
};

template <template <class...> typename Trait, typename... Args>
struct is_detected_impl<Trait, std::void_t<Trait<Args...>>, Args...> :
    std::true_type {
};
}  // namespace detail

/** Alias for determining if a trait is valid with @a T as its template
 * argument.
 *
 * This is primarily used for detecting if a type has a member that supports an
 * expression with an operator:
 * @code
 * template <typename T>
 * using reserve_checker = decltype(std::declval<T&>().reserve(std::declval<std::size_t>()));
 *
 * is_detected<reserve_checker, std::vector<char>>; // Alias for std::true_type
 * is_detected<reserve_checker, std::array<char, 4>>; // Alias for std::false_type
 * @endcode
 * @note The constness of @a T is preserved
 * @tparam Trait Trait to test with @a T
 * @tparam T Type to test against @a Trait
 */
template <template <class...> typename Trait, typename T>
using is_detected = detail::is_detected_impl<Trait, void, T>;

/** Helper variable for is_detected.
 *
 * @tparam Trait Trait to test with @a T
 * @tparam T Type to test against @a Trait
 */
template <template <class...> typename Trait, typename T>
constexpr bool is_detected_v = is_detected<Trait, T>::value;

/** Returns the L1 cache size.
 *
 * @return L1 cache size in bytes
 */
inline constexpr std::size_t l1_cache_size()
{
    // https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size
#ifdef __cpp_lib_hardware_interference_size
    return std::hardware_destructive_interference_size;
#else
    return 2 * sizeof(std::max_align_t);
#endif
}

/** Create a <TT>std::reference_wrapper<T></TT>.
 *
 * @tparam T Type to wrap
 */
template <typename T>
struct add_reference_wrapper {
    using type = std::reference_wrapper<T>;
};

/** Helper alias for add_reference_wrapper.
 *
 * @tparam T Type to wrap
 */
template <typename T>
using add_reference_wrapper_t = typename add_reference_wrapper<T>::type;

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
        std::integer_sequence<std::size_t, I...>) :
        Params{std::get<I>(std::move(params))}...
    {
    }

public:
    constexpr explicit unpack_and_derive(T<Params...> params) :
        unpack_and_derive{std::move(params),
                          std::make_index_sequence<sizeof...(Params)>{}}
    {
    }
};
}  // namespace traits
}  // namespace arg_router
