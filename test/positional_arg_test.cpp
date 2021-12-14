#include "arg_router/positional_arg.hpp"
#include "arg_router/policy/count.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace std::string_literals;

BOOST_AUTO_TEST_SUITE(positional_arg_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(
        is_tree_node_v<arg_router::positional_arg_t<std::vector<int>>>,
        "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(match_test)
{
    auto f = [](const auto& test_node,
                auto token,
                const auto& current_result,
                bool expected_result) {
        using test_node_type = std::decay_t<decltype(test_node)>;

        const auto result = test_node.match(
            token,
            [&](const auto& node) {
                using node_type = std::decay_t<decltype(node)>;
                static_assert(std::is_same_v<node_type, test_node_type>,
                              "match fail");
            },
            current_result);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{positional_arg<std::vector<int>>(
                           policy::long_name<S_("hello")>),
                       parsing::token_type{parsing::prefix_type::NONE, "1"},
                       std::optional<std::vector<int>>{},
                       true},
            std::tuple{positional_arg<std::vector<int>>(
                           policy::long_name<S_("goodbye")>,
                           policy::max_count<3>),
                       parsing::token_type{parsing::prefix_type::NONE, "1"},
                       std::optional<std::vector<int>>{},
                       true},
            std::tuple{positional_arg<std::vector<int>>(
                           policy::long_name<S_("goodbye")>,
                           policy::max_count<3>),
                       parsing::token_type{parsing::prefix_type::NONE, "1"},
                       std::optional{std::vector{42}},
                       true},
            std::tuple{positional_arg<int>(policy::long_name<S_("goodbye")>,
                                           policy::count<1>),
                       parsing::token_type{parsing::prefix_type::NONE, "1"},
                       std::optional<int>{},
                       true},
            std::tuple{positional_arg<std::vector<int>>(
                           policy::long_name<S_("goodbye")>,
                           policy::max_count<3>),
                       parsing::token_type{parsing::prefix_type::NONE, "1"},
                       std::optional{std::vector{42, 43, 44}},
                       false},
            std::tuple{positional_arg<std::vector<int>>(
                           policy::long_name<S_("goodbye")>,
                           policy::max_count<3>),
                       parsing::token_type{parsing::prefix_type::NONE, "1"},
                       std::optional{std::vector{42, 43, 44, 45}},
                       false},
        });
}

BOOST_AUTO_TEST_CASE(parse_test)
{
    auto f = [](const auto& test_node,
                auto tokens,
                auto expected_tokens,
                auto expected_result,
                auto fail_message) {
        try {
            const auto result = test_node.parse(tokens);
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK_EQUAL(result, expected_result);
            BOOST_CHECK_EQUAL(tokens, expected_tokens);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{positional_arg<int>(policy::long_name<S_("node")>,
                                           policy::count<1>),
                       parsing::token_list{{parsing::prefix_type::NONE, "13"}},
                       parsing::token_list{},
                       13,
                       ""s},
            std::tuple{
                positional_arg<std::vector<int>>(policy::long_name<S_("node")>),
                parsing::token_list{{parsing::prefix_type::NONE, "1"},
                                    {parsing::prefix_type::NONE, "2"},
                                    {parsing::prefix_type::NONE, "3"}},
                parsing::token_list{},
                std::vector{1, 2, 3},
                ""s},
            std::tuple{
                positional_arg<std::vector<int>>(policy::long_name<S_("node")>,
                                                 policy::min_count<2>),
                parsing::token_list{{parsing::prefix_type::NONE, "1"},
                                    {parsing::prefix_type::NONE, "2"}},
                parsing::token_list{},
                std::vector{1, 2},
                ""s},
            std::tuple{
                positional_arg<std::vector<int>>(policy::long_name<S_("node")>,
                                                 policy::min_count<2>),
                parsing::token_list{{parsing::prefix_type::NONE, "1"}},
                parsing::token_list{},
                std::vector<int>{},
                "Minimum count not reached: --node"s},
            std::tuple{
                positional_arg<std::vector<int>>(policy::long_name<S_("node")>,
                                                 policy::max_count<2>),
                parsing::token_list{{parsing::prefix_type::NONE, "1"},
                                    {parsing::prefix_type::NONE, "2"}},
                parsing::token_list{},
                std::vector{1, 2},
                ""s},
            std::tuple{
                positional_arg<std::vector<int>>(policy::long_name<S_("node")>,
                                                 policy::max_count<2>),
                parsing::token_list{{parsing::prefix_type::NONE, "1"},
                                    {parsing::prefix_type::NONE, "2"},
                                    {parsing::prefix_type::NONE, "3"}},
                parsing::token_list{{parsing::prefix_type::NONE, "3"}},
                std::vector{1, 2},
                ""s},
        });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(only_policies_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/positional_arg.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = positional_arg<std::vector<int>>(
        policy::long_name<S_("hello")>,
        flag(policy::short_name<'b'>),
        policy::short_name<'H'>
    );
    return 0;
}
    )",
        "Positional args must only contain policies (not other nodes)");
}

BOOST_AUTO_TEST_CASE(push_back_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/long_name.hpp"
#include "arg_router/positional_arg.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = positional_arg<int>(policy::long_name<S_("hello")>);
    return 0;
}
    )",
        "value_type must have a push_back() method");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
