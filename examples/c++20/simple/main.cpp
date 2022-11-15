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
        ar::list{ar::flag(arp::long_name_t{"force"_S},
                          arp::short_name_t{"f"_S},
                          arp::description_t{"Force overwrite existing files"_S}),
                 ar::positional_arg<fs::path>(arp::required,
                                              arp::display_name_t{"DST"_S},
                                              arp::description_t{"Destination directory"_S},
                                              arp::fixed_count<1>)};

    ar::root(arp::validation::default_validator,
             ar::help(arp::long_name_t{"help"_S},
                      arp::short_name_t{"h"_S},
                      arp::description_t{"Display this help and exit"_S},
                      arp::program_name_t{"simple"_S},
                      arp::program_version_t{"v0.1"_S},
                      arp::program_intro_t{"A simple file copier and mover."_S},
                      arp::program_addendum_t{"An example program for arg_router."_S},
                      arp::flatten_help,
                      arp::colour_help_formatter),
             ar::mode(arp::none_name_t{"copy"_S},
                      arp::description_t{"Copy source files to destination"_S},
                      common_args,
                      ar::positional_arg<std::vector<fs::path>>(
                          arp::required,
                          arp::display_name_t{"SRC"_S},
                          arp::description_t{"Source file paths"_S},
                          arp::min_count<1>),
                      arp::router{[](bool force, fs::path dest, std::vector<fs::path> srcs) {
                          copy_mode(force, std::move(dest), std::move(srcs));
                      }}),
             ar::mode(arp::none_name_t{"move"_S},
                      arp::description_t{"Move source file to destination"_S},
                      common_args,
                      ar::positional_arg<fs::path>(arp::required,
                                                   arp::display_name_t{"SRC"_S},
                                                   arp::description_t{"Source file path"_S},
                                                   arp::fixed_count<1>),
                      arp::router{[](bool force, fs::path dest, fs::path src) {
                          move_mode(force, std::move(dest), std::move(src));
                      }}))
        .parse(argc, argv);
}
