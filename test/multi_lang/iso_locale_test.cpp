// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/multi_lang/iso_locale.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(multi_lang_suite)

BOOST_AUTO_TEST_CASE(iso_locale_test)
{
    auto f = [](auto input, auto expected) {
        const auto result = multi_lang::iso_locale(input);
        BOOST_CHECK_EQUAL(result, expected);
    };

    test::data_set(f,
                   {
                       std::tuple{"en-US", "en_US"},
                       std::tuple{"en_GB.UTF-8", "en_GB"},
                       std::tuple{"fr.UTF-8", "fr"},
                       std::tuple{"fr", "fr"},
                       std::tuple{"POSIX", "POSIX"},
                       std::tuple{"C", "C"},
                       std::tuple{"", ""},
                   });
}

BOOST_AUTO_TEST_SUITE_END()
