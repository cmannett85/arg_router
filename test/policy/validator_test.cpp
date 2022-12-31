// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/policy/validator.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(validator_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::validation::validator<int>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_SUITE(rule_suite)

BOOST_AUTO_TEST_CASE(despecialised_unique_in_owner_test)
{
    policy::validation::despecialised_unique_in_owner::check<
        policy::long_name_t<AR_STRING("test")>,
        flag_t<policy::short_name_t<AR_STRING('a')>, policy::long_name_t<AR_STRING("test")>>>();

    policy::validation::despecialised_unique_in_owner::check<
        policy::long_name_t<AR_STRING("test")>>();
}

BOOST_AUTO_TEST_CASE(policy_unique_from_owner_parent_to_mode_or_root_test)
{
    policy::validation::policy_unique_from_owner_parent_to_mode_or_root<arg_router::mode_t>::check<
        policy::long_name_t<AR_STRING("test")>>();

    policy::validation::policy_unique_from_owner_parent_to_mode_or_root<arg_router::mode_t>::check<
        policy::long_name_t<AR_STRING("test")>,
        flag_t<policy::short_name_t<AR_STRING('a')>, policy::long_name_t<AR_STRING("test")>>>();

    policy::validation::policy_unique_from_owner_parent_to_mode_or_root<arg_router::mode_t>::check<
        policy::long_name_t<AR_STRING("test1")>,
        flag_t<policy::short_name_t<AR_STRING('a')>, policy::long_name_t<AR_STRING("test1")>>,
        arg_router::mode_t<
            policy::none_name_t<AR_STRING("mode1")>,
            flag_t<policy::short_name_t<AR_STRING('a')>, policy::long_name_t<AR_STRING("test1")>>>,
        root_t<arg_router::mode_t<policy::none_name_t<AR_STRING("mode1")>,
                                  flag_t<policy::short_name_t<AR_STRING('a')>,
                                         policy::long_name_t<AR_STRING("test1")>>>,
               arg_router::mode_t<policy::none_name_t<AR_STRING("mode2")>,
                                  flag_t<policy::short_name_t<AR_STRING('a')>,
                                         policy::long_name_t<AR_STRING("test1")>>>,
               std::decay_t<decltype(policy::validation::default_validator)>>>();
}

BOOST_AUTO_TEST_CASE(parent_types_test)
{
    policy::validation::parent_types<policy::validation::parent_index_pair_type<0, flag_t>>::check<
        policy::router<std::less<>>,
        flag_t<policy::short_name_t<AR_STRING('a')>,
               policy::long_name_t<AR_STRING("test")>,
               policy::router<std::less<>>>>();

    policy::validation::parent_types<policy::validation::parent_index_pair_type<1, root_t>>::check<
        policy::router<std::less<>>,
        flag_t<policy::short_name_t<AR_STRING('a')>,
               policy::long_name_t<AR_STRING("test1")>,
               policy::router<std::less<>>>,
        root_t<flag_t<policy::short_name_t<AR_STRING('a')>,
                      policy::long_name_t<AR_STRING("test1")>,
                      policy::router<std::less<>>>,
               flag_t<policy::short_name_t<AR_STRING('b')>,
                      policy::long_name_t<AR_STRING("test2")>,
                      policy::router<std::less<>>>>>();

    policy::validation::parent_types<
        policy::validation::parent_index_pair_type<0, arg_router::mode_t>,
        policy::validation::parent_index_pair_type<1, root_t>>::
        check<policy::router<std::less<>>,
              flag_t<policy::short_name_t<AR_STRING('a')>,
                     policy::long_name_t<AR_STRING("test1")>,
                     policy::router<std::less<>>>,
              root_t<flag_t<policy::short_name_t<AR_STRING('a')>,
                            policy::long_name_t<AR_STRING("test1")>,
                            policy::router<std::less<>>>,
                     flag_t<policy::short_name_t<AR_STRING('b')>,
                            policy::long_name_t<AR_STRING("test2")>,
                            policy::router<std::less<>>>>>();
}

BOOST_AUTO_TEST_CASE(must_have_policies_test)
{
    policy::validation::must_have_policies<policy::long_name_t>::check<
        flag_t<policy::short_name_t<AR_STRING('a')>, policy::long_name_t<AR_STRING("test")>>>();

    policy::validation::must_have_policies<policy::long_name_t, policy::description_t>::check<
        flag_t<policy::short_name_t<AR_STRING('a')>,
               policy::description_t<AR_STRING("desc")>,
               policy::long_name_t<AR_STRING("test")>>>();
}

