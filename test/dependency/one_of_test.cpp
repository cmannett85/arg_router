#include "arg_router/dependency/one_of.hpp"
#include "arg_router/arg.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/default_value.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
namespace ard = arg_router::dependency;

BOOST_AUTO_TEST_SUITE(dependency_suite)

BOOST_AUTO_TEST_SUITE(one_of_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(
        is_tree_node_v<
            ard::one_of_t<arg_t<int, policy::long_name_t<S_("arg1")>>,
                          arg_t<double, policy::long_name_t<S_("arg2")>>,
                          policy::default_value<int>>>,
        "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(value_type_test)
{
    {
        using one_of_type =
            ard::one_of_t<arg_t<int, policy::long_name_t<S_("arg1")>>,
                          arg_t<double, policy::long_name_t<S_("arg2")>>,
                          policy::default_value<int>>;
        static_assert(std::is_same_v<typename one_of_type::value_type,
                                     std::variant<int, double>>,
                      "value_type test fail");
    }

    {
        using one_of_type = ard::one_of_t<
            arg_t<int, policy::long_name_t<S_("arg1")>>,
            arg_t<double,
                  policy::long_name_t<S_("arg2")>,
                  policy::alias_t<policy::long_name_t<S_("arg1")>>>,
            policy::default_value<int>>;
        static_assert(std::is_same_v<typename one_of_type::value_type, int>,
                      "value_type test fail");
    }
}

BOOST_AUTO_TEST_CASE(display_name_test)
{
    {
        const auto of = ard::one_of(arg<int>(policy::long_name<S_("arg1")>),
                                    arg<double>(policy::long_name<S_("arg2")>),
                                    policy::required);
        BOOST_CHECK_EQUAL(of.display_name(), "One of: --arg1,--arg2");
    }

    {
        const auto of = ard::one_of(
            arg<int>(policy::long_name<S_("arg1")>),
            arg<double>(policy::long_name<S_("arg2")>,
                        policy::alias(policy::long_name<S_("arg1")>)),
            flag(policy::short_name<'f'>),
            policy::required);
        BOOST_CHECK_EQUAL(of.display_name(), "One of: --arg1,--arg2,-f");
    }
}

BOOST_AUTO_TEST_CASE(match_test)
{
    auto f = [](const auto& node, auto token, auto expected_child_index) {
        using parent_type = std::decay_t<decltype(node)>;
        using children_type = typename parent_type::children_type;
        constexpr auto valid_child_index =
            expected_child_index < std::tuple_size_v<children_type>;

        auto visitor = [&](const auto& child) {
            using child_type = std::decay_t<decltype(child)>;

            if constexpr (valid_child_index) {
                using expected_child_type =
                    std::tuple_element_t<expected_child_index, children_type>;
                BOOST_CHECK((std::is_same_v<child_type, expected_child_type>));
            } else {
                BOOST_CHECK_MESSAGE(false,
                                    "visitor should not have been called");
            }
        };
        const auto result = node.match(token, visitor);
        BOOST_CHECK_EQUAL(result, valid_child_index);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{ard::one_of(arg<int>(policy::long_name<S_("arg1")>),
                                   arg<double>(policy::long_name<S_("arg2")>),
                                   policy::required),
                       parsing::token_type{parsing::prefix_type::LONG, "arg1"},
                       traits::integral_constant<std::size_t{0}>{}},
            std::tuple{ard::one_of(arg<int>(policy::long_name<S_("arg1")>),
                                   arg<double>(policy::long_name<S_("arg2")>),
                                   policy::required),
                       parsing::token_type{parsing::prefix_type::LONG, "arg2"},
                       traits::integral_constant<std::size_t{1}>{}},
            std::tuple{ard::one_of(arg<int>(policy::long_name<S_("arg1")>),
                                   arg<double>(policy::long_name<S_("arg2")>),
                                   policy::required),
                       parsing::token_type{parsing::prefix_type::LONG, "arg3"},
                       traits::integral_constant<std::size_t{42}>{}},
        });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(must_have_two_children_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
namespace ard = arg_router::dependency;

int main() {
    auto f = ard::one_of(arg<int>(policy::long_name<S_("arg1")>));
    return 0;
}
    )",
        "one_of must have at least one two child nodes");
}

BOOST_AUTO_TEST_CASE(cannot_have_long_name_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
namespace ard = arg_router::dependency;

int main() {
    auto f = ard::one_of(arg<int>(policy::long_name<S_("arg1")>),
                         arg<double>(policy::long_name<S_("arg2")>),
                         policy::long_name<S_("one_of")>);
    return 0;
}
    )",
        "one_of must not have a long name policy");
}

