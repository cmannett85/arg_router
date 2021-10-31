#include "arg_router/mode.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/list.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(mode_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(is_tree_node_v<arg_router::mode_t<flag_t<>>>,
                  "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(anonymous_single_flag_match_test)
{
    const auto m = mode(flag(policy::long_name<S_("hello")>,
                             policy::short_name<'l'>,
                             policy::description<S_("Hello arg")>));

    auto result = m.match({parsing::prefix_type::LONG, "hello"});
    BOOST_CHECK(result);

    result = m.match({parsing::prefix_type::LONG, "goodbye"});
    BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(anonymous_triple_flag_match_test)
{
    const auto m = mode(
        flag(policy::long_name<S_("hello")>,
             policy::short_name<'l'>,
             policy::description<S_("Hello arg")>),
        flag(policy::long_name<S_("foo")>, policy::description<S_("Foo arg")>),
        flag(policy::short_name<'b'>, policy::description<S_("b arg")>));

    auto result = m.match({parsing::prefix_type::LONG, "hello"});
    BOOST_CHECK(result);

    result = m.match({parsing::prefix_type::LONG, "foo"});
    BOOST_CHECK(result);

    result = m.match('b');
    BOOST_CHECK(result);

    result = m.match('g');
    BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(named_single_flag_match_test)
{
    const auto m = mode(policy::long_name<S_("my-mode")>,
                        flag(policy::long_name<S_("hello")>,
                             policy::short_name<'l'>,
                             policy::description<S_("Hello arg")>));

    auto result = m.match({parsing::prefix_type::NONE, "my-mode"});
    BOOST_CHECK(result);

    result = m.match({parsing::prefix_type::LONG, "hello"});
    BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(named_triple_flag_match_test)
{
    const auto m = mode(
        policy::long_name<S_("my-mode")>,
        flag(policy::long_name<S_("hello")>,
             policy::short_name<'l'>,
             policy::description<S_("Hello arg")>),
        flag(policy::long_name<S_("foo")>, policy::description<S_("Foo arg")>),
        flag(policy::short_name<'b'>, policy::description<S_("b arg")>));

    auto result = m.match({parsing::prefix_type::NONE, "my-mode"});
    BOOST_CHECK(result);

    result = m.match('b');
    BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(anonymous_empty_match_test)
{
    const auto m = mode(flag());

    auto result = m.match({parsing::prefix_type::LONG, "my-mode"});
    BOOST_CHECK(!result);

    result = m.match('b');
    BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(named_empty_match_test)
{
    const auto m = mode(policy::long_name<S_("my-mode")>, flag());

    auto result = m.match({parsing::prefix_type::NONE, "my-mode"});
    BOOST_CHECK(result);

    result = m.match('b');
    BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(anonymous_triple_flag_single_list_match_test)
{
    const auto flags = list{
        flag(policy::long_name<S_("hello")>,
             policy::short_name<'l'>,
             policy::description<S_("Hello arg")>),
        flag(policy::long_name<S_("foo")>, policy::description<S_("Foo arg")>),
        flag(policy::short_name<'b'>, policy::description<S_("b arg")>)};
    const auto m = mode(flags);

    auto result = m.match({parsing::prefix_type::LONG, "hello"});
    BOOST_CHECK(result);

    result = m.match({parsing::prefix_type::LONG, "foo"});
    BOOST_CHECK(result);

    result = m.match('b');
    BOOST_CHECK(result);

    result = m.match('g');
    BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(named_triple_flag_double_list_match_test)
{
    const auto list1 = list{
        flag(policy::long_name<S_("hello")>,
             policy::short_name<'l'>,
             policy::description<S_("Hello arg")>),
        flag(policy::long_name<S_("foo")>, policy::description<S_("Foo arg")>)};
    const auto list2 =
        list{flag(policy::short_name<'b'>, policy::description<S_("b arg")>)};
    const auto m = mode(policy::long_name<S_("my-mode")>, list1, list2);

    auto result = m.match({parsing::prefix_type::NONE, "my-mode"});
    BOOST_CHECK(result);

    result = m.match('b');
    BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(no_children_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"

using namespace arg_router;

int main() {
    const auto m = mode();
    return 0;
}
    )",
        "mode_t must have at least one child node");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
