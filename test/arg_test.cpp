#include "arg_router/arg.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace std::string_literals;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(arg_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(is_tree_node_v<arg_t<int, policy::long_name_t<S_("hello")>>>,
                  "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(policies_test)
{
    auto f = arg<int>(policy::long_name<S_("hello")>, policy::short_name<'H'>);
    static_assert(f.long_name() == "hello"sv, "Long name test fail");
    static_assert(f.short_name() == "H", "Short name test fail");
}

BOOST_AUTO_TEST_CASE(match_test)
{
    auto f = [](const auto& test_node,
                const parsing::token_type& token,
                bool expected_result) {
        using test_node_type = std::decay_t<decltype(test_node)>;

        const auto result = test_node.match(token, [&](const auto& node) {
            using node_type = std::decay_t<decltype(node)>;
            static_assert(std::is_same_v<node_type, test_node_type>,
                          "match fail");
        });
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{arg<int>(policy::long_name<S_("hello")>),
                       parsing::token_type{parsing::prefix_type::LONG, "hello"},
                       true},
            std::tuple{arg<int>(policy::short_name<'h'>),
                       parsing::token_type{parsing::prefix_type::SHORT, "h"},
                       true},
            std::tuple{arg<int>(policy::long_name<S_("hello")>,
                                policy::short_name<'h'>),
                       parsing::token_type{parsing::prefix_type::SHORT, "h"},
                       true},
            std::tuple{arg<int>(policy::long_name<S_("hello")>,
                                policy::short_name<'h'>),
                       parsing::token_type{parsing::prefix_type::LONG, "hello"},
                       true},
            std::tuple{arg<int>(policy::long_name<S_("goodbye")>),
                       parsing::token_type{parsing::prefix_type::LONG, "hello"},
                       false}});
}

BOOST_AUTO_TEST_CASE(parse_test)
{
    auto router_hit = false;
    auto f = [&](auto node,
                 auto tokens,
                 auto expected_tokens,
                 auto expected_result,
                 auto expected_router_hit,
                 auto fail_message,
                 auto... parents) {
        router_hit = false;
        try {
            const auto result = node.parse(tokens, parents...);
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK_EQUAL(result, expected_result);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }

        BOOST_CHECK_EQUAL(tokens.pending_view(), expected_tokens);
        BOOST_CHECK_EQUAL(router_hit, expected_router_hit);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{arg<int>(policy::long_name<S_("test")>),
                       parsing::token_list{{parsing::prefix_type::LONG, "test"},
                                           {parsing::prefix_type::NONE, "42"}},
                       parsing::token_list{},
                       42,
                       false,
                       ""s},
            std::tuple{
                arg<int>(policy::long_name<S_("test")>),
                parsing::token_list{{parsing::prefix_type::LONG, "test"}},
                parsing::token_list{{parsing::prefix_type::LONG, "test"}},
                0,
                false,
                "Missing argument: --test"s},
            std::tuple{
                arg<int>(policy::long_name<S_("test")>,
                         policy::alias(policy::short_name<'h'>,
                                       policy::short_name<'g'>)),
                parsing::token_list{{parsing::prefix_type::LONG, "test"},
                                    {parsing::prefix_type::NONE, "42"}},
                parsing::token_list{{parsing::prefix_type::LONG, "second"},
                                    {parsing::prefix_type::NONE, "42"},
                                    {parsing::prefix_type::SHORT, "g"},
                                    {parsing::prefix_type::NONE, "42"}},
                42,
                false,
                ""s,
                mode(arg<int>(policy::long_name<S_("test")>,
                              policy::alias(policy::short_name<'h'>,
                                            policy::short_name<'g'>)),
                     arg<int>(policy::long_name<S_("second")>,
                              policy::short_name<'h'>),
                     arg<int>(policy::short_name<'g'>))},
            std::tuple{arg<int>(policy::long_name<S_("test")>,
                                policy::router{[&](int result) {
                                    BOOST_CHECK_EQUAL(result, 42);
                                    router_hit = true;
                                }}),
                       parsing::token_list{{parsing::prefix_type::LONG, "test"},
                                           {parsing::prefix_type::NONE, "42"}},
                       parsing::token_list{},
                       42,
                       true,
                       ""s}});
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(only_policies_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/arg.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = arg<int>(
        policy::long_name<S_("hello")>,
        flag(policy::short_name<'b'>),
        policy::short_name<'H'>
    );
    return 0;
}
    )",
        "Args must only contain policies (not other nodes)");
}

BOOST_AUTO_TEST_CASE(must_be_named_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/arg.hpp"

using namespace arg_router;

int main() {
    auto a = arg<int>();
    return 0;
}
    )",
        "Arg must have a long and/or short name policy");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
