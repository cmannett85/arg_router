// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <arg_router/arg_router.hpp>

#include <cstdlib>
#include <filesystem>

namespace ar = arg_router;
namespace arp = ar::policy;
namespace fs = std::filesystem;

using namespace std::string_view_literals;
using namespace std::string_literals;
using namespace ar::utility::string_view_ops;

namespace arg_router::multi_lang
{
// Apologies for any translation faux pas - Google Translate did it for me!
template <>
class translation<S_("en_GB")>
{
public:
    using force = S_("force");
    using force_description = S_("Force overwrite existing files");
    using destination = S_("DST");
    using destination_description = S_("Destination directory");
    using help = S_("help");
    using help_description = S_("Display this help and exit");
    using program_intro = S_("A simple file copier and mover.");
    using program_addendum = S_("An example program for arg_router.");
    using copy = S_("copy");
    using copy_description = S_("Copy source files to destination");
    using source = S_("SRC");
    using sources_description = S_("Source file paths");
    using move = S_("move");
    using move_description = S_("Move source file to destination");
    using source_description = S_("Source file path");
};

template <>
class translation<S_("fr")>
{
public:
    using force = S_("forcer");
    using force_description = S_("Forcer l'écrasement des fichiers existants");
    using destination = S_("DST");
    using destination_description = S_("Répertoire de destination");
    using help = S_("aider");
    using help_description = S_("Afficher cette aide et quitter");
    using program_intro = S_("Un simple copieur et déménageur de fichiers.");
    using program_addendum = S_("Un exemple de programme pour arg_router.");
    using copy = S_("copier");
    using copy_description = S_("Copier les fichiers source vers la destination");
    using source = S_("SRC");
    using sources_description = S_("Chemins des fichiers sources");
    using move = S_("déplacer");
    using move_description = S_("Déplacer le fichier source vers la destination");
    using source_description = S_("Chemin du fichier source");

    using error_code_translations = std::tuple<
        std::pair<traits::integral_constant<error_code::unknown_argument>, S_("Argument inconnu")>,
        std::pair<traits::integral_constant<error_code::unhandled_arguments>,
                  S_("Arguments non gérés")>,
        std::pair<traits::integral_constant<error_code::argument_has_already_been_set>,
                  S_("L'argument a déjà été défini")>,
        std::pair<traits::integral_constant<error_code::failed_to_parse>, S_("L'analyse a échoué")>,
        std::pair<traits::integral_constant<error_code::no_arguments_passed>,
                  S_("Aucun argument passé")>,
        std::pair<traits::integral_constant<error_code::minimum_value_not_reached>,
                  S_("Valeur minimale non atteinte")>,
        std::pair<traits::integral_constant<error_code::maximum_value_exceeded>,
                  S_("Valeur maximale dépassée")>,
        std::pair<traits::integral_constant<error_code::minimum_count_not_reached>,
                  S_("Nombre minimum non atteint")>,
        std::pair<traits::integral_constant<error_code::maximum_count_exceeded>,
                  S_("Nombre maximal dépassé")>,
        std::pair<traits::integral_constant<error_code::unknown_argument_with_suggestion>,
                  S_("Argument inconnu : {}. Vouliez-vous dire { }?")>,
        std::pair<traits::integral_constant<error_code::mode_requires_arguments>,
                  S_("Le mode nécessite des arguments")>,
        std::pair<traits::integral_constant<error_code::missing_required_argument>,
                  S_("Argument requis manquant")>,
        std::pair<traits::integral_constant<error_code::too_few_values_for_alias>,
                  S_("Trop peu de valeurs pour l'alias")>,
        std::pair<traits::integral_constant<error_code::dependent_argument_missing>,
                  S_("Argument dépendant manquant (doit être avant le jeton requis sur la ligne de "
                     "commande)")>,
        std::pair<traits::integral_constant<error_code::one_of_selected_type_mismatch>,
                  S_("Un seul argument d'un \"One Of\" peut être utilisé à la fois")>,
        std::pair<traits::integral_constant<error_code::missing_value_separator>,
                  S_("Attendu un séparateur de valeur")>>;
};

template <>
class translation<S_("ja")>
{
public:
    using force = S_("強制");
    using force_description = S_("既存のファイルを強制的に上書きする");
    using destination = S_("先");
    using destination_description = S_("宛先ディレクトリ");
    using help = S_("ヘルプ");
    using help_description = S_("このヘルプを表示して終了");
    using program_intro = S_("ファイルをコピーおよび移動するためのシンプルなプログラム。");
    using program_addendum = S_("「arg_router」のサンプルプログラム。");
    using copy = S_("コピー");
    using copy_description = S_("ソース ファイルを宛先にコピーする");
    using source = S_("出典");
    using sources_description = S_("ソース ファイルのパス");
    using move = S_("移動");
    using move_description = S_("ソース ファイルを宛先に移動する");
    using source_description = S_("ソース ファイル パス");

