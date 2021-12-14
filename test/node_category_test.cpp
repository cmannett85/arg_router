#include "arg_router/node_category.hpp"
#include "arg_router/arg.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/default_value.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/positional_arg.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(node_category_suite)

BOOST_AUTO_TEST_CASE(has_fixed_count_test)
{
    {
        using type = decltype(flag(policy::long_name<S_("hello")>,
                                   policy::description<S_("This is a hello")>,
                                   policy::short_name<'h'>));
        static_assert(node_category::has_fixed_count_v<type, 0>, "Fail");
        static_assert(!node_category::has_fixed_count_v<type, 1>, "Fail");
    }

    {
        using type =
            decltype(arg<int>(policy::long_name<S_("hello")>,
                              policy::description<S_("This is a hello")>,
                              policy::default_value{42},
                              policy::short_name<'h'>));
        static_assert(!node_category::has_fixed_count_v<type, 0>, "Fail");
        static_assert(node_category::has_fixed_count_v<type, 1>, "Fail");
    }

    {
        using type = decltype(positional_arg<std::vector<int>>(
            policy::long_name<S_("hello")>,
            policy::description<S_("This is a hello")>));
        static_assert(!node_category::has_fixed_count_v<type, 0>, "Fail");
        static_assert(!node_category::has_fixed_count_v<type, 1>, "Fail");
    }

    {
        using type = decltype(positional_arg<std::vector<int>>(
            policy::long_name<S_("hello")>,
            policy::description<S_("This is a hello")>,
            policy::count<5>));
        static_assert(!node_category::has_fixed_count_v<type, 0>, "Fail");
        static_assert(node_category::has_fixed_count_v<type, 5>, "Fail");
    }

    {
        using type = decltype(positional_arg<std::vector<int>>(
            policy::long_name<S_("hello")>,
            policy::description<S_("This is a hello")>,
            policy::min_count<5>,
            policy::max_count<5>));
        static_assert(!node_category::has_fixed_count_v<type, 0>, "Fail");
        static_assert(node_category::has_fixed_count_v<type, 5>, "Fail");
    }

    {
        using type = decltype(positional_arg<std::vector<int>>(
            policy::long_name<S_("hello")>,
            policy::description<S_("This is a hello")>,
            policy::min_count<5>));
        static_assert(!node_category::has_fixed_count_v<type, 0>, "Fail");
        static_assert(!node_category::has_fixed_count_v<type, 5>, "Fail");
    }

    {
        using type = decltype(positional_arg<std::vector<int>>(
            policy::long_name<S_("hello")>,
            policy::description<S_("This is a hello")>,
            policy::min_count<2>,
            policy::max_count<5>));
        static_assert(!node_category::has_fixed_count_v<type, 0>, "Fail");
        static_assert(!node_category::has_fixed_count_v<type, 2>, "Fail");
        static_assert(!node_category::has_fixed_count_v<type, 5>, "Fail");
    }
}

BOOST_AUTO_TEST_CASE(has_no_count_test)
{
    {
        using type = decltype(flag(policy::long_name<S_("hello")>,
                                   policy::description<S_("This is a hello")>,
                                   policy::short_name<'h'>));
        static_assert(!node_category::has_no_count_v<type>, "Fail");
    }

    {
        using type = decltype(positional_arg<std::vector<int>>(
            policy::long_name<S_("hello")>,
            policy::description<S_("This is a hello")>));
        static_assert(node_category::has_no_count_v<type>, "Fail");
    }

    {
        using type = decltype(positional_arg<std::vector<int>>(
            policy::long_name<S_("hello")>,
            policy::description<S_("This is a hello")>,
            policy::count<5>));
        static_assert(!node_category::has_no_count_v<type>, "Fail");
    }

    {
        using type = decltype(positional_arg<std::vector<int>>(
            policy::long_name<S_("hello")>,
            policy::description<S_("This is a hello")>,
            policy::min_count<5>));
        static_assert(!node_category::has_no_count_v<type>, "Fail");
    }
}

