// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <arg_router/arg_router.hpp>

namespace ar = arg_router;
namespace arp = ar::policy;

int main(int argc, char* argv[])
{
    ar::root(arp::validation::default_validator,
             ar::help(S_("help"){},
                      S_('h'){},
                      S_("Display this help and exit"){},
                      arp::program_name<S_("just-cats")>,
                      arp::program_intro<S_("Prints cats!")>,
                      arp::program_addendum<S_("An example program for arg_router.")>),
             ar::flag(S_("cat"){},  //
                      S_("English cat"){},
                      arp::router{[](bool) { std::cout << "cat" << std::endl; }}),
             ar::flag(S_("猫"){},  //
                      arp::description<S_("日本語の猫")>,
                      arp::router{[](bool) { std::cout << "猫" << std::endl; }}),
             ar::flag(S_("🐱"){},  //
                      arp::description<S_("Emoji cat")>,
                      arp::router{[](bool) { std::cout << "🐱" << std::endl; }}),
             ar::flag(S_("แมว"){},  //
                      S_("แมวไทย"){},
                      arp::router{[](bool) { std::cout << "แมว" << std::endl; }}),
             ar::flag(S_("кіт"){},  //
                      S_("український кіт"){},
                      arp::router{[](bool) { std::cout << "кіт" << std::endl; }}))
        .parse(argc, argv);

    return EXIT_SUCCESS;
}
