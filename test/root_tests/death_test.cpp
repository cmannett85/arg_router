/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(root_suite)

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({{R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

int main() {
    arg_router::root_t<
        arg_router::flag_t<
            arg_router::policy::short_name_t<S_('a')>,
            arg_router::policy::long_name_t<S_("test")>,
            arg_router::policy::router<std::less<>>>>();
    return 0;
}
    )",
                               "Root must have a validator policy, use "
                               "policy::validation::default_validator unless you have created a "
                               "custom one",
                               "must_have_validator_policy_test"},
                              {
                                  R"(
#include "arg_router/policy/validator.hpp"

using namespace arg_router;

using default_validator_type =
    std::decay_t<decltype(policy::validation::default_validator)>;

int main() {
    arg_router::root_t<default_validator_type>();
    return 0;
}
    )",
                                  "Root must have at least one child",
                                  "must_have_at_least_one_child_test"},
                              {
                                  R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

using default_validator_type =
    std::decay_t<decltype(policy::validation::default_validator)>;

int main() {
    arg_router::root_t<default_validator_type,
                       flag_t<policy::long_name_t<S_("f1")>>>();
    return 0;
}
    )",
                                  "All root children must have routers, unless they have no value",
                                  "single_child_must_have_router_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto m = root(policy::alias(policy::long_name<S_("foo")>),
                        flag(policy::long_name<S_("hello")>));
    return 0;
}
    )",
                                  "Root does not support policies with any parsing phases",
                                  "pre_parse_phase_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto m = root(policy::custom_parser<int>{[](auto) { return false; }},
                        flag(policy::long_name<S_("hello")>));
    return 0;
}
    )",
                                  "Root does not support policies with any parsing phases",
                                  "parse_phase_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto m = root(policy::min_max_value{1, 3},
                        flag(policy::long_name<S_("hello")>));
    return 0;
}
    )",
                                  "Root does not support policies with any parsing phases",
                                  "validation_phase_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto m = root(policy::router{[](std::string_view) { return true; }},
                        flag(policy::long_name<S_("hello")>));
    return 0;
}
    )",
                                  "Root does not support policies with any parsing phases",
                                  "routing_phase_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto m = root(policy::required,
                        flag(policy::long_name<S_("hello")>));
    return 0;
}
    )",
                                  "Root does not support policies with any parsing phases",
                                  "missing_phase_test"}});
}

BOOST_AUTO_TEST_SUITE_END()
