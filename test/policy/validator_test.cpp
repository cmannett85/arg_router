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

BOOST_AUTO_TEST_CASE(parent_type_test)
{
    policy::validation::parent_type<0, flag_t<>>::check<
        policy::router<std::less<>>,
        flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
               policy::long_name_t<S_("test")>,
               policy::router<std::less<>>>>();

    policy::validation::parent_type<1, root_t<>>::check<
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

BOOST_AUTO_TEST_CASE(multiple_named_modes_test)
{
    policy::validation::multiple_named_modes<arg_router::mode_t>::check<root_t<
        arg_router::mode_t<
            policy::long_name_t<S_("mode1")>,
            flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                   policy::long_name_t<S_("test1")>>>,
        arg_router::mode_t<
            policy::long_name_t<S_("mode2")>,
            flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                   policy::long_name_t<S_("test1")>>>,
        std::decay_t<decltype(policy::validation::default_validator)>>>();

    policy::validation::multiple_named_modes<arg_router::mode_t>::check<root_t<
        arg_router::mode_t<
            policy::long_name_t<S_("mode1")>,
            flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                   policy::long_name_t<S_("test1")>>>,
        std::decay_t<decltype(policy::validation::default_validator)>>>();

    policy::validation::multiple_named_modes<arg_router::mode_t>::check<root_t<
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

BOOST_AUTO_TEST_CASE(owner_types_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"

int main() {
    arg_router::policy::validation::parent_type<0>::check<
        arg_router::policy::router<std::less<>>,
        arg_router::root_t<
            arg_router::flag_t<
                arg_router::policy::short_name_t<
                    arg_router::traits::integral_constant<'a'>>,
                arg_router::policy::long_name_t<S_("test")>>,
            arg_router::policy::router<std::less<>>,
            std::decay_t<decltype(arg_router::policy::validation::default_validator)>>>();
    return 0;
}
    )",
        "Must be at least one owner type");

    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"

int main() {
    arg_router::policy::validation::parent_type<0, arg_router::root_t<>>::check<
        arg_router::policy::router<std::less<>>,
        arg_router::flag_t<
            arg_router::policy::short_name_t<
                arg_router::traits::integral_constant<'a'>>,
            arg_router::policy::long_name_t<S_("test")>,
            arg_router::policy::router<std::less<>>>,
        arg_router::root_t<
            arg_router::flag_t<
                arg_router::policy::short_name_t<
                    arg_router::traits::integral_constant<'a'>>,
                arg_router::policy::long_name_t<S_("test")>,
                arg_router::policy::router<std::less<>>>,
            std::decay_t<decltype(arg_router::policy::validation::default_validator)>>>();
    return 0;
}
    )",
        "Policy's owner must be one of a set of specific types");
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

BOOST_AUTO_TEST_CASE(multiple_named_modes_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
using namespace arg_router;

int main() {
    policy::validation::multiple_named_modes<arg_router::mode_t>::check<
        root_t<
            arg_router::mode_t<
                policy::long_name_t<S_("mode1")>,
                flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                       policy::long_name_t<S_("test1")>>>,
            arg_router::mode_t<
                flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                       policy::long_name_t<S_("test1")>>>,
            std::decay_t<decltype(policy::validation::default_validator)>>>();
    return 0;
}
    )",
        "If there are multiple modes, all must be named");
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

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