BOOST_AUTO_TEST_CASE(must_not_have_policies_test)
{
    policy::validation::must_not_have_policies<policy::required_t>::check<
        flag_t<policy::short_name_t<AR_STRING('a')>, policy::long_name_t<AR_STRING("test")>>>();

    policy::validation::must_not_have_policies<policy::required_t, policy::description_t>::check<
        flag_t<policy::short_name_t<AR_STRING('a')>, policy::long_name_t<AR_STRING("test")>>>();
}

BOOST_AUTO_TEST_CASE(child_must_have_policy_test)
{
    policy::validation::child_must_have_policy<policy::router>::check<
        root_t<flag_t<policy::short_name_t<AR_STRING('a')>,
                      policy::long_name_t<AR_STRING("test1")>,
                      policy::router<std::less<>>>,
               flag_t<policy::short_name_t<AR_STRING('b')>,
                      policy::long_name_t<AR_STRING("test2")>,
                      policy::router<std::less<>>>,
               std::decay_t<decltype(policy::validation::default_validator)>>>();
}

BOOST_AUTO_TEST_CASE(policy_parent_must_not_have_policy_test)
{
    policy::validation::policy_parent_must_not_have_policy<policy::display_name_t>::check<
        policy::display_name_t<AR_STRING("hello")>,
        flag_t<policy::long_name_t<AR_STRING("hello")>, policy::short_name_t<AR_STRING('a')>>>();
}

BOOST_AUTO_TEST_CASE(single_anonymous_mode_test)
{
    policy::validation::single_anonymous_mode<arg_router::mode_t>::check<root_t<
        arg_router::mode_t<
            policy::none_name_t<AR_STRING("mode1")>,
            flag_t<policy::short_name_t<AR_STRING('a')>, policy::long_name_t<AR_STRING("test1")>>>,
        arg_router::mode_t<
            policy::none_name_t<AR_STRING("mode2")>,
            flag_t<policy::short_name_t<AR_STRING('a')>, policy::long_name_t<AR_STRING("test1")>>>,
        std::decay_t<decltype(policy::validation::default_validator)>>>();

    policy::validation::single_anonymous_mode<arg_router::mode_t>::check<root_t<
        arg_router::mode_t<
            policy::none_name_t<AR_STRING("mode1")>,
            flag_t<policy::short_name_t<AR_STRING('a')>, policy::long_name_t<AR_STRING("test1")>>>,
        arg_router::mode_t<
            flag_t<policy::short_name_t<AR_STRING('a')>, policy::long_name_t<AR_STRING("test1")>>>,
        std::decay_t<decltype(policy::validation::default_validator)>>>();

    policy::validation::single_anonymous_mode<arg_router::mode_t>::check<
        root_t<arg_router::mode_t<policy::none_name_t<AR_STRING("mode1")>,
                                  flag_t<policy::short_name_t<AR_STRING('a')>,
                                         policy::long_name_t<AR_STRING("test1")>>>,
               std::decay_t<decltype(policy::validation::default_validator)>>>();

    policy::validation::single_anonymous_mode<arg_router::mode_t>::check<
        root_t<arg_router::mode_t<flag_t<policy::short_name_t<AR_STRING('a')>,
                                         policy::long_name_t<AR_STRING("test1")>>>,
               std::decay_t<decltype(policy::validation::default_validator)>>>();
}

BOOST_AUTO_TEST_CASE(at_least_one_of_policies_test)
{
    policy::validation::at_least_one_of_policies<policy::long_name_t, policy::short_name_t>::check<
        flag_t<policy::long_name_t<AR_STRING("hello")>>>();

    policy::validation::at_least_one_of_policies<policy::long_name_t, policy::short_name_t>::check<
        flag_t<policy::short_name_t<AR_STRING('a')>>>();

    policy::validation::at_least_one_of_policies<policy::long_name_t, policy::short_name_t>::check<
        flag_t<policy::long_name_t<AR_STRING("long")>, policy::short_name_t<AR_STRING('s')>>>();
}

