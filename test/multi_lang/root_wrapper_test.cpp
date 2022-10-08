/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/multi_lang/root_wrapper.hpp"
#include "arg_router/multi_lang/string_selector.hpp"
#include "arg_router/policy/validator.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(multi_lang_suite)

BOOST_AUTO_TEST_SUITE(root_wrapper_suite)

BOOST_AUTO_TEST_CASE(parse_english_test)
{
    auto result = std::optional<int>{};
    const auto r = multi_lang::root_wrapper<S_("en_GB"), S_("fr"), S_("es")>("en_GB", [&](auto I) {
        return root(
            mode(arg<int>(
                     policy::long_name<SM_(I, "hello", "bonjour", "hola")>,
                     policy::required,
                     policy::description<
                         SM_(I, "Hello description", "Bonjour descriptif", "Hola descripción")>),
                 policy::router{[&](auto value) {
                     BOOST_CHECK(!result);
                     result = value;
                 }}),
            policy::validation::default_validator);
    });

    auto args = std::vector{"foo", "--hello", "42"};
    r.parse(args.size(), const_cast<char**>(args.data()));
    BOOST_REQUIRE(!!result);
    BOOST_CHECK_EQUAL(*result, 42);

    args = std::vector{"foo", "--bonjour", "42"};
    BOOST_CHECK_EXCEPTION(  //
        r.parse(args.size(), const_cast<char**>(args.data())),
        parse_exception,
        [](const auto& e) { return e.what() == "Unknown argument: --bonjour"sv; });
}

BOOST_AUTO_TEST_CASE(parse_french_test)
{
    auto result = std::optional<int>{};
    const auto r = multi_lang::root_wrapper<S_("en_GB"), S_("fr"), S_("es")>("fr", [&](auto I) {
        return root(
            mode(arg<int>(
                     policy::long_name<SM_(I, "hello", "bonjour", "hola")>,
                     policy::required,
                     policy::description<
                         SM_(I, "Hello description", "Bonjour descriptif", "Hola descripción")>),
                 policy::router{[&](auto value) {
                     BOOST_CHECK(!result);
                     result = value;
                 }}),
            policy::validation::default_validator);
    });

    auto args = std::vector{"foo", "--bonjour", "42"};
    r.parse(args.size(), const_cast<char**>(args.data()));
    BOOST_REQUIRE(!!result);
    BOOST_CHECK_EQUAL(*result, 42);

    args = std::vector{"foo", "--hello", "42"};
    BOOST_CHECK_EXCEPTION(  //
        r.parse(args.size(), const_cast<char**>(args.data())),
        parse_exception,
        [](const auto& e) { return e.what() == "Unknown argument: --hello"sv; });
}

BOOST_AUTO_TEST_CASE(parse_default_test)
{
    for (auto input : {"da", "en-us", "POSIX", "*", "C", ""}) {
        auto result = std::optional<int>{};
        const auto r =
            multi_lang::root_wrapper<S_("en_GB"), S_("fr"), S_("es")>(input, [&](auto I) {
                return root(mode(arg<int>(policy::long_name<SM_(I, "hello", "bonjour", "hola")>,
                                          policy::required,
                                          policy::description<SM_(I,
                                                                  "Hello description",
                                                                  "Bonjour descriptif",
                                                                  "Hola descripción")>),
                                 policy::router{[&](auto value) {
                                     BOOST_CHECK(!result);
                                     result = value;
                                 }}),
                            policy::validation::default_validator);
            });

        auto args = std::vector{"foo", "--hello", "42"};
        r.parse(args.size(), const_cast<char**>(args.data()));
        BOOST_REQUIRE(!!result);
        BOOST_CHECK_EQUAL(*result, 42);
    }
}

BOOST_AUTO_TEST_CASE(help_test)
{
    auto f = [](auto input, auto expected_output) {
        auto result = std::optional<int>{};
        const auto r = multi_lang::root_wrapper<S_("en_GB"),
                                                S_("fr"),
                                                S_("es")>(input, [&](auto I) {
            return root(
                help(
                    policy::long_name<SM_(I, "help", "aider", "ayuda")>,
                    policy::short_name<'h'>,
                    policy::description<SM_(I, "Display help", "Afficher l'aide", "Mostrar ayuda")>,
                    policy::program_name<S_("foo")>,
                    policy::program_version<S_("v3.14")>,
                    policy::program_intro<S_("Fooooooo")>),
                mode(
                    arg<int>(
                        policy::long_name<SM_(I, "hello", "bonjour", "hola")>,
                        policy::required,
                        policy::description<
                            SM_(I, "Hello description", "Bonjour descriptif", "Hola descripción")>),
                    policy::router{[&](auto value) {
                        BOOST_CHECK(!result);
                        result = value;
                    }}),
                policy::validation::default_validator);
        });

        auto stream = std::ostringstream{};
        r.help(stream);

        BOOST_CHECK_EQUAL(stream.str(), expected_output);
    };

    test::data_set(f,
                   {
                       std::tuple{"en_GB",
                                  "foo v3.14\n\nFooooooo\n\n"
                                  "    --help,-h              Display help\n"
                                  "        --hello <Value>    Hello description\n"},
                       std::tuple{"fr",
                                  "foo v3.14\n\nFooooooo\n\n"
                                  "    --aider,-h               Afficher l'aide\n"
                                  "        --bonjour <Value>    Bonjour descriptif\n"},
                       std::tuple{"es",
                                  "foo v3.14\n\nFooooooo\n\n"
                                  "    --ayuda,-h            Mostrar ayuda\n"
                                  "        --hola <Value>    Hola descripción\n"},
                       std::tuple{"en-us",
                                  "foo v3.14\n\nFooooooo\n\n"
                                  "    --help,-h              Display help\n"
                                  "        --hello <Value>    Hello description\n"},
                   });
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({
        {
            R"(
#include "arg_router/arg.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/multi_lang/root_wrapper.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto r = multi_lang::root_wrapper<S_("en_GB")>("en_GB", [&](auto I) {
        return root(
            mode(arg<int>(
                     policy::long_name<S_("hello")>,
                     policy::required,
                     policy::description<S_("Hello description")>),
                 policy::router{[&]([[maybe_unused]] auto value) {}}),
            policy::validation::default_validator);
    });
    return 0;
}
    )",
            "Must be more than one language provided",
            "must_be_more_than_one_language_provided_test"},
        {
            R"(
#include "arg_router/arg.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/multi_lang/root_wrapper.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto r = multi_lang::root_wrapper<S_("en_GB"), S_("en_GB")>("en_GB", [&](auto I) {
        return root(
            mode(arg<int>(
                     policy::long_name<S_("hello")>,
                     policy::required,
                     policy::description<S_("Hello description")>),
                 policy::router{[&]([[maybe_unused]] auto value) {}}),
            policy::validation::default_validator);
    });
    return 0;
}
    )",
            "Supported ISO language codes must be unique",
            "unique_iso_codes1_test"},
        {
            R"(
#include "arg_router/arg.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/multi_lang/root_wrapper.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto r = multi_lang::root_wrapper<S_("fr"), S_("en_GB"), S_("en_GB")>(
        "en_GB",
        [&](auto I) {
            return root(
                mode(arg<int>(
                         policy::long_name<S_("hello")>,
                         policy::required,
                         policy::description<S_("Hello description")>),
                     policy::router{[&]([[maybe_unused]] auto value) {}}),
                policy::validation::default_validator);
        });
    return 0;
}
    )",
            "Supported ISO language codes must be unique",
            "unique_iso_codes2_test"},
    });
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
