#include "arg_router/policy/validator.hpp"

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
        policy::long_name_t<S_("test")>,
        flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
               policy::long_name_t<S_("test")>>>();

    policy::validation::despecialised_unique_in_owner::check<
        policy::long_name_t<S_("test")>>();
}

BOOST_AUTO_TEST_CASE(policy_unique_from_owner_parent_to_mode_or_rootest)
{
    policy::validation::policy_unique_from_owner_parent_to_mode_or_root<
        arg_router::mode_t>::check<policy::long_name_t<S_("test")>>();

    policy::validation::policy_unique_from_owner_parent_to_mode_or_root<
        arg_router::mode_t>::
        check<policy::long_name_t<S_("test")>,
              flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                     policy::long_name_t<S_("test")>>>();

    policy::validation::policy_unique_from_owner_parent_to_mode_or_root<
        arg_router::mode_t>::
        check<
            policy::long_name_t<S_("test1")>,
            flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                   policy::long_name_t<S_("test1")>>,
            arg_router::mode_t<
                policy::long_name_t<S_("mode1")>,
                flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                       policy::long_name_t<S_("test1")>>>,
            root_t<
                arg_router::mode_t<
                    policy::long_name_t<S_("mode1")>,
                    flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                           policy::long_name_t<S_("test1")>>>,
                arg_router::mode_t<
                    policy::long_name_t<S_("mode2")>,
                    flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                           policy::long_name_t<S_("test1")>>>,
                std::decay_t<
                    decltype(policy::validation::default_validator)>>>();
}

BOOST_AUTO_TEST_CASE(parent_types_test)
{
    policy::validation::
        parent_types<std::pair<traits::integral_constant<0>, flag_t<>>>::check<
            policy::router<std::less<>>,
            flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                   policy::long_name_t<S_("test")>,
                   policy::router<std::less<>>>>();

    policy::validation::
        parent_types<std::pair<traits::integral_constant<1>, root_t<>>>::check<
            policy::router<std::less<>>,
            flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                   policy::long_name_t<S_("test1")>,
                   policy::router<std::less<>>>,
            root_t<flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                          policy::long_name_t<S_("test1")>,
                          policy::router<std::less<>>>,
                   flag_t<policy::short_name_t<traits::integral_constant<'b'>>,
                          policy::long_name_t<S_("test2")>,
                          policy::router<std::less<>>>>>();

    policy::validation::parent_types<
        std::pair<traits::integral_constant<0>, arg_router::mode_t<flag_t<>>>,
        std::pair<traits::integral_constant<1>, root_t<>>>::
        check<
            policy::router<std::less<>>,
            flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                   policy::long_name_t<S_("test1")>,
                   policy::router<std::less<>>>,
            root_t<flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                          policy::long_name_t<S_("test1")>,
                          policy::router<std::less<>>>,
                   flag_t<policy::short_name_t<traits::integral_constant<'b'>>,
                          policy::long_name_t<S_("test2")>,
                          policy::router<std::less<>>>>>();
}

BOOST_AUTO_TEST_CASE(must_have_policy_test)
{
    policy::validation::must_have_policy<policy::long_name_t>::check<
        flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
               policy::long_name_t<S_("test")>>>();
}

BOOST_AUTO_TEST_CASE(must_not_have_policy_test)
{
    policy::validation::must_not_have_policy<policy::required_t>::check<
        flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
               policy::long_name_t<S_("test")>>>();
}

BOOST_AUTO_TEST_CASE(child_must_have_policy_test)
{
    policy::validation::child_must_have_policy<policy::router>::check<root_t<
        flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
               policy::long_name_t<S_("test1")>,
               policy::router<std::less<>>>,
        flag_t<policy::short_name_t<traits::integral_constant<'b'>>,
               policy::long_name_t<S_("test2")>,
               policy::router<std::less<>>>,
        std::decay_t<decltype(policy::validation::default_validator)>>>();
}