BOOST_AUTO_TEST_CASE(node_types_must_be_at_end_test)
{
    policy::validation::node_types_must_be_at_end<arg_router::positional_arg_t>::check<
        arg_router::mode_t<
            flag_t<policy::long_name_t<AR_STRING("test1")>>,
            arg_t<int, policy::long_name_t<AR_STRING("test2")>>,
            positional_arg_t<std::vector<int>, policy::display_name_t<AR_STRING("test3")>>>>();

    policy::validation::node_types_must_be_at_end<arg_router::positional_arg_t>::check<
        arg_router::mode_t<
            flag_t<policy::long_name_t<AR_STRING("test1")>>,
            arg_t<int, policy::long_name_t<AR_STRING("test2")>>,
            positional_arg_t<std::vector<int>, policy::display_name_t<AR_STRING("test3")>>,
            positional_arg_t<std::vector<int>, policy::display_name_t<AR_STRING("test4")>>>>();
}

BOOST_AUTO_TEST_CASE(anonymous_mode_must_be_at_end_test)
{
    policy::validation::anonymous_mode_must_be_at_end<arg_router::mode_t>::check<
        root_t<flag_t<policy::long_name_t<AR_STRING("test1")>, policy::router<std::less<>>>,
               arg_t<int, policy::long_name_t<AR_STRING("test2")>, policy::router<std::less<>>>,
               std::decay_t<decltype(policy::validation::default_validator)>>>();

    policy::validation::anonymous_mode_must_be_at_end<arg_router::mode_t>::check<
        root_t<flag_t<policy::long_name_t<AR_STRING("test1")>, policy::router<std::less<>>>,
               arg_t<int, policy::long_name_t<AR_STRING("test2")>, policy::router<std::less<>>>,
               arg_router::mode_t<  //
                   flag_t<policy::long_name_t<AR_STRING("test3")>>>,
               std::decay_t<decltype(policy::validation::default_validator)>>>();

    policy::validation::anonymous_mode_must_be_at_end<arg_router::mode_t>::check<
        root_t<flag_t<policy::long_name_t<AR_STRING("test1")>, policy::router<std::less<>>>,
               arg_router::mode_t<  //
                   policy::none_name_t<AR_STRING("mode1")>,
                   flag_t<policy::long_name_t<AR_STRING("test3")>>>,
               arg_t<int, policy::long_name_t<AR_STRING("test2")>, policy::router<std::less<>>>,
               std::decay_t<decltype(policy::validation::default_validator)>>>();
}

BOOST_AUTO_TEST_CASE(positional_args_must_have_fixed_count_if_not_at_end_test)
{
    policy::validation::
        positional_args_must_have_fixed_count_if_not_at_end<arg_router::positional_arg_t>::check<
            arg_router::mode_t<
                flag_t<policy::long_name_t<AR_STRING("test1")>>,
                arg_t<int, policy::long_name_t<AR_STRING("test2")>>,
                positional_arg_t<std::vector<int>, policy::display_name_t<AR_STRING("test3")>>>>();

    policy::validation::positional_args_must_have_fixed_count_if_not_at_end<
        arg_router::positional_arg_t>::
        check<arg_router::mode_t<
            flag_t<policy::long_name_t<AR_STRING("test1")>>,
            arg_t<int, policy::long_name_t<AR_STRING("test2")>>,
            positional_arg_t<int,
                             policy::display_name_t<AR_STRING("test3")>,
                             policy::min_max_count_t<traits::integral_constant<std::size_t{1}>,
                                                     traits::integral_constant<std::size_t{1}>>>,
            positional_arg_t<std::vector<int>, policy::display_name_t<AR_STRING("test4")>>>>();

    policy::validation::positional_args_must_have_fixed_count_if_not_at_end<
        arg_router::positional_arg_t>::
        check<arg_router::mode_t<
            flag_t<policy::long_name_t<AR_STRING("test1")>>,
            arg_t<int, policy::long_name_t<AR_STRING("test2")>>,
            positional_arg_t<int,
                             policy::display_name_t<AR_STRING("test3")>,
                             policy::min_max_count_t<traits::integral_constant<std::size_t{1}>,
                                                     traits::integral_constant<std::size_t{1}>>>,
            positional_arg_t<std::vector<int>,
                             policy::display_name_t<AR_STRING("test4")>,
                             policy::min_max_count_t<traits::integral_constant<std::size_t{3}>,
                                                     traits::integral_constant<std::size_t{3}>>>,
            positional_arg_t<std::vector<int>, policy::display_name_t<AR_STRING("test5")>>>>();
}

