// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(root_suite)

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({{R"(
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    root_t<
        flag_t<
            policy::short_name_t<str<'a'>>,
            policy::long_name_t<str<"test">>,
            policy::router<std::less<>>>>();
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
                       flag_t<policy::long_name_t<str<"f1">>>>();
    return 0;
}
    )",
                                  "All root children must have routers, unless they have no value",
                                  "single_child_must_have_router_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto m = root(policy::alias(policy::long_name_t{"foo"_S}),
                        flag(policy::long_name_t{"hello"_S}));
    return 0;
}
    )",
                                  "Root does not support policies with any parsing phases",
                                  "pre_parse_phase_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto m = root(policy::custom_parser<int>{[](auto) { return false; }},
                        flag(policy::long_name_t{"hello"_S}));
    return 0;
}
    )",
                                  "Root does not support policies with any parsing phases",
                                  "parse_phase_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto m = root(policy::min_max_value<1, 3>(),
                        flag(policy::long_name_t{"hello"_S}));
    return 0;
}
    )",
                                  "Root does not support policies with any parsing phases",
                                  "validation_phase_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto m = root(policy::router{[](std::string_view) { return true; }},
                        flag(policy::long_name_t{"hello"_S}));
    return 0;
}
    )",
                                  "Root does not support policies with any parsing phases",
                                  "routing_phase_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto m = root(policy::required,
                        flag(policy::long_name_t{"hello"_S}));
    return 0;
}
    )",
                                  "Root does not support policies with any parsing phases",
                                  "missing_phase_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto m = root(policy::long_name_t{"root"_S},
                        policy::validation::default_validator,
                        flag(policy::long_name_t{"hello"_S}));
    return 0;
}
    )",
                                  "Root cannot have name or description policies",
                                  "long_name_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto m = root(policy::short_name_t{"r"_S},
                        policy::validation::default_validator,
                        flag(policy::long_name_t{"hello"_S}));
    return 0;
}
    )",
                                  "Root cannot have name or description policies",
                                  "short_name_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto m = root(policy::display_name_t{"root"_S},
                        policy::validation::default_validator,
                        flag(policy::long_name_t{"hello"_S}));
    return 0;
}
    )",
                                  "Root cannot have name or description policies",
                                  "display_name_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto m = root(policy::none_name_t{"root"_S},
                        policy::validation::default_validator,
                        flag(policy::long_name_t{"hello"_S}));
    return 0;
}
    )",
                                  "Root cannot have name or description policies",
                                  "none_name_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/error_name.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto m = root(policy::error_name_t{"root"_S},
                        policy::validation::default_validator,
                        flag(policy::long_name_t{"hello"_S}));
    return 0;
}
    )",
                                  "Root cannot have name or description policies",
                                  "error_name_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto m = root(policy::description_t{"root"_S},
                        policy::validation::default_validator,
                        flag(policy::long_name_t{"hello"_S}));
    return 0;
}
    )",
                                  "Root cannot have name or description policies",
                                  "description_name_test"}});
}

BOOST_AUTO_TEST_SUITE_END()