BOOST_AUTO_TEST_CASE(single_anonymous_mode_test)
{
    policy::validation::single_anonymous_mode<arg_router::mode_t>::check<root_t<
        arg_router::mode_t<
            policy::long_name_t<S_("mode1")>,
            flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                   policy::long_name_t<S_("test1")>>>,
        arg_router::mode_t<
            policy::long_name_t<S_("mode2")>,
            flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                   policy::long_name_t<S_("test1")>>>,
        std::decay_t<decltype(policy::validation::default_validator)>>>();

    policy::validation::single_anonymous_mode<arg_router::mode_t>::check<root_t<
        arg_router::mode_t<
            policy::long_name_t<S_("mode1")>,
            flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                   policy::long_name_t<S_("test1")>>>,
        arg_router::mode_t<
            flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                   policy::long_name_t<S_("test1")>>>,
        std::decay_t<decltype(policy::validation::default_validator)>>>();

    policy::validation::single_anonymous_mode<arg_router::mode_t>::check<root_t<
        arg_router::mode_t<
            policy::long_name_t<S_("mode1")>,
            flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                   policy::long_name_t<S_("test1")>>>,
        std::decay_t<decltype(policy::validation::default_validator)>>>();

    policy::validation::single_anonymous_mode<arg_router::mode_t>::check<root_t<
        arg_router::mode_t<
            flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                   policy::long_name_t<S_("test1")>>>,
        std::decay_t<decltype(policy::validation::default_validator)>>>();
}

BOOST_AUTO_TEST_CASE(at_least_one_of_policies_test)
{
    policy::validation::at_least_one_of_policies<policy::long_name_t,
                                                 policy::short_name_t>::
        check<flag_t<policy::description_t<S_("desc")>,
                     policy::long_name_t<S_("hello")>>>();

    policy::validation::at_least_one_of_policies<policy::long_name_t,
                                                 policy::short_name_t>::
        check<flag_t<policy::description_t<S_("desc")>,
                     policy::short_name_t<traits::integral_constant<'a'>>>>();

    policy::validation::at_least_one_of_policies<policy::long_name_t,
                                                 policy::short_name_t>::
        check<flag_t<policy::description_t<S_("desc")>,
                     policy::long_name_t<S_("long")>,
                     policy::short_name_t<traits::integral_constant<'s'>>>>();
}

BOOST_AUTO_TEST_CASE(one_of_policies_if_parent_is_not_root_test)
{
    policy::validation::one_of_policies_if_parent_is_not_root<
        policy::required_t,
        policy::default_value>::check<arg_t<int,
                                            policy::description_t<S_("desc")>,
                                            policy::required_t<>>>();

    policy::validation::one_of_policies_if_parent_is_not_root<
        policy::required_t,
        policy::default_value>::check<arg_t<int,
                                            policy::description_t<S_("desc")>,
                                            policy::default_value<int>>>();

    policy::validation::one_of_policies_if_parent_is_not_root<
        policy::required_t,
        policy::default_value>::
        check<arg_t<int,
                    policy::description_t<S_("desc")>,
                    policy::long_name_t<S_("long")>,
                    policy::required_t<>,
                    policy::short_name_t<traits::integral_constant<'s'>>>>();

    policy::validation::one_of_policies_if_parent_is_not_root<
        policy::required_t,
        policy::default_value>::
        check<arg_t<int,
                    policy::description_t<S_("desc")>,
                    policy::long_name_t<S_("long")>,
                    policy::default_value<int>,
                    policy::short_name_t<traits::integral_constant<'s'>>>>();

    policy::validation::one_of_policies_if_parent_is_not_root<
        policy::required_t,
        policy::default_value>::
        check<arg_t<int, policy::description_t<S_("desc")>>,
              root_t<
                  std::decay_t<decltype(policy::validation::default_validator)>,
                  arg_t<int, policy::description_t<S_("desc")>>>>();
}

BOOST_AUTO_TEST_CASE(min_child_count_test)
{
    policy::validation::min_child_count<2>::check<root_t<
        flag_t<policy::long_name_t<S_("test1")>>,
        flag_t<policy::long_name_t<S_("test2")>>,
        std::decay_t<decltype(policy::validation::default_validator)>>>();

    policy::validation::min_child_count<2>::check<root_t<
        flag_t<policy::long_name_t<S_("test1")>>,
        flag_t<policy::long_name_t<S_("test2")>>,
        flag_t<policy::long_name_t<S_("test3")>>,
        std::decay_t<decltype(policy::validation::default_validator)>>>();
}

BOOST_AUTO_TEST_CASE(aliased_must_not_be_in_owner_test)
{
    policy::validation::aliased_must_not_be_in_owner::check<
        policy::alias_t<policy::long_name_t<S_("hello")>>,
        flag_t<policy::short_name_t<traits::integral_constant<'a'>>>>();

    policy::validation::aliased_must_not_be_in_owner::check<
        policy::alias_t<policy::long_name_t<S_("hello")>>,
        flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
               policy::long_name_t<S_("aaaa")>>>();

    policy::validation::aliased_must_not_be_in_owner::check<
        policy::alias_t<policy::long_name_t<S_("hello")>,
                        policy::long_name_t<S_("bbbb")>>,
        flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
               policy::long_name_t<S_("aaaa")>>>();
}

