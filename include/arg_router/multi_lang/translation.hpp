// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/traits.hpp"

namespace arg_router::multi_lang
{
/** Used to define compile-time strings for translation.
 *
 * See multi_lang::root_t for instructions of its use.
 *
 * @note The unspecialised version will trigger a static_assert failure - all language variants
 * known to multi_lang::root_t must be specialised
 *
 * @tparam LanguageID Language ID type, must match one of the IDs used in the template parameters
 * of multi_lang::root_t
 */
template <typename LanguageID>
class translation
{
    static_assert(traits::always_false_v<LanguageID>, "Unhandled language ID");
};
}  // namespace arg_router::multi_lang
