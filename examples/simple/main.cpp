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
    const auto common_args =
        ar::list{ar::flag(arp::long_name<S_("force")>,
                          arp::short_name<'f'>,
                          arp::description<S_("Force overwrite existing files")>),
                 ar::positional_arg<fs::path>(arp::required,
                                              arp::display_name<S_("DST")>,
                                              arp::description<S_("Destination directory")>,
                                              arp::fixed_count<1>)};

    ar::root(arp::validation::default_validator,
             ar::help(arp::long_name<S_("help")>,
                      arp::short_name<'h'>,
                      arp::description<S_("Display this help and exit")>,
                      arp::program_name<S_("simple")>,
                      arp::program_version<S_("v0.1")>,
                      arp::program_intro<S_("A simple file copier and mover.")>,
                      arp::program_addendum<S_("An example program for arg_router.")>,
                      arp::flatten_help,
                      arp::colour_help_formatter),
             ar::mode(arp::none_name<S_("copy")>,
                      arp::description<S_("Copy source files to destination")>,
                      common_args,
                      ar::positional_arg<std::vector<fs::path>>(
                          arp::required,
                          arp::display_name<S_("SRC")>,
                          arp::description<S_("Source file paths")>,
                          arp::min_count<1>),
                      arp::router{[](bool force, fs::path dest, std::vector<fs::path> srcs) {
                          copy_mode(force, std::move(dest), std::move(srcs));
                      }}),
             ar::mode(arp::none_name<S_("move")>,
                      arp::description<S_("Move source file to destination")>,
                      common_args,
                      ar::positional_arg<fs::path>(arp::required,
                                                   arp::display_name<S_("SRC")>,
                                                   arp::description<S_("Source file path")>,
                                                   arp::fixed_count<1>),
                      arp::router{[](bool force, fs::path dest, fs::path src) {
                          move_mode(force, std::move(dest), std::move(src));
                      }}))
        .parse(argc, argv);
}