BOOST_AUTO_TEST_CASE(positional_args_must_be_at_end_test)
{
    policy::validation::positional_args_must_be_at_end<positional_arg_t>::check<
        arg_router::mode_t<
            flag_t<policy::long_name_t<S_("test1")>>,
            arg_t<int, policy::long_name_t<S_("test2")>>,
            positional_arg_t<int, policy::long_name_t<S_("test3")>>>>();

    policy::validation::positional_args_must_be_at_end<positional_arg_t>::check<
        arg_router::mode_t<
            flag_t<policy::long_name_t<S_("test1")>>,
            arg_t<int, policy::long_name_t<S_("test2")>>,
            positional_arg_t<int, policy::long_name_t<S_("test3")>>,
            positional_arg_t<int, policy::long_name_t<S_("test4")>>>>();
}

BOOST_AUTO_TEST_CASE(positional_args_must_have_fixed_count_if_not_at_end_test)
{
    policy::validation::positional_args_must_have_fixed_count_if_not_at_end<
        positional_arg_t>::
        check<arg_router::mode_t<
            flag_t<policy::long_name_t<S_("test1")>>,
            arg_t<int, policy::long_name_t<S_("test2")>>,
            positional_arg_t<int, policy::long_name_t<S_("test3")>>>>();

    policy::validation::positional_args_must_have_fixed_count_if_not_at_end<
        positional_arg_t>::
        check<arg_router::mode_t<
            flag_t<policy::long_name_t<S_("test1")>>,
            arg_t<int, policy::long_name_t<S_("test2")>>,
            positional_arg_t<
                int,
                policy::long_name_t<S_("test3")>,
                policy::count_t<traits::integral_constant<std::size_t{1}>>>,
            positional_arg_t<int, policy::long_name_t<S_("test4")>>>>();

    policy::validation::positional_args_must_have_fixed_count_if_not_at_end<
        positional_arg_t>::
        check<arg_router::mode_t<
            flag_t<policy::long_name_t<S_("test1")>>,
            arg_t<int, policy::long_name_t<S_("test2")>>,
            positional_arg_t<
                int,
                policy::long_name_t<S_("test3")>,
                policy::count_t<traits::integral_constant<std::size_t{1}>>>,
            positional_arg_t<
                int,
                policy::long_name_t<S_("test4")>,
                policy::count_t<traits::integral_constant<std::size_t{3}>>>,
            positional_arg_t<int, policy::long_name_t<S_("test5")>>>>();

    policy::validation::positional_args_must_have_fixed_count_if_not_at_end<
        positional_arg_t>::
        check<arg_router::mode_t<
            flag_t<policy::long_name_t<S_("test1")>>,
            arg_t<int, policy::long_name_t<S_("test2")>>,
            positional_arg_t<
                int,
                policy::long_name_t<S_("test3")>,
                policy::min_count_t<traits::integral_constant<std::size_t{1}>>,
                policy::max_count_t<traits::integral_constant<std::size_t{1}>>>,
            positional_arg_t<int, policy::long_name_t<S_("test4")>>>>();
}

BOOST_AUTO_TEST_CASE(validate_counts_test)
{
    policy::validation::validate_counts::check<positional_arg_t<
        int,
        policy::long_name_t<S_("test1")>,
        policy::min_count_t<traits::integral_constant<std::size_t{1}>>,
        policy::max_count_t<traits::integral_constant<std::size_t{1}>>>>();

    policy::validation::validate_counts::check<positional_arg_t<
        int,
        policy::long_name_t<S_("test1")>,
        policy::min_count_t<traits::integral_constant<std::size_t{0}>>,
        policy::max_count_t<traits::integral_constant<std::size_t{0}>>>>();

    policy::validation::validate_counts::check<positional_arg_t<
        int,
        policy::long_name_t<S_("test1")>,
        policy::min_count_t<traits::integral_constant<std::size_t{1}>>,
        policy::max_count_t<traits::integral_constant<std::size_t{3}>>>>();

    policy::validation::validate_counts::check<positional_arg_t<
        int,
        policy::long_name_t<S_("test1")>,
        policy::min_count_t<traits::integral_constant<std::size_t{42}>>,
        policy::max_count_t<traits::integral_constant<std::size_t{84}>>>>();
}

