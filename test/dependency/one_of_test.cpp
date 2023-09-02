// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/dependency/one_of.hpp"
#include "arg_router/arg.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/default_value.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/policy/runtime_enable.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace arg_router::literals;
namespace ard = arg_router::dependency;

namespace
{
template <typename... Policies>
class stub_node : public tree_node<Policies...>
{
public:
    using value_type = bool;

    constexpr explicit stub_node(Policies... policies) :
        tree_node<Policies...>{std::move(policies)...}
    {
    }

    template <typename Validator, bool HasTarget, typename... Parents>
    std::optional<parsing::parse_target> pre_parse(
        parsing::pre_parse_data<Validator, HasTarget> pre_parse_data,
        const Parents&... parents) const
    {
        if (return_value) {
            return parsing::parse_target{std::move(pre_parse_data.args()), *this, parents...};
        }

        return {};
    }

    template <typename... Parents>
    [[nodiscard]] value_type parse(parsing::parse_target, const Parents&... parents) const
    {
        static_assert(sizeof...(parents) == 1);
        const auto& parent = std::get<0>(std::tuple{std::cref(parents)...}).get();
        const auto addr = reinterpret_cast<std::ptrdiff_t>(std::addressof(parent));
        BOOST_CHECK_EQUAL(addr, parent_addr);

        return true;
    }

    bool return_value;
    std::ptrdiff_t parent_addr;
};
}  // namespace

BOOST_AUTO_TEST_SUITE(dependency_suite)

BOOST_AUTO_TEST_SUITE(one_of_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(is_tree_node_v<ard::one_of_t<arg_t<int, policy::long_name_t<str<"arg1">>>,
                                               arg_t<double, policy::long_name_t<str<"arg2">>>,
                                               policy::default_value<int>>>,
                  "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(value_type_test)
{
    {
        using one_of_type = ard::one_of_t<arg_t<int, policy::long_name_t<str<"arg1">>>,
                                          arg_t<double, policy::long_name_t<str<"arg2">>>,
                                          policy::default_value<int>>;
        static_assert(std::is_same_v<typename one_of_type::value_type, std::variant<int, double>>,
                      "value_type test fail");
    }

    {
        using one_of_type = ard::one_of_t<arg_t<int, policy::long_name_t<str<"arg1">>>,
                                          arg_t<double,
                                                policy::long_name_t<str<"arg2">>,
                                                policy::alias_t<policy::long_name_t<str<"arg1">>>>,
                                          policy::default_value<int>>;
        static_assert(std::is_same_v<typename one_of_type::value_type, int>,
                      "value_type test fail");
    }
}

BOOST_AUTO_TEST_CASE(name_test)
{
    {
        const auto of = ard::one_of(arg<int>(policy::long_name_t{"arg1"_S}),
                                    arg<double>(policy::long_name_t{"arg2"_S}),
                                    policy::required);
        static_assert(of.display_name() == "One of: ");
        static_assert(of.error_name() == "One of: --arg1,--arg2");
    }

    {
        const auto of = ard::one_of(arg<int>(policy::long_name_t{"arg1"_S}),
                                    arg<double>(policy::long_name_t{"arg2"_S},
                                                policy::alias(policy::long_name_t{"arg1"_S})),
                                    flag(policy::short_name_t{"f"_S}),
                                    policy::required);
        static_assert(of.display_name() == "One of: ");
        static_assert(of.error_name() == "One of: --arg1,--arg2,-f");
    }
}

BOOST_AUTO_TEST_CASE(pre_parse_test)
{
    auto f = [](auto node, auto child_index, auto expected_args, auto expected_result) {
        auto fake_parent = stub_node{policy::long_name_t{"parent"_S}};

        auto& expected_child = std::get<child_index>(node.children());
        expected_child.return_value = expected_result;

        auto& not_expected_child = std::get<(child_index == 0 ? 1 : 0)>(node.children());
        not_expected_child.return_value = false;

        auto expected_args_copy = expected_args;
        auto result = node.pre_parse(parsing::pre_parse_data{expected_args_copy}, fake_parent);
        BOOST_CHECK_EQUAL(!result, !expected_result);

        if (result) {
            BOOST_CHECK(expected_args_copy.empty());
            BOOST_CHECK_EQUAL(expected_args, result->tokens());

            const auto index = utility::type_hash<std::decay_t<decltype(expected_child)>>();
            BOOST_CHECK_EQUAL(result->node_type(), index);

            expected_child.parent_addr =
                reinterpret_cast<std::ptrdiff_t>(std::addressof(fake_parent));
            const auto parse_result = (*result)();
            BOOST_CHECK(parse_result.template get<bool>());
        }
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{ard::one_of(stub_node{policy::long_name_t{"arg1"_S}},
                                   stub_node{policy::long_name_t{"arg2"_S}},
                                   policy::required),
                       traits::integral_constant<0>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "hello1"}},
                       true},
            std::tuple{ard::one_of(stub_node{policy::long_name_t{"arg1"_S}},
                                   stub_node{policy::long_name_t{"arg2"_S}},
                                   policy::required),
                       traits::integral_constant<1>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "hello2"}},
                       true},
            std::tuple{ard::one_of(stub_node{policy::long_name_t{"arg1"_S}},
                                   stub_node{policy::long_name_t{"arg2"_S}},
                                   policy::required),
                       traits::integral_constant<0>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "hello3"}},
                       false},
        });
}