BOOST_AUTO_TEST_CASE(is_named_test)
{
    {
        using type = decltype(flag(policy::long_name<S_("hello")>,
                                   policy::description<S_("This is a hello")>,
                                   policy::short_name<'h'>));
        static_assert(node_category::is_named_v<type>, "Fail");
    }

    {
        using type = decltype(flag(policy::long_name<S_("hello")>,
                                   policy::description<S_("This is a hello")>));
        static_assert(node_category::is_named_v<type>, "Fail");
    }

    {
        using type = decltype(flag(policy::description<S_("This is a hello")>,
                                   policy::short_name<'h'>));
        static_assert(node_category::is_named_v<type>, "Fail");
    }

    {
        using type =
            decltype(mode(flag(policy::long_name<S_("hello")>,
                               policy::description<S_("Hello description")>)));
        static_assert(!node_category::is_named_v<type>, "Fail");
    }

    {
        using type =
            decltype(mode(flag(policy::long_name<S_("hello")>,
                               policy::description<S_("Hello description")>),
                          policy::long_name<S_("mode")>));
        static_assert(node_category::is_named_v<type>, "Fail");
    }
}

BOOST_AUTO_TEST_CASE(is_generic_flag_like_test)
{
    {
        using type = decltype(flag(policy::long_name<S_("hello")>,
                                   policy::description<S_("This is a hello")>,
                                   policy::short_name<'h'>));
        static_assert(node_category::is_generic_flag_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(arg<int>(policy::long_name<S_("hello")>,
                              policy::description<S_("This is a hello")>,
                              policy::default_value{42},
                              policy::short_name<'h'>));
        static_assert(!node_category::is_generic_flag_like_v<type>, "Fail");
    }

    {
        using type = decltype(positional_arg<std::vector<int>>(
            policy::long_name<S_("hello")>,
            policy::description<S_("This is a hello")>));
        static_assert(!node_category::is_generic_flag_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(mode(flag(policy::long_name<S_("hello")>,
                               policy::description<S_("Hello description")>)));
        static_assert(!node_category::is_generic_flag_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(mode(flag(policy::long_name<S_("hello")>,
                               policy::description<S_("Hello description")>),
                          policy::long_name<S_("mode")>));
        static_assert(!node_category::is_generic_flag_like_v<type>, "Fail");
    }
}

BOOST_AUTO_TEST_CASE(is_flag_like_test)
{
    {
        using type = decltype(flag(policy::long_name<S_("hello")>,
                                   policy::description<S_("This is a hello")>,
                                   policy::short_name<'h'>));
        static_assert(node_category::is_flag_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(arg<int>(policy::long_name<S_("hello")>,
                              policy::description<S_("This is a hello")>,
                              policy::default_value{42},
                              policy::short_name<'h'>));
        static_assert(!node_category::is_flag_like_v<type>, "Fail");
    }

    {
        using type = decltype(positional_arg<std::vector<int>>(
            policy::long_name<S_("hello")>,
            policy::description<S_("This is a hello")>));
        static_assert(!node_category::is_flag_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(mode(flag(policy::long_name<S_("hello")>,
                               policy::description<S_("Hello description")>)));
        static_assert(!node_category::is_flag_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(mode(flag(policy::long_name<S_("hello")>,
                               policy::description<S_("Hello description")>),
                          policy::long_name<S_("mode")>));
        static_assert(!node_category::is_flag_like_v<type>, "Fail");
    }
}

BOOST_AUTO_TEST_CASE(is_arg_like_test)
{
    {
        using type = decltype(flag(policy::long_name<S_("hello")>,
                                   policy::description<S_("This is a hello")>,
                                   policy::short_name<'h'>));
        static_assert(!node_category::is_arg_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(arg<int>(policy::long_name<S_("hello")>,
                              policy::description<S_("This is a hello")>,
                              policy::default_value{42},
                              policy::short_name<'h'>));
        static_assert(node_category::is_arg_like_v<type>, "Fail");
    }

    {
        using type = decltype(positional_arg<std::vector<int>>(
            policy::long_name<S_("hello")>,
            policy::description<S_("This is a hello")>));
        static_assert(!node_category::is_arg_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(mode(flag(policy::long_name<S_("hello")>,
                               policy::description<S_("Hello description")>)));
        static_assert(!node_category::is_arg_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(mode(flag(policy::long_name<S_("hello")>,
                               policy::description<S_("Hello description")>),
                          policy::long_name<S_("mode")>));
        static_assert(!node_category::is_arg_like_v<type>, "Fail");
    }
}

BOOST_AUTO_TEST_CASE(is_positional_arg_like_test)
{
    {
        using type = decltype(flag(policy::long_name<S_("hello")>,
                                   policy::description<S_("This is a hello")>,
                                   policy::short_name<'h'>));
        static_assert(!node_category::is_positional_arg_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(arg<int>(policy::long_name<S_("hello")>,
                              policy::description<S_("This is a hello")>,
                              policy::default_value{42},
                              policy::short_name<'h'>));
        static_assert(!node_category::is_positional_arg_like_v<type>, "Fail");
    }

    {
        using type = decltype(positional_arg<std::vector<int>>(
            policy::long_name<S_("hello")>,
            policy::description<S_("This is a hello")>));
        static_assert(node_category::is_positional_arg_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(mode(flag(policy::long_name<S_("hello")>,
                               policy::description<S_("Hello description")>)));
        static_assert(!node_category::is_positional_arg_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(mode(flag(policy::long_name<S_("hello")>,
                               policy::description<S_("Hello description")>),
                          policy::long_name<S_("mode")>));
        static_assert(!node_category::is_positional_arg_like_v<type>, "Fail");
    }
}

BOOST_AUTO_TEST_CASE(is_generic_mode_like_test)
{
    {
        using type = decltype(flag(policy::long_name<S_("hello")>,
                                   policy::description<S_("This is a hello")>,
                                   policy::short_name<'h'>));
        static_assert(!node_category::is_generic_mode_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(arg<int>(policy::long_name<S_("hello")>,
                              policy::description<S_("This is a hello")>,
                              policy::default_value{42},
                              policy::short_name<'h'>));
        static_assert(!node_category::is_generic_mode_like_v<type>, "Fail");
    }

    {
        using type = decltype(positional_arg<std::vector<int>>(
            policy::long_name<S_("hello")>,
            policy::description<S_("This is a hello")>));
        static_assert(!node_category::is_generic_mode_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(mode(flag(policy::long_name<S_("hello")>,
                               policy::description<S_("Hello description")>)));
        static_assert(node_category::is_generic_mode_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(mode(flag(policy::long_name<S_("hello")>,
                               policy::description<S_("Hello description")>),
                          policy::long_name<S_("mode")>));
        static_assert(node_category::is_generic_mode_like_v<type>, "Fail");
    }
}

BOOST_AUTO_TEST_CASE(is_anonymous_mode_like_test)
{
    {
        using type = decltype(flag(policy::long_name<S_("hello")>,
                                   policy::description<S_("This is a hello")>,
                                   policy::short_name<'h'>));
        static_assert(!node_category::is_anonymous_mode_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(arg<int>(policy::long_name<S_("hello")>,
                              policy::description<S_("This is a hello")>,
                              policy::default_value{42},
                              policy::short_name<'h'>));
        static_assert(!node_category::is_anonymous_mode_like_v<type>, "Fail");
    }

    {
        using type = decltype(positional_arg<std::vector<int>>(
            policy::long_name<S_("hello")>,
            policy::description<S_("This is a hello")>));
        static_assert(!node_category::is_anonymous_mode_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(mode(flag(policy::long_name<S_("hello")>,
                               policy::description<S_("Hello description")>)));
        static_assert(node_category::is_anonymous_mode_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(mode(flag(policy::long_name<S_("hello")>,
                               policy::description<S_("Hello description")>),
                          policy::long_name<S_("mode")>));
        static_assert(!node_category::is_anonymous_mode_like_v<type>, "Fail");
    }
}

BOOST_AUTO_TEST_CASE(is_named_mode_like_test)
{
    {
        using type = decltype(flag(policy::long_name<S_("hello")>,
                                   policy::description<S_("This is a hello")>,
                                   policy::short_name<'h'>));
        static_assert(!node_category::is_named_mode_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(arg<int>(policy::long_name<S_("hello")>,
                              policy::description<S_("This is a hello")>,
                              policy::default_value{42},
                              policy::short_name<'h'>));
        static_assert(!node_category::is_named_mode_like_v<type>, "Fail");
    }

    {
        using type = decltype(positional_arg<std::vector<int>>(
            policy::long_name<S_("hello")>,
            policy::description<S_("This is a hello")>));
        static_assert(!node_category::is_named_mode_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(mode(flag(policy::long_name<S_("hello")>,
                               policy::description<S_("Hello description")>)));
        static_assert(!node_category::is_named_mode_like_v<type>, "Fail");
    }

    {
        using type =
            decltype(mode(flag(policy::long_name<S_("hello")>,
                               policy::description<S_("Hello description")>),
                          policy::long_name<S_("mode")>));
        static_assert(node_category::is_named_mode_like_v<type>, "Fail");
    }
}

BOOST_AUTO_TEST_SUITE_END()