BOOST_AUTO_TEST_CASE(if_count_not_one_value_type_must_support_push_back_test)
{
    policy::validation::if_count_not_one_value_type_must_support_push_back::
        check<positional_arg_t<
            int,
            policy::long_name_t<S_("test1")>,
            policy::count_t<traits::integral_constant<std::size_t{1}>>>>();

    policy::validation::if_count_not_one_value_type_must_support_push_back::
        check<positional_arg_t<
            std::vector<int>,
            policy::long_name_t<S_("test1")>,
            policy::count_t<traits::integral_constant<std::size_t{3}>>>>();

    policy::validation::if_count_not_one_value_type_must_support_push_back::
        check<positional_arg_t<
            int,
            policy::long_name_t<S_("test1")>,
            policy::min_count_t<traits::integral_constant<std::size_t{1}>>,
            policy::max_count_t<traits::integral_constant<std::size_t{1}>>>>();

    policy::validation::if_count_not_one_value_type_must_support_push_back::
        check<positional_arg_t<
            std::vector<int>,
            policy::long_name_t<S_("test1")>,
            policy::min_count_t<traits::integral_constant<std::size_t{3}>>,
            policy::max_count_t<traits::integral_constant<std::size_t{3}>>>>();

    policy::validation::if_count_not_one_value_type_must_support_push_back::
        check<positional_arg_t<
            std::vector<int>,
            policy::long_name_t<S_("test1")>,
            policy::min_count_t<traits::integral_constant<std::size_t{1}>>,
            policy::max_count_t<traits::integral_constant<std::size_t{3}>>>>();
}

BOOST_AUTO_TEST_CASE(cannot_have_fixed_count_of_zero_test)
{
    policy::validation::if_count_not_one_value_type_must_support_push_back::
        check<positional_arg_t<std::vector<int>,
                               policy::long_name_t<S_("test1")>>>();

    policy::validation::if_count_not_one_value_type_must_support_push_back::
        check<positional_arg_t<
            std::vector<int>,
            policy::long_name_t<S_("test1")>,
            policy::min_count_t<traits::integral_constant<std::size_t{0}>>>>();

    policy::validation::if_count_not_one_value_type_must_support_push_back::
        check<positional_arg_t<
            std::vector<int>,
            policy::long_name_t<S_("test1")>,
            policy::min_count_t<traits::integral_constant<std::size_t{3}>>>>();

    policy::validation::if_count_not_one_value_type_must_support_push_back::
        check<positional_arg_t<
            std::vector<int>,
            policy::long_name_t<S_("test1")>,
            policy::max_count_t<traits::integral_constant<std::size_t{0}>>>>();

    policy::validation::if_count_not_one_value_type_must_support_push_back::
        check<positional_arg_t<
            std::vector<int>,
            policy::long_name_t<S_("test1")>,
            policy::max_count_t<traits::integral_constant<std::size_t{3}>>>>();

    policy::validation::if_count_not_one_value_type_must_support_push_back::
        check<positional_arg_t<
            std::vector<int>,
            policy::long_name_t<S_("test1")>,
            policy::min_count_t<traits::integral_constant<std::size_t{1}>>,
            policy::max_count_t<traits::integral_constant<std::size_t{1}>>>>();
}

BOOST_AUTO_TEST_CASE(child_mode_must_be_named_test)
{
    policy::validation::child_mode_must_be_named::check<arg_router::mode_t<
        arg_router::mode_t<policy::long_name_t<S_("mode")>,
                           flag_t<policy::long_name_t<S_("test1")>>>>>();
}

BOOST_AUTO_TEST_CASE(mode_router_requirements_test)
{
    policy::validation::mode_router_requirements<policy::router>::check<
        arg_router::mode_t<policy::router<std::less<>>,
                           policy::long_name_t<S_("mode1")>,
                           flag_t<policy::long_name_t<S_("flag1")>>,
                           flag_t<policy::long_name_t<S_("flag2")>>>>();

    policy::validation::mode_router_requirements<policy::router>::check<
        arg_router::mode_t<
            policy::router<std::less<>>,
            policy::long_name_t<S_("mode1")>,
            arg_router::flag_t<policy::long_name_t<S_("flag1")>>,
            arg_router::mode_t<policy::long_name_t<S_("mode2")>,
                               flag_t<policy::long_name_t<S_("flag2")>>>>>();

    policy::validation::mode_router_requirements<policy::router>::check<
        arg_router::mode_t<
            policy::long_name_t<S_("mode1")>,
            arg_router::mode_t<policy::long_name_t<S_("mode1_1")>,
                               flag_t<policy::long_name_t<S_("flag1")>>>,
            arg_router::mode_t<policy::long_name_t<S_("mode1_2")>,
                               flag_t<policy::long_name_t<S_("flag2")>>>>>();
}

