![Documentation Generator](https://github.com/cmannett85/arg_router/workflows/Documentation%20Generator/badge.svg) ![Unit test coverage](https://img.shields.io/badge/Unit_Test_Coverage-97.7%25-brightgreen)

# arg_router
`arg_router` is a C++17 command line parser and router.  It uses policy-based objects hierarchically, so the parsing code is self-describing.  Rather than just providing a parsing service that returns a map of `variant`s/`any`s, it allows you to bind `Callable` instances to points in the parse structure, so complex command line arguments can directly call functions with the expected arguments - rather than you having to do this yourself.

## Features
- Use policies to define the properties and constraints of arguments at compile-time
- Group arguments together to define mutually exclusive operating modes, for more complex applications
- Define logical connections between arguments
- Detects invalid or ambiguous parse trees at compile-time
- Generates its help output, which you can modify at runtime using a `Callable`, or tweak its formatting using policies
- Easy custom parsers by using `Callable`s inline for specific arguments, or you can implement a specialisation to cover all instances of that type
- Unicode compliant by supporting UTF-8 encoded compile-time strings ([details](#unicode-compliance))
- Support of runtime language selection

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
        arp::program_name<S_("my-cat")>,
        arp::program_version<S_("v3.14")>,
        arp::description<S_("Display this help and exit")>),
    ar::flag(
        arp::long_name<S_("version")>,
        arp::description<S_("Output version information and exit")>,
        arp::router{[](bool v) { ... }}),
    ar::mode(
        ar::flag(
            arp::long_name<S_("show-all")>,
            arp::description<S_("Equivalent to -nE")>,
            arp::short_name<'A'>,
            arp::alias(arp::short_name<'E'>, arp::short_name<'n'>)),
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
            arp::value_separator<'='>,
            arp::default_value{-1}),
        ar::positional_arg<std::vector<std::string_view>>(
            arp::required,
            arp::min_count<1>,
            arp::display_name<S_("FILES")>,
            arp::description<S_("Files to read")>),
        arp::router{[](bool show_ends,
                       bool show_non_printing,
                       int max_lines,
                       std::vector<std::string_view> files) { ... }})).
    parse(argc, argv);
```
Let's start from the top, as the name suggests `root` is the root of the parse tree and provides the `parse(argc, argv)` method.  Only children of the root can (and must) have a `router` policy (except for nested `mode`s, more [on that later](#modes)) and therefore act as access points into the program. The root's children are implicitly mutually exclusive, so trying to pass `--version --help` in the command line is a runtime error.

The `arp::validation::default_validator` instance provides the default validator that the root uses to validate the parse tree at compile-time.  It is a required policy of the `root`.  Unless you have implemented your own policy or tree node you will never need to specify anything else.

The `help` node is used by the `root` to generate the argument documentation for the help output, by default it just prints directly to the console and then exits, but a `router` can be attached that accepts the formatted output and do something else with it.  The optional `program_name` and `program_version` policies add a header to the help output.

Now let's introduce some 'policies'.  Policies define common behaviours across node types, a basic one is `long_name` which provides long-form argument definition.  By default, a standard unix double hyphen prefix for long names is added automatically.  Having the name defined at compile-time means we detect duplicate names and fail the build - one less unit test you have to worry about.  `short_name` is the single character short-form name, by default a single hyphen is prefixed automatically.  `arg_router` supports short name collapsing for flags, so if you have defined flags like `-a -b -c` then `-abc` will be accepted or `-bca`, etc. (**note** short-form name collapsing is disabled if the library has been configured to have the same long and short prefix).

In order to group arguments under a specific operating mode, you put them under a `mode` instance.  In this case our simple cat program only has one mode, so it is anonymous i.e. there's no long name or description associated with it - it is a build error to have more than one anonymous mode under the root of a parse tree.

`arg<T>` does exactly what you would expect, it defines an argument that expects a value to follow it on the command line.  If an argument is not `required` then it may have a `default_value` (if neither are set, then a default initialised value is used instead), this is passed to the `router`'s `Callable` on parsing if it isn't specified by the user on the command line.

A `flag` is essentially an `arg<bool>{default_value{false}}`, except that it doesn't expect an argument value to follow on the command line as it _is_ the value.  Flags cannot have default arguments or be marked as required.

An `alias` policy allows you to define an argument that acts as a link to other arguments, so in our example above passing `-A` on the command line would actually set the `-E` and `-n` flags to true.  You can use either the long or short name of the aliased flag, but the `value_type`s (`bool` for a flag) must be the same.

By default whitespace is used to separate out the command line tokens, this is done by the terminal/OS invoking the program, but often '=' is used a name/value token separator.  `arg_router` supports this with the `value_separator` policy as used in the `arg<int>` node in the example.  Be aware that it is a compile-time error to specify a short name with `value_separator` as it becomes ambiguous with multiple flag collapsed short name style.

`positional_arg<T>` does not use a 'marker' token on the command line for which its value follows, the value's position in the command line arguments determines what it is for.  The order that arguments are specified on the command line normally don't matter, but for positional arguments they do; for example in our cat program the files must be specified after the arguments so passing `myfile.hpp -n` would trigger the parser to land on the `positional_arg` for `myfile.hpp` which would then greedily consume the `-n` causing the application to try to open the file `-n`...  We'll cover constrained `positional_arg`s in later examples.  The `display_name` policy is used when generating help or error output - it is not used when parsing.

Assuming parsing was successful, the final `router` is called with the parsed argument e.g. if the user passed `-E file1 file2` then the `router` is passed `(true, false, -1, {"file1", "file2"})`.

You may have noticed that the nodes are constructed with parentheses whilst the policies use braces, this is necessary due to CTAD rules that affect nodes which return a user-defined value type.  This can be circumvented using a function to return the required instance, for example the actual type of a flag is `flag_t`, `flag(...)` is a function that creates one for you.

## Conditional Arguments
Let's add another feature to our cat program where we can handle lines over a certain length differently.
```cpp
namespace ard = ar::dependency;
ar::mode(
    ...
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
            arp::description<S_("Shortens line length to maximum with the "
                                "given suffix if max line length reached")>,
            arp::value_separator<'='>)),
    ...
    arp::router{[](bool show_all,
                   bool show_ends,
                   bool show_non_printing,
                   int max_lines,
                   std::optional<std::size_t> max_line_length,
                   std::variant<bool, std::string_view> max_line_handling,
                   std::vector<std::string_view>> files) { ... }}))
