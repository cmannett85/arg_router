![Documentation Generator](https://github.com/cmannett85/arg_router/workflows/Documentation%20Generator/badge.svg) [![Merge to main Checker](https://github.com/cmannett85/arg_router/actions/workflows/merge_checker.yml/badge.svg)](https://github.com/cmannett85/arg_router/actions/workflows/merge_checker.yml) ![Unit test coverage](https://img.shields.io/badge/Unit_Test_Coverage-97.6%25-brightgreen)

# arg_router
`arg_router` is a C++17/20 command line parser and router.  It uses policy-based objects hierarchically, so the parsing code is self-describing.  Rather than just providing a parsing service that returns a map of `variant`s/`any`s, it allows you to bind `Callable` instances to points in the parse structure, so complex command line arguments can directly call functions with the expected arguments - rather than you having to do this yourself.

## Features
- Use policies to define the properties and constraints of arguments at compile-time
- Group arguments together to define mutually exclusive operating modes, for more complex applications
- Define logical connections between arguments
- Detects invalid or ambiguous parse trees at compile-time
- Generates its help output, which you can modify at runtime using a `Callable`, or tweak its formatting using policies
- Easy custom parsers by using `Callable`s inline for specific arguments, or you can implement a specialisation to cover all instances of that type
- Unicode compliant by supporting UTF-8 encoded compile-time strings ([details](#unicode-compliance))
- Support of runtime language selection
- Uses a macro to ease compile-time string generation when using C++17.  For C++20 and above, compile-time string literals can be used directly in constructors
- [Available](https://github.com/microsoft/vcpkg/tree/master/ports/arg-router) on vcpkg!
- [Available](https://conan.io/center/arg_router) on Conan Center!

### Example of the Benefits of a Compile-Time Parse Tree
It's not immediately obvious why defining a parse tree at compile would bring any benefits, so before we show you _how_ `arg_router` is used, let us show you _why_.  Here is a very contrived parse tree defined using the very popular and well-made [argparse](https://github.com/p-ranav/argparse):
```cpp
#include <argparse/argparse.hpp>

namespace ap = argparse;

int main(int argc, char* argv[])
{
    auto program = ap::ArgumentParser{"fail!"};
    program.add_argument("--verbose", "-v")
        .help("Lots of output")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("--value", "-v")
        .help("Do something with a value")
        .default_value(false)
        .implicit_value(true);

    program.parse_args(argc, argv);
    std::cout << program.get<bool>("--verbose") << ", "
              << program.get<bool>("--value") << std::endl;

    return EXIT_SUCCESS;
}
```
This parser has an easy to make error, both flags have a short name that is the same, resulting in an ambiguous parse tree.  In this example it stands out as the whole program is so small, but it wouldn't be so obvious on a larger, more real-world program.  Let's build and run it:
```
$ ./fail --verbose
1, 0
$ ./fail -v
0, 1
```
There is no runtime error detection so you would need to write a test for this scenario.  Let's try the equivalent in `arg_router`:
```cpp
#include <arg_router/arg_router.hpp>

namespace ar = arg_router;
namespace arp = ar::policy;
using namespace ar::literals;

int main(int argc, char* argv[])
{
    ar::root(arp::validation::default_validator,
             ar::help("help"_S, "h"_S, arp::program_name_t{"fail!"_S}),
             ar::mode(ar::flag("verbose"_S, "v"_S, "Lots of output"_S),
                      ar::flag("value"_S, "v"_S, "Do something with a value"_S),
                      arp::router{[](auto verbose, auto value) {
                          std::cout << verbose << ", " << value << std::endl;
                      }}))
        .parse(argc, argv);

    return EXIT_SUCCESS;
}
```
Let's build it:
```
arg_router/policy/validator.hpp:206:17: error: static_assert failed due to requirement '!std::is_same_v<arg_router::policy::short_name_t<arg_router::utility::str<{{{118, 0}}}>>, arg_router::policy::short_name_t<arg_router::utility::str<{{{118, 0}}}>>>' "Policy must be unique in the parse tree up to the nearest mode or root"
                static_assert(!std::is_same_v<Policy, Current>,
                ^             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
```
`"Policy must be unique in the parse tree up to the nearest mode or root"` is a formal way of saying that you've got a duplicate policy, and it's a `policy::short_name_t`.  By having the tree defined at compile-time, it can verify itself via C++ template metaprogramming and cause a build failure.  This is a simple example, but there is a great many more checks done during compilation to help you write safer code with less explicit testing.

## Installation
There are several ways to install `arg_router`, the most appropriate depends on your project's configuration.
* If using vcpkg as your package manager, simply add `arg-router` (note the hyphen) to `dependencies` in your `vcpkg.json`
* If using Conan as your package manager, simply add `arg_router` to your requirements
* If using a DEB package based Linux distribution, download the [release](https://github.com/cmannett85/arg_router/releases) Debian package and install it
* If you just want to do a traditional install then download the [release](https://github.com/cmannett85/arg_router/releases) zip file and decompress it where you want.  Then point your project at the `include/arg_router` directory or package config location `share/arg_router`
* Same as above but installed from the checked out repo, i.e. using the invocation `cmake ../arg_router -DINSTALLATION_ONLY=ON; cmake --install .` (see [developer installation](https://github.com/cmannett85/arg_router/wiki/Developer-Installation))

### Dependencies
If you're a library user, you will need the following dependencies in order to build:
* Boost.mp11 v1.74+
* Boost.Lexical_Cast v1.74+
* Boost.Preprocessor v1.74+ (only needed if building against C++17)
* [span-lite](https://github.com/martinmoene/span-lite) (only needed if building against C++17)

If you're a vcpkg user, then these will be brought in automatically for you.  `arg_router` is header-only (due to all the templates) and so are the above dependencies.

**Note** currently `arg_router` requires exception support, but _not_ RTTI.

## Basics
Let's start simple, with this `cat`-like program:
```cpp
namespace ar = arg_router;
namespace arp = ar::policy

using namespace ar::literals;

ar::root(
    arp::validation::default_validator,
    ar::help(
        arp::long_name_t{"help"_S},
        arp::short_name_t{"h"_S},
        arp::program_name_t{"my-cat"_S},
        arp::program_version_t{"v3.14"_S},
        arp::description_t{"Display this help and exit"_S}),
    ar::flag(
        arp::long_name_t{"version"},
        arp::description_t{"Output version information and exit"_S},
        arp::router{[](bool v) { ... }}),
    ar::mode(
        ar::flag(
            arp::long_name_t{"show-all"_S},
            arp::description_t{"Equivalent to -nE"_S},
            arp::short_name_t{"A"_S},
            arp::alias_t{arp::short_name_t{"E"_S}, arp::short_name_t{"n"_S}}),
        ar::flag(
            arp::long_name_t{"show-ends"_S},
            arp::description_t{"Display $ at end of each line"_S},
            arp::short_name_t{"E"_S}),
        ar::flag(
            arp::long_name_t{"show-nonprinting"_S},
            arp::description_t{"Use ^ and M- notation, except for LFD and TAB"_S},
            arp::short_name_t{"n"_S}),
        ar::arg<int>(
            arp::long_name_t{"max-lines"_S},
            arp::description_t{"Maximum lines to output"_S},
            arp::value_separator_t{"="_S},
            arp::default_value{-1}),
        ar::positional_arg<std::vector<std::string_view>>(
            arp::required,
            arp::min_count<1>,
            arp::display_name_t{"FILES"_S},
            arp::description_t{"Files to read"+S}),
        arp::router{[](bool show_ends,
                       bool show_non_printing,
                       int max_lines,
                       std::vector<std::string_view> files) { ... }}))
    .parse(argc, argv);
```
Let's start from the top, as the name suggests `root` is the root of the parse tree and provides the `parse(argc, argv)` method.  Only children of the root can (and must) have a `router` policy (except for nested `mode`s, more [on that later](#modes)) and therefore act as access points into the program. The root's children are implicitly mutually exclusive, so trying to pass `--version --help` in the command line is a runtime error.

The `arp::validation::default_validator` instance provides the default validator that the root uses to validate the parse tree at compile-time.  It is a required policy of the `root`.  Unless you have implemented your own policy or tree node you will never need to specify anything else.

The `help` node is used by the `root` to generate the argument documentation for the help output, by default it just prints directly to the console and then exits, but a `router` can be attached that accepts the formatted output to do something else with it.  The optional `program_name_t` and `program_version_t` policies add a header to the help output.

Now let's introduce some 'policies'.  Policies define common behaviours across node types, a basic one is `long_name_t` which provides long-form argument definition.  By default, a standard unix double hyphen prefix for long names is added automatically.  `short_name_t` is the single character short-form name, by default a single hyphen is prefixed automatically.  `arg_router` supports short name collapsing for flags, so if you have defined flags like `-a -b -c` then `-abc` will be accepted or `-bca`, etc. (**note** short-form name collapsing is disabled if the library has been configured to have the same long and short prefix, which is common on Windows).

Compile-time strings are created via the `""_S` string literal, which creates an instance of a `ar::str` type.  **Note** Advanced NTTP language support that allows for this is not present until C++20, see [compile-time string support](#compile-time-string-support) for what to do in C++17.

In order to group arguments under a specific operating mode, you put them under a `mode` instance.  In this case our simple cat program only has one mode, so it is anonymous i.e. there's no long name or description associated with it - it is a build error to have more than one anonymous mode under the root of a parse tree.

`arg<T>` does exactly what you would expect, it defines an argument that expects a value to follow it on the command line.  If an argument is not `required` then it may have a `default_value` (if neither are set, then a default initialised value is used instead), this is passed to the `router`'s `Callable` on parsing if it isn't specified by the user on the command line.

A `flag` is essentially an `arg<bool>{default_value{false}}`, except that it doesn't expect an argument value to follow on the command line as it _is_ the value.  Flags cannot have default arguments or be marked as required.

An `alias` policy allows you to define an argument that acts as a link to other arguments, so in our example above passing `-A` on the command line would actually set the `-E` and `-n` flags to true.  You can use either the long or short name of the aliased flag, but the `value_type`s (`bool` for a flag) must be the same.

By default whitespace is used to separate out the command line tokens, this is done by the terminal/OS invoking the program, but often '=' is used a name/value token separator.  `arg_router` supports this with the `value_separator_t` policy as used in the `arg<int>` node in the example.

`positional_arg<T>` does not use a 'marker' token on the command line for which its value follows, the value's position in the command line arguments determines what it is for.  The order that arguments are specified on the command line normally don't matter, but for positional arguments they do; for example in our cat program the files must be specified after the arguments so passing `myfile.hpp -n` would trigger the parser to land on the `positional_arg` for `myfile.hpp` which would then greedily consume the `-n` causing the application to try to open the file `-n`...  We'll cover constrained `positional_arg`s in later examples.  The `display_name_t` policy is used when generating help or error output - it is not used when parsing.

Assuming parsing was successful, the final `router` is called with the parsed argument e.g. if the user passed `-E file1 file2` then the `router` is passed `(true, false, -1, {"file1", "file2"})`.

You may have noticed that the nodes are constructed with parentheses whilst the policies use braces, this is necessary due to CTAD rules that affect nodes which return a user-defined value type.  This can be circumvented using a function to return the required instance, for example the actual type of a flag is `flag_t`, `flag(...)` is a factory function.

### Implicit String Policies
Although explicit, which may make it easier to read, the name policies are also verbose.  To ease this most of the built-in nodes support implicit string-to-policy mapping, which allows bare compile-time strings to be passed to the node factory functions which are then mapped to appropriate built-in policies.  The rules vary from node to node, but typically they are:
1. The first multi-character string becomes a `policy::long_name_t`
2. The second multi-character string becomes a `policy::description_t`
3. The first single-character string becomes a `policy::short_name_t`
The above are unicode aware.  The strings can be passed in any order relative to the other policies, but it is recommended to put them first to ease reading.

Re-writing the `cat`-like program using implicit strings shortens it considerably:
```cpp
ar::root(
    arp::validation::default_validator,
    ar::help("help"_S, "h"_S, "Display this help and exit"_S,
        arp::program_name_t{"my-cat"_S},
        arp::program_version_t{"v3.14"_S}),
    ar::flag("version"_S, "Output version information and exit"_S,
        arp::router{[](bool v) { ... }}),
    ar::mode(
        ar::flag("show-all"_S, "A"_S, "Equivalent to -nE"_S,
            arp::alias_t{arp::short_name_t{"E"_S}, arp::short_name_t{"n"_S}}),
        ar::flag("show-ends"_S, "E"_S, "Display $ at end of each line"_S),
        ar::flag("show-nonprinting"_S, "n"_S, "Use ^ and M- notation, except for LFD and TAB"_S),
        ar::arg<int>("max-lines"_S, "Maximum lines to output"_S,
            arp::value_separator_t{"="_S},
            arp::default_value{-1}),
        ar::positional_arg<std::vector<std::string_view>>("FILES"_S, "Files to read"_S,
            arp::required,
            arp::min_count<1>),
        arp::router{[](bool show_ends,
                       bool show_non_printing,
                       int max_lines,
                       std::vector<std::string_view> files) { ... }}))
    .parse(argc, argv);
```
This documentation will use implicit string policies going forward, but new nodes will always be introduced with explicit policies.

## Conditional Arguments
Let's add another feature to our cat program where we can handle lines over a certain length differently.
```cpp
namespace ard = ar::dependency;
ar::mode(
    ...
    ar::arg<std::optional<std::size_t>>("max-line-length"_S, "Maximum line length"_S,
        arp::value_separator_t{"="_S},
        arp::default_value{std::optional<std::size_t>{}}),
    ard::one_of(
        arp::default_value{"..."},
        ar::flag("skip-line"_S, "s"_S, "Skips line output if max line length reached"_S,
            arp::dependent_t{arp::long_name_t{"max-line-length"_S}}),
        ar::arg<std::string_view>("line-suffix"_S,
            "Shortens line length to maximum with the given suffix if max line length reached"_S,
            arp::dependent_t{arp::long_name_t{"max-line-length"_S}},
            arp::value_separator_t{"="_S})),
    ...
    arp::router{[](bool show_all,
                   bool show_ends,
                   bool show_non_printing,
                   int max_lines,
                   std::optional<std::size_t> max_line_length,
                   std::variant<bool, std::string_view> max_line_handling,
                   std::vector<std::string_view>> files) { ... }}))
```
We've defined a new argument `--max-line-length` but rather than using `-1` as the "no limit" indicator like we did for `--max-lines`, we specify the argument type to be `std::optional<std::size_t>` and have the default value be an empty optional - this allows the code to define our intent better.

What do we do with lines that reach the limit if it has been set?  In our example we can either skip the line output, or truncate it with a suffix.  It doesn't make any sense to allow both of these options, so we declare them under a `one_of` node.  Under this node, only one is valid when parsing at runtime, if the user specifies both then it is an error.  A `one_of` must be marked as required or have a default value in case the user passes none of the arguments it handles.

To express the 'one of' concept better in code, the `one_of` node has a single representation in the `router`'s arguments - a variant that encompasses all the value types of each entry in it.  In our example's case, a bool for the `--skip-line` flag and a `string_view` for the `--line-suffix` case.

What happens if a user passes `--skip-line` or `--line-suffix` without `--max-line-length`?  Normally the developer will have to check that `max-line-length` is not empty and either ignore or throw if it is.  But by specifying the `one_of` as `dependent_t` on `max-line-length`, `arg_router` will throw on your behalf in this scenario.  To be clear, the `dependent_t` policy just means that the owner cannot appear on the command line without the `arg` or `flag` named in its parameters also appearing on the command line.

The smart developer may have noticed an edge case here.  If both the children in the above example had the same `value_type`, then the variant will be created with multiples of the same type.  `std::variant` supports this but it means that the common type-based visitation pattern will become ambiguous on those identical types i.e. you won't know which flag was used.  This may or may not be important to you, but if it is, you will have to `switch` on `std::variant::index()` instead.

## Custom Parsing
Firstly, custom parsing is only needed if the type does not have a constructor that takes a `std::string_view` (or a type implicitly convertible from it).

Custom parsing for `arg` and `positional_arg` types can be done in one of two ways, the first is using a `custom_parser` policy.  Let's add theme coloring to our console output.
```cpp
enum class theme_t {
    NONE,
    CLASSIC,
    SOLARIZED
};

ar::mode(
    ...
    ar::arg<theme_t>("theme"_S, "Set the output colour theme"_S,
        arp::value_separator_t{"="_S},
        arp::default_value{theme_t::NONE},
        arp::custom_parser<theme_t>{[](std::string_view arg) {
            return theme_from_string(arg); }})
    ...
    arp::router{[](bool show_all,
        bool show_ends,
        bool show_non_printing,
        int max_lines,
        std::optional<std::size_t> max_line_length,
        std::variant<bool, std::string_view> max_line_handling,
        theme_t theme,
        std::vector<std::string_view>> files) { ... }}))
```
This is a convenient solution to one-off type parsing in-tree, I think it's pretty self-explanatory.  However this is not convenient if you have lots of arguments that should return the same custom type, as you would have to copy and paste the lambda or bind calls to the conversion function for each one.  In that case we can specialise on the `arg_router::parse` function.
```cpp
template <>
struct arg_router::parser<theme_t> {
    [[nodiscard]] static inline theme_t parse(std::string_view arg)
    {
        if (arg == "NONE") {
            return theme_t::NONE;
        } else if (arg == "CLASSIC") {
            return theme_t::CLASSIC;
        } else if (arg == "SOLARIZED") {
            return theme_t::SOLARIZED;
        }
    
        throw parse_exception{"Unknown theme argument: "s + arg};
    }
};
```
With this declared in a place visible to the parse tree declaration, `theme_t` can be converted from a string without the need for a `custom_parser`.  It should be noted that `custom_parser` can still be used, and will be preferred over the `parse()` specialisation.

## Counting Flags
Another Unix feature that is fairly common is flags that are repeatable i.e. you can declare it multiple times on the command line and it's value will increase with each repeat.  A classic example of this is 'verbosity levels' for program output:
```cpp
enum class verbosity_level_t {
    ERROR,
    WARNING,
    INFO,
    DEBUG
};

ar::mode(
    ...
    ar::counting_flag<verbosity_level_t>("v"_S,
        arp::max_value<verbosity_level_t::DEBUG>(),
        arp::description_t{"Verbosity level, number of 'v's sets level"_S},
        arp::default_value{verbosity_level_t::ERROR}),
    ...
    arp::router{[](bool show_all,
        bool show_ends,
        bool show_non_printing,
        int max_lines,
        std::optional<std::size_t> max_line_length,
        std::variant<bool, std::string_view> max_line_handling,
        theme_t theme,
        verbosity_level_t verbosity_level,
        std::vector<std::string_view>> files) { ... }}))
```
The number of times `-v` appears in the command line will increment the returned value, e.g. `-vv` will result in `verbosity_level_t::INFO`.

Note that even though we are using a custom enum, we haven't specified a `custom_parser`.  `counting_flag` will count up in `std::size_t` and then `static_cast` to the user-specified type, so as long as your requested type is explicitly convertible to/from `std::size_t` it will just work.  If it isn't, then you'll have to modify your type or not use counting arguments, as attaching a `custom_parser` to any kind of flag will result in a build failure.

Short name collapsing still works as expected, so passing `-Evnv` will result in `show_ends` and `show_non_printing` being true, and `verbosity_level` will be `verbosity_level_t::INFO` in the `router` call.

We can constrain the amount of flags the user can provide by using the `max_value` policy, so passing `-vvvv` will result in a runtime error.  There are min/max and min variants too.  Here we are using the compile-time variant of the policy, we can do that because the value type is an integral/enum, but if your value type cannot be used as a template parameter then there equivalent runtime variants that take the parameters as function parameters instead.  The compile-time variants should be used when possible due to extra checks e.g. setting the max value less than the min.

The `long_name_t` policy is allowed but usually leads to ugly invocations, however there is a better option:
```cpp
ar::mode(
    ...
    ard::alias_group(
        arp::default_value{verbosity_level_t::INFO},
        arp::max_value<verbosity_level_t::DEBUG>(),
        ar::counting_flag<verbosity_level_t>("v"_S,
            arp::description_t{"Verbosity level, number of 'v's sets level"_S}),
        ar::arg<verbosity_level_t>("verbose"_S, "Verbosity level"_S,
            arp::value_separator_t{"="_S})),
    ...
    arp::router{[](bool show_all,
        bool show_ends,
        bool show_non_printing,
        int max_lines,
        std::optional<std::size_t> max_line_length,
        std::variant<bool, std::string_view> max_line_handling,
        theme_t theme,
        verbosity_level_t verbosity_level,
        std::vector<std::string_view>> files) { ... }}))
    ...
    
template <>
struct arg_router::parser<verbosity_level_t> {
    [[nodiscard]] static inline verbosity_level_t parse(std::string_view arg)
    {
        ...
    }
};
```
We can declare a new `arg` that takes a string equivalent of the enum and put them both into an `alias_group`, so now you can use `--verbose=INFO` or `-vv`.  Short name collapsing still works as expected.

What's this new `alias_group`?  `policy::alias` is an _input_ alias, it works by duplicating the value tokens to each of the aliased nodes it refers to i.e. it forms a one-to-many aliasing relationship.  The limitations of that are:
- All aliased nodes must have the same value token count (could be zero in the case of a flag)
- All aliased nodes must be able to parse the same token formats

`alias_group` is almost the opposite of `policy::alias`, it is an _output_ alias.  Each of its child nodes are aliases of the same output value i.e. it forms a many-to-one aliasing relationship.  You can also think of `alias_group` as a variation on `one_of`, but instead of the output being a `std::variant` of all the child node output types, `alias_group` requires that they all have the _same_ output type.

## Constrained Positional Arguments
The `positional_arg` shown in the first example is unconstrained, i.e. it will consume all command line tokens that follow.  This isn't always desired, so we can use policies to constrain the amount of tokens consumed.  Let's use a file copier as an example:
```
$ simple-copy -f dest-dir source-path-1 source-path-2
```
Our simple copier takes multiple source paths and copies the files to a single destination directory, a 'force' flag will overwrite existing files silently.
```cpp
ar::root(
    arp::validation::default_validator,
    ar::help("help"_S, "h"_S, "Display this help and exit"_S),
    ar::mode(
        ar::flag("force"_S, "f"_S, "Force overwrite existing files"_S),
        ar::positional_arg<std::filesystem::path>("DST"_S, "Destination directory"_S,
            arp::required,
            arp::fixed_count<1>),
        ar::positional_arg<std::vector<std::filesystem::path>>("SRC"_S, "Source file paths"_S,
            arp::required,
            arp::min_count<1>),
        arp::router{[](bool force,
                       std::filesystem::path dest,
                       std::vector<std::filesystem::path> srcs) { ... }}))

```
It was noted in the [Basics](#basics) section that ordering does matter for positional arguments, so we can see here that the destination path is specified first and therefore comes first on the command line.  There can only be one destination path so we specify the `count` to one.  Because the `count` is 1 the return type doesn't need to be a container.

Following the destination path are the source paths, we need at least one so we mark it as required, as our range is unbounded the `value_type` needs to be a container.  `positional_arg` uses a `push_back` call on the container, so `std::vector` is the typical type to use.

Only the last `positional_arg` may be of variable length - unless a `token_end_maker` policy is used.  A runtime error will only occur if there are no unbounded variable length `postional_arg`s and there are more arguments than the maximum or less than the minimum.

It should be noted that setting a non-zero minimum count (`min_count`, `fixed_count`, or `min_max_count`) does _not_ imply a requirement, the minimum count check only applies when there is at least one argument for the node to process.  So as with an `arg`, you should use a `required` policy to explicitly state that at least one argument needs to be present, or a `default_value` policy - otherwise a default initialised value will be used instead.  For `positional_arg` nodes that are marked as `required`, it is a compile-time error to have a minimum count policy value of 0.

### Token End Marker Policy
The `token_end_maker` policy allows for multiple adjacent variable length `positional_arg` nodes to be defined.  It does this by defining a token that marks the end of the value token list for that node.  A trivial example could be a simple launcher application (which is available as a [buildable example](https://cmannett85.github.io/arg_router/c_09_0920_2launcher_2main_8cpp-example.html)) e.g.:
```
$ example_launcher_cpp prog1 prog2 prog3 -- arg1 arg2 arg3
```
Where the argument tokens are used invoke the programs as child processes.  Here the `--` token is used to separate the two adjacent `positional_args`:
```
ar::positional_arg<std::vector<std::string_view>>(arp::required,
                                                  "PROGS"_S,
                                                  "Programs to run"_S,
                                                  arp::token_end_marker_t{"--"_S},
                                                  arp::min_count<1>),
ar::positional_arg<std::vector<std::string_view>>("ARGS"_S,
                                                  "Arguments to pass to programs"_S),
```
There is no limit to number of `positional_arg` nodes that can be chained together like this, but they must still follow `positional_arg` rules such as being at the end of the child node list.

### Multi-Arg and Forwarding Arg
Another variation of multi-value arguments are `multi_arg` and `forwarding_arg`. `multi_arg` is almost the same as `arg` but accepts multiple (at least one) value tokens from the command line, you can tune the min/max value token count using the `min_max_count_t` policy - although you can't use a `value_separator` policy.

`forwarding_arg` is similar to `multi_arg` except that it is tuned for simply forwarding arguments, so has no naming restrictions, no min/max count, and is has a fixed `value_type` of `vector<std::string>`.  You could re-write the launcher example above to use it:
```
ar::forwarding_arg(arp::required,
                   "--"_S,
                   "Programs to run"_S,
                   arp::token_end_marker_t{"--"_S},
                   arp::min_count<1>),
ar::positional_arg<std::vector<std::string_view>>("ARGS"_S,
                                                  "Arguments to pass to programs"_S),
```
Which would look like this on the command line:
```
$ example_launcher_cpp -f -- prog1 prog2 prog3 -- arg1 arg2 arg3
```

## Modes
As noted in [Basics](#basics), `mode`s allow you to group command line components under an initial token on the command line.  A common example of this developers will be aware of is `git`, for example in our parlance `git clean -ffxd`; `clean` would be the mode and `ffxd` would be the flags that are available under that mode.

As an example, let's take the `simple-copy` above and split it into two modes:
```
$ simple copy -f dest-dir source-path-1 source-path-2
$ simple move -f dest-dir source-path-1
```
```cpp
ar::root(
    arp::validation::default_validator,
    ar::help("help"_S, "h"_S, "Display this help and exit"_S),
    ar::mode(
        arp::none_name_t{"copy"_S},
        arp::description_t{"Copy source files to destination"_S},
        ar::flag("force"_S, "f"_S, "Force overwrite existing files"_S),
        ar::positional_arg<std::filesystem::path>("DST"_S, "Destination directory"_S,
            arp::required,
            arp::fixed_count<1>),
        ar::positional_arg<std::vector<std::filesystem::path>>("SRC"_S, "Source file paths"_S,
            arp::required,
            arp::min_count<1>),
        arp::router{[](bool force,
                       std::filesystem::path dest,
                       std::vector<std::filesystem::path> srcs) { ... }}),
    ar::mode("move"_S, "Move source file to destination"_S,
        ar::flag("force"_S, "f"_S, "Force overwrite existing files"_S),
        ar::positional_arg<std::filesystem::path>("DST"_S, "Destination directory"_S,
            arp::required,
            arp::fixed_count<1>),
        // Can only have one
        ar::positional_arg<std::filesystem::path>("SRC"_S, "Source file path"_S,
            arp::required,
            arp::fixed_count<1>),
        arp::router{[](bool force,
                       std::filesystem::path dest,
                       std::filesystem::path src) { ... }}))
```
The name of a `mode` can only be the none version, as it doesn't use a prefix.  We now have two named modes: `copy` and `move`.

Named `mode`s can be nested too!  Only one mode can be invoked, so attempting to use flags from parent modes is a runtime failure.  Another stipulation is that every `mode` needs a `router` unless _all_ of its children are `mode`s as well.

### Flags Common Between Nodes
An obvious ugliness to the above example is that we now have duplicated code.  We can split that out, and then use copies in the root declaration.
```cpp
constexpr auto common_args = ar::list{
    ar::flag("force"_S, "f"_S, "Force overwrite existing files"_S),
    ar::positional_arg<std::filesystem::path>("DST"_S, "Destination directory"_S,
        arp::required,
        arp::fixed_count<1>)};

ar::root(
    arp::validation::default_validator,
    ar::mode("copy"_S, "Copy source files to destination"_S,
        common_args,
        ar::positional_arg<std::vector<std::filesystem::path>>("SRC"_S, "Source file paths"_S,
            arp::required,
            arp::min_count<1>),
        arp::router{[](bool force,
                       std::filesystem::path dest,
                       std::vector<std::filesystem::path> srcs) { ... }}),
    ar::mode("move"_S, "Move source file to destination"_S,
        common_args,
        ar::positional_arg<std::filesystem::path>("SRC"_S, "Source file path"_S,
            arp::required,
            arp::fixed_count<1>),
        arp::router{[](bool force,
                       std::filesystem::path dest,
                       std::filesystem::path src) { ... }}))
```
`ar::list` is a simple `arg` and `flag` container that `mode` and `root` instances detect and add the contents to their child/policy lists.  Also don't be afraid of the copies, the majority of `arg_router` types hold no data (the advantage of compile-time!) and those that do (e.g. `default_value`) generally have small types like primitives or `std::string_view`.

## Help Output
As shown in prior sections, a `help` node can be a child of the `root` (and only the `root`!), which acts like an `arg`, and generates the help output when requested by the user.  This node is optional, without it there is no help command line argument.  As the node is `arg`-like, it requires a long and/or short name.

The output can be embellished with the following policies:
- `program_name_t`, the name of the program as it should be displayed to the user
- `program_version_t`, version string.  This is not shown if a `program_name_t` is not specified
- `program_intro_t`, used to give some more information on the program, before the argument ouput
- `program_addendum_t`, used to add supplementary text after the argument output
- `flatten_help_t`, by default only top-level arguments and/or those in an anonymous mode are displayed.  Child modes are shown by requesting the mode's 'path' on the command line (e.g. `app --help mode sub-mode`).  The presence of this policy will make the entire requested subtree's (or root's, if no mode path was requested) help output be displayed

Unlike string data everywhere else in the library, the formatted help output is created at runtime so we don't need to keep duplicate read-only text data.

For example the slightly embellished tree from above:
```cpp
const auto common_args = ar::list{
    ar::flag("force"_S, "f"_S, "Force overwrite existing files"_S),
    ar::positional_arg<std::filesystem::path>("DST"_S, "Destination directory"_S,
        arp::required,
        arp::fixed_count<1>)};

ar::root(
    arp::validation::default_validator,
    ar::help("help"_S, "h"_S, "Display this help and exit"_S,
        arp::program_name_t{"simple"_S},
        arp::program_version_t{"v0.1"_S},
        arp::program_intro_t{"A simple file copier and mover."_S},
        arp::program_addendum_t{"An example program for arg_router."_S},
        arp::flatten_help),
    ar::mode("copy"_S, "Copy source files to destination"_S,
        common_args,
        ar::positional_arg<std::vector<std::filesystem::path>>("SRC"_S, "Source file paths"_S,
            arp::required,
            arp::min_count<1>),
        arp::router{[](bool force,
                       std::filesystem::path dest,
                       std::vector<std::filesystem::path> srcs) { ... }}),
    ar::mode("move"_S, "Move source file to destination"_S,
        common_args,
        ar::positional_arg<std::filesystem::path>("SRC"_S, "Source file path"_S,
            arp::required,
            arp::fixed_count<1>),
        arp::router{[](bool force,
                       std::filesystem::path dest,
                       std::filesystem::path src) { ... }}))
```
Would output this to the terminal:
```
$ simple --help
simple v0.1

A simple file copier and mover.

    --help,-h         Display this help and exit
    copy              Copy source files to destination
        --force,-f    Force overwrite existing files
        <DST> [1]     Destination directory
        <SRC> [1,N]   Source file paths
    move              Move source file to destination
        --force,-f    Force overwrite existing files
        <DST> [1]     Destination directory
        <SRC> [1]     Source file path

An example program for arg_router.
```
As you can see positional arguments are wrapped in angle brackets, and counts are displayed using interval notation.

Removing the `flatten_help` policy would change it to this:
```
$ simple --help
simple v0.1

A simple file copier and mover.

    --help,-h    Display this help and exit
    copy         Copy source files to destination
    move         Move source file to destination

An example program for arg_router.
```
In either case specifying the mode as an argument to the help argument displays just the sub-arguments of that mode:
```
$ simple --help copy
simple v0.1

A simple file copier and mover.

copy              Copy source files to destination
    --force,-f    Force overwrite existing files
    <DST> [1]     Destination directory
    <SRC> [1,N]   Source file paths

An example program for arg_router.
```
### Programmatic Access
By default when parsed, `help` will output its contents to `std::cout` and then exit the application with `EXIT_SUCCESS`.  Obviously this won't always be desired, so a `router` policy can be attached that will pass a `std::ostringstream` to the user-provided `Callable`.  The stream will have already been populated with the help data shown above, but it can now be appended to or converted to a string for use somewhere else.

Often programmatic access is desired for the help output outside of the user requesting it, for example if a parse exception is thrown, generally the exception error is printed to the terminal followed by the help output.  This is exposed by the `help()` or `help(std::ostringstream&)` methods of the root object.

### Customisation
Help output can be customised in several ways:
1. Use a `router` policy to capture the output and modify it.  This is useful for appending string data, but anything more sophisticated becomes a chore
2. Write your own `help` node.  This is the nuclear option as it gives maximal control but is a lot of work, it is very rarely necessary to do this as the node is primarily just the `tree_node` implementation, the actual formatting is delegated
3. Write your own help formatter policy for the built-in `help` node.  The `help` node delegates the formatting to a policy, if no formatter is specified when defining a `help` node specialisation the `default_help_formatter` is used.  This is still non-trivial as the formatter policy needs to perform the compile-time tree iteration in order to process the per-node help data
4. Write your own line formatter and/or preamble formatter. The `default_help_formatter` further delegates formatting to three sub-policies, one that generates the 'preamble' text (i.e. the program name, version, intro), another that generates each argument in the argument output, and a final one to generate the addendum.  `default_helper_formatter` uses `help_formatter_component::default_preamble_formatter`, `help_formatter_component::default_line_formatter`, and `help_formatter_component::default_addendum_formatter` respectively by default

## Unicode Compliance
A faintly ridiculous example of Unicode support from the `just_cats` example:
```
$ ./example_just_cats -h
just-cats

Prints cats!

    --help,-h    Display this help and exit
    --cat        English cat
    -猫          日本語の猫
    -🐱          Emoji cat
    --แมว        แมวไทย
    --кіт        український кіт
$ ./example_just_cats --cat
cat
$ ./example_just_cats -猫
猫
$ ./example_just_cats -🐱
🐱
$ ./example_just_cats --แมว
แมว
$ ./example_just_cats --кіт
кіт
```

`arg_router` only supports UTF-8 encoding, and will stay that way for the medium term.  If you want other encodings (e.g. UTF-16 on Windows), then you'll need to convert the input tokens to UTF-8 before calling `parse(..)`.  The compile-time strings used in the parse tree _must_ be UTF-8 because that's the only encoding supported by `arg_router`'s string checkers and line-break algorithm.  Likewise the help output generated will be UTF-8, so you'll need to capture the output (by attaching a `policy::router`) and then convert before printing

### Why have you written your own Unicode algorithms!?
We didn't want to...  Normally an application will link to ICU for its Unicode needs, but unfortunately we can't do that here as ICU is not `constexpr` and therefore cannot be used for our compile-time needs - so we need to roll our own.

## Runtime Language Selection
`arg_router` has support for runtime language selection, using `multi_lang::root`.  This wraps around a `Callable` that returns a `root` instance 'tweaked' for a given language.  Let's take the start of the simple file copier/mover application and convert it to use `multi_lang::root` - start by defining translations of the tree's strings:
```cpp
namespace arg_router::multi_lang
{
template <>
class translation<str<"en_GB">>
{
public:
    using force = str<"force">;
    using force_description = str<"Force overwrite existing files">;
    using destination = str<"DST">;
    using destination_description = str<"Destination directory">;
    using help = str<"help">;
    using help_description = str<"Display this help and exit">;
    using program_intro = str<"A simple file copier and mover.">;
    using program_addendum = str<"An example program for arg_router.">;
    using copy = str<"copy">;
    using copy_description = str<"Copy source files to destination">;
    using source = str<"SRC">;
    using sources_description = str<"Source file paths">;
    using move = str<"move">;
    using move_description = str<"Move source file to destination">;
    using source_description = str<"Source file path">;
};

template <>
class translation<str<"fr">>
{
public:
    using force = str<"forcer">;
    using force_description = str<"Forcer l'écrasement des fichiers existants">;
    using destination = str<"DST">;
    using destination_description = str<"Répertoire de destination">;
    using help = str<"aider">;
    using help_description = str<"Afficher cette aide et quitter">;
    using program_intro = str<"Un simple copieur et déménageur de fichiers.">;
    using program_addendum = str<"Un exemple de programme pour arg_router.">;
    using copy = str<"copier">;
    using copy_description = str<"Copier les fichiers source vers la destination">;
    using source = str<"SRC">;
    using sources_description = str<"Chemins des fichiers sources">;
    using move = str<"déplacer">;
    using move_description = str<"Déplacer le fichier source vers la destination">;
    using source_description = str<"Chemin du fichier source">;
};

template <>
class translation<str<"ja">>
{
public:
    using force = str<"強制">;
    using force_description = str<"既存のファイルを強制的に上書きする">;
    using destination = str<"先">;
    using destination_description = str<"宛先ディレクトリ">;
    using help = str<"ヘルプ">;
    using help_description = str<"このヘルプを表示して終了">;
    using program_intro = str<"ファイルをコピーおよび移動するためのシンプルなプログラム。">;
    using program_addendum = str<"「arg_router」のサンプルプログラム。">;
    using copy = str<"コピー">;
    using copy_description = str<"ソース ファイルを宛先にコピーする">;
    using source = str<"出典">;
    using sources_description = str<"ソース ファイルのパス">;
    using move = str<"移動">;
    using move_description = str<"ソース ファイルを宛先に移動する">;
    using source_description = str<"ソース ファイル パス">;
};
}  // namespace arg_router::multi_lang
```
There is nothing special about the `translation` type, the unspecialised version will simply static assert if used to remind you to implement specialisations for all language IDs (the template parameter type) - otherwise it is just an empty type.  Its use is still recommended though as functionality may be added to it in the future.
```cpp
int main(int argc, char* argv[])
{
    // Apologies for any translation faux pas - Google Translate did it for me!
    ar::multi_lang::root<ar::str<"en_GB">, ar::str<"fr">, ar::str<"ja">>(  //
        ar::multi_lang::iso_locale(locale_name()),
        [&](auto tr_) {
            // This isn't necessary with C++20 lambda template params
            using tr = decltype(tr_);

            const auto common_args = ar::list{
                ar::flag(typename tr::force{}, "f"_S, typename tr::force_description{}),
                ar::positional_arg<fs::path>(typename tr::destination{},
                                             typename tr::destination_description{},
                                             arp::required,
                                             arp::fixed_count<1>)};

            return ar::root(
                arp::validation::default_validator,
                ar::help(typename tr::help{}, "h"_S, typename tr::help_description{},
                         arp::program_name_t{"simple"_S},
                         arp::program_version_t{"v0.1"_S},
                         arp::program_intro<typename tr::program_intro>,
                         arp::program_addendum<typename tr::program_addendum>,
                         arp::flatten_help,
                         arp::colour_help_formatter),
                ...
        );
    }).parse(argc, argv);
```
`multi_lang::root` takes a set of supported language identifiers (ISO language/country codes are recommended for readability/ease, but not required), these are used as the language IDs for `translation` specialisation.  The runtime-selected `translation` instance is then passed to the `Callable` provided to `multi_lang::root`, the type of which is then used to access the translated compile-time string.  Any missing string translation will cause a compilation error.

The first argument to `multi_lang::root` is a string provided by the user that should match one of the language identifiers, if it doesn't `arg_router` will fall back to using the first defined language (UK English in this case).  As a convenience `arg_router` provides a simple function that takes the OS locale name and standardises it to an ISO language/country identifier.

`multi_lang::root`'s interface is designed to mimic `ar::root`, so it can be used as a drop in replacement.

The above code (which is available as a [buildable example](https://cmannett85.github.io/arg_router/c_09_0920_2simple_ml_2main_8cpp-example.html)) will yield the following help output:
```
$ ./example_simple_ml_cpp20 -h
simple v0.1

A simple file copier and mover.

    --help,-h          Display this help and exit
    copy               Copy source files to destination
        --force,-f     Force overwrite existing files
        <DST> [1]      Destination directory
        <SRC> [1,N]    Source file paths
    move               Move source file to destination
        --force,-f     Force overwrite existing files
        <DST> [1]      Destination directory
        <SRC> [1]      Source file path

An example program for arg_router.

$ AR_LOCALE_OVERRIDE=fr ./example_simple_ml_cpp20 -h
simple v0.1

Un simple copieur et déménageur de fichiers.

    --aider,-h         Afficher cette aide et quitter
    copier             Copier les fichiers source vers la destination
        --forcer,-f    Forcer l'écrasement des fichiers existants
        <DST> [1]      Répertoire de destination
        <SRC> [1,N]    Chemins des fichiers sources
    déplacer           Déplacer le fichier source vers la destination
        --forcer,-f    Forcer l'écrasement des fichiers existants
        <DST> [1]      Répertoire de destination
        <SRC> [1]      Chemin du fichier source

Un exemple de programme pour arg_router.

$ AR_LOCALE_OVERRIDE=ja ./example_simple_ml_cpp20 -h
simple v0.1

ファイルをコピーおよび移動するためのシンプルなプログラム。

    --ヘルプ,-h         このヘルプを表示して終了
    コピー              ソース ファイルを宛先にコピーする
        --強制,-f       既存のファイルを強制的に上書きする
        <先> [1]        宛先ディレクトリ
        <出典> [1,N]    ソース ファイルのパス
    移動                ソース ファイルを宛先に移動する
        --強制,-f       既存のファイルを強制的に上書きする
        <先> [1]        宛先ディレクトリ
        <出典> [1]      ソース ファイル パス

「arg_router」のサンプルプログラム。

$ AR_LOCALE_OVERRIDE=foo ./example_simple_ml_cpp20 -h
simple v0.1

A simple file copier and mover.

    --help,-h          Display this help and exit
    copy               Copy source files to destination
        --force,-f     Force overwrite existing files
        <DST> [1]      Destination directory
        <SRC> [1,N]    Source file paths
    move               Move source file to destination
        --force,-f     Force overwrite existing files
        <DST> [1]      Destination directory
        <SRC> [1]      Source file path

An example program for arg_router.
```
Optionally, you can also provide translations for the exception messages by defining an `error_code_translations` subtype that consists of a tuple of pairs that form a mapping between the error code and translation string.  If this isn't provided then an internal `en_GB` one is automatically used instead.
```cpp
template <>
class translation<str<"ja">>
{
public:
    ...

    using error_code_translations = std::tuple<
        std::pair<traits::integral_constant<error_code::unknown_argument>, str<"不明な引数">>,
        std::pair<traits::integral_constant<error_code::unhandled_arguments>, str<"未処理の引数">>,
        std::pair<traits::integral_constant<error_code::argument_has_already_been_set>,
                  str<"引数はすでに設定されています">>,
        std::pair<traits::integral_constant<error_code::failed_to_parse>,
                  str<"解析に失敗しました">>,
        std::pair<traits::integral_constant<error_code::no_arguments_passed>,
                  str<"引数が渡されませんでした">>,
        std::pair<traits::integral_constant<error_code::minimum_value_not_reached>,
                  str<"最小値に達していません">>,
        std::pair<traits::integral_constant<error_code::maximum_value_exceeded>,
                  str<"最大値を超えました">>,
        std::pair<traits::integral_constant<error_code::minimum_count_not_reached>,
                  str<"最小数に達していません">>,
        std::pair<traits::integral_constant<error_code::mode_requires_arguments>,
                  str<"モードには引数が必要です">>,
        std::pair<traits::integral_constant<error_code::missing_required_argument>,
                  str<"必要な引数がありません">>,
        std::pair<traits::integral_constant<error_code::too_few_values_for_alias>,
                  str<"エイリアス値が少なすぎる">>,
        std::pair<
            traits::integral_constant<error_code::dependent_argument_missing>,
            str<"従属引数がありません (コマンドラインで必要なトークンの前に置く必要があります)">>>;
};
```
Could yield:
```
$ AR_LOCALE_OVERRIDE=ja ./example_simple_ml_cpp20 -🐱
terminate called after throwing an instance of 'arg_router::parse_exception'
  what():  不明な引数: -🐱
```

### Note ###
`multi_lang::root_wrapper` from v1.0 is still present and supported, but is now marked as deprecated - new code should use `multi_lang::root`.  It is not supported when using C++20 compile-time strings (see [compile-time string support](#compile-time-string-support)).

## Compile-time String Support
In v1.0 the library exclusively used the `S_()` macro for compile-time string generation, this was an almost necessary convenience as that version of C++ had relatively poor NTTP support.  In v1.1 we added an alternative for those using C++20 and above, which is the `str` type and its `""_S` string literal.

For those targetting C++20 with existing v1.0 code, upgrading to a newer library version will cause a compilation failure as the two methods are not compatible.  But don't worry!  Code changes aren't ncessary, but you will need to add the define `AR_DISABLE_CPP20_STRINGS=true` to your build.  Those still targetting C++17 will not need to do anything.

**Note** C++17 will be supported as a first class citizen until v2.0, after that C++20 will be the minimum so I can strip out a ton of code and get better diagnostics by using concepts.

## Error Handling
Currently `arg_router` only supports exceptions as error handling.  If parsing fails for some reason a `arg_router::parse_exception` is thrown carrying information on the failure.

## Configuration
Low-level tweaking of the library is achieved via some defines and/or CMake variables, documented [here](https://cmannett85.github.io/arg_router/configuration.html).

## Supported Compilers/Platforms
The CI system attached to this repo builds the unit tests and examples with:
* Ubuntu 22.04 (Ninja), Clang 14, gcc-12, gcc-9, gcc-9 32bit
* Windows Server 2022 (Ninja, MSBuild), Clang 14.0.5, MSVC 19.34(C++20 only)
* MacOS 12 (Ninja), Clang 14

You can build the unit tests on Windows using MSBuild but you must set the CMake variable `DEATH_TEST_PARALLEL` to 1 otherwise the parallel tests will attempt to write to the project-wide `lastSuccessfulBuild` file simultaneously, which causes the build to fail. MSVC is supported but only when using the C++20-style compile-time strings due [this](https://developercommunity.visualstudio.com/t/1395099) MSVC bug.

Other compiler versions and platform combinations may work, but I'm currently limited by the built-in GitHub runners and how much I'm willing to spend on Actions!

## Tips for Users
### Do **NOT** Make the Parse Tree Type Accessible
The parse tree is _very_ expensive to construct due to all the compile-time checking and meta-programming shennanigans, so do **NOT** define it in a header and have multiple source files include it - it will cause the tree to be built/checked in every source file it is included in.

This is especially important on Windows as `windows.h` is included (needed for calculating terminal column width) with `NOMINMAX` and `WIN32_LEAN_AND_MEAN` set by default.

### Minimise Static Storage Bloat
Despite not using `typeid` or `dynamic_cast` in the library, compilers will still generate class name data if RTTI is enabled, because it is used in the standard library implementations (e.g. `std::function` on Clang).  Due to the highly nested templates that make up the parse tree, these class names can become huge and occupy large amount of static storage in the executable.  As an example, the `basic_cat` project in the repo will create ~100KB of class name data in the binary - this data is not used and cannot be stripped out.

Disabling RTTI is rarely feasible for most projects, but it is possible to disable RTTI for a single CMake target.  So if it was deemed worth it for the size reduction, the command line parsing could be the application's executable (compiled without RTTI) and then the wider application logic could be in a static library (compiled with RTTI).  This does not affect exceptions, as their type information is always added by the compiler regardless of RTTI status.

### Newer Compilers are Better with Templates
This may seem like an obvious point, but people need reminding: The rise of TMP use has been quicker than the compiler optimisations for it.  In practice this means that although `arg_router` can be compiled on e.g. GCC v9, it will use staggeringly more memory than e.g. GCC v11.

## Extra Documentation
Complete Doxygen-generated API documentation is available [here](https://cmannett85.github.io/arg_router/).  Examples are provided in the `examples` directory of the repo or online [here](https://cmannett85.github.io/arg_router/examples.html).  Doxygen theming is provided by [Doxygen Awesome CSS](https://github.com/jothepro/doxygen-awesome-css).

The latest unit test coverage report is found [here](https://cmannett85.github.io/arg_router/gcov_html/).

## Future Work
Take a look at the [issues](https://github.com/cmannett85/arg_router/issues) page for all upcoming features and fixes.