BOOST_AUTO_TEST_CASE(must_have_at_least_min_count_of_1_if_required_test)
{
    policy::validation::must_have_at_least_min_count_of_1_if_required::check<
        positional_arg_t<std::vector<int>, policy::display_name_t<AR_STRING("test1")>>>();

    policy::validation::must_have_at_least_min_count_of_1_if_required::check<
        positional_arg_t<int,
                         policy::display_name_t<AR_STRING("test1")>,
                         policy::min_max_count_t<traits::integral_constant<std::size_t{1}>,
                                                 traits::integral_constant<std::size_t{1}>>>>();

    policy::validation::must_have_at_least_min_count_of_1_if_required::check<
        positional_arg_t<int,
                         policy::required_t<>,
                         policy::display_name_t<AR_STRING("test1")>,
                         policy::min_max_count_t<traits::integral_constant<std::size_t{1}>,
                                                 traits::integral_constant<std::size_t{1}>>>>();

    policy::validation::must_have_at_least_min_count_of_1_if_required::check<
        positional_arg_t<std::vector<int>,
                         policy::required_t<>,
                         policy::display_name_t<AR_STRING("test1")>,
                         policy::min_max_count_t<traits::integral_constant<std::size_t{2}>,
                                                 traits::integral_constant<std::size_t{100}>>>>();
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile(
        {{R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

int main() {
    arg_router::policy::validation::despecialised_unique_in_owner::check<
        arg_router::policy::long_name_t<AR_STRING("test1")>,
        arg_router::flag_t<
            arg_router::policy::short_name_t<AR_STRING('a')>,
            arg_router::policy::long_name_t<AR_STRING("test1")>,
            arg_router::policy::long_name_t<AR_STRING("test2")>>>();
    return 0;
}
    )",
          "Policy must be present and unique in owner",
          "despecialised_unique_in_owner_test"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::
        policy_unique_from_owner_parent_to_mode_or_root<arg_router::mode_t>::
            check<
                policy::long_name_t<AR_STRING("test")>,
                flag_t<
                    policy::short_name_t<
                        AR_STRING('a')>,
                    policy::long_name_t<AR_STRING("test")>>,
                root_t<
                    flag_t<
                        policy::short_name_t<
                            AR_STRING('a')>,
                        policy::long_name_t<AR_STRING("test")>,
                        policy::router<std::less<>>>,
                    flag_t<
                        policy::long_name_t<AR_STRING("test")>,
                        policy::router<std::less<>>>,
                    std::decay_t<decltype(policy::validation::default_validator)>
        >>();
    return 0;
}
    )",
             "Policy must be unique in the parse tree up to the nearest mode or "
             "root",
             "policy_unique_from_owner_parent_to_mode_or_root_test"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
int main() {
    policy::validation::parent_types<>::check<
        policy::router<std::less<>>,
        root_t<
            flag_t<
                policy::short_name_t<
                    AR_STRING('a')>,
                policy::long_name_t<AR_STRING("test")>>,
            policy::router<std::less<>>,
            std::decay_t<decltype(policy::validation::default_validator)>>>();
    return 0;
}
    )",
             "Must be at least one parent_index_pair_type",
             "parent_types_test_1"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
