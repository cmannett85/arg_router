// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/multi_lang/string_selector.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(multi_lang_suite)

BOOST_AUTO_TEST_CASE(SM_macro_test)
{
    {
        using type = SM_(0, "hello", "world");
        static_assert(std::is_same_v<type, S_("hello")>, "Test fail");
    }

    {
        using type = SM_(1, "hello", "world");
        static_assert(std::is_same_v<type, S_("world")>, "Test fail");
    }
}

BOOST_AUTO_TEST_SUITE_END()