BOOST_AUTO_TEST_CASE(anonymous_mode_cannot_have_mode_childrens_test)
{
    policy::validation::anonymous_mode_cannot_have_mode_children::check<
        arg_router::mode_t<
            policy::router<std::less<>>,
            policy::long_name_t<S_("mode1")>,
            arg_t<int, policy::long_name_t<S_("flag1")>>,
            arg_router::mode_t<policy::router<std::less<>>,
                               policy::long_name_t<S_("mode2")>,
                               flag_t<policy::long_name_t<S_("flag1")>>,
                               flag_t<policy::long_name_t<S_("flag2")>>>>>();

    policy::validation::anonymous_mode_cannot_have_mode_children::check<
        arg_router::mode_t<policy::router<std::less<>>,
                           arg_t<int, policy::long_name_t<S_("flag1")>>,
                           flag_t<policy::long_name_t<S_("flag1")>>>>();
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(despecialised_unique_in_owner_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"

int main() {
    arg_router::policy::validation::despecialised_unique_in_owner::check<
        arg_router::policy::long_name_t<S_("test1")>,
        arg_router::flag_t<
            arg_router::policy::short_name_t<
                arg_router::traits::integral_constant<'a'>>,
            arg_router::policy::long_name_t<S_("test1")>,
            arg_router::policy::long_name_t<S_("test2")>>>();
    return 0;
}
    )",
        "Policy must be present and unique in owner");
}

BOOST_AUTO_TEST_CASE(policy_unique_from_owner_parent_to_mode_or_rootest)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::policy_unique_from_owner_parent_to_mode_or_root<
        arg_router::mode_t>::check<
            policy::long_name_t<S_("test")>,
            flag_t<
                policy::short_name_t<
                    traits::integral_constant<'a'>>,
                policy::long_name_t<S_("test")>>,
            root_t<
                flag_t<
                    policy::short_name_t<
                        traits::integral_constant<'a'>>,
                    policy::long_name_t<S_("test")>,
                    policy::router<std::less<>>>,
                flag_t<
                    policy::long_name_t<S_("test")>,
                    policy::router<std::less<>>>,
                std::decay_t<decltype(policy::validation::default_validator)>>>();
    return 0;
}
    )",
        "Policy must be unique in the parse tree up to the nearest mode or "
        "root");
}

BOOST_AUTO_TEST_CASE(parent_types_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"

using namespace arg_router;
int main() {
    policy::validation::parent_types<>::check<
        policy::router<std::less<>>,
        root_t<
            flag_t<
                policy::short_name_t<
                    traits::integral_constant<'a'>>,
                policy::long_name_t<S_("test")>>,
            policy::router<std::less<>>,
            std::decay_t<decltype(policy::validation::default_validator)>>>();
    return 0;
}
    )",
        "Must be at least one index/parent pair");

    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"

using namespace arg_router;
int main() {
    policy::validation::
        parent_types<std::pair<traits::integral_constant<1>,
                               arg_router::mode_t<flag_t<>>>>::check<
            policy::router<std::less<>>,
            flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                   policy::long_name_t<S_("test1")>,
                   policy::router<std::less<>>>,
            root_t<flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                          policy::long_name_t<S_("test1")>,
                          policy::router<std::less<>>>,
                   flag_t<policy::short_name_t<traits::integral_constant<'b'>>,
                          policy::long_name_t<S_("test2")>,
                          policy::router<std::less<>>>>>();
    return 0;
}
    )",
        "Parent must be one of a set of types");
}

BOOST_AUTO_TEST_CASE(must_have_policy_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::must_have_policy<policy::required_t>::check<
        flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
               policy::long_name_t<S_("test")>>>();
    return 0;
}
    )",
        "T must have this policy");
}

BOOST_AUTO_TEST_CASE(must_not_have_policy_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::must_not_have_policy<policy::required_t>::check<
        flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
               policy::long_name_t<S_("test")>,
               policy::required_t<>>>();
    return 0;
}
    )",
        "T must not have this policy");
}

