// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <arg_router/arg_router.hpp>

namespace ar = arg_router;
namespace arp = ar::policy;

using namespace ar::literals;
using namespace std::string_view_literals;

namespace
{
constexpr auto version = "v3.14"sv;
constexpr auto license_env_var = "AR_EXAMPLE_LICENSE";
}  // namespace

int main(int argc, char* argv[])
{
    const auto advanced = std::getenv(license_env_var) != nullptr;
    ar::root(
        arp::validation::default_validator,
        ar::help(S_("help"){},
                 S_('h'){},
                 S_("Display this help and exit"){},
                 arp::flatten_help,
                 arp::program_name<S_("runtime_node_enable")>,
                 arp::program_version<S_(version)>,
                 arp::program_addendum<S_("An example program for arg_router.")>),
        ar::flag(S_("version"){},
                 S_("Output version information and exit"){},
                 arp::router{[](bool) {
                     std::cout << version << std::endl;
                     std::exit(EXIT_SUCCESS);
                 }}),
        ar::mode(S_("advanced"){},
                 S_("Advanced features"){},
                 ar::flag(S_("feature1"){}, S_("First feature"){}),
                 // NOLINTNEXTLINE(readability-magic-numbers)
                 ar::arg<int>(S_("feature2"){}, S_("Second feature"){}, arp::default_value{42}),
                 arp::router{[](bool f1, int f2) {
                     std::cout << "F1: " << std::boolalpha << f1 << ", F2: " << f2 << std::endl;
                 }},
                 arp::runtime_enable{advanced}),
        ar::mode(
            ar::flag(S_("foo"){}, S_("Foo flag"){}, S_('f'){}),
            ar::flag(S_("bar"){}, S_("Bar flag"){}, S_('b'){}),
            ar::arg<std::string_view>(S_("advance-foo"){},
                                      S_("Licensed foo"){},
                                      arp::runtime_enable_required<std::string_view>{advanced}),
            arp::router{[](bool f, bool b, std::string_view advance_foo) {
                std::cout << "F: " << std::boolalpha << f << ", B: " << b
                          << ", Advance-foo: " << advance_foo << std::endl;
            }}))
        .parse(argc, argv);

    return EXIT_SUCCESS;
}
