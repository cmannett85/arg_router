![Documentation Generator](https://github.com/cmannett85/arg_router/workflows/Documentation%20Generator/badge.svg) ![Unit test coverage](https://img.shields.io/badge/Unit_Test_Coverage-98.8%25-brightgreen)

# arg_router
`arg_router` is a C++17 command line parser and router.  It uses policy-based objects hierarchically, so the parsing code is self-describing.  Rather than just providing a parsing service that returns a map of `variant`s/`any`s, it allows you to bind `Callable` instances to points in the parse structure, so complex command line arguments can directly call functions with the expected arguments - rather than you having to do this yourself.

## Features
- Use policies to define the properties and constraints of arguments at compile-time
- Group arguments together to define mutually exclusive operating modes, for complex applications
- Define logical connections between arguments
- Detects invalid or ambiguous parse trees at compile-time
- Generates its help output, which you can modify at runtime using a `Callable`
- Easy custom parsers by  using `Callable`s inline for specific arguments, or you can implement a specialisation to cover all instances of that type

## Basics
Let's start simple, with this `cat`-like program:
```cpp
namespace ar = arg_router;
namespace arp = ar::policy
ar::root(
    arp::validation::default_validator,
    ar::help(
        arp::long_name<S_("help")>,
        arp::short_name<'h'>,
        arp::description<S_("Display this help and exit")>,
        arp::router{[](std::string_view arg_docs) { ... }}),
    ar::flag(
        arp::long_name<S_("version")>,
        arp::description<S_("Output version information and exit")>,
        arp::router{[]() { ... }}),
    ar::mode(
        ar::flag(
            arp::long_name<S_("show-all")>,
            arp::description<S_("Equivalent to -nE")>,
            arp::short_name<'A'>,
            arp::alias(ar::short_name<'E'>, ar::short_name<'n'>)),
        ar::flag(
            arp::long_name<S_("show-ends")>,
            arp::description<S_("Display $ at end of each line")>,
            arp::short_name<'E'>),
        ar::flag(
            arp::long_name<S_("show-nonprinting")>,
            arp::description<S_("Use ^ and M- notation, except for LFD and TAB")>,
            arp::short_name<'n'>),
        ar::arg<int>(
            arp::long_name<S_("max-lines")>,
            arp::description<S_("Maximum lines to output")>,
            arp::default_value{-1}),
        ar::positional_arg<std::vector<std::string_view>>(
            arp::min_count<1>,
            arp::long_name<S_("FILES")>,
            arp::description<S_("Files to read")>),
        arp::router{[](bool show_ends,
                       bool show_non_printing,
                       int max_lines,
                       std::vector<std::string_view>> files) { ... }})).
    parse(argc, argv);
```
Let's start from the top, as the name suggests `root` is the root of the parse tree and provides the `parse(argc, argv)` method.  Only children of the root can (and must) have a `router` policy and therefore act as access points into the program. The root's children are implicitly mutually exclusive, so trying to pass `--version --help` in the command line is a runtime error.

The `arp::validation::default_validator` instance provides the default validator that the root uses to validate the parse tree at compile-time.  It is a required policy of the `root`.  Unless you have implemented your own policy or tree node you will never need to specify anything else.

The `help` node is used by the `root` to generate the argument documentation for the help output, rather than print directly to the console, a `router` is attached that accepts a string so the user can add other documentation to it (e.g. version info, examples, etc.).

Now let's introduce some 'policies'.  Policies define common behaviours across node types, a basic one is `long_name` which provides long form argument definition.  The standard unix double hyphen prefix for long names is added automatically when not used in a `mode`.  Having the name defined at compile-time means we detect duplicate names and fail the build - one less unit test you have to worry about.  `short_name` is the single character short form name, a single hyphen is prefixed automatically.  `arg_router` supports short name collapsing for flags, so if you have defined flags like `-a -b -c` then `-abc` will be accepted or `-bca`, etc.

In order to group arguments under a specific operating mode, you put them under a `mode` instance.  In this case our simple cat program only has one mode, so it is anonymous i.e. there's not long name or description associated with it - it is a build error to have more than one anonymous mode in the parse tree.

`arg<T>` does exactly what you would expect, it defines an argument that expects a value to follow it on the command line.  If an argument is not `required` then it must have a `default_value`, this is passed to the `router`'s `Callable` on parsing if it isn't specified by the user on the command line.

A `flag` is essentially an `arg<bool>{default_value{false}}`, except that it doesn't expect an argument value to follow on the command line as it _is_ the value.  Flags cannot have default arguments or be marked as required.

An `alias` policy allows you to define an argument that acts as a link to other arguments, so in our example above passing `-A` on the command line would actually set the `-E` and `-n` flags to true.  You can use either the long or short name of the aliased flag, but the `value_type`s (`bool` for a flag) must be the same.

`positional_arg<T>` does not use a 'marker' token on the command line for which its value follows, the values position in the command line arguments determines what it is for.  The order that arguments are specified on the command line normally don't matter, but for positional arguments they do; for example in our cat program the files must be specified after the arguments so passing `myfile.hpp -n` would trigger the parser to land on the `positional_arg` for `myfile.hpp` which would then greedily consume the `-n` causing the application to try to open the file `-n`...  We'll cover constrained `positional_arg`s in later examples.

Assuming parsing was successful, the final `router` is called with the parsed argument e.g. if the user passed `-E file1 file2` then the `router` is passed `(true, false, -1, {"file1", "file2"})`.

You may have noticed that the nodes are constructed with parentheses whilst the policies use braces, this is necessary due to CTAD rules that affect nodes which return a user-defined value type.  This can be circumvented using a function to return the required instance, for example the actual type of a flag is `flag_t`, `flag(...)` is a function that creates one for you.

## Conditional Arguments
Let's add another feature to our cat program where we can handle lines over a certain length differently.
```cpp
using namespace ard = ar::dependency;
ar::mode(
    ...
    ar::arg<std::optional<std::size_t>>(
        arp::long_name<S_("max-line-length")>,
        arp::description<S_("Maximum line length")>,
        arp::default_value{std::optional<std::size_t>{}}),
    ard::one_of(
        ard::dependent<arp::long_name<S_("max-line-length")>>,
        ar::flag(
            arp::long_name<S_("skip-line")>,
            arp::short_name<'s'>,
            arp::description<S_("Skips line output if max line length "
                                "reached")>),
        ar::arg<std::string_view>(
            arp::long_name<S_("line-suffix")>,
            arp::description<S_("Shortens line length to maximum with the "
                                "given suffix if max line length reached")>,
            arp::default_value{"..."})),
    ...
    arp::router{[](bool show_all,
                   bool show_ends,
                   bool show_non_printing,
                   int max_lines,
                   std::optional<std::size_t> max_line_length,
                   std::variant<bool, std::string_view> max_line_handling,
                   std::vector<std::string_view>> files) { ... }})
```
We've defined a new argument `--max-line-length` but rather than using `-1` as the "no limit" indicator like we did for `--max-lines`, we specify the argument type to be `std::optional<std::size_t>` and have the default value by an empty optional - this allows the code to define our intent better.

What do we do with lines that reach the limit if it has been set?  In our example we can either skip the line output, or truncate it with a suffix.  It doesn't make any sense to allow both of these options, so we declare them under a `one_of` node.  Under this node, only one is valid when parsing at runtime, if the user specifies both then it is an error.  For obvious reasons it is an error if any `arg`s under a `one_of` are marked as required, which means that they all have to have default values assigned.  This leads to an ambiguity when the user _does not_ specify any argument - which value is picked?  It is the first `arg` specified, or if there no `arg`s (i.e. only `flag`s) then it is a runtime error.

To express the 'one of' concept better in code, the `one_of` node has a single representation in the `router`'s arguments - a variant that encompasses all the value types of each entry in it.  In our example's case, a bool for the `--skip-line` flag and a `string_view` for the `--line-suffix` case.

What happens if a user passes `--skip-line` without `--max-line-length`?  Normally the developer will have to check that `max-line-length` is not empty and either ignore or throw if it is.  But by specifying the `one_of` as `dependent` on `max-line-length`, `arg_router` will throw on your behalf in this scenario.  To be clear, the `dependent` policy just means that the owner cannot appear on the command line without the `arg` or `flag` named in its parameters also appearing on the command line.

The smart developer may have noticed an edge case here.  If both the children in the above example had the same `value_type`, then the variant will be created with multiples of the same type.  `std::variant` supports this but it means that the common type-based visitation pattern will become ambiguous on those identical types i.e. you won't know which flag was used.  This may or may not be important to you, but if it is, you will have to `switch` on `std::variant::index()` instead.

## Custom Parsing
Custom parsing for `arg` types can be done in one of two ways, the first is using a `custom_parser` policy.  Let's add theme coloring to our console output.
```cpp
enum class theme_t {
    NONE,
    CLASSIC,
    SOLARIZED
};

ar::mode(
    ...
    ar::arg<theme_t>(
        arp::long_name<S_("theme")>,
        arp::short_name<'t'>,
        arp::description<S_("Set the output colour theme")>,
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
        std::vector<std::string_view>> files) { ... }})
```
This is a convenient solution to one-off type parsing in-tree, I think it's pretty self-explanatory.  However this is not convenient if you have lots of arguments that should return the same custom type, as you would have to copy and paste the lambda or bind calls to the conversion function for each one.  In that case we can specialise on the `arg_router::parse` function.
```cpp
template <>
auto arg_router::parse<theme_t>(std::string_view arg)
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
    ar::counting_flag<verbosity_level_t>(
        arp::short_name<'v'>,
        arp::max_count<3>,
        arp::description<S_("Verbosity level, number of 'v's sets level")>,
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
        std::vector<std::string_view>> files) { ... }})
```
The number of times `-v` appears in the command line will increment the returned value, e.g. `-vv` will result in `verbosity_level_t::INFO`.

Note that even though we are using a custom enum, we haven't specified a `custom_parser`.  `counting_flag` will count up in `std::size_t` and then `static_cast` to the user-specified type, so as long as your requested type is explicitly convertible from `std::size_t` it will just work.  If it isn't, then you'll have to modify your type or not use counting arguments, as attaching a `custom_parser` to any kind of flag will result in a build failure.

Short name collapsing still works as expected, so passing `-Evnv` will result in `show_ends` and `show_non_printing` being true, and `verbosity_level` will be `verbosity_level_t::INFO` in the `router` call.

We can constrain the amount of flags the user can provide by using the `max_count` policy, so passing `-vvvv` will result in a runtime error.

The `long_name` policy is not allowed, but we can use the knowledge gained so far to provide the best of both:
```cpp
ar::mode(
    ...
    ard::one_of(
        ar::counting_flag<verbosity_level_t>(
            arp::short_name<'v'>,
            arp::max_count<3>,
            arp::description<S_("Verbosity level, number of 'v's sets level")>,
            arp::alias(arp::long_name<S_("verbose")>)),
        ar::arg<verbosity_level_t>(
            arp::long_name<S_("verbose")>,
            arp::description<S_("Verbosity level")>,
            arp::default_value{verbosity_level_t::INFO},
            arp::custom_parser<verbosity_level_t>{
                [](std::string_view arg) {
                    return verbosity_level_from_string(arg); }}))),
    ...
    arp::router{[](bool show_all,
        bool show_ends,
        bool show_non_printing,
        int max_lines,
        std::optional<std::size_t> max_line_length,
        std::variant<bool, std::string_view> max_line_handling,
        theme_t theme,
        verbosity_level_t verbosity_level,
        std::vector<std::string_view>> files) { ... }}}}
```
We can declare a new `arg` that takes a string equivalent of the enum and then make the `-v` flag an alias of it, so now you can use `--verbose INFO` or `-vv`.  Of course using both at the same time is confusing, so we can put both under a `one_of`.  Now the clever part, because `-v` is an alias it doesn't provide its own output so the resulting `one_of` variant would be `std::variant<verbosity_level_t>`, `arg_router` detects this and just collapses it to `verbosity_level_t`.

## Constrained Positional Arguments
The `positional_arg` shown in the first example is unconstrained, i.e. it will consume all command line tokens that follow.  This isn't always desired, so we can use policies to constrain the amount of tokens consumed.  Let's use a file copier as an example:
```
$ simple-copy -f dest-dir source-path-1 source-path-2
```
Our simple copier takes multiple source paths and copies the files to a single destination directory, a 'force' flag will overwrite existing files silently.
```cpp
ar::root(
    arp::validation::default_validator,
    ar::help(
        arp::long_name<S_("help")>,
        arp::short_name<'h'>,
        arp::description<S_("Display this help and exit")>,
        arp::router{[](std::string_view arg_docs) { ... }),
    ar::mode(
        ar::flag(
            arp::long_name<S_("force")>,
            arp::short_name<'f'>,
            arp::description<S_("Force overwrite existing files")>),
        ar::positional_arg<std::filesystem::path>(
            arp::long_name<S_("DST")>,
            arp::description<S_("Destination directory")>,
            arp::count<1>),
        ar::positional_arg<std::vector<std::filesystem::path>>(
            arp::long_name<S_("SRC")>,
            arp::description<S_("Source file paths")>,
            arp::min_count<1>),
        arp::router{[](bool force,
                       std::filesystem::path dest,
                       std::vector<std::filesystem::path> srcs) { ... }}))

```
It was noted in the [Basics](#basics) section that ordering does matter for positional arguments, so we can see here that the destination path is specified first and therefore comes first on the command line.  There can only be one destination path so we specify the `count` to one, this implicitly marks the argument as required.  Because the `count` is 1 the return type doesn't need to be a container.

Following the destination path are the source paths, as there is only a `min_count` policy our range is unbounded and therefore the `value_type` needs to be a container.  `positional_arg` uses a `push_back` call on the container, so `std::vector` is the typical type to use.

Only the last `positional_arg` may be of variable length.  A runtime error will only occur if there are no variable length `postional_arg`s and there are more arguments than the maximum or less than the minimum.

## Modes
As noted in [Basics](#Basics), `mode`s allow you to group command line components under an initial token on the command line.  A common example of this developers will be aware of is `git`, for example in our parlance `git clean -ffxd`; `clean` would be the mode and `ffxd` would be be the flags that are available under that mode.

As an example, let's take the `simple-copy` above and split it into two modes:
```
$ simple copy -f dest-dir source-path-1 source-path-2
$ simple move -f dest-dir source-path-1
```
```cpp
ar::root(
    arp::validation::default_validator,
    ar::help(
        arp::long_name<S_("help")>,
        arp::short_name<'h'>,
        arp::description<S_("Display this help and exit")>,
        arp::router{[](std::string_view arg_docs) { ... }}),
    ar::mode(
        arp::long_name<S_("copy")>,
        arp::description<S_("Copy source files to destination")>,
        ar::flag(
            arp::long_name<S_("force")>,
            arp::short_name<'f'>,
            arp::description<S_("Force overwrite existing files")>),
        ar::positional_arg<std::filesystem::path>(
            arp::required,
            arp::long_name<S_("DST")>,
            arp::description<S_("Destination directory")>,
            arp::count<1>),
        ar::positional_arg<std::vector<std::filesystem::path>>(
            arp::required,
            arp::long_name<S_("SRC")>,
            arp::description<S_("Source file paths")>,
            arp::min_count<1>),
        arp::router{[](bool force,
                       std::filesystem::path dest,
                       std::vector<std::filesystem::path> srcs) { ... }}),
    ar::mode(
        arp::long_name<S_("move")>,
        arp::description<S_("Move source file to destination")>,
        ar::flag(
            arp::long_name<S_("force")>,
            arp::short_name<'f'>,
            arp::description<S_("Force overwrite existing files")>),
        ar::positional_arg<std::filesystem::path>(
            arp::required,
            arp::long_name<S_("DST")>,
            arp::description<S_("Destination directory")>,
            arp::count<1>),
        // Can only have one
        ar::positional_arg<std::filesystem::path>(
            arp::required,
            arp::long_name<S_("SRC")>,
            arp::description<S_("Source file path")>,
            arp::count<1>),
        arp::router{[](bool force,
                       std::filesystem::path dest,
                       std::filesystem::path src) { ... }}))
```
You can't have an anonymous `mode` if there are named ones, and the name can only be the long version.  So we now have two named modes: `copy` and `move`.  Notice that the double hyphen prefix is _not_ automatically added to the `mode`'s long name, this matches typical Unix patterns - it is a build error to add them yourself!

### Flags Common Between Nodes
An obvious ugliness to the above example is that we now have duplicated code.  We can split that out, and then use copies in the root declaration.
```cpp
constexpr auto common_args = ar::list{
    ar::flag(
        arp::long_name<S_("force")>,
        arp::short_name<'f'>,
        arp::description<S_("Force overwrite existing files")>),
    ar::positional_arg<std::filesystem::path>(
        arp::required,
        arp::long_name<S_("DST")>,
        arp::description<S_("Destination directory")>,
        arp::count<1>)};

ar::root(
    arp::validation::default_validator,
    ar::help(
        arp::long_name<S_("help")>,
        arp::short_name<'h'>,
        arp::description<S_("Display this help and exit")>,
        arp::router{[](std::string_view arg_docs) { ... }}),
    ar::mode(
        arp::long_name<S_("copy")>,
        arp::description<S_("Copy source files to destination")>,
        common_args,
        ar::positional_arg<std::vector<std::filesystem::path>>(
            arp::required,
            arp::long_name<S_("SRC")>,
            arp::description<S_("Source file paths")>,
            arp::min_count<1>),
        arp::router{[](bool force,
                       std::filesystem::path dest,
                       std::vector<std::filesystem::path> srcs) { ... }}),
    ar::mode(
        arp::long_name<S_("move")>,
        arp::description<S_("Move source file to destination")>,
        common_args,
        ar::positional_arg<std::filesystem::path>(
            arp::required,
            arp::long_name<S_("SRC")>,
            arp::description<S_("Source file path")>,
            arp::min_count<1>,
            arp::max_count<1>),
        arp::router{[](bool force,
                       std::filesystem::path dest,
                       std::filesystem::path src) { ... }}))
```
`ar::list` is a simple `arg` and `flag` container that `mode` and `root` instances detect and add the contents to their child/policy lists - it's nicer than having an instance per type.  Also don't be afraid of the copies, the majority of `arg_router` types hold no data (the advantage of compile-time!) and those that do (e.g. `default_value`) generally have small types like primitives or `std::string_view`.

## Error Handling
Currently `arg_router` only supports exceptions as error handling.  If a parsing fails for some reason a `arg_router::parse_exception` is thrown carrying information on the failure.
