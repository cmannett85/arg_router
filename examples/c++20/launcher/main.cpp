// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <arg_router/arg_router.hpp>

#include <boost/process/args.hpp>
#include <boost/process/child.hpp>
#include <boost/process/search_path.hpp>

#include <cstdlib>
#include <filesystem>

namespace ar = arg_router;
namespace arp = ar::policy;
namespace bp = boost::process;

using namespace std::literals;
using namespace ar::literals;
using namespace ar::utility::string_view_ops;

namespace
{
// NOLINTNEXTLINE(*-avoid-c-arrays)
constexpr char version[] = "v3.14";

void print_invocs(const std::vector<std::string_view>& progs,
                  const std::vector<std::string_view>& args)
{
    for (auto prog : progs) {
        std::cout << prog;
        for (auto arg : args) {
            std::cout << " " << arg;
        }
        std::cout << std::endl;
    }
}

int run_invocs(const std::vector<std::string_view>& progs,
               const std::vector<std::string_view>& args)
{
    auto children = std::vector<bp::child>{};
    children.reserve(progs.size());
    for (auto prog : progs) {
        auto path = boost::filesystem::path{prog};
        if (!path.has_parent_path()) {
            path = bp::search_path(path);
        }
        children.emplace_back(path, bp::args = args);
    }

    auto result = EXIT_SUCCESS;
    for (auto& child : children) {
        child.wait();
        const auto ec = child.exit_code();
        if (ec > result) {
            result = ec;
        }
    }

    return result;
}
}  // namespace

int main(int argc, char* argv[])
{
    ar::root(
        arp::validation::default_validator,
        ar::help("help"_S,
                 "h"_S,
                 "Display this help and exit"_S,
                 arp::program_name_t{"launcher"_S},
                 arp::program_version<ar::str<version>>,
                 arp::program_addendum_t{"An example program for arg_router."_S}),
        ar::flag("version"_S, "Output version information and exit"_S, arp::router{[](bool) {
                     std::cout << &version[0] << std::endl;
                     std::exit(EXIT_SUCCESS);
                 }}),
        ar::mode(
            ar::flag("dry-run"_S, "Just print launch invocations, do not execute them"_S, "d"_S),
            ar::positional_arg<std::vector<std::string_view>>(arp::required,
                                                              "PROGS"_S,
                                                              "Programs to run"_S,
                                                              arp::token_end_marker_t{"--"_S},
                                                              arp::min_count<1>),
            ar::positional_arg<std::vector<std::string_view>>("ARGS"_S,
                                                              "Arguments to pass to programs"_S),
            arp::router{[](bool dry_run,
                           const std::vector<std::string_view>& progs,
                           const std::vector<std::string_view>& args) {
                if (dry_run) {
                    print_invocs(progs, args);
                    return;
                }

                exit(run_invocs(progs, args));
            }}))
        .parse(argc, argv);

    return EXIT_SUCCESS;
}
