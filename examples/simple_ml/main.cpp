// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <arg_router/arg_router.hpp>

#include <cstdlib>
#include <filesystem>

namespace ar = arg_router;
namespace arp = ar::policy;
namespace fs = std::filesystem;

using namespace std::literals;
using namespace ar::literals;
using namespace ar::utility::string_view_ops;

namespace arg_router::multi_lang
{
// Apologies for any translation faux pas - Google Translate did it for me!
template <>
class translation<str<"en_GB">>
{
public:
    using force = str<"force">;
    using force_description = str<"Force overwrite existing files">;
    using destination = str<"DST">;
    using destination_description = str<"Destination directory">;
    using help = str<"help">;
    using help_description = str<"Display this help and exit">;
    using program_intro = str<"A simple file copier and mover.">;
    using program_addendum = str<"An example program for arg_router.">;
    using copy = str<"copy">;
    using copy_description = str<"Copy source files to destination">;
    using source = str<"SRC">;
    using sources_description = str<"Source file paths">;
    using move = str<"move">;
    using move_description = str<"Move source file to destination">;
    using source_description = str<"Source file path">;
};

template <>
class translation<str<"fr">>
{
public:
    using force = str<"forcer">;
    using force_description = str<"Forcer l'écrasement des fichiers existants">;
    using destination = str<"DST">;
    using destination_description = str<"Répertoire de destination">;
    using help = str<"aider">;
    using help_description = str<"Afficher cette aide et quitter">;
    using program_intro = str<"Un simple copieur et déménageur de fichiers.">;
    using program_addendum = str<"Un exemple de programme pour arg_router.">;
    using copy = str<"copier">;
    using copy_description = str<"Copier les fichiers source vers la destination">;
    using source = str<"SRC">;
    using sources_description = str<"Chemins des fichiers sources">;
    using move = str<"déplacer">;
    using move_description = str<"Déplacer le fichier source vers la destination">;
    using source_description = str<"Chemin du fichier source">;

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
        std::pair<traits::integral_constant<error_code::minimum_count_not_reached>,
                  str<"Nombre minimum non atteint">>,
        std::pair<traits::integral_constant<error_code::maximum_count_exceeded>,
                  str<"Nombre maximal dépassé">>,
        std::pair<traits::integral_constant<error_code::unknown_argument_with_suggestion>,
                  str<"Argument inconnu : {}. Vouliez-vous dire { }?">>,
        std::pair<traits::integral_constant<error_code::mode_requires_arguments>,
                  str<"Le mode nécessite des arguments">>,
        std::pair<traits::integral_constant<error_code::missing_required_argument>,
                  str<"Argument requis manquant">>,
        std::pair<traits::integral_constant<error_code::too_few_values_for_alias>,
                  str<"Trop peu de valeurs pour l'alias">>,
        std::pair<
            traits::integral_constant<error_code::dependent_argument_missing>,
            str<"Argument dépendant manquant (doit être avant le jeton requis sur la ligne de "
                "commande)">>,
        std::pair<traits::integral_constant<error_code::one_of_selected_type_mismatch>,
                  str<"Un seul argument d'un \"One Of\" peut être utilisé à la fois">>,
        std::pair<traits::integral_constant<error_code::missing_value_separator>,
                  str<"Attendu un séparateur de valeur">>>;
};

template <>
class translation<str<"ja">>
{
public:
    using force = str<"強制">;
    using force_description = str<"既存のファイルを強制的に上書きする">;
    using destination = str<"先">;
    using destination_description = str<"宛先ディレクトリ">;
    using help = str<"ヘルプ">;
    using help_description = str<"このヘルプを表示して終了">;
    using program_intro = str<"ファイルをコピーおよび移動するためのシンプルなプログラム。">;
    using program_addendum = str<"「arg_router」のサンプルプログラム。">;
    using copy = str<"コピー">;
    using copy_description = str<"ソース ファイルを宛先にコピーする">;
    using source = str<"出典">;
    using sources_description = str<"ソース ファイルのパス">;
    using move = str<"移動">;
    using move_description = str<"ソース ファイルを宛先に移動する">;
    using source_description = str<"ソース ファイル パス">;

