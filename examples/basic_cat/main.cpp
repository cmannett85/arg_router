/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include <arg_router/arg_router.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>

namespace ar = arg_router;
namespace arp = ar::policy;
namespace ard = ar::dependency;

using namespace std::string_view_literals;
using namespace std::string_literals;
using namespace ar::utility::string_view_ops;

namespace
{
enum class theme_t { none, classic, solarized };

enum class verbosity_level_t { error, warning, info, debug };

constexpr auto version = "v3.14"sv;
constexpr auto no_line_limit = -1;

constexpr auto tab_num = std::uint8_t{9};
constexpr auto nl_num = std::uint8_t{10};
constexpr auto printing_range = std::pair{std::uint8_t{32}, std::uint8_t{126}};
constexpr auto ascii_max = std::uint8_t{127};
constexpr auto offset = std::uint8_t{64};

theme_t theme_from_string(std::string_view arg)
{
    if (arg == "none") {
        return theme_t::none;
    } else if (arg == "classic") {
        return theme_t::classic;
    } else if (arg == "solarized") {
        return theme_t::solarized;
    }

    throw ar::parse_exception{"Unknown theme argument: "s + arg};
}

void set_theme(theme_t theme)
{
    // In no-one's world are these a 'theme', but it's just example code...
    switch (theme) {
    case theme_t::classic: std::cout << "\033[31m"; break;
    case theme_t::solarized: std::cout << "\033[32m"; break;
    default: std::cout << "\033[0m"; break;
    }
}

// This is almost certainly wrong for unicode stuff, and it's definitely
// inefficient, but it's only an example
std::string m_notation(std::string input)
{
    auto result = ""s;
    result.reserve(input.size() * 2);

    for (auto c : input) {
        const auto num = static_cast<std::uint8_t>(c);
        if ((num >= printing_range.first && num <= printing_range.second) ||  //
            num == tab_num ||                                                 //
            num == nl_num) {
            result.push_back(c);
        } else if (num <= ascii_max) {
            result += "^"s + static_cast<char>(num + offset);
        } else {
            result += "M-^"s + static_cast<char>(num - offset);
        }
    }

    return result;
}

void cat(bool show_ends,
         bool show_non_printing,
         int max_lines,
         std::optional<std::size_t> max_line_length,
         std::variant<bool, std::string_view> max_line_handling,
         std::vector<std::string_view> files)
{
    if (max_lines == no_line_limit) {
        max_lines = std::numeric_limits<int>::max();
    }

    for (auto file : files) {
        if (!std::filesystem::exists(file)) {
            throw std::invalid_argument{"File does not exist: "s + file};
        }

        auto stream = std::ifstream{file.data()};

        auto line_number = 0;
        for (auto line = ""s; std::getline(stream, line);) {
            if (++line_number == max_lines) {
                break;
            }

            if (max_line_length && (line.size() > *max_line_length)) {
                if (max_line_handling.index() == 0) {
                    // Skip the line
                    continue;
                } else {
                    const auto suffix = std::get<1>(max_line_handling);
                    line = line.substr(0, *max_line_length) + suffix;
                }
            }

            if (show_non_printing) {
                line = m_notation(std::move(line));
            }

            std::cout << line;
            if (show_ends) {
                std::cout << "$";
            }
            std::cout << "\n";
        }
    }
}
}  // namespace

template <>
struct ar::parser<verbosity_level_t> {
    [[nodiscard]] static inline verbosity_level_t parse(std::string_view arg)
    {
        if (arg == "error") {
            return verbosity_level_t::error;
        } else if (arg == "warning") {
            return verbosity_level_t::warning;
        } else if (arg == "info") {
            return verbosity_level_t::info;
        } else if (arg == "debug") {
            return verbosity_level_t::debug;
        }

        throw ar::parse_exception{"Unknown verbosity argument: "s + arg};
    }
};

