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

using namespace std::string_view_literals;
using namespace std::string_literals;
using namespace ar::utility::string_view_ops;

namespace
{
constexpr auto version = "v3.14"sv;

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
        ar::help(S_("help"){},
                 S_('h'){},
                 S_("Display this help and exit"){},
                 arp::program_name<S_("launcher")>,
                 arp::program_version<S_(version)>,
                 arp::program_addendum<S_("An example program for arg_router.")>),
        ar::flag(S_("version"){},
                 S_("Output version information and exit"){},
                 arp::router{[](bool) {
                     std::cout << version << std::endl;
                     std::exit(EXIT_SUCCESS);
                 }}),
        ar::mode(ar::flag(S_("dry-run"){},
                          S_("Just print launch invocations, do not execute them"){},
                          S_('d'){}),
                 ar::positional_arg<std::vector<std::string_view>>(arp::required,
                                                                   S_("PROGS"){},
                                                                   S_("Programs to run"){},
                                                                   arp::token_end_marker<S_("--")>,
                                                                   arp::min_count<1>),
                 ar::positional_arg<std::vector<std::string_view>>(
                     S_("ARGS"){},
                     S_("Arguments to pass to programs"){}),
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
