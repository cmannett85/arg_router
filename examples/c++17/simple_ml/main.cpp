// Copyright (C) 2022 by Camden Mannett.
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
    // Apologies for any translation faux pas - Google Translate did it for me!
    ar::multi_lang::root<S_("en_GB"), S_("fr"), S_("ja")>(  //
        ar::multi_lang::iso_locale(locale_name()),
        [&](auto tr_) {
            using tr = decltype(tr_);

            const auto common_args = ar::list{
                ar::flag(arp::long_name<typename tr::force>,
                         arp::short_name<'f'>,
                         arp::description<typename tr::force_description>),
                ar::positional_arg<fs::path>(arp::required,
                                             arp::display_name<typename tr::destination>,
                                             arp::description<typename tr::destination_description>,
                                             arp::fixed_count<1>)};

            return ar::root(
                arp::validation::default_validator,
                ar::help(arp::long_name<typename tr::help>,
                         arp::short_name<'h'>,
                         arp::description<typename tr::help_description>,
                         arp::program_name<S_("simple")>,
                         arp::program_version<S_("v0.1")>,
                         arp::program_intro<typename tr::program_intro>,
                         arp::program_addendum<typename tr::program_addendum>,
                         arp::flatten_help,
                         arp::colour_help_formatter),
                ar::mode(arp::none_name<typename tr::copy>,
                         arp::description<typename tr::copy_description>,
                         common_args,
                         ar::positional_arg<std::vector<fs::path>>(
                             arp::required,
                             arp::display_name<typename tr::source>,
                             arp::description<typename tr::sources_description>,
                             arp::min_count<1>),
                         arp::router{[](bool force, fs::path dest, std::vector<fs::path> srcs) {
                             copy_mode(force, std::move(dest), std::move(srcs));
                         }}),
                ar::mode(
                    arp::none_name<typename tr::move>,
                    arp::description<typename tr::move_description>,
                    common_args,
                    ar::positional_arg<fs::path>(arp::required,
                                                 arp::display_name<typename tr::source>,
                                                 arp::description<typename tr::source_description>,
                                                 arp::fixed_count<1>),
                    arp::router{[](bool force, fs::path dest, fs::path src) {
                        move_mode(force, std::move(dest), std::move(src));
                    }}));
        })
        .parse(argc, argv);
}
