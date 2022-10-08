/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/basic_types.hpp"

namespace arg_router::multi_lang
{
/** Converts a locale name (i.e. a string returned by <TT>std::locale</TT>) into a standardised
 * language code format.
 *
 * The most common locale formats are:
 * @code
 * <Lowercase ISO 639-1 language>_<Uppercase ISO 3166 country>.<encoding>
 * <Lowercase ISO 639-1 language>-<Uppercase ISO 3166 country>
 * <Lowercase ISO 639-1 language>-<Initial uppercase ISO 15924 script>-<Uppercase ISO 3166 country>
 * @endcode
 * This function will strip off the encoding if present, and change the dividing character to an
 * underscore.  For example:
 * @code
 * iso_country_code("en-US") == "en_US"
 * iso_country_code("en_GB.UTF-8") == "en_GB"
 * iso_country_code("fr.UTF-8") == "fr"
 * iso_country_code("uz-Latn-UZ") == "uz_Latn_UZ"
 * @endcode
 * An empty string in yields an empty string out.
 *
 * Typically @a locale_name is under the SSO string size, so allocation doesn't occur.
 *
 * @param locale_name Local name to standardise
 * @return Standardised locale name
 */
inline string iso_locale(std::string_view locale_name)
{
    // No need to worry about UTF-8 here as the codes are required to be ASCII
    // Start by stripping off the encoding
    auto result = string{};
    {
        const auto pos = locale_name.find('.');
        result = locale_name.substr(0, pos);
    }

    // Change hypens to underscores
    for (auto& c : result) {
        if (c == '-') {
            c = '_';
        }
    }

    return result;
}
}  // namespace arg_router::multi_lang