int main(int argc, char* argv[])
{
    ar::root(
        arp::validation::default_validator,
        ar::help(arp::long_name<S_("help")>,
                 arp::short_name<'h'>,
                 arp::program_name<S_("my-cat")>,
                 arp::program_version<S_(version)>,
                 arp::description<S_("Display this help and exit")>),
        ar::flag(arp::long_name<S_("version")>,
                 arp::description<S_("Output version information and exit")>,
                 arp::router{[](bool) {
                     std::cout << version << std::endl;
                     std::exit(EXIT_SUCCESS);
                 }}),
        ar::mode(
            ar::flag(arp::long_name<S_("show-all")>,
                     arp::description<S_("Equivalent to -nE")>,
                     arp::short_name<'A'>,
                     arp::alias(arp::short_name<'E'>, arp::short_name<'n'>)),
            ar::flag(arp::long_name<S_("show-ends")>,
                     arp::description<S_("Display $ at end of each line")>,
                     arp::short_name<'E'>),
            ar::flag(arp::long_name<S_("show-nonprinting")>,
                     arp::description<S_(
                         "Use ^ and M- notation, except for LFD and TAB")>,
                     arp::short_name<'n'>),
            ar::arg<int>(arp::long_name<S_("max-lines")>,
                         arp::description<S_("Maximum lines to output")>,
                         arp::value_separator<'='>,
                         arp::default_value{-1}),
            ar::arg<std::optional<std::size_t>>(
                arp::long_name<S_("max-line-length")>,
                arp::description<S_("Maximum line length")>,
                arp::value_separator<'='>,
                arp::default_value{std::optional<std::size_t>{}}),
            ard::one_of(
                arp::default_value{"..."},
                ar::flag(
                    arp::dependent(arp::long_name<S_("max-line-length")>),
                    arp::long_name<S_("skip-line")>,
                    arp::short_name<'s'>,
                    arp::description<S_("Skips line output if max line length "
                                        "reached")>),
                ar::arg<std::string_view>(
                    arp::dependent(arp::long_name<S_("max-line-length")>),
                    arp::long_name<S_("line-suffix")>,
                    arp::description<S_(
                        "Shortens line length to maximum with the "
                        "given suffix if max line length reached")>,
                    arp::value_separator<'='>)),
            ar::arg<theme_t>(
                arp::long_name<S_("theme")>,
                arp::description<S_("Set the output colour theme")>,
                arp::value_separator<'='>,
                arp::default_value{theme_t::none},
                arp::custom_parser<theme_t>{[](std::string_view arg) {
                    return theme_from_string(arg);
                }}),
            ard::alias_group(
                arp::default_value{verbosity_level_t::info},
                ar::counting_flag<verbosity_level_t>(
                    arp::short_name<'v'>,
                    arp::description<S_(
                        "Verbosity level, number of 'v's sets level")>),
                ar::arg<verbosity_level_t>(
                    arp::long_name<S_("verbose")>,
                    arp::description<S_("Verbosity level")>,
                    arp::value_separator<'='>),
                arp::min_max_value{verbosity_level_t::error,
                                   verbosity_level_t::debug}),
            ar::positional_arg<std::vector<std::string_view>>(
                arp::required,
                arp::display_name<S_("FILES")>,
                arp::description<S_("Files to read")>),
            arp::router{
                [](bool show_ends,
                   bool show_non_printing,
                   int max_lines,
                   std::optional<std::size_t> max_line_length,
                   std::variant<bool, std::string_view> max_line_handling,
                   theme_t theme,
                   verbosity_level_t /*verbosity_level*/,
                   std::vector<std::string_view> files) {
                    set_theme(theme);
                    cat(show_ends,
                        show_non_printing,
                        max_lines,
                        max_line_length,
                        max_line_handling,
                        std::move(files));
                    set_theme(theme_t::none);
                }}))
        .parse(argc, argv);

    return EXIT_SUCCESS;
}
