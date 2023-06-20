// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <arg_router/arg_router.hpp>

namespace ar = arg_router;
namespace arp = ar::policy;

using namespace ar::literals;

namespace
{
// NOLINTNEXTLINE(*-avoid-c-arrays)
constexpr char version[] = "v3.14";

constexpr auto license_env_var = "AR_EXAMPLE_LICENSE";
}  // namespace

int main(int argc, char* argv[])
{
    const auto advanced = std::getenv(license_env_var) != nullptr;
    ar::root(
        arp::validation::default_validator,
        ar::help("help"_S,
                 "h"_S,
                 "Display this help and exit"_S,
                 arp::flatten_help,
                 arp::program_name_t{"runtime_node_enable"_S},
                 arp::program_version<ar::str<version>>,
                 arp::program_addendum_t{"An example program for arg_router."_S}),
        ar::flag("version"_S, "Output version information and exit"_S, arp::router{[](bool) {
                     std::cout << &version[0] << std::endl;
                     std::exit(EXIT_SUCCESS);
                 }}),
        ar::mode("advanced"_S,
                 "Advanced features"_S,
                 ar::flag("feature1"_S, "First feature"_S),
                 // NOLINTNEXTLINE(readability-magic-numbers)
                 ar::arg<int>("feature2"_S, "Second feature"_S, arp::default_value{42}),
                 arp::router{[](bool f1, int f2) {
                     std::cout << "F1: " << std::boolalpha << f1 << ", F2: " << f2 << std::endl;
                 }},
                 arp::runtime_enable{advanced}),
        ar::mode(
            ar::flag("foo"_S, "Foo flag"_S, "f"_S),
            ar::flag("bar"_S, "Bar flag"_S, "b"_S),
            ar::arg<std::string_view>("advance-foo"_S,
                                      "Licensed foo"_S,
                                      arp::runtime_enable_required<std::string_view>{advanced}),
            arp::router{[](bool f, bool b, std::string_view advance_foo) {
                std::cout << "F: " << std::boolalpha << f << ", B: " << b
                          << ", Advance-foo: " << advance_foo << std::endl;
            }}))
        .parse(argc, argv);

    return EXIT_SUCCESS;
}
