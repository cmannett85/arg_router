// Copyright (C) 2022 by Camden Mannett.
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
    ar::multi_lang::root<ar::str<"en_GB">, ar::str<"fr">, ar::str<"ja">>(  //
        ar::multi_lang::iso_locale(locale_name()),
        [&](auto tr_) {
            using tr = decltype(tr_);

            const auto common_args = ar::list{
                ar::flag(arp::long_name<typename tr::force>,
                         arp::short_name_t{"f"_S},
                         arp::description<typename tr::force_description>),
                ar::positional_arg<fs::path>(arp::required,
                                             arp::display_name<typename tr::destination>,
                                             arp::description<typename tr::destination_description>,
                                             arp::fixed_count<1>)};

            return ar::root(
                arp::validation::default_validator,
                ar::help(arp::long_name<typename tr::help>,
                         arp::short_name_t{"h"_S},
                         arp::description<typename tr::help_description>,
                         arp::program_name_t{"simple"_S},
                         arp::program_version_t{"v0.1"_S},
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