int main() {
    policy::validation::
        parent_types<
            policy::validation::parent_index_pair_type<1, arg_router::mode_t>
            >::check<
            policy::router<std::less<>>,
            flag_t<policy::short_name_t<AR_STRING('a')>,
                   policy::long_name_t<AR_STRING("test1")>,
                   policy::router<std::less<>>>,
            root_t<flag_t<policy::short_name_t<AR_STRING('a')>,
                          policy::long_name_t<AR_STRING("test1")>,
                          policy::router<std::less<>>>,
                   flag_t<policy::short_name_t<AR_STRING('b')>,
                          policy::long_name_t<AR_STRING("test2")>,
                          policy::router<std::less<>>>>>();
    return 0;
}
    )",
             "Parent must be one of a set of types",
             "parent_types_test_2"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::must_have_policies<policy::required_t>::check<
        flag_t<policy::short_name_t<AR_STRING('a')>,
               policy::long_name_t<AR_STRING("test")>>>();
    return 0;
}
    )",
             "T must have all these policies",
             "must_have_policies_test"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::must_have_policies<
        policy::required_t, policy::short_name_t>::check<
            flag_t<policy::short_name_t<AR_STRING('a')>,
                   policy::long_name_t<AR_STRING("test")>>>();
    return 0;
}
    )",
             "T must have all these policies",
             "must_have_policies_multiple_test"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::must_not_have_policies<policy::required_t>::check<
        flag_t<policy::short_name_t<AR_STRING('a')>,
               policy::long_name_t<AR_STRING("test")>,
               policy::required_t<>>>();
    return 0;
}
    )",
             "T must have none of these policies",
             "must_not_have_policies_test"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::must_not_have_policies<
        policy::required_t, policy::long_name_t>::check<
            flag_t<policy::short_name_t<AR_STRING('a')>,
                   policy::long_name_t<AR_STRING("test")>,
                   policy::required_t<>>>();
    return 0;
}
    )",
             "T must have none of these policies",
             "must_not_have_policies_multiple_test"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::child_must_have_policy<policy::long_name_t>::check<
        root_t<flag_t<policy::short_name_t<AR_STRING('a')>,
                      policy::long_name_t<AR_STRING("test1")>,
                      policy::router<std::less<>>>,
               flag_t<policy::short_name_t<AR_STRING('b')>,
                      policy::router<std::less<>>>,
               std::decay_t<decltype(policy::validation::default_validator)>>>();
    return 0;
}
    )",
             "All children of T must have this policy",
             "child_must_have_policy_test"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::
        policy_parent_must_not_have_policy<policy::long_name_t>::check<
            double,
            flag_t<policy::display_name_t<AR_STRING("hello")>,
                   policy::short_name_t<AR_STRING('a')>>>();
}
    )",
             "T must be a policy",
             "not_policy_policy_parent_must_not_have_policy_test"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::
        policy_parent_must_not_have_policy<policy::long_name_t>::check<
            policy::display_name_t<AR_STRING("hello")>>();
}
    )",
             "Must be at least one parent",
             "no_parent_policy_parent_must_not_have_policy_test"},
         {
             R"(
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::
        policy_parent_must_not_have_policy<policy::long_name_t>::check<
            policy::description_t<AR_STRING("hello")>,
            flag_t<policy::description_t<AR_STRING("hello")>,
                   policy::long_name_t<AR_STRING("flag")>>>();
}
    )",
             "Parent must not have this policy",
             "has_policy_policy_parent_must_not_have_policy_test"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::single_anonymous_mode<arg_router::mode_t>::check<root_t<
        arg_router::mode_t<
            flag_t<policy::short_name_t<AR_STRING('a')>,
                   policy::long_name_t<AR_STRING("test1")>>>,
        arg_router::mode_t<
            flag_t<policy::short_name_t<AR_STRING('a')>,
                   policy::long_name_t<AR_STRING("test1")>>>,
        std::decay_t<decltype(policy::validation::default_validator)>>>();
}
    )",
             "Only one child mode can be anonymous",
             "single_anonymous_mode_test"},
         {
             R"(
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::at_least_one_of_policies<
        policy::display_name_t,
        policy::short_name_t>::check<arg_t<int,
                                           policy::long_name_t<AR_STRING("test1")>,
                                           policy::description_t<AR_STRING("desc")>>>();
    return 0;
}
    )",
             "T must have at least one of the policies",
             "none_in_at_least_one_of_policies_test"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::node_types_must_be_at_end<positional_arg_t>::check<
        arg_router::mode_t<
            flag_t<policy::long_name_t<AR_STRING("test1")>>,
            positional_arg_t<std::vector<int>,
                             policy::display_name_t<AR_STRING("test3")>>,
            arg_t<int, policy::long_name_t<AR_STRING("test2")>>>>();
    return 0;
}
    )",
             "Node types must all appear at the end of child list for a node",
             "node_types_must_be_at_end_test"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::anonymous_mode_must_be_at_end<arg_router::mode_t>::check<
        root_t<flag_t<policy::long_name_t<AR_STRING("test1")>, policy::router<std::less<>>>,
               arg_router::mode_t<  //
                   flag_t<policy::long_name_t<AR_STRING("test3")>>>,
               arg_t<int, policy::long_name_t<AR_STRING("test2")>, policy::router<std::less<>>>,
               std::decay_t<decltype(policy::validation::default_validator)>>>();
    return 0;
}
    )",
             "Node types must all appear at the end of child list for a node",
             "anonymous_mode_must_be_at_end_test"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::node_types_must_be_at_end<positional_arg_t>::check<
        arg_router::mode_t<
            positional_arg_t<std::vector<int>,
                             policy::display_name_t<AR_STRING("test3")>>,
            flag_t<policy::long_name_t<AR_STRING("test1")>>,
            arg_t<int, policy::long_name_t<AR_STRING("test2")>>>>();
    return 0;
}
    )",
             "Node types must all appear at the end of child list for a node",
             "positional_args_at_beginning_test"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::
        positional_args_must_have_fixed_count_if_not_at_end<positional_arg_t>::
            check<arg_router::mode_t<
                flag_t<policy::long_name_t<AR_STRING("test1")>>,
                arg_t<int, policy::long_name_t<AR_STRING("test2")>>,
                positional_arg_t<std::vector<int>,
                                 policy::display_name_t<AR_STRING("test3")>>,
                positional_arg_t<std::vector<int>,
                                 policy::display_name_t<AR_STRING("test4")>>>>();
    return 0;
}
    )",
             "Positional args not at the end of the list must have a fixed count",
             "positional_args_must_have_fixed_count_if_not_at_end_no_counts_test"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::
        positional_args_must_have_fixed_count_if_not_at_end<positional_arg_t>::
            check<arg_router::mode_t<
                flag_t<policy::long_name_t<AR_STRING("test1")>>,
                arg_t<int, policy::long_name_t<AR_STRING("test2")>>,
                positional_arg_t<std::vector<int>,
                                 policy::display_name_t<AR_STRING("test3")>>,
                positional_arg_t<
                    int,
                    policy::display_name_t<AR_STRING("test3")>,
                    policy::min_max_count_t<traits::integral_constant<std::size_t{1}>,
                                            traits::integral_constant<std::size_t{1}>>>>>();
    return 0;
}
    )",
             "Positional args not at the end of the list must have a fixed count",
             "positional_args_must_have_fixed_count_if_not_at_end_last_has_count_test"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::
        positional_args_must_have_fixed_count_if_not_at_end<positional_arg_t>::
            check<arg_router::mode_t<
                flag_t<policy::long_name_t<AR_STRING("test1")>>,
                arg_t<int, policy::long_name_t<AR_STRING("test2")>>,
                positional_arg_t<
                    std::vector<int>,
                    policy::display_name_t<AR_STRING("test3")>,
                    std::decay_t<decltype(policy::min_count<1>)>>,
                positional_arg_t<std::vector<int>,
                                 policy::display_name_t<AR_STRING("test4")>>>>();
    return 0;
}
    )",
             "Positional args not at the end of the list must have a fixed count",
             "positional_args_must_have_fixed_count_if_not_at_end_min_no_max_test"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::
        positional_args_must_have_fixed_count_if_not_at_end<positional_arg_t>::
            check<arg_router::mode_t<
                flag_t<policy::long_name_t<AR_STRING("test1")>>,
                arg_t<int, policy::long_name_t<AR_STRING("test2")>>,
                positional_arg_t<
                    std::vector<int>,
                    policy::display_name_t<AR_STRING("test3")>,
                    std::decay_t<decltype(policy::max_count<1>)>>,
                positional_arg_t<std::vector<int>,
                                 policy::display_name_t<AR_STRING("test4")>>>>();
    return 0;
}
    )",
             "Positional args not at the end of the list must have a fixed count",
             "positional_args_must_have_fixed_count_if_not_at_end_max_no_min_test"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::
        positional_args_must_have_fixed_count_if_not_at_end<positional_arg_t>::
            check<arg_router::mode_t<
                flag_t<policy::long_name_t<AR_STRING("test1")>>,
                arg_t<int, policy::long_name_t<AR_STRING("test2")>>,
                positional_arg_t<
                    std::vector<int>,
                    policy::display_name_t<AR_STRING("test3")>,
                    policy::min_max_count_t<traits::integral_constant<std::size_t{1}>,
                                            traits::integral_constant<std::size_t{3}>>>,
                positional_arg_t<std::vector<int>,
                                 policy::display_name_t<AR_STRING("test4")>>>>();
    return 0;
}
    )",
             "Positional args not at the end of the list must have a fixed count",
             "positional_args_must_have_fixed_count_if_not_at_end_unequal_min_max_test"},
         {
             R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::must_have_at_least_min_count_of_1_if_required::check<
        positional_arg_t<std::vector<int>,
        policy::required_t<>,
        policy::display_name_t<AR_STRING("test1")>>>();
    return 0;
}
    )",
             "T must have a minimum count of at least 1 if required (it improves help output)",
             "must_have_at_least_min_count_of_1_if_required_test"}});
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
