/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include <arg_router/arg_router.hpp>

#include <cstdlib>
#include <filesystem>

namespace ar = arg_router;
namespace arp = ar::policy;
namespace fs = std::filesystem;

using namespace std::string_view_literals;
using namespace std::string_literals;
using namespace ar::utility::string_view_ops;

namespace
{
// You can use an env var to override the locale for testing (doesn't affect your machine's real
// locale)
std::string locale_name()
{
    if (auto env_locale = std::getenv("AR_LOCALE_OVERRIDE")) {
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

template <>
struct ar::parser<fs::path> {
    [[nodiscard]] static inline fs::path parse(std::string_view arg) { return arg; }
};

int main(int argc, char* argv[])
{
    // Apologies for any translation faux pas - Google Translate did it for me!
    ar::multi_lang::root_wrapper<S_("en_GB"), S_("fr"), S_("ja")>(  //
        ar::multi_lang::iso_locale(locale_name()),
        [&](auto I) {
            const auto common_args = ar::list{
                ar::flag(arp::long_name<SM_(I, "force", "forcer", "強制")>,
                         arp::short_name<'f'>,
                         arp::description<SM_(I,
                                              "Force overwrite existing files",
                                              "Forcer l'écrasement des fichiers existants",
                                              "既存のファイルを強制的に上書きする")>),
                ar::positional_arg<fs::path>(arp::required,
                                             arp::display_name<SM_(I, "DST", "DST", "先")>,
                                             arp::description<SM_(I,
                                                                  "Destination directory",
                                                                  "Répertoire de destination",
                                                                  "宛先ディレクトリ")>,
                                             arp::fixed_count<1>)};

            return ar::root(
                arp::validation::default_validator,
                ar::help(arp::long_name<SM_(I, "help", "aider", "ヘルプ")>,
                         arp::short_name<'h'>,
                         arp::description<SM_(I,
                                              "Display this help and exit",
                                              "Afficher cette aide et quitter",
                                              "このヘルプを表示して終了")>,
                         arp::program_name<S_("simple")>,
                         arp::program_version<S_("v0.1")>,
                         arp::program_intro<SM_(
                             I,
                             "A simple file copier and mover.",
                             "Un simple copieur et déménageur de fichiers.",
                             "ファイルをコピーおよび移動するためのシンプルなプログラム。")>,
                         arp::program_addendum<SM_(I,
                                                   "An example program for arg_router.",
                                                   "Un exemple de programme pour arg_router.",
                                                   "「arg_router」のサンプルプログラム。")>,
                         arp::flatten_help,
                         arp::colour_help_formatter),
                ar::mode(arp::none_name<SM_(I, "copy", "copier", "コピー")>,
                         arp::description<SM_(I,
                                              "Copy source files to destination",
                                              "Copier les fichiers source vers la destination",
                                              "ソース ファイルを宛先にコピーする")>,
                         common_args,
                         ar::positional_arg<std::vector<fs::path>>(
                             arp::required,
                             arp::display_name<SM_(I, "SRC", "SRC", "出典")>,
                             arp::description<SM_(I,
                                                  "Source file paths",
                                                  "Chemins des fichiers sources",
                                                  "ソース ファイルのパス")>,
                             arp::min_count<1>),
                         arp::router{[](bool force, fs::path dest, std::vector<fs::path> srcs) {
                             copy_mode(force, std::move(dest), std::move(srcs));
                         }}),
                ar::mode(
                    arp::none_name<SM_(I, "move", "déplacer", "移動")>,
                    arp::description<SM_(I,
                                         "Move source file to destination",
                                         "Déplacer le fichier source vers la destination",
                                         "ソース ファイルを宛先に移動する")>,
                    common_args,
                    ar::positional_arg<fs::path>(arp::required,
                                                 arp::display_name<SM_(I, "SRC", "SRC", "出典")>,
                                                 arp::description<SM_(I,
                                                                      "Source file path",
                                                                      "Chemin du fichier source",
                                                                      "ソース ファイル パス")>,
                                                 arp::fixed_count<1>),
                    arp::router{[](bool force, fs::path dest, fs::path src) {
                        move_mode(force, std::move(dest), std::move(src));
                    }}));
        })
        .parse(argc, argv);
}