BOOST_AUTO_TEST_CASE(one_of_fail_test)
{
    auto fake_parent = stub_node{policy::long_name_t{"parent"_S}};
    auto node = ard::one_of(stub_node{policy::long_name_t{"arg1"_S}},
                            stub_node{policy::long_name_t{"arg2"_S}},
                            policy::required);

    std::get<0>(node.children()).return_value = true;
    std::get<1>(node.children()).return_value = false;
    auto expected_args = std::vector<parsing::token_type>{{parsing::prefix_type::none, "hello"}};
    auto result = node.pre_parse(parsing::pre_parse_data{expected_args}, fake_parent);
    BOOST_CHECK(result);

    std::get<0>(node.children()).return_value = false;
    std::get<1>(node.children()).return_value = true;
    BOOST_CHECK_EXCEPTION(  //
        (void)node.pre_parse(parsing::pre_parse_data{expected_args}, fake_parent),
        multi_lang_exception,
        [](const auto& e) {
            return (e.ec() == error_code::one_of_selected_type_mismatch) &&
                   (e.tokens().size() == 1) &&
                   (e.tokens().front() ==
                    parsing::token_type{parsing::prefix_type::none, "One of: --arg1,--arg2"});
        });
}

BOOST_AUTO_TEST_CASE(help_test)
{
    auto f = [](const auto& node, auto expected_child_strings) {
        const auto filter = [](const auto& child) {
            using child_type = std::decay_t<decltype(child)>;

            if constexpr (traits::has_runtime_enabled_method_v<child_type>) {
                return child.runtime_enabled();
            }
            return true;
        };

        const auto help_data = help_data::generate<false>(node, filter);
        const auto flattened_help_data = help_data::generate<true>(node, filter);

        BOOST_CHECK_EQUAL(help_data, flattened_help_data);

        BOOST_CHECK_EQUAL(help_data.label, "One of: ");
        BOOST_CHECK_EQUAL(help_data.description, "");

        BOOST_REQUIRE_EQUAL(expected_child_strings.size(), help_data.children.size());
        for (auto i = 0u; i < help_data.children.size(); ++i) {
            BOOST_CHECK_EQUAL(help_data.children[i].label, expected_child_strings[i].first);
            BOOST_CHECK_EQUAL(help_data.children[i].description, expected_child_strings[i].second);
            BOOST_CHECK(help_data.children[i].children.empty());
        }
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{ard::one_of(arg<int>(policy::long_name_t{"arg1"_S}),
                                   arg<double>(policy::long_name_t{"arg2"_S}),
                                   policy::required),
                       std::vector{
                           std::pair{"┌ --arg1 <Value>", ""},
                           std::pair{"└ --arg2 <Value>", ""},
                       }},
            std::tuple{ard::one_of(
                           arg<int>(policy::long_name_t{"arg1"_S}, policy::runtime_enable{true}),
                           arg<double>(policy::long_name_t{"arg2"_S}, policy::runtime_enable{true}),
                           policy::required),
                       std::vector{
                           std::pair{"┌ --arg1 <Value>", ""},
                           std::pair{"└ --arg2 <Value>", ""},
                       }},
            std::tuple{
                ard::one_of(
                    arg<int>(policy::long_name_t{"arg1"_S}, policy::runtime_enable{true}),
                    arg<double>(policy::long_name_t{"arg2"_S}, policy::runtime_enable{false}),
                    policy::required),
                std::vector{std::pair{"--arg1 <Value>", ""}}},
            std::tuple{ard::one_of(
                           arg<int>(policy::long_name_t{"arg1"_S}, policy::runtime_enable{false}),
                           arg<double>(policy::long_name_t{"arg2"_S}, policy::runtime_enable{true}),
                           policy::required),
                       std::vector{std::pair{"--arg2 <Value>", ""}}},
            std::tuple{
                ard::one_of(
                    arg<int>(policy::long_name_t{"arg1"_S}, policy::runtime_enable{false}),
                    arg<double>(policy::long_name_t{"arg2"_S}, policy::runtime_enable{false}),
                    policy::required),
                std::vector<std::pair<const char*, const char*>>{}},
            std::tuple{ard::one_of(arg<int>(policy::long_name_t{"arg1"_S}),
                                   flag(policy::long_name_t{"flag"_S},
                                        policy::short_name_t{"f"_S},
                                        policy::description_t{"Hello"_S},
                                        policy::runtime_enable{false}),
                                   arg<double>(policy::short_name_t{"b"_S},
                                               policy::description_t{"A desc"_S}),
                                   policy::required),
                       std::vector{
                           std::pair{"┌ --arg1 <Value>", ""},
                           std::pair{"└ -b <Value>", "A desc"},
                       }},
        });
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile(
        {{R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;
namespace ard = arg_router::dependency;

int main() {
    auto f = ard::one_of(arg<int>(policy::long_name_t{"arg1"_S}));
    return 0;
}
    )",
          "basic_one_of_t must have at least one two child nodes",
          "must_have_two_children_test"},
         {
             R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;
namespace ard = arg_router::dependency;

int main() {
    auto f = ard::one_of(arg<int>(policy::long_name_t{"arg1"_S}),
                         arg<double>(policy::long_name_t{"arg2"_S}),
                         policy::long_name_t{"one_of"_S});
    return 0;
}
    )",
             "basic_one_of_t must not have a long name policy",
             "cannot_have_long_name_test"},
         {
             R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;
namespace ard = arg_router::dependency;

int main() {
    auto f = ard::one_of(arg<int>(policy::long_name_t{"arg1"_S}),
                         arg<double>(policy::long_name_t{"arg2"_S}),
                         policy::short_name_t{"o"_S});
    return 0;
}
    )",
             "basic_one_of_t must not have a short name policy",
             "cannot_have_short_name_test"},
         {
             R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;
namespace ard = arg_router::dependency;

int main() {
    auto f = ard::one_of(arg<int>(policy::long_name_t{"arg1"_S}),
                         arg<double>(policy::long_name_t{"arg2"_S}),
                         policy::none_name_t{"none"_S});
    return 0;
}
    )",
             "basic_one_of_t must not have a none name policy",
             "cannot_have_none_name_test"},
         {
             R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;
namespace ard = arg_router::dependency;

int main() {
    auto f = ard::one_of(arg<int>(policy::long_name_t{"arg1"_S}),
                         arg<double>(policy::long_name_t{"arg2"_S}),
                         policy::description_t{"description"_S});
    return 0;
}
    )",
             "basic_one_of_t must not have a description policy",
             "cannot_have_description_test"},
         {
             R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/default_value.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;
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
    auto f = ard::one_of(arg<int>(policy::long_name_t{"arg1"_S}),
                         arg<bool>(policy::short_name_t{"b"_S}),
                         stub_node{},
                         policy::default_value{42});
    return 0;
}
    )",
             "Node does not have a name",
             "all_children_must_be_named_test"},
         {
             R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/default_value.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;
namespace ard = arg_router::dependency;

int main() {
    auto f = ard::one_of(arg<int>(policy::long_name_t{"arg1"_S},
                                  policy::alias(policy::long_name_t{"arg2"_S})),
                         arg<double>(policy::long_name_t{"arg2"_S},
                                     policy::alias(policy::long_name_t{"arg1"_S})),
                         policy::default_value{42});
    return 0;
}
    )",
             "basic_one_of_t must have at least one child with a value_type",
             "at_least_one_value_type_test"},
         {
             R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;
namespace ard = arg_router::dependency;

namespace
{
template <typename... Params>
class stub_node : public policy::multi_stage_value<std::size_t, bool>,
                  public tree_node<Params...>
{
public:
    using value_type = double;

    constexpr explicit stub_node(Params... params) :
        policy::multi_stage_value<std::size_t, bool>{[](auto&, auto&&) {}},
        tree_node<Params...>{std::move(params)...}
    {}
};
} // namespace

int main() {
    auto f = ard::one_of(arg<int>(policy::long_name_t{"arg1"_S}),
                         stub_node(policy::long_name_t{"arg2"_S}));
    return 0;
}
    )",
             "one_of children must not use a multi_stage_value policy",
             "no_children_can_be_multi_stage_value_test"},
         {
             R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;
namespace ard = arg_router::dependency;

int main() {
    auto f = ard::one_of(arg<int>(policy::long_name_t{"arg1"_S}),
                         arg<double>(policy::long_name_t{"arg2"_S}));
    return 0;
}
    )",
             "basic_one_of_t must have a missing phase method, a policy::required "
             "or policy::default_value are commonly used",
             "missing_missing_phase_test"},
         {
             R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;
namespace ard = arg_router::dependency;

int main() {
    const auto of = ard::one_of(
        arg<int>(policy::long_name_t{"arg1"_S}),
        arg<double>(policy::long_name_t{"arg2"_S}),
        policy::required,
        policy::alias(policy::long_name_t{"arg3"_S}));
    return 0;
}
    )",
             "basic_one_of_t does not support policies with pre-parse, parse, "
             "or routing phases; as it delegates those to its children",
             "pre_parse_phase_test"},
         {
             R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;
namespace ard = arg_router::dependency;

int main() {
    const auto of = ard::one_of(
        arg<int>(policy::long_name_t{"arg1"_S}),
        arg<double>(policy::long_name_t{"arg2"_S}),
        policy::required,
        policy::custom_parser<std::variant<int, double>>{[](std::string_view) {
            return std::variant<int, double>{}; }});
    return 0;
}
    )",
             "basic_one_of_t does not support policies with pre-parse, parse, "
             "or routing phases; as it delegates those to its children",
             "parse_phase_test"},
         {
             R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;
namespace ard = arg_router::dependency;

int main() {
    const auto of = ard::one_of(
        arg<int>(policy::long_name_t{"arg1"_S}),
        arg<double>(policy::long_name_t{"arg2"_S}),
        policy::required,
        policy::min_max_value<42, 84>());
    return 0;
}
    )",
             "one_of does not support policies with validation phases; as it "
             "delegates those to its children",
             "validation_phase_test"},
         {
             R"(
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;
namespace ard = arg_router::dependency;

int main() {
    const auto of = ard::one_of(
        arg<int>(policy::long_name_t{"arg1"_S}),
        arg<double>(policy::long_name_t{"arg2"_S}),
        policy::required,
        policy::router{[](std::variant<int, double>) {}});
    return 0;
}
    )",
             "basic_one_of_t does not support policies with pre-parse, parse, "
             "or routing phases; as it delegates those to its children",
             "router_phase_test"}});
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
