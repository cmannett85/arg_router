// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/multi_lang/iso_locale.hpp"

namespace arg_router::multi_lang
{
std::string iso_locale(std::string_view locale_name) noexcept
{
    // No need to worry about UTF-8 here as the codes are required to be ASCII
    // Start by stripping off the encoding
    auto result = std::string{};
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
