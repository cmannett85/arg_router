/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

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
