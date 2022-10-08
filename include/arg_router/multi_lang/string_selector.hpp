/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/utility/compile_time_string.hpp"

#include <boost/preprocessor/variadic.hpp>

/** @file
 */

namespace arg_router::multi_lang
{
/** A compile time string container that acts a drop-in replacement for compile_time_string by
 * compile-time selection of an element.
 *
 * @tparam I String index to use
 * @tparam S Strings to contain
 */
template <std::size_t I, typename... S>
using string_selector = std::tuple_element_t<I, std::tuple<S...>>;
}  // namespace arg_router::multi_lang

#define AR_SM_UNPACK(z, n, var_list) S_(BOOST_PP_ARRAY_ELEM(n, var_list))

/** A multi-language equivalent to <TT>S_</TT>, only for use within a multi_lang::root call.
 *
 * Specify the string language variants in the order they are declared in the owning
 * multi_lang::root.
 * @code
 * arp::long_name<SM_("help", "aider", "ayuda")>
 * @endcode
 *
 * The macro creates compile_time_string types from the inputs and puts them into a string_selector.
 * There is no requirement to use this macro, it's just a convenience.
 * @param I Index to select
 */
#define SM_(I, ...)                                                                  \
    arg_router::multi_lang::string_selector<I,                                       \
                                            BOOST_PP_ENUM(                           \
                                                BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), \
                                                AR_SM_UNPACK,                        \
                                                BOOST_PP_VARIADIC_TO_ARRAY(__VA_ARGS__))>