BOOST_AUTO_TEST_CASE(child_must_have_policy_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::child_must_have_policy<policy::router>::check<
        root_t<flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                    policy::long_name_t<S_("test1")>,
                    policy::router<std::less<>>>,
             flag_t<policy::short_name_t<traits::integral_constant<'b'>>,
                    policy::long_name_t<S_("test2")>>,
             std::decay_t<decltype(policy::validation::default_validator)>>>();
    return 0;
}
    )",
        "All children of T must have this policy");
}

BOOST_AUTO_TEST_CASE(single_anonymous_mode_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::single_anonymous_mode<arg_router::mode_t>::check<root_t<
        arg_router::mode_t<
            flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                   policy::long_name_t<S_("test1")>>>,
        arg_router::mode_t<
            flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                   policy::long_name_t<S_("test1")>>>,
        std::decay_t<decltype(policy::validation::default_validator)>>>();
}
    )",
        "Only one child mode can be anonymous");
}

BOOST_AUTO_TEST_CASE(none_in_at_least_one_of_policies_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::at_least_one_of_policies<
        policy::long_name_t,
        policy::short_name_t>::check<arg_t<int,
                                           policy::description_t<S_("desc")>>>();
    return 0;
}
    )",
        "T must have at least one of the policies");
}

BOOST_AUTO_TEST_CASE(none_in_one_of_policies_if_parent_is_not_root_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::one_of_policies_if_parent_is_not_root<
        policy::required_t,
        policy::default_value>::check<arg_t<int,
                                            policy::description_t<S_("desc")>>>();
    return 0;
}
    )",
        "T must have one of the assigned policies");
}

BOOST_AUTO_TEST_CASE(multiple_in_one_of_policies_if_parent_is_not_root_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::one_of_policies_if_parent_is_not_root<
        policy::required_t,
        policy::default_value>::check<arg_t<int,
                                            policy::required_t<>,
                                            policy::default_value<int>,
                                            policy::description_t<S_("desc")>>>();
    return 0;
}
    )",
        "T must have one of the assigned policies");
}

BOOST_AUTO_TEST_CASE(min_child_count_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::min_child_count<5>::check<root_t<
        flag_t<policy::long_name_t<S_("test1")>>,
        flag_t<policy::long_name_t<S_("test2")>>,
        flag_t<policy::long_name_t<S_("test3")>>,
        std::decay_t<decltype(policy::validation::default_validator)>>>();
    return 0;
}
    )",
        "Minimum child count not reached");
}

BOOST_AUTO_TEST_CASE(must_have_an_owner_aliased_must_not_be_in_owner_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::aliased_must_not_be_in_owner::check<
        policy::alias_t<policy::long_name_t<S_("hello")>>>();
    return 0;
}
    )",
        "Alias must have an owner");
}

BOOST_AUTO_TEST_CASE(single_aliased_must_not_be_in_owner_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::aliased_must_not_be_in_owner::check<
        policy::alias_t<policy::long_name_t<S_("hello")>>,
        flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
               policy::long_name_t<S_("hello")>>>();
    return 0;
}
    )",
        "Alias names cannot appear in owner");
}

BOOST_AUTO_TEST_CASE(multiple_aliased_must_not_be_in_owner_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::aliased_must_not_be_in_owner::check<
        policy::alias_t<policy::long_name_t<S_("hello")>,
                        policy::long_name_t<S_("bbbb")>>,
        flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
               policy::long_name_t<S_("bbbb")>>>();
    return 0;
}
    )",
        "Alias names cannot appear in owner");
}

BOOST_AUTO_TEST_CASE(positional_args_not_at_end_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::positional_args_must_be_at_end<positional_arg_t>::check<
        arg_router::mode_t<
            flag_t<policy::long_name_t<S_("test1")>>,
            positional_arg_t<int, policy::long_name_t<S_("test3")>>,
            arg_t<int, policy::long_name_t<S_("test2")>>>>();
    return 0;
}
    )",
        "Positional args must all appear at the end of nodes/policy list for a "
        "node");
}

BOOST_AUTO_TEST_CASE(positional_args_at_beginning_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::positional_args_must_be_at_end<positional_arg_t>::check<
        arg_router::mode_t<
            positional_arg_t<int, policy::long_name_t<S_("test3")>>,
            flag_t<policy::long_name_t<S_("test1")>>,
            arg_t<int, policy::long_name_t<S_("test2")>>>>();
    return 0;
}
    )",
        "Positional args must all appear at the end of nodes/policy list for a "
        "node");
}

