// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/multi_lang/root.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/validator.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace arg_router::literals;
using namespace std::string_view_literals;

namespace arg_router::multi_lang
{
template <>
class translation<str<"en_GB">>
{
public:
    using hello = str<"hello">;
    using hello_description = str<"Hello description">;
    using help = str<"help">;
    using help_description = str<"Display help">;
};

template <>
class translation<str<"fr">>
{
public:
    using hello = str<"bonjour">;
    using hello_description = str<"Bonjour descriptif">;
    using help = str<"aider">;
    using help_description = str<"Afficher l'aide">;

    using error_code_translations = std::tuple<
        std::pair<traits::integral_constant<error_code::unknown_argument>, str<"Argument inconnu">>,
        std::pair<traits::integral_constant<error_code::unhandled_arguments>,
                  str<"Arguments non gérés">>,
        std::pair<traits::integral_constant<error_code::argument_has_already_been_set>,
                  str<"L'argument a déjà été défini">>,
        std::pair<traits::integral_constant<error_code::failed_to_parse>,
                  str<"L'analyse a échoué">>,
        std::pair<traits::integral_constant<error_code::no_arguments_passed>,
                  str<"Aucun argument passé">>,
        std::pair<traits::integral_constant<error_code::minimum_value_not_reached>,
                  str<"Valeur minimale non atteinte">>,
        std::pair<traits::integral_constant<error_code::maximum_value_exceeded>,
                  str<"Valeur maximale dépassée">>,
        std::pair<traits::integral_constant<error_code::unknown_argument_with_suggestion>,
                  str<"Argument inconnu: {}. Vous avez dit { }?">>,
        std::pair<traits::integral_constant<error_code::minimum_count_not_reached>,
                  str<"Nombre minimum non atteint">>,
        std::pair<traits::integral_constant<error_code::mode_requires_arguments>,
                  str<"Le mode nécessite des arguments">>,
        std::pair<traits::integral_constant<error_code::missing_required_argument>,
                  str<"Argument requis manquant">>,
        std::pair<traits::integral_constant<error_code::too_few_values_for_alias>,
                  str<"Trop peu de valeurs pour l'alias">>,
        std::pair<traits::integral_constant<error_code::dependent_argument_missing>,
                  str<"Argument dépendant manquant (doit être avant le jeton "
                      "requis sur la ligne de commande)">>>;
};

template <>
class translation<str<"es">>
{
public:
    using hello = str<"hola">;
    using hello_description = str<"Hola descripción">;
    using help = str<"ayuda">;
    using help_description = str<"Mostrar ayuda">;

    using error_code_translations = std::tuple<
        std::pair<traits::integral_constant<error_code::unknown_argument>,
                  str<"Argumento desconocido">>,
        std::pair<traits::integral_constant<error_code::unhandled_arguments>,
                  str<"Argumentos no manejados">>,
        std::pair<traits::integral_constant<error_code::argument_has_already_been_set>,
                  str<"El argumento ya ha sido definido">>,
        std::pair<traits::integral_constant<error_code::failed_to_parse>, str<"No pude analizar">>,
        std::pair<traits::integral_constant<error_code::no_arguments_passed>,
                  str<"No se pasaron argumentos">>,
        std::pair<traits::integral_constant<error_code::minimum_value_not_reached>,
                  str<"Valor mínimo no alcanzado">>,
        std::pair<traits::integral_constant<error_code::maximum_value_exceeded>,
                  str<"Valor máximo excedido">>,
        std::pair<traits::integral_constant<error_code::unknown_argument_with_suggestion>,
                  str<"Argumento desconocido: {}. ¿Querías decir { }?">>,
        std::pair<traits::integral_constant<error_code::minimum_count_not_reached>,
                  str<"Valor máximo excedido">>,
        std::pair<traits::integral_constant<error_code::mode_requires_arguments>,
                  str<"El modo requiere argumentos">>,
        std::pair<traits::integral_constant<error_code::missing_required_argument>,
                  str<"Falta el argumento requerido">>,
        std::pair<traits::integral_constant<error_code::too_few_values_for_alias>,
                  str<"Muy pocos valores para el alias">>,
        std::pair<traits::integral_constant<error_code::dependent_argument_missing>,
                  str<"Falta argumento dependiente (debe estar antes del token "
                      "requerido en la línea de comando)">>>;
};
}  // namespace arg_router::multi_lang

