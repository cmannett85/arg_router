// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <arg_router/arg_router.hpp>

#include "translations/en_GB.hpp"
#include "translations/fr.hpp"
#include "translations/ja.hpp"

#include <cstdlib>
#include <filesystem>

namespace ar = arg_router;
namespace arp = ar::policy;
namespace fs = std::filesystem;

using namespace std::string_view_literals;
using namespace std::string_literals;
using namespace ar::utility::string_view_ops;

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