BOOST_AUTO_TEST_CASE(
    positional_args_must_have_fixed_count_if_not_at_end_no_counts_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::positional_args_must_have_fixed_count_if_not_at_end<
        positional_arg_t>::
        check<arg_router::mode_t<
            flag_t<policy::long_name_t<S_("test1")>>,
            arg_t<int, policy::long_name_t<S_("test2")>>,
            positional_arg_t<int, policy::long_name_t<S_("test3")>>,
            positional_arg_t<int, policy::long_name_t<S_("test4")>>>>();
    return 0;
}
    )",
        "Positional args not at the end of the list must have a fixed count");
}

BOOST_AUTO_TEST_CASE(
    positional_args_must_have_fixed_count_if_not_at_end_last_has_count_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::positional_args_must_have_fixed_count_if_not_at_end<
        positional_arg_t>::
        check<arg_router::mode_t<
            flag_t<policy::long_name_t<S_("test1")>>,
            arg_t<int, policy::long_name_t<S_("test2")>>,
            positional_arg_t<int, policy::long_name_t<S_("test3")>>,
            positional_arg_t<
                int,
                policy::long_name_t<S_("test3")>,
                policy::count_t<traits::integral_constant<std::size_t{1}>>>>>();
    return 0;
}
    )",
        "Positional args not at the end of the list must have a fixed count");
}

BOOST_AUTO_TEST_CASE(
    positional_args_must_have_fixed_count_if_not_at_end_min_no_max_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::positional_args_must_have_fixed_count_if_not_at_end<
        positional_arg_t>::
        check<arg_router::mode_t<
            flag_t<policy::long_name_t<S_("test1")>>,
            arg_t<int, policy::long_name_t<S_("test2")>>,
            positional_arg_t<
                int,
                policy::long_name_t<S_("test3")>,
                policy::min_count_t<traits::integral_constant<std::size_t{1}>>>,
            positional_arg_t<int, policy::long_name_t<S_("test4")>>>>();
    return 0;
}
    )",
        "Positional args not at the end of the list must have a fixed count");
}

BOOST_AUTO_TEST_CASE(
    positional_args_must_have_fixed_count_if_not_at_end_max_no_min_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::positional_args_must_have_fixed_count_if_not_at_end<
        positional_arg_t>::
        check<arg_router::mode_t<
            flag_t<policy::long_name_t<S_("test1")>>,
            arg_t<int, policy::long_name_t<S_("test2")>>,
            positional_arg_t<
                int,
                policy::long_name_t<S_("test3")>,
                policy::max_count_t<traits::integral_constant<std::size_t{1}>>>,
            positional_arg_t<int, policy::long_name_t<S_("test4")>>>>();
    return 0;
}
    )",
        "Positional args not at the end of the list must have a fixed count");
}

BOOST_AUTO_TEST_CASE(
    positional_args_must_have_fixed_count_if_not_at_end_unequal_min_max_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::positional_args_must_have_fixed_count_if_not_at_end<
        positional_arg_t>::
        check<arg_router::mode_t<
            flag_t<policy::long_name_t<S_("test1")>>,
            arg_t<int, policy::long_name_t<S_("test2")>>,
            positional_arg_t<
                int,
                policy::long_name_t<S_("test3")>,
                policy::min_count_t<traits::integral_constant<std::size_t{1}>>,
                policy::max_count_t<traits::integral_constant<std::size_t{3}>>>,
            positional_arg_t<int, policy::long_name_t<S_("test4")>>>>();
    return 0;
}
    )",
        "Positional args not at the end of the list must have a fixed count");
}

BOOST_AUTO_TEST_CASE(validate_counts_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::validate_counts::check<positional_arg_t<
        int,
        policy::long_name_t<S_("test1")>,
        policy::min_count_t<traits::integral_constant<std::size_t{2}>>,
        policy::max_count_t<traits::integral_constant<std::size_t{1}>>>>();
    return 0;
}
    )",
        "Minimum count must be less than maximum count");
}

BOOST_AUTO_TEST_CASE(
    if_count_not_one_value_type_must_support_push_back_count_3_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::if_count_not_one_value_type_must_support_push_back::
        check<positional_arg_t<
            int,
            policy::long_name_t<S_("test1")>,
            policy::count_t<traits::integral_constant<std::size_t{3}>>>>();
    return 0;
}
    )",
        "If T does not have a fixed count of 1, then its value_type must have "
        "a push_back() method");
}