BOOST_AUTO_TEST_SUITE(multi_lang_suite)

BOOST_AUTO_TEST_SUITE(root_suite)

BOOST_AUTO_TEST_CASE(parse_test)
{
    auto f = [](auto lang, auto args, auto parse_result, std::string exception_message) {
        auto result = std::optional<int>{};
        const auto r = multi_lang::root<str<"en_GB">, str<"fr">, str<"es">>(lang, [&](auto tr_) {
            using tr = decltype(tr_);

            return root(mode(arg<int>(policy::long_name_t{typename tr::hello{}},
                                      policy::required,
                                      policy::description_t{typename tr::hello_description{}}),
                             policy::router{[&](auto value) {
                                 BOOST_CHECK(!result);
                                 result = value;
                             }}),
                        policy::validation::default_validator,
                        policy::exception_translator<tr>);
        });

        const auto parse_invocations =
            std::vector<std::pair<std::string_view, std::function<void(std::vector<const char*>)>>>{
                {"vector<parsing::token_type> overload",
                 [&](std::vector<const char*> args) {
                     args.erase(args.begin());
                     auto tt = std::vector<parsing::token_type>{};
                     for (auto arg : args) {
                         tt.emplace_back(parsing::prefix_type::none, arg);
                     }
                     r.parse(std::move(tt));
                 }},
                {"int, char** overload",
                 [&](std::vector<const char*> args) {
                     r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
                 }},
                {"Iter, Iter overload",
                 [&](std::vector<const char*> args) {  //
                     args.erase(args.begin());
                     r.parse(args.begin(), args.end());
                 }},
                {"Container overload",
                 [&](std::vector<const char*> args) {  //
                     args.erase(args.begin());
                     r.parse(args);
                 }},
                {"Container overload (with strings)",  //
                 [&](std::vector<const char*> args) {
                     args.erase(args.begin());
                     auto strings = std::vector<std::string>{};
                     for (auto arg : args) {
                         strings.push_back(arg);
                     }
                     r.parse(std::move(strings));
                 }}};

        for (const auto& [name, invoc] : parse_invocations) {
            BOOST_TEST_MESSAGE("\t" << name);
            try {
                result.reset();
                invoc(args);
                BOOST_CHECK(exception_message.empty());
                BOOST_REQUIRE(!!result);
                BOOST_CHECK_EQUAL(*result, parse_result);
            } catch (parse_exception& e) {
                BOOST_CHECK_EQUAL(e.what(), exception_message);
            }
        }
    };

    test::data_set(f,
                   {
                       // English
                       std::tuple{"en_GB", std::vector{"foo", "--hello", "42"}, 42, ""},
                       std::tuple{"en_GB",
                                  std::vector{"foo", "--bonjour", "42"},
                                  42,
                                  "Unknown argument: --bonjour. Did you mean --hello?"},

                       // French
                       std::tuple{"fr", std::vector{"foo", "--bonjour", "42"}, 42, ""},
                       std::tuple{"fr",
                                  std::vector{"foo", "--hello", "42"},
                                  42,
                                  "Argument inconnu: --hello. Vous avez dit --bonjour?"},

                       // Spanish
                       std::tuple{"es", std::vector{"foo", "--hola", "42"}, 42, ""},
                       std::tuple{"es",
                                  std::vector{"foo", "--hello", "42"},
                                  42,
                                  "Argumento desconocido: --hello. ¿Querías decir --hola?"},
                   });
}