BOOST_AUTO_TEST_CASE(cannot_have_short_name_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
namespace ard = arg_router::dependency;

int main() {
    auto f = ard::one_of(arg<int>(policy::long_name<S_("arg1")>),
                         arg<double>(policy::long_name<S_("arg2")>),
                         policy::short_name<'o'>);
    return 0;
}
    )",
        "one_of must not have a short name policy");
}

BOOST_AUTO_TEST_CASE(cannot_have_none_name_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
namespace ard = arg_router::dependency;

int main() {
    auto f = ard::one_of(arg<int>(policy::long_name<S_("arg1")>),
                         arg<double>(policy::long_name<S_("arg2")>),
                         policy::none_name<S_("none")>);
    return 0;
}
    )",
        "one_of must not have a none name policy");
}

BOOST_AUTO_TEST_CASE(all_children_must_be_named_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/policy/default_value.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
namespace ard = arg_router::dependency;

namespace
{
template <typename... Params>
class stub_node : public tree_node<Params...>
{
public:
    using value_type = double;

    constexpr explicit stub_node(Params... params) :
        tree_node<Params...>{std::move(params)...}
    {}
};
} // namespace

int main() {
    auto f = ard::one_of(arg<int>(policy::long_name<S_("arg1")>),
                         arg<bool>(policy::short_name<'b'>),
                         stub_node{},
                         policy::default_value{42});
    return 0;
}
    )",
        "All one_of children must be named");
}

BOOST_AUTO_TEST_CASE(at_least_one_value_type_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
namespace ard = arg_router::dependency;

int main() {
    auto f = ard::one_of(arg<int>(policy::long_name<S_("arg1")>,
                                  policy::alias(policy::long_name<S_("arg2")>)),
                         arg<double>(policy::long_name<S_("arg2")>,
                                     policy::alias(policy::long_name<S_("arg1")>)));
    return 0;
}
    )",
        "one_of must have at least one child with a value_type");
}

BOOST_AUTO_TEST_CASE(missing_missing_phase_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
namespace ard = arg_router::dependency;

int main() {
    auto f = ard::one_of(arg<int>(policy::long_name<S_("arg1")>),
                         arg<double>(policy::long_name<S_("arg2")>));
    return 0;
}
    )",
        "one_of must have a missing phase method, a policy::required or "
        "policy::default_value are commonly used");
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
namespace ard = arg_router::dependency;

int main() {
    const auto of = ard::one_of(
        arg<int>(policy::long_name<S_("arg1")>),
        arg<double>(policy::long_name<S_("arg2")>),
        policy::required,
        policy::alias(policy::long_name<S_("arg3")>));
    return 0;
}
    )",
        "one_of does not support policies with pre-parse, parse, validation, "
        "or routing phases; as it delegates those to its children");
}

BOOST_AUTO_TEST_CASE(parse_phase_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
namespace ard = arg_router::dependency;

int main() {
    const auto of = ard::one_of(
        arg<int>(policy::long_name<S_("arg1")>),
        arg<double>(policy::long_name<S_("arg2")>),
        policy::required,
        policy::custom_parser<std::variant<int, double>>{[](std::string_view) {
            return std::variant<int, double>{}; }});
    return 0;
}
    )",
        "one_of does not support policies with pre-parse, parse, validation, "
        "or routing phases; as it delegates those to its children");
}

BOOST_AUTO_TEST_CASE(validation_phase_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
namespace ard = arg_router::dependency;

int main() {
    const auto of = ard::one_of(
        arg<int>(policy::long_name<S_("arg1")>),
        arg<double>(policy::long_name<S_("arg2")>),
        policy::required,
        policy::min_max_value{42, 84});
    return 0;
}
    )",
        "one_of does not support policies with pre-parse, parse, validation, "
        "or routing phases; as it delegates those to its children");
}

BOOST_AUTO_TEST_CASE(router_phase_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
namespace ard = arg_router::dependency;

int main() {
    const auto of = ard::one_of(
        arg<int>(policy::long_name<S_("arg1")>),
        arg<double>(policy::long_name<S_("arg2")>),
        policy::required,
        policy::router{[](std::variant<int, double>) {}});
    return 0;
}
    )",
        "one_of does not support policies with pre-parse, parse, validation, "
        "or routing phases; as it delegates those to its children");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
