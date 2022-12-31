// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/utility/type_hash.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/validator.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

namespace
{
using primitive_types =
    std::tuple<char, std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t, float, double>;
}

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_SUITE(type_hash_suite)

BOOST_AUTO_TEST_CASE(negative_primitives_test)
{
    utility::tuple_type_iterator<primitive_types>([](auto i) {
        utility::tuple_type_iterator<primitive_types>([i](auto j) {
            using first_type = std::tuple_element_t<i.value, primitive_types>;
            using second_type = std::tuple_element_t<j.value, primitive_types>;

            if constexpr (i != j) {
                BOOST_CHECK_NE(utility::type_hash<first_type>(), utility::type_hash<second_type>());
            } else {
                BOOST_CHECK_EQUAL(utility::type_hash<first_type>(),
                                  utility::type_hash<second_type>());
            }
        });
    });
}

BOOST_AUTO_TEST_CASE(positive_primitives_test)
{
    utility::tuple_type_iterator<primitive_types>([](auto i) {
        using primitive_type = std::tuple_element_t<i, primitive_types>;

        BOOST_CHECK_EQUAL(utility::type_hash<primitive_type>(),
                          utility::type_hash<primitive_type>());
    });
}

BOOST_AUTO_TEST_CASE(node_test)
{
    const auto a = root(mode(arg<int>(policy::long_name<AR_STRING("hello")>,
                                      policy::required,
                                      policy::description<AR_STRING("Hello description")>),
                             policy::router{[&](auto) {}}),
                        policy::validation::default_validator);
    const auto b = root(mode(arg<int>(policy::long_name<AR_STRING("goodbye")>,
                                      policy::required,
                                      policy::description<AR_STRING("Hello description")>),
                             policy::router{[&](auto) {}}),
                        policy::validation::default_validator);

    BOOST_CHECK_NE(utility::type_hash<decltype(a)>(), utility::type_hash<decltype(b)>());
    BOOST_CHECK_EQUAL(utility::type_hash<decltype(a)>(), utility::type_hash<decltype(a)>());
    BOOST_CHECK_EQUAL(utility::type_hash<decltype(b)>(), utility::type_hash<decltype(b)>());
}

BOOST_AUTO_TEST_CASE(const_test)
{
    BOOST_CHECK_NE(utility::type_hash<const int>(), utility::type_hash<int>());
}

BOOST_AUTO_TEST_CASE(alias_test)
{
    using alias_type = std::uint64_t;
    BOOST_CHECK_EQUAL(utility::type_hash<alias_type>(), utility::type_hash<std::uint64_t>());
}

BOOST_AUTO_TEST_CASE(short_name_test)
{
    auto a = flag(policy::short_name<'a'>);
    auto b = flag(policy::short_name<'b'>);
    BOOST_CHECK_NE(utility::type_hash<decltype(a)>(), utility::type_hash<decltype(b)>());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
