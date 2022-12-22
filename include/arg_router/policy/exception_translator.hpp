// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/exception.hpp"
#include "arg_router/policy/policy.hpp"
#include "arg_router/utility/tuple_iterator.hpp"

#include <boost/lexical_cast.hpp>

/** Exception message translator policy.
 *
 * Provides the mapping between the internal multi_lang_exception error code to a translated string,
 * and then re-throws with a parse_exception.
 *
 * If no matching translation string can be found, an parse_exception is thrown with a message of
 * the form:
 * @code
 * "Untranslated error code (<EC>): <tokens>"
 * @endcode
 *
 * @tparam TranslationType multi_lang::translation type for the current language
 * @tparam FallbackTranslationType multi_lang::translation type to use as a fall back language if
 * a matching error_code is not found in @a TranslationType.  Set to <TT>void</TT> if no fallback is
 * required
 */
namespace arg_router::policy
{
template <typename TranslationType,
          typename FallbackTranslationType = default_error_code_translations>
class exception_translator_t
{
    template <typename T>
    using get_translations = typename T::error_code_translations;

    template <typename T>
    using get_translations_or_default =
        boost::mp11::mp_eval_if_c<!traits::has_error_code_translations_type_v<T>,  //
                                  std::tuple<>,
                                  get_translations,
                                  T>;

    using translations =
        boost::mp11::mp_append<get_translations_or_default<TranslationType>,
                               get_translations_or_default<FallbackTranslationType>>;

public:
    /** Translates the error code in @a e and re-throws as an parse_exception.
     *
     * @param e Exception to translate
     * @exception parse_exception
     */
    [[noreturn]] static void translate_exception(const multi_lang_exception& e)
    {
        utility::tuple_type_iterator<translations>([&](auto i) {
            using entry_type = std::tuple_element_t<i, translations>;

            constexpr auto this_ec = boost::mp11::mp_front<entry_type>::value;
            using this_msg = boost::mp11::mp_back<entry_type>;

            if (this_ec == e.ec()) {
                throw parse_exception{string{this_msg::get()}, e.tokens()};
            }
        });

        const auto ec = static_cast<std::underlying_type_t<error_code>>(e.ec());

        // We use boost::lexical_cast here instead of std::to_string as arg_router::string may have
        // a non-std::allocator allocator type
        // NOLINTNEXTLINE(boost-use-to-string)
        throw parse_exception{"Untranslated error code (" + boost::lexical_cast<string>(ec) + ")",
                              e.tokens()};
    }
};

/** Constant variable helper.
 *
 * @tparam TranslationType multi_lang::translation type for the current language
 * @tparam FallbackTranslationType multi_lang::translation type to use as a fall back language if
 * a matching error_code is not found in @a TranslationType.  Set to <TT>void</TT> if no fallback is
 * required
 */
template <typename TranslationType,
          typename FallbackTranslationType = default_error_code_translations>
constexpr auto exception_translator =
    exception_translator_t<TranslationType, FallbackTranslationType>{};

template <typename TranslationType, typename FallbackTranslationType>
struct is_policy<exception_translator_t<TranslationType, FallbackTranslationType>> :
    std::true_type {
};
}  // namespace arg_router::policy
