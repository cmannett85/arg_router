// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#undef AR_LONG_PREFIX
#define AR_LONG_PREFIX "-"

#include "arg_router/counting_flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace arg_router::literals;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(counting_flag_suite)

BOOST_AUTO_TEST_CASE(no_short_form_expander_test)
{
    [[maybe_unused]] auto f =
        counting_flag<int>(policy::long_name_t{"hello"_S}, policy::short_name_t{"H"_S});
    static_assert(f.long_name() == "hello"sv, "Long name test fail");
    static_assert(f.short_name() == "H", "Short name test fail");
    static_assert(f.minimum_count() == 0, "Minimum count test fail");
    static_assert(f.maximum_count() == 0, "Maximum count test fail");

    static_assert(
        boost::mp11::mp_none_of_q<typename std::decay_t<decltype(f)>::policies_type,
                                  boost::mp11::mp_bind<traits::is_same_when_despecialised,
                                                       boost::mp11::_1,
                                                       policy::short_form_expander_t<>>>::value,
        "Expected short_form_expander policy");
}

BOOST_AUTO_TEST_SUITE_END()