BOOST_AUTO_TEST_CASE(default_parse_test)
{
    for (auto input : {"da", "en-us", "POSIX", "*", "C", ""}) {
        auto result = std::optional<int>{};
        const auto r = multi_lang::root<str<"en_GB">, str<"fr">, str<"es">>(input, [&](auto tr_) {
            using tr = decltype(tr_);

            return root(mode(arg<int>(policy::long_name_t{typename tr::hello{}},
                                      policy::required,
                                      policy::description_t{typename tr::hello_description{}}),
                             policy::router{[&](auto value) {
                                 BOOST_CHECK(!result);
                                 result = value;
                             }}),
                        policy::validation::default_validator);
        });

        auto args = std::vector{"foo", "--hello", "42"};
        r.parse(static_cast<int>(args.size()), const_cast<char**>(args.data()));
        BOOST_REQUIRE(!!result);
        BOOST_CHECK_EQUAL(*result, 42);
    }
}

BOOST_AUTO_TEST_CASE(help_test)
{
    auto f = [](auto input, auto expected_output) {
        auto result = std::optional<int>{};
        const auto r = multi_lang::root<str<"en_GB">, str<"fr">, str<"es">>(input, [&](auto tr_) {
            using tr = decltype(tr_);

            return root(help(policy::long_name_t{typename tr::help{}},
                             policy::short_name_t{"h"_S},
                             policy::description_t{typename tr::help_description{}},
                             policy::program_name_t{"foo"_S},
                             policy::program_version_t{"v3.14"_S},
                             policy::program_intro_t{"Fooooooo"_S}),
                        mode(arg<int>(policy::long_name_t{typename tr::hello{}},
                                      policy::required,
                                      policy::description_t{typename tr::hello_description{}}),
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
                                  "     \n"
                                  "        --hello <Value>    Hello description\n"},
                       std::tuple{"fr",
                                  "foo v3.14\n\nFooooooo\n\n"
                                  "    --aider,-h               Afficher l'aide\n"
                                  "     \n"
                                  "        --bonjour <Value>    Bonjour descriptif\n"},
                       std::tuple{"es",
                                  "foo v3.14\n\nFooooooo\n\n"
                                  "    --ayuda,-h            Mostrar ayuda\n"
                                  "     \n"
                                  "        --hola <Value>    Hola descripción\n"},
                       std::tuple{"en-us",
                                  "foo v3.14\n\nFooooooo\n\n"
                                  "    --help,-h              Display help\n"
                                  "     \n"
                                  "        --hello <Value>    Hello description\n"},
                   });
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({
        {
            R"(
#include "arg_router/arg.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/multi_lang/root.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto r = multi_lang::root<str<"en_GB">>("en_GB", [&]([[maybe_unused]] auto tr_) {
        return root(
            mode(arg<int>(
                     policy::long_name_t{"hello"_S},
                     policy::required,
                     policy::description_t{"Hello description"_S}),
                 policy::router{[&]([[maybe_unused]] auto value) {}}),
            policy::validation::default_validator);
    });
    return 0;
}
    )",
            "Must be more than one language supported",
            "must_be_more_than_one_language_provided_test"},
        {
            R"(
#include "arg_router/arg.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/multi_lang/root.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

namespace arg_router::multi_lang
{
template <>
class translation<str<"en_GB">> {};
}

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto r = multi_lang::root<str<"en_GB">, str<"en_GB">>(
        "en_GB", [&]([[maybe_unused]] auto tr_) {
        return root(
            mode(arg<int>(
                     policy::long_name_t{"hello"_S},
                     policy::required,
                     policy::description_t{"Hello description"_S}),
                 policy::router{[&]([[maybe_unused]] auto value) {}}),
            policy::validation::default_validator);
    });
    return 0;
}
    )",
            "Supported languages must be unique",
            "unique_iso_codes1_test"},
        {
            R"(
#include "arg_router/arg.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/multi_lang/root.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto r = multi_lang::root<str<"fr">, str<"en_GB">, str<"en_GB">>(
        "en_GB",
        [&]([[maybe_unused]] auto tr_) {
            return root(
                mode(arg<int>(
                         policy::long_name_t{"hello"_S},
                         policy::required,
                         policy::description_t{"Hello description"_S}),
                     policy::router{[&]([[maybe_unused]] auto value) {}}),
                policy::validation::default_validator);
        });
    return 0;
}
    )",
            "Supported languages must be unique",
            "unique_iso_codes2_test"},
    });
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