BOOST_AUTO_TEST_CASE(
    if_count_not_one_value_type_must_support_push_back_min_max_3_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::if_count_not_one_value_type_must_support_push_back::
        check<positional_arg_t<
            int,
            policy::long_name_t<S_("test1")>,
            policy::min_count_t<traits::integral_constant<std::size_t{3}>>,
            policy::max_count_t<traits::integral_constant<std::size_t{3}>>>>();
    return 0;
}
    )",
        "If T does not have a fixed count of 1, then its value_type must have "
        "a push_back() method");
}

BOOST_AUTO_TEST_CASE(
    if_count_not_one_value_type_must_support_push_back_min_1_max_3_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::if_count_not_one_value_type_must_support_push_back::
        check<positional_arg_t<
            int,
            policy::long_name_t<S_("test1")>,
            policy::min_count_t<traits::integral_constant<std::size_t{1}>>,
            policy::max_count_t<traits::integral_constant<std::size_t{3}>>>>();
    return 0;
}
    )",
        "If T does not have a fixed count of 1, then its value_type must have "
        "a push_back() method");
}

BOOST_AUTO_TEST_CASE(cannot_have_fixed_count_of_zero_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::cannot_have_fixed_count_of_zero::
        check<positional_arg_t<
            std::vector<int>,
            policy::long_name_t<S_("test1")>,
            policy::min_count_t<traits::integral_constant<std::size_t{0}>>,
            policy::max_count_t<traits::integral_constant<std::size_t{0}>>>>();
    return 0;
}
    )",
        "Cannot have a fixed count of zero");
}

BOOST_AUTO_TEST_CASE(child_mode_must_be_named_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::child_mode_must_be_named::check<arg_router::mode_t<
        arg_router::mode_t<flag_t<policy::long_name_t<S_("test1")>>>>>();
    return 0;
}
    )",
        "All child modes must be named");
}

BOOST_AUTO_TEST_CASE(mode_router_requirements_no_router_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::mode_router_requirements<policy::router>::check<
        arg_router::mode_t<policy::long_name_t<S_("mode1")>,
                           flag_t<policy::long_name_t<S_("flag1")>>,
                           flag_t<policy::long_name_t<S_("flag2")>>>>();
    return 0;
}
    )",
        "Mode must have a router or all its children are also modes");
}

BOOST_AUTO_TEST_CASE(mode_router_requirements_no_router_mixed_children_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::mode_router_requirements<policy::router>::check<
        arg_router::mode_t<
            policy::long_name_t<S_("mode1")>,
            arg_router::flag_t<policy::long_name_t<S_("flag1")>>,
            arg_router::mode_t<policy::long_name_t<S_("mode2")>,
                               flag_t<policy::long_name_t<S_("flag2")>>>>>();
    return 0;
}
    )",
        "Mode must have a router or all its children are also modes");
}

BOOST_AUTO_TEST_CASE(mode_router_requirements_router_all_children_modes_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::mode_router_requirements<policy::router>::check<
        arg_router::mode_t<
            policy::long_name_t<S_("mode1")>,
            policy::router<std::less<>>,
            arg_router::mode_t<policy::long_name_t<S_("mode1_1")>,
                               flag_t<policy::long_name_t<S_("flag1")>>>,
            arg_router::mode_t<policy::long_name_t<S_("mode1_2")>,
                               flag_t<policy::long_name_t<S_("flag2")>>>>>();
    return 0;
}
    )",
        "Mode must have a router or all its children are also modes");
}

BOOST_AUTO_TEST_CASE(anonymous_mode_cannot_have_mode_children_named_mode_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::anonymous_mode_cannot_have_mode_children::check<
        arg_router::mode_t<
            policy::router<std::less<>>,
            arg_t<int, policy::long_name_t<S_("flag1")>>,
            arg_router::mode_t<policy::router<std::less<>>,
                               policy::long_name_t<S_("mode2")>,
                               flag_t<policy::long_name_t<S_("flag1")>>,
                               flag_t<policy::long_name_t<S_("flag2")>>>>>();
    return 0;
}
    )",
        "An anonymous mode cannot have any children that are modes");
}

BOOST_AUTO_TEST_CASE(
    anonymous_mode_cannot_have_mode_children_anonymous_mode_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::anonymous_mode_cannot_have_mode_children::check<
        arg_router::mode_t<
            policy::router<std::less<>>,
            arg_t<int, policy::long_name_t<S_("flag1")>>,
            arg_router::mode_t<policy::router<std::less<>>,
                               flag_t<policy::long_name_t<S_("flag1")>>,
                               flag_t<policy::long_name_t<S_("flag2")>>>>>();
    return 0;
}
    )",
        "An anonymous mode cannot have any children that are modes");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