    using error_code_translations = std::tuple<
        std::pair<traits::integral_constant<error_code::unknown_argument>, S_("不明な引数")>,
        std::pair<traits::integral_constant<error_code::unhandled_arguments>, S_("未処理の引数")>,
        std::pair<traits::integral_constant<error_code::argument_has_already_been_set>,
                  S_("引数はすでに設定されています")>,
        std::pair<traits::integral_constant<error_code::failed_to_parse>, S_("解析に失敗しました")>,
        std::pair<traits::integral_constant<error_code::no_arguments_passed>,
                  S_("引数が渡されませんでした")>,
        std::pair<traits::integral_constant<error_code::minimum_value_not_reached>,
                  S_("最小値に達していません")>,
        std::pair<traits::integral_constant<error_code::maximum_value_exceeded>,
                  S_("最大値を超えました")>,
        std::pair<traits::integral_constant<error_code::minimum_count_not_reached>,
                  S_("最小数に達していません")>,
        std::pair<traits::integral_constant<error_code::maximum_count_exceeded>,
                  S_("最大数を超えました")>,
        std::pair<traits::integral_constant<error_code::unknown_argument_with_suggestion>,
                  S_("不明な引数 {}。 { } という意味でしたか？")>,
        std::pair<traits::integral_constant<error_code::mode_requires_arguments>,
                  S_("モードには引数が必要です")>,
        std::pair<traits::integral_constant<error_code::missing_required_argument>,
                  S_("必要な引数がありません")>,
        std::pair<traits::integral_constant<error_code::too_few_values_for_alias>,
                  S_("エイリアス値が少なすぎる")>,
        std::pair<
            traits::integral_constant<error_code::dependent_argument_missing>,
            S_("従属引数がありません (コマンドラインで必要なトークンの前に置く必要があります)")>,
        std::pair<traits::integral_constant<error_code::one_of_selected_type_mismatch>,
                  S_("一度に許可される「One Of」引数は1つだけです")>,
        std::pair<traits::integral_constant<error_code::missing_value_separator>,
                  S_("値の区切り文字が必要です")>>;
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
    ar::multi_lang::root<S_("en_GB"), S_("fr"), S_("ja")>(  //
        ar::multi_lang::iso_locale(locale_name()),
        [&](auto tr_) {
            using tr = decltype(tr_);

            const auto common_args = ar::list{
                ar::flag(typename tr::force{}, S_('f'){}, typename tr::force_description{}),
                ar::positional_arg<fs::path>(arp::required,
                                             typename tr::destination{},
                                             typename tr::destination_description{},
                                             arp::fixed_count<1>)};

            return ar::root(
                arp::validation::default_validator,
                arp::exception_translator<tr>,
                ar::help(typename tr::help{},
                         S_('h'){},
                         typename tr::help_description{},
                         arp::program_name<S_("simple")>,
                         arp::program_version<S_("v0.1")>,
                         arp::program_intro<typename tr::program_intro>,
                         arp::program_addendum<typename tr::program_addendum>,
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
                ar::mode(arp::none_name<typename tr::move>,
                         arp::description<typename tr::move_description>,
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
