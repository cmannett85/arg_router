#include "arg_router/policy/validator.hpp"
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
        policy::long_name_t<S_("test")>,
        flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
               policy::long_name_t<S_("test")>>>();

    policy::validation::despecialised_unique_in_owner::check<
        policy::long_name_t<S_("test")>>();
}

BOOST_AUTO_TEST_CASE(policy_unique_from_owner_parent_to_mode_or_root_test)
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
    policy::validation::parent_types<
        policy::validation::parent_index_pair_type<0, flag_t>>::
        check<policy::router<std::less<>>,
              flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                     policy::long_name_t<S_("test")>,
                     policy::router<std::less<>>>>();

    policy::validation::parent_types<
        policy::validation::parent_index_pair_type<1, root_t>>::
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

    policy::validation::parent_types<
        policy::validation::parent_index_pair_type<0, arg_router::mode_t>,
        policy::validation::parent_index_pair_type<1, root_t>>::
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
        check<flag_t<policy::long_name_t<S_("hello")>>>();

    policy::validation::at_least_one_of_policies<policy::long_name_t,
                                                 policy::short_name_t>::
        check<flag_t<policy::short_name_t<traits::integral_constant<'a'>>>>();

    policy::validation::at_least_one_of_policies<policy::long_name_t,
                                                 policy::short_name_t>::
        check<flag_t<policy::long_name_t<S_("long")>,
                     policy::short_name_t<traits::integral_constant<'s'>>>>();
}

BOOST_AUTO_TEST_CASE(one_of_policies_if_parent_is_not_root_test)
{
    policy::validation::one_of_policies_if_parent_is_not_root<
        policy::required_t,
        policy::default_value>::check<arg_t<int,
                                            policy::long_name_t<S_("long")>,
                                            policy::required_t<>>>();

    policy::validation::one_of_policies_if_parent_is_not_root<
        policy::required_t,
        policy::default_value>::check<arg_t<int,
                                            policy::long_name_t<S_("long")>,
                                            policy::default_value<int>>>();

    policy::validation::one_of_policies_if_parent_is_not_root<
        policy::required_t,
        policy::default_value>::
        check<arg_t<int,
                    policy::long_name_t<S_("long")>,
                    policy::required_t<>,
                    policy::short_name_t<traits::integral_constant<'s'>>>>();

    policy::validation::one_of_policies_if_parent_is_not_root<
        policy::required_t,
        policy::default_value>::
        check<arg_t<int,
                    policy::long_name_t<S_("long")>,
                    policy::default_value<int>,
                    policy::short_name_t<traits::integral_constant<'s'>>>>();

    policy::validation::one_of_policies_if_parent_is_not_root<
        policy::required_t,
        policy::default_value>::
        check<arg_t<int, policy::long_name_t<S_("long")>>,
              root_t<
                  std::decay_t<decltype(policy::validation::default_validator)>,
                  arg_t<int, policy::long_name_t<S_("long")>>>>();
}

BOOST_AUTO_TEST_CASE(positional_args_must_be_at_end_test)
{
    policy::validation::
        positional_args_must_be_at_end<arg_router::positional_arg_t>::check<
            arg_router::mode_t<
                flag_t<policy::long_name_t<S_("test1")>>,
                arg_t<int, policy::long_name_t<S_("test2")>>,
                positional_arg_t<std::vector<int>,
                                 policy::long_name_t<S_("test3")>>>>();

    policy::validation::
        positional_args_must_be_at_end<arg_router::positional_arg_t>::check<
            arg_router::mode_t<
                flag_t<policy::long_name_t<S_("test1")>>,
                arg_t<int, policy::long_name_t<S_("test2")>>,
                positional_arg_t<std::vector<int>,
                                 policy::long_name_t<S_("test3")>>,
                positional_arg_t<std::vector<int>,
                                 policy::long_name_t<S_("test4")>>>>();
}

