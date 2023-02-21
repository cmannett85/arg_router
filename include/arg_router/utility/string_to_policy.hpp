// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/policy/policy.hpp"
#include "arg_router/utility/compile_time_string.hpp"

/** Namespace for converting compile-time strings to policies when used in node factory functions.
 */
namespace arg_router::utility::string_to_policy
{
namespace detail
{
template <typename T>
struct is_second_element_policy_or_child {
    using second_type = boost::mp11::mp_second<T>;
    constexpr static bool value = policy::is_policy_v<second_type> || is_tree_node_v<second_type>;
};

template <typename T>
using string_finder = traits::is_compile_time_string_like<boost::mp11::mp_second<T>>;

template <typename T>
using multi_char_finder = traits::integral_constant<(utility::utf8::count(T::get()) > 1)>;

template <typename T>
using single_char_finder = traits::integral_constant<(utility::utf8::count(T::get()) == 1)>;

template <typename Strings>
struct mapping_fn {
    template <typename Mapping>
    using fn = typename Mapping::template type<Strings>;
};

template <typename U, typename... I>
[[nodiscard]] constexpr auto tuple_builder(U&& input, [[maybe_unused]] std::tuple<I...> Is) noexcept
{
    return std::tuple{std::get<I::value>(std::forward<U>(input))...};
}
}  // namespace detail

/** Maps the policy to use on the first multi-character string, if present.
 *
 * @note For how to use, see convert(Params...)
 * @tparam Policy Policy to use
 */
template <template <typename> typename Policy>
class first_string_mapper
{
    template <typename Strings>
    struct inner {
        using index = boost::mp11::mp_find_if<Strings, detail::multi_char_finder>;
        using string_type = boost::mp11::mp_eval_if_c<index::value == std::tuple_size_v<Strings>,
                                                      void,
                                                      boost::mp11::mp_at,
                                                      Strings,
                                                      index>;
        using type = boost::mp11::
            mp_eval_if_c<std::is_void_v<string_type>, string_type, Policy, string_type>;
    };

public:
    /** Policy type initialised with the compile-time string, or void if not found. */
    template <typename Strings>
    using type = typename inner<Strings>::type;
};

/** Maps the policy to use on the second multi-character string, if present.
 *
 * @note For how to use, see convert(Params...)
 * @tparam Policy Policy to use
 */
template <template <typename> typename Policy>
class second_string_mapper
{
    template <typename Strings>
    struct inner {
        constexpr static std::size_t first_index =
            boost::mp11::mp_find_if<Strings, detail::multi_char_finder>::value;
        using dropped_first_strings =
            boost::mp11::mp_eval_if_c<first_index == std::tuple_size_v<Strings>,
                                      std::tuple<>,
                                      boost::mp11::mp_drop,
                                      Strings,
                                      traits::integral_constant<first_index + 1>>;

        using index = boost::mp11::mp_find_if<dropped_first_strings, detail::multi_char_finder>;
        using string_type =
            boost::mp11::mp_eval_if_c<index::value == std::tuple_size_v<dropped_first_strings>,
                                      void,
                                      boost::mp11::mp_at,
                                      dropped_first_strings,
                                      index>;
        using type = boost::mp11::
            mp_eval_if_c<std::is_void_v<string_type>, string_type, Policy, string_type>;
    };

public:
    /** Policy type initialised with the compile-time string, or void if not found. */
    template <typename Strings>
    using type = typename inner<Strings>::type;
};

/** Maps the policy to use on the first single character string, if present.
 *
 * @note For how to use, see convert(Params...)
 * @tparam Policy Policy to use
 */
template <template <typename> typename Policy>
class single_char_mapper
{
    template <typename Strings>
    struct inner {
        using index = boost::mp11::mp_find_if<Strings, detail::single_char_finder>;
        using string_type = boost::mp11::mp_eval_if_c<index::value == std::tuple_size_v<Strings>,
                                                      void,
                                                      boost::mp11::mp_at,
                                                      Strings,
                                                      index>;
        using type = boost::mp11::
            mp_eval_if_c<std::is_void_v<string_type>, string_type, Policy, string_type>;
    };

public:
    /** Policy type initialised with the compile-time string, or void if not found. */
    template <typename Strings>
    using type = typename inner<Strings>::type;
};

/** Tuple of mappers
 *
 * This for use as the <TT>MappingCollection</TT> template parameter of convert(Params...).
 * @tparam Mappers Pack of mapper types
 */
template <typename... Mappers>
using mappers_collection = std::tuple<std::decay_t<Mappers>...>;

/** Converts the input parameter pack to a tuple where any compile-time strings have been mapped to
 * policies.
 *
 * @tparam MappingCollection Tuple of mappers, which define which strings get mapped to which
 * policies
 * @param params Input parameters destined for a node's factory function
 * @return Tuple of parameters
 */
template <typename... Mappings, typename... Params>
[[nodiscard]] constexpr auto convert(Params&&... params) noexcept
{
    // Zip together an index-sequence of ParamTuple and the elements of it
    using zipped = algorithm::zip_t<boost::mp11::mp_iota_c<sizeof...(Params)>,
                                    std::tuple<std::decay_t<Params>...>>;

    // Get a tuple of the indices of things that are not strings
    using not_string_indices = typename algorithm::unzip<
        boost::mp11::mp_filter_q<boost::mp11::mp_not_fn<detail::string_finder>,
                                 zipped>>::first_type;

    // Get a tuple of the string types.  We don't use the indices here as the string content is a
    // part of the type
    using strings = typename algorithm::unzip<
        boost::mp11::mp_filter<detail::string_finder, zipped>>::second_type;

    // Iterate over the mappings, and pass the strings to each.  Any valid mapping will have a
    // non-void type, use those to populate the new policies tuple
    using mapped_tuple = boost::mp11::mp_remove_if<
        boost::mp11::mp_transform_q<detail::mapping_fn<strings>, std::tuple<Mappings...>>,
        std::is_void>;

    static_assert(std::tuple_size_v<strings> == std::tuple_size_v<mapped_tuple>,
                  "Unhandled bare strings passed");

    return std::tuple_cat(
        mapped_tuple{},
        detail::tuple_builder(std::tuple{std::forward<Params>(params)...}, not_string_indices{}));
}
}  // namespace arg_router::utility::string_to_policy