```
We've defined a new argument `--max-line-length` but rather than using `-1` as the "no limit" indicator like we did for `--max-lines`, we specify the argument type to be `std::optional<std::size_t>` and have the default value by an empty optional - this allows the code to define our intent better.

What do we do with lines that reach the limit if it has been set?  In our example we can either skip the line output, or truncate it with a suffix.  It doesn't make any sense to allow both of these options, so we declare them under a `one_of` node.  Under this node, only one is valid when parsing at runtime, if the user specifies both then it is an error.  A `one_of` must be marked as required or have a default value in case the user passes none of the arguments it handles.

To express the 'one of' concept better in code, the `one_of` node has a single representation in the `router`'s arguments - a variant that encompasses all the value types of each entry in it.  In our example's case, a bool for the `--skip-line` flag and a `string_view` for the `--line-suffix` case.

What happens if a user passes `--skip-line` or `--line-suffix` without `--max-line-length`?  Normally the developer will have to check that `max-line-length` is not empty and either ignore or throw if it is.  But by specifying the `one_of` as `dependent` on `max-line-length`, `arg_router` will throw on your behalf in this scenario.  To be clear, the `dependent` policy just means that the owner cannot appear on the command line without the `arg` or `flag` named in its parameters also appearing on the command line.

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
        arp::description<S_("Set the output colour theme")>,
        arp::value_separator<'='>,
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
    ar::counting_flag<verbosity_level_t>(
        arp::short_name<'v'>,
        arp::max_value<verbosity_level_t::DEBUG>(),
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
        std::vector<std::string_view>> files) { ... }}))
```
The number of times `-v` appears in the command line will increment the returned value, e.g. `-vv` will result in `verbosity_level_t::INFO`.

Note that even though we are using a custom enum, we haven't specified a `custom_parser`.  `counting_flag` will count up in `std::size_t` and then `static_cast` to the user-specified type, so as long as your requested type is explicitly convertible to/from `std::size_t` it will just work.  If it isn't, then you'll have to modify your type or not use counting arguments, as attaching a `custom_parser` to any kind of flag will result in a build failure.

Short name collapsing still works as expected, so passing `-Evnv` will result in `show_ends` and `show_non_printing` being true, and `verbosity_level` will be `verbosity_level_t::INFO` in the `router` call.