BOOST_AUTO_TEST_CASE(positional_args_must_have_fixed_count_if_not_at_end_test)
{
    policy::validation::positional_args_must_have_fixed_count_if_not_at_end<
        arg_router::positional_arg_t>::
        check<arg_router::mode_t<
            flag_t<policy::long_name_t<S_("test1")>>,
            arg_t<int, policy::long_name_t<S_("test2")>>,
            positional_arg_t<std::vector<int>,
                             policy::long_name_t<S_("test3")>>>>();

    policy::validation::positional_args_must_have_fixed_count_if_not_at_end<
        arg_router::positional_arg_t>::
        check<arg_router::mode_t<
            flag_t<policy::long_name_t<S_("test1")>>,
            arg_t<int, policy::long_name_t<S_("test2")>>,
            positional_arg_t<
                int,
                policy::long_name_t<S_("test3")>,
                policy::count_t<traits::integral_constant<std::size_t{1}>>>,
            positional_arg_t<std::vector<int>,
                             policy::long_name_t<S_("test4")>>>>();

    policy::validation::positional_args_must_have_fixed_count_if_not_at_end<
        arg_router::positional_arg_t>::
        check<arg_router::mode_t<
            flag_t<policy::long_name_t<S_("test1")>>,
            arg_t<int, policy::long_name_t<S_("test2")>>,
            positional_arg_t<
                int,
                policy::long_name_t<S_("test3")>,
                policy::count_t<traits::integral_constant<std::size_t{1}>>>,
            positional_arg_t<
                std::vector<int>,
                policy::long_name_t<S_("test4")>,
                policy::count_t<traits::integral_constant<std::size_t{3}>>>,
            positional_arg_t<std::vector<int>,
                             policy::long_name_t<S_("test5")>>>>();

    policy::validation::positional_args_must_have_fixed_count_if_not_at_end<
        arg_router::positional_arg_t>::
        check<arg_router::mode_t<
            flag_t<policy::long_name_t<S_("test1")>>,
            arg_t<int, policy::long_name_t<S_("test2")>>,
            positional_arg_t<
                int,
                policy::long_name_t<S_("test3")>,
                policy::min_count_t<traits::integral_constant<std::size_t{1}>>,
                policy::max_count_t<traits::integral_constant<std::size_t{1}>>>,
            positional_arg_t<std::vector<int>,
                             policy::long_name_t<S_("test4")>>>>();
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(despecialised_unique_in_owner_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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

BOOST_AUTO_TEST_CASE(policy_unique_from_owner_parent_to_mode_or_root_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::
        policy_unique_from_owner_parent_to_mode_or_root<arg_router::mode_t>::
            check<
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
                    std::decay_t<decltype(policy::validation::default_validator)>
        >>();
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
#include "arg_router/utility/compile_time_string.hpp"

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
        "Must be at least one parent_index_pair_type");

    test::death_test_compile(
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
#include "arg_router/utility/compile_time_string.hpp"

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
#include "arg_router/utility/compile_time_string.hpp"

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
#include "arg_router/utility/compile_time_string.hpp"

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
#include "arg_router/utility/compile_time_string.hpp"

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
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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

BOOST_AUTO_TEST_CASE(positional_args_not_at_end_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::positional_args_must_be_at_end<positional_arg_t>::check<
        arg_router::mode_t<
            flag_t<policy::long_name_t<S_("test1")>>,
            positional_arg_t<std::vector<int>, policy::long_name_t<S_("test3")>>,
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
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::positional_args_must_be_at_end<positional_arg_t>::check<
        arg_router::mode_t<
            positional_arg_t<std::vector<int>, policy::long_name_t<S_("test3")>>,
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
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::
        positional_args_must_have_fixed_count_if_not_at_end<positional_arg_t>::
            check<arg_router::mode_t<
                flag_t<policy::long_name_t<S_("test1")>>,
                arg_t<int, policy::long_name_t<S_("test2")>>,
                positional_arg_t<std::vector<int>, policy::long_name_t<S_("test3")>>,
                positional_arg_t<std::vector<int>, policy::long_name_t<S_("test4")>>>>();
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
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::
        positional_args_must_have_fixed_count_if_not_at_end<positional_arg_t>::
            check<arg_router::mode_t<
                flag_t<policy::long_name_t<S_("test1")>>,
                arg_t<int, policy::long_name_t<S_("test2")>>,
                positional_arg_t<std::vector<int>, policy::long_name_t<S_("test3")>>,
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
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::
        positional_args_must_have_fixed_count_if_not_at_end<positional_arg_t>::
            check<arg_router::mode_t<
                flag_t<policy::long_name_t<S_("test1")>>,
                arg_t<int, policy::long_name_t<S_("test2")>>,
                positional_arg_t<
                    std::vector<int>,
                    policy::long_name_t<S_("test3")>,
                    policy::min_count_t<traits::integral_constant<std::size_t{1}>>>,
                positional_arg_t<std::vector<int>, policy::long_name_t<S_("test4")>>>>();
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
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::
        positional_args_must_have_fixed_count_if_not_at_end<positional_arg_t>::
            check<arg_router::mode_t<
                flag_t<policy::long_name_t<S_("test1")>>,
                arg_t<int, policy::long_name_t<S_("test2")>>,
                positional_arg_t<
                    std::vector<int>,
                    policy::long_name_t<S_("test3")>,
                    policy::max_count_t<traits::integral_constant<std::size_t{1}>>>,
                positional_arg_t<std::vector<int>, policy::long_name_t<S_("test4")>>>>();
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
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    policy::validation::
        positional_args_must_have_fixed_count_if_not_at_end<positional_arg_t>::
            check<arg_router::mode_t<
                flag_t<policy::long_name_t<S_("test1")>>,
                arg_t<int, policy::long_name_t<S_("test2")>>,
                positional_arg_t<
                    std::vector<int>,
                    policy::long_name_t<S_("test3")>,
                    policy::min_count_t<traits::integral_constant<std::size_t{1}>>,
                    policy::max_count_t<traits::integral_constant<std::size_t{3}>>>,
                positional_arg_t<std::vector<int>, policy::long_name_t<S_("test4")>>>>();
    return 0;
}
    )",
        "Positional args not at the end of the list must have a fixed count");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
