/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include <arg_router/arg_router.hpp>

namespace ar = arg_router;
namespace arp = ar::policy;

int main(int argc, char* argv[])
{
    ar::root(arp::validation::default_validator,
             ar::help(arp::long_name<S_("help")>,
                      arp::short_name<'h'>,
                      arp::program_name<S_("just-cats")>,
                      arp::program_intro<S_("Prints cats!")>,
                      arp::description<S_("Display this help and exit")>),
             ar::flag(arp::long_name<S_("cat")>,
                      arp::description<S_("English cat")>,
                      arp::router{[](bool) { std::cout << "cat" << std::endl; }}),
             ar::flag(arp::short_name_utf8<S_("猫")>,
                      arp::description<S_("日本語の猫")>,
                      arp::router{[](bool) { std::cout << "猫" << std::endl; }}),
             ar::flag(arp::short_name_utf8<S_("🐱")>,
                      arp::description<S_("Emoji cat")>,
                      arp::router{[](bool) { std::cout << "🐱" << std::endl; }}),
             ar::flag(arp::long_name<S_("แมว")>,
                      arp::description<S_("แมวไทย")>,
                      arp::router{[](bool) { std::cout << "แมว" << std::endl; }}),
             ar::flag(arp::long_name<S_("кіт")>,
                      arp::description<S_("український кіт")>,
                      arp::router{[](bool) { std::cout << "кіт" << std::endl; }}))
        .parse(argc, argv);

    return EXIT_SUCCESS;
}