    using error_code_translations = std::tuple<
        std::pair<traits::integral_constant<error_code::unknown_argument>, str<"不明な引数">>,
        std::pair<traits::integral_constant<error_code::unhandled_arguments>, str<"未処理の引数">>,
        std::pair<traits::integral_constant<error_code::argument_has_already_been_set>,
                  str<"引数はすでに設定されています">>,
        std::pair<traits::integral_constant<error_code::failed_to_parse>,
                  str<"解析に失敗しました">>,
        std::pair<traits::integral_constant<error_code::no_arguments_passed>,
                  str<"引数が渡されませんでした">>,
        std::pair<traits::integral_constant<error_code::minimum_value_not_reached>,
                  str<"最小値に達していません">>,
        std::pair<traits::integral_constant<error_code::maximum_value_exceeded>,
                  str<"最大値を超えました">>,
        std::pair<traits::integral_constant<error_code::minimum_count_not_reached>,
                  str<"最小数に達していません">>,
        std::pair<traits::integral_constant<error_code::maximum_count_exceeded>,
                  str<"最大数を超えました">>,
        std::pair<traits::integral_constant<error_code::unknown_argument_with_suggestion>,
                  str<"不明な引数 {}。 { } という意味でしたか？">>,
        std::pair<traits::integral_constant<error_code::mode_requires_arguments>,
                  str<"モードには引数が必要です">>,
        std::pair<traits::integral_constant<error_code::missing_required_argument>,
                  str<"必要な引数がありません">>,
        std::pair<traits::integral_constant<error_code::too_few_values_for_alias>,
                  str<"エイリアス値が少なすぎる">>,
        std::pair<
            traits::integral_constant<error_code::dependent_argument_missing>,
            str<"従属引数がありません (コマンドラインで必要なトークンの前に置く必要があります)">>,
        std::pair<traits::integral_constant<error_code::one_of_selected_type_mismatch>,
                  str<"一度に許可される「One Of」引数は1つだけです">>,
        std::pair<traits::integral_constant<error_code::missing_value_separator>,
                  str<"値の区切り文字が必要です">>>;
};
}  // namespace arg_router::multi_lang

template <>
struct ar::parser<fs::path> {
    [[nodiscard]] static inline fs::path parse(std::string_view arg) { return arg; }
};

namespace
{
// You can use an env var to override the locale for testing (doesn't affect your machine's real
// locale)
std::string locale_name()
{
    if (auto* env_locale = std::getenv("AR_LOCALE_OVERRIDE")) {
        return env_locale;
    }

    return std::locale("").name();
}

void copy_mode(bool force, fs::path&& dest, std::vector<fs::path>&& srcs)
{
    const auto options = force ? fs::copy_options::overwrite_existing : fs::copy_options::none;

    for (const auto& src : srcs) {
        fs::copy(src, dest, options);
    }
}

void move_mode(bool force, fs::path&& dest, fs::path&& src)
{
    if (force && fs::exists(dest)) {
        fs::remove(src);
    }

    fs::rename(src, dest);
}
}  // namespace

int main(int argc, char* argv[])
{
    ar::multi_lang::root<ar::str<"en_GB">, ar::str<"fr">, ar::str<"ja">>(  //
        ar::multi_lang::iso_locale(locale_name()),
        [&](auto tr_) {
            using tr = decltype(tr_);

            const auto common_args =
                ar::list{ar::flag(typename tr::force{}, "f"_S, typename tr::force_description{}),
                         ar::positional_arg<fs::path>(arp::required,
                                                      typename tr::destination{},
                                                      typename tr::destination_description{},
                                                      arp::fixed_count<1>)};

            return ar::root(
                arp::validation::default_validator,
                arp::exception_translator<tr>,
                ar::help(typename tr::help{},
                         "h"_S,
                         typename tr::help_description{},
                         arp::program_name_t{"simple"_S},
                         arp::program_version_t{"v0.1"_S},
                         arp::program_intro_t{typename tr::program_intro{}},
                         arp::program_addendum_t{typename tr::program_addendum{}},
                         arp::flatten_help,
                         arp::colour_help_formatter),
                ar::mode(
                    typename tr::copy{},
                    typename tr::copy_description{},
                    common_args,
                    ar::positional_arg<std::vector<fs::path>>(arp::required,
                                                              typename tr::source{},
                                                              typename tr::sources_description{},
                                                              arp::min_count<1>),
                    arp::router{[](bool force, fs::path dest, std::vector<fs::path> srcs) {
                        copy_mode(force, std::move(dest), std::move(srcs));
                    }}),
                ar::mode(typename tr::move{},
                         typename tr::move_description{},
                         common_args,
                         ar::positional_arg<fs::path>(arp::required,
                                                      typename tr::source{},
                                                      typename tr::source_description{},
                                                      arp::fixed_count<1>),
                         arp::router{[](bool force, fs::path dest, fs::path src) {
                             move_mode(force, std::move(dest), std::move(src));
                         }}));
        })
        .parse(argc, argv);
}
