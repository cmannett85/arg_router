#include "arg_router/root.hpp"
#include "arg_router/policy/validator.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace std::string_literals;

namespace
{
using default_validator_type =
    std::decay_t<decltype(policy::validation::default_validator)>;
}

BOOST_AUTO_TEST_SUITE(root_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(is_tree_node_v<root<default_validator_type>>,
                  "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(validator_type_test)
{
    static_assert(
        std::is_same_v<typename root<default_validator_type>::validator_type,
                       default_validator_type>,
        "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(constructor_validation_test)
{
    const auto r = root{
        policy::validation::default_validator,
        flag{policy::long_name<S_("hello")>,
             policy::description<S_("This is a hello")>,
             policy::short_name<'h'>,
             policy::router{[]() {}}},
        flag{policy::long_name<S_("goodbye")>,
             policy::description<S_("This is a goodbye flag")>,
             policy::short_name<'g'>,
             policy::router{[]() {}}},
    };

    BOOST_CHECK_EQUAL(std::get<0>(r.children()).long_name(), "hello");
    BOOST_CHECK_EQUAL(std::get<1>(r.children()).long_name(), "goodbye");
}

BOOST_AUTO_TEST_CASE(no_children_parse_test)
{
    const auto r = root{policy::validation::default_validator};

    auto args = std::vector{"foo"};
    BOOST_CHECK_EXCEPTION(r.parse(args.size(), const_cast<char**>(args.data())),
                          std::invalid_argument,
                          [](const auto& e) {
                              return e.what() ==
                                     "Anonymous mode support not added yet!"s;
                          });
}

BOOST_AUTO_TEST_CASE(unknown_argument_parse_test)
{
    auto router_hit = false;
    const auto r = root{flag{policy::long_name<S_("hello")>,
                             policy::description<S_("Hello description")>,
                             policy::router{[&](bool) { router_hit = true; }}},
                        policy::validation::default_validator};

    auto args = std::vector{"foo", "--foo"};
    BOOST_CHECK_EXCEPTION(
        r.parse(args.size(), const_cast<char**>(args.data())),
        std::invalid_argument,
        [](const auto& e) { return e.what() == "Unknown argument: --foo"s; });
    BOOST_CHECK(!router_hit);
}

BOOST_AUTO_TEST_CASE(unhandled_parse_test)
{
    auto router_hit = false;
    const auto r = root{flag{policy::long_name<S_("hello")>,
                             policy::description<S_("Hello description")>,
                             policy::router{[&](bool) { router_hit = true; }}},
                        policy::validation::default_validator};

    auto args = std::vector{"foo", "--hello", "--foo"};
    BOOST_CHECK_EXCEPTION(
        r.parse(args.size(), const_cast<char**>(args.data())),
        std::invalid_argument,
        [](const auto& e) { return e.what() == "Unhandled argument: --foo"s; });
    BOOST_CHECK(!router_hit);
}

BOOST_AUTO_TEST_CASE(single_flag_parse_test)
{
    auto router_hit = false;
    const auto r = root{flag{policy::long_name<S_("hello")>,
                             policy::description<S_("Hello description")>,
                             policy::router{[&](bool) { router_hit = true; }}},
                        policy::validation::default_validator};

    auto args = std::vector{"foo", "--hello"};
    r.parse(args.size(), const_cast<char**>(args.data()));
    BOOST_CHECK(router_hit);
}

BOOST_AUTO_TEST_CASE(triple_flag_parse_test)
{
    auto router_hit1 = false;
    auto router_hit2 = false;
    const auto r = root{flag{policy::long_name<S_("flag1")>,
                             policy::description<S_("First description")>,
                             policy::router{[&](bool) { router_hit1 = true; }}},
                        flag{policy::long_name<S_("flag2")>,
                             policy::description<S_("Second description")>,
                             policy::router{[&](bool) { router_hit2 = true; }}},
                        policy::validation::default_validator};

    auto args = std::vector{"foo", "--flag1"};
    r.parse(args.size(), const_cast<char**>(args.data()));
    BOOST_CHECK(router_hit1);
    BOOST_CHECK(!router_hit2);

    router_hit1 = false;
    router_hit2 = false;
    args = std::vector{"foo", "--flag2"};
    r.parse(args.size(), const_cast<char**>(args.data()));
    BOOST_CHECK(!router_hit1);
    BOOST_CHECK(router_hit2);
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(must_have_validator_policy_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/validator.hpp"

int main() {
        arg_router::root<
            arg_router::flag<
                arg_router::policy::short_name_t<
                    arg_router::traits::integral_constant<'a'>>,
                arg_router::policy::long_name_t<S_("test")>,
                arg_router::policy::router<std::less<>>>>();
    return 0;
}
    )",
        "Root must have a validator policy, use "
        "policy::validation::default_validator unless you have created a "
        "custom one");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
