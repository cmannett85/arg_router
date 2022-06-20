/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/parsing/pre_parse_data.hpp"
#include "arg_router/tree_node.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;

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

    template <typename... Parents>
    [[nodiscard]] value_type parse(parsing::parse_target,
                                   const Parents&...) const
    {
        return true;
    }
};
}  // namespace

BOOST_AUTO_TEST_SUITE(parsing_suite)

BOOST_AUTO_TEST_SUITE(pre_parse_data_suite)

BOOST_AUTO_TEST_CASE(no_target_constructor_test)
{
    auto args =
        std::vector<parsing::token_type>{{parsing::prefix_type::none, "-f"},
                                         {parsing::prefix_type::none, "42"}};
    const auto false_validator = [](const auto&...) { return false; };

    {
        auto ppd = parsing::pre_parse_data{args};
        BOOST_CHECK_EQUAL(ppd.args(), args);
        BOOST_CHECK(ppd.validator()());
    }

    {
        const auto ppd = parsing::pre_parse_data{args};
        BOOST_CHECK_EQUAL(ppd.args(), args);
        BOOST_CHECK(ppd.validator()());
    }

    {
        auto ppd = parsing::pre_parse_data{args, false_validator};
        BOOST_CHECK_EQUAL(ppd.args(), args);
        BOOST_CHECK(!ppd.validator()());
    }

    {
        const auto ppd = parsing::pre_parse_data{args, false_validator};
        BOOST_CHECK_EQUAL(ppd.args(), args);
        BOOST_CHECK(!ppd.validator()());
    }
}

BOOST_AUTO_TEST_CASE(target_constructor_test)
{
    auto args =
        std::vector<parsing::token_type>{{parsing::prefix_type::none, "-f"},
                                         {parsing::prefix_type::none, "42"}};
    auto target_tokens =
        std::vector<parsing::token_type>{{parsing::prefix_type::none, "hello"}};
    auto node = stub_node{};
    const auto target = parsing::parse_target{target_tokens, node};
    const auto false_validator = [](const auto&...) { return false; };

    {
        auto ppd = parsing::pre_parse_data{args, target};
        BOOST_CHECK_EQUAL(ppd.args(), args);
        BOOST_CHECK(ppd.validator()());
        BOOST_CHECK_EQUAL(ppd.target().tokens(), target_tokens);
    }

    {
        const auto ppd = parsing::pre_parse_data{args, target};
        BOOST_CHECK_EQUAL(ppd.args(), args);
        BOOST_CHECK(ppd.validator()());
        BOOST_CHECK_EQUAL(ppd.target().tokens(), target_tokens);
    }

    {
        auto ppd = parsing::pre_parse_data{args, target, false_validator};
        BOOST_CHECK_EQUAL(ppd.args(), args);
        BOOST_CHECK(!ppd.validator()());
        BOOST_CHECK_EQUAL(ppd.target().tokens(), target_tokens);
    }

    {
        const auto ppd = parsing::pre_parse_data{args, target, false_validator};
        BOOST_CHECK_EQUAL(ppd.args(), args);
        BOOST_CHECK(!ppd.validator()());
        BOOST_CHECK_EQUAL(ppd.target().tokens(), target_tokens);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