We can constrain the amount of flags the user can provide by using the `max_value` policy, so passing `-vvvv` will result in a runtime error.  There are min/max and min variants too.  Here we are using the compile-time variant of the policy, we can do that because the value type is an integral/enum, but if your value type cannot be used as a template parameter then there equivalent runtime variants that take the paramters as function parameters instead.  The compile-time variants should be used when possible due to extra checks e.g. setting the max value less than the min.

The `long_name` policy is allowed but usually leads to ugly invocations, however there is a better option:
```cpp
ar::mode(
    ...
    ard::alias_group(
        arp::default_value{verbosity_level_t::INFO},
        arp::max_value<verbosity_level_t::DEBUG>(),
        ar::counting_flag<verbosity_level_t>(
            arp::short_name<'v'>,
            arp::description<S_("Verbosity level, number of 'v's sets level")>),
        ar::arg<verbosity_level_t>(
            arp::long_name<S_("verbose")>,
            arp::value_separator<'='>,
            arp::description<S_("Verbosity level")>)),
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

`alias_group` is almost the opposite of `policy::alias`, it is an _output_ alias.  Each of its child nodes are aliases of the same output value i.e. it forms a many-to-one aliasing relationship.  You can also think of `alias_group` as a variation on `one_of`, but instead of the output being a `std::variant` of all the child node output types `alias_group` requires that they all have the _same_ output type.

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
        arp::description<S_("Display this help and exit")>),
    ar::mode(
        ar::flag(
            arp::long_name<S_("force")>,
            arp::short_name<'f'>,
            arp::description<S_("Force overwrite existing files")>),
        ar::positional_arg<std::filesystem::path>(
            arp::display_name<S_("DST")>,
            arp::description<S_("Destination directory")>,
            arp::required,
            arp::fixed_count<1>),
        ar::positional_arg<std::vector<std::filesystem::path>>(
            arp::display_name<S_("SRC")>,
            arp::description<S_("Source file paths")>,
            arp::required,
            arp::min_count<1>),
        arp::router{[](bool force,
                       std::filesystem::path dest,
                       std::vector<std::filesystem::path> srcs) { ... }}))

```
It was noted in the [Basics](#basics) section that ordering does matter for positional arguments, so we can see here that the destination path is specified first and therefore comes first on the command line.  There can only be one destination path so we specify the `count` to one.  Because the `count` is 1 the return type doesn't need to be a container.

Following the destination path are the source paths, we need at least one so we mark it as required, as our range is unbounded the `value_type` needs to be a container.  `positional_arg` uses a `push_back` call on the container, so `std::vector` is the typical type to use.

Only the last `positional_arg` may be of variable length.  A runtime error will only occur if there are no unbounded variable length `postional_arg`s and there are more arguments than the maximum or less than the minimum.

It should be noted that setting a non-zero minimum count (`min_count`, `fixed_count`, or `min_max_count`) does _not_ imply a requirement, the minimum count check only applies when there is at least one argument for the node to process.  So as with an `arg`, you should use a `required` policy to explicitly state that at least one argument needs to be present, or a `default_value` policy - otherwise a default initialised value will be used instead.  For `positional_arg` nodes that are marked as `required`, it is a compile-time error to have a minimum count policy value of 0.

## Modes
As noted in [Basics](#basics), `mode`s allow you to group command line components under an initial token on the command line.  A common example of this developers will be aware of is `git`, for example in our parlance `git clean -ffxd`; `clean` would be the mode and `ffxd` would be be the flags that are available under that mode.

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
        arp::description<S_("Display this help and exit")>),
    ar::mode(
        arp::none_name<S_("copy")>,
        arp::description<S_("Copy source files to destination")>,
        ar::flag(
            arp::long_name<S_("force")>,
            arp::short_name<'f'>,
            arp::description<S_("Force overwrite existing files")>),
        ar::positional_arg<std::filesystem::path>(
            arp::required,
            arp::display_name<S_("DST")>,
            arp::description<S_("Destination directory")>,
            arp::fixed_count<1>),
        ar::positional_arg<std::vector<std::filesystem::path>>(
            arp::required,
            arp::display_name<S_("SRC")>,
            arp::description<S_("Source file paths")>,
            arp::min_count<1>),
        arp::router{[](bool force,
                       std::filesystem::path dest,
                       std::vector<std::filesystem::path> srcs) { ... }}),
    ar::mode(
        arp::none_name<S_("move")>,
        arp::description<S_("Move source file to destination")>,
        ar::flag(
            arp::long_name<S_("force")>,
            arp::short_name<'f'>,
            arp::description<S_("Force overwrite existing files")>),
        ar::positional_arg<std::filesystem::path>(
            arp::required,
            arp::display_name<S_("DST")>,
            arp::description<S_("Destination directory")>,
            arp::fixed_count<1>),
        // Can only have one
        ar::positional_arg<std::filesystem::path>(
            arp::required,
            arp::display_name<S_("SRC")>,
            arp::description<S_("Source file path")>,
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
    ar::flag(
        arp::long_name<S_("force")>,
        arp::short_name<'f'>,
        arp::description<S_("Force overwrite existing files")>),
    ar::positional_arg<std::filesystem::path>(
        arp::required,
        arp::display_name<S_("DST")>,
        arp::description<S_("Destination directory")>,
        arp::fixed_count<1>)};

ar::root(
    arp::validation::default_validator,
    ar::mode(
        arp::none_name<S_("copy")>,
        arp::description<S_("Copy source files to destination")>,
        common_args,
        ar::positional_arg<std::vector<std::filesystem::path>>(
            arp::required,
            arp::display_name<S_("SRC")>,
            arp::description<S_("Source file paths")>,
            arp::min_count<1>),
        arp::router{[](bool force,
                       std::filesystem::path dest,
                       std::vector<std::filesystem::path> srcs) { ... }}),
    ar::mode(
        arp::none_name<S_("move")>,
        arp::description<S_("Move source file to destination")>,
        common_args,
        ar::positional_arg<std::filesystem::path>(
            arp::required,
            arp::display_name<S_("SRC")>,
            arp::description<S_("Source file path")>,
            arp::fixed_count<1>),
        arp::router{[](bool force,
                       std::filesystem::path dest,
                       std::filesystem::path src) { ... }}))
```
`ar::list` is a simple `arg` and `flag` container that `mode` and `root` instances detect and add the contents to their child/policy lists - it's nicer than having an instance per type.  Also don't be afraid of the copies, the majority of `arg_router` types hold no data (the advantage of compile-time!) and those that do (e.g. `default_value`) generally have small types like primitives or `std::string_view`.

## Help Output
A shown in prior sections, a `help` node can be a child of the `root` (and only the `root`!), which acts like an `arg`, and generates the help output when requested by the user.  This node is optional, without it there is no help command line argument.  As the node is `arg`-like, it requires a long and/or short name.

The output can be tweaked using policies:
- `program_name`, this is printed first.  Without it neither this nor the version are printed
- `program_version`, this followes the name
- `program_intro`, used to give some more information on the program.  This is printed two new lines away from the name and version
- `program_addendum`, used to add supplementary text after the argument output
- `flatten_help`, by default only top-level arguments and/or those in an anonymous mode are displayed.  Child modes are shown by requesting the mode's 'path' on the command line (e.g. `app --help mode sub-mode`).  The presence of this policy will make the entire requested subtree's (or root's, if no mode path was requested) help output be displayed

Unlike string data everywhere else in the library, the formatted help output is created at runtime using `std::string` so we don't need to keep duplicate read-only text data.

For example the slightly embellished tree from above:
```cpp
const auto common_args = ar::list{
    ar::flag(
        arp::long_name<S_("force")>,
        arp::short_name<'f'>,
        arp::description<S_("Force overwrite existing files")>),
    ar::positional_arg<std::filesystem::path>(
        arp::required,
        arp::display_name<S_("DST")>,
        arp::description<S_("Destination directory")>,
        arp::fixed_count<1>)};

ar::root(
    arp::validation::default_validator,
    ar::help(
        arp::long_name<S_("help")>,
        arp::short_name<'h'>,
        arp::description<S_("Display this help and exit")>,
        arp::program_name<S_("simple")>,
        arp::program_version<S_("v0.1")>,
        arp::program_intro<S_("A simple file copier and mover.")>,
        arp::program_addendum<S_("An example program for arg_router.")>,
        arp::flatten_help),
    ar::mode(
        arp::none_name<S_("copy")>,
        arp::description<S_("Copy source files to destination")>,
        common_args,
        ar::positional_arg<std::vector<std::filesystem::path>>(
            arp::required,
            arp::display_name<S_("SRC")>,
            arp::description<S_("Source file paths")>,
            arp::min_count<1>),
        arp::router{[](bool force,
                       std::filesystem::path dest,
                       std::vector<std::filesystem::path> srcs) { ... }}),
    ar::mode(
        arp::none_name<S_("move")>,
        arp::description<S_("Move source file to destination")>,
        common_args,
        ar::positional_arg<std::filesystem::path>(
            arp::required,
            arp::display_name<S_("SRC")>,
            arp::description<S_("Source file path")>,
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
By default when parsed `help` will output its contents to `std::cout` and then exit the application with `EXIT_SUCCESS`.  Obviously this won't always be desired, so a `router` policy can be attached that will pass a `std::ostringstream` to the user-provided `Callable`.  The stream will have already been populated with the help data shown above, but it can now be appended to or converted to string for use somewhere else.

Often programmatic access is desired for the help output outside of the user requesting it, for example if a parse exception is thrown, generally the exception error is printed to the terminal followed by the help output.  This is exposed by the `help()` or `help(std::ostringstream&)` methods of the root object.

### Customisation
Help output can be customised in several ways:
1. Use a `router` policy to capture the output and modify it.  This is useful for appending string data, but anything more sophisticated becomes a chore
2. Write your own `help` node.  This is the nuclear option as it gives maximal control but is a lot of work, it is very rarely necessary to do this as the node is primarily just the `tree_node` implementation, the actual formatting is delegated
3. Write your own help formatter policy for the built-in `help` node.  The `help` node delegates the formatting to a policy, if no formatter is specified when defining a `help` node specialisation the `default_help_formatter` is used.  This is still non-trivial as the formatter policy needs to perform the compile-time tree iteration in order to process the per-node help data
4. Write your own line formatter and/or preamble formatter. The `default_help_formatter` further delegates formatting to two sub-policies, one that generates the 'preamble' text (i.e. the program name, version, intro) and another that generates each argument in the argument output.  `default_helper_formatter` uses `help_formatter_component::default_preamble_formatter` and `help_formatter_component::default_line_formatter` respectively by default

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
`arg_router` has support for runtime language selection, using `multi_lang::root_wrapper`.  As the name suggests, this wraps around a `Callable` that returns a `root` instance 'tweaked' for a given language.  Let's take the start of the simple file copier/mover application, and convert it:
```cpp
// Apologies for any translation faux pas - Google Translate did it for me!
ar::multi_lang::root_wrapper<S_("en_GB"), S_("fr"), S_("ja")>(  //
    ar::multi_lang::iso_locale(std::locale("").name()),
    [&](auto I) {
        const auto common_args = ar::list{
            ar::flag(arp::long_name<SM_(I, "force", "forcer", "強制")>,
                     arp::short_name<'f'>,
                     arp::description<SM_(I,
                                          "Force overwrite existing files",
                                          "Forcer l'écrasement des fichiers existants",
                                          "既存のファイルを強制的に上書きする")>),
            ar::positional_arg<fs::path>(arp::required,
                                         arp::display_name<SM_(I, "DST", "DST", "先")>,
                                         arp::description<SM_(I,
                                                              "Destination directory",
                                                              "Répertoire de destination",
                                                              "宛先ディレクトリ")>,
                                         arp::fixed_count<1>)};

        return ar::root(
            arp::validation::default_validator,
            ar::help(arp::long_name<SM_(I, "help", "aider", "ヘルプ")>,
                     arp::short_name<'h'>,
                     arp::description<SM_(I,
                                          "Display this help and exit",
                                          "Afficher cette aide et quitter",
                                          "このヘルプを表示して終了")>,
                     arp::program_name<S_("simple")>,
                     arp::program_version<S_("v0.1")>,
                     arp::program_intro<SM_(
                        I,
                        "A simple file copier and mover.",
                        "Un simple copieur et déménageur de fichiers.",
                        "ファイルをコピーおよび移動するためのシンプルなプログラム。")>,
                     arp::program_addendum<SM_(I,
                                               "An example program for arg_router.",
                                               "Un exemple de programme pour arg_router.",
                                               "「arg_router」のサンプルプログラム。")>,
                     arp::flatten_help,
                     arp::colour_help_formatter),
                ...
        );
    }).parse(argc, argv);
```
The `multi_lang::root_wrapper` takes a set of supported language identifiers (ISO language/country codes are recommended for readability, but not required), the number and order of these defines the number and order of translations needed for each translated compile-time string in the root definition.  Core to this functionality is the `SM_` macro, which just expands to an element selector in a tuple of compile-time strings - depending on the value of the index passed in, it resolves to one of the translation strings.

The first argument is a string provided by the user that should match one of the language identifiers, if it doesn't `arg_router` will fall back to using the first defined language (UK English in this case).  As a convenience `arg_router` provides a simple function that takes the OS locale name and standardises it to an ISO language/country identifier.

As you may have guessed from the interface, `multi_lang::root_wrapper` works by simply finding a match for the input against the supported language identifiers, and instantiating a `root` instance via the user-provided `Callable` with the index of the selected language.  This root instance is then held in a variant, which `root_wrapper` accesses when its `root`-compatible interface is used.

## Error Handling
Currently `arg_router` only supports exceptions as error handling.  If a parsing fails for some reason a `arg_router::parse_exception` is thrown carrying information on the failure.

## Installation and Dependencies
If you're simply a library user, then download the pre-packaged release and install somewhere.  You will need the following dependencies in order to build:
* Boost.mp11 v1.74+
* Boost.Preprocessor v1.74+
* Boost.Lexical_Cast v1.74+
* [span-lite](https://github.com/martinmoene/span-lite) (only needed if building against C++17)

`arg_router` is header-only (due to all the templates) and so are the above dependencies.

**Note** currently arg_router requires exception support, but _not_ RTTI.

To get a pre-release `arg_router`, or build the unit tests and examples, simply check out the repo and build via CMake in the usual way - the unit tests will be built by default:
```
$ cd arg_router
$ mkdir build
$ cd ./build
$ cmake -G "Ninja" ..
$ cmake --build .
$ cmake --install .
```
Building these targets will require more dependencies:
* clang-format
* Python v3 (used for copyright checking)
* Doxygen
* Boost.Test v1.74+
* Boost.Filesystem v1.74+
* Boost.Process v1.74+
* Git

By default all these dependencies are provided by `vcpkg` automatically, please **note** that `vcpkg` is provided via a submodule and therefore will need initialising (`git submodule update`).  If you would rather the dependencies came from the system then simply set `-DDISABLE_VCPKG=OFF`, and CMake will not bootstrap `vcpkg` and therefore try to find the packages locally.

## Configuration
Low-level tweaking of the library is achieved via some defines and/or CMake variables, documented [here](https://cmannett85.github.io/arg_router/configuration.html).

## Supported Compilers/Platforms
The CI system attached to this repo builds the unit tests and examples with:
* Ubuntu 22.04 (Ninja), Clang 14/gcc-9
* Windows Server 2022 (Ninja, MSBuild), Clang 14.0.5
* MacOS 12 (Ninja), Apple Clang 13.1.6

Other compiler versions and platform combinations may work, but I'm currently limited by the built-in GitHub runners and how much I'm willing to spend on Actions!

You can build on Windows with the VS 2022 generator (MSBuild) but you must set the CMake variable `DEATH_TEST_PARALLEL` to 1 otherwise the parallel tests will attempt to write to the project-wide `lastSuccessfulBuild` file simultaneously, which causes the build to fail.

You'll notice the big omission: No MSVC.  That's because even with the latest version, I get nothing but ICEs out of it with no useful diagnostics making fixing the issues close to impossible.

## Tips for Users
### Do **NOT** Make the Parse Tree Type Accessible
The parse tree is _very_ expensive to construct due to all the compile-time checking and meta-programming shennanigans, so do **NOT** define it in a header and have multiple source files include it - it will cause the tree to be built/checked in every source file it is included in.

### Minimise Static Storage Bloat
Despite not using `typeid` or `dynamic_cast` in the library, compilers will still generate class name data if RTTI is enabled.  Due to the highly nested templates that make up the parse tree, these class names can become huge and occupy large amount of static storage in the executable.  As an example, the `basic_cat` project in the repo will create ~100KB of class name data in the binary - this data is not used and cannot be stripped out.

Disabling RTTI is rarely feasible for most projects, but it is possible to disable RTTI for a single CMake target.  So if it was deemed worth it for the size reduction, the command line parsing could be the application's executable (compiled without RTTI) and then the wider application logic could be in a static library (compiled with RTTI).  This does not affect exceptions, as their type information is always added by the compiler regardless of RTTI status.

## Extra Documentation
Complete Doxygen-generated API documentation is available [here](https://cmannett85.github.io/arg_router/).  Examples are provided in the `examples` directory of the repo or online [here](https://github.com/cmannett85/arg_router/tree/develop/examples).

## Future Work
Take a look at the [issues](https://github.com/cmannett85/arg_router/issues) page for all upcoming features and fixes.  Highlights are:
* Finally get it working on MSVC ([#102](https://github.com/cmannett85/arg_router/issues/102))
* Add to vcpkg ([#8](https://github.com/cmannett85/arg_router/issues/8))
