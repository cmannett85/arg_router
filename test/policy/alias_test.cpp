#include "arg_router/policy/alias.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(alias_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::alias_t<>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(aliased_node_indices_test)
{
    using mode_type = arg_router::mode_t<
        flag_t<policy::long_name_t<S_("aaa")>>,
        flag_t<policy::short_name_t<traits::integral_constant<'a'>>>,
        flag_t<policy::long_name_t<S_("bbb")>,
               policy::short_name_t<traits::integral_constant<'b'>>>>;

    {
        using result = typename policy::alias_t<policy::long_name_t<S_(
            "aaa")>>::template aliased_node_indices<mode_type>;
        static_assert(
            std::is_same_v<result,
                           std::tuple<std::integral_constant<std::size_t, 0>>>,
            "Incorrect result");
    }

    {
        using result = typename policy::alias_t<policy::long_name_t<S_(
            "bbb")>>::template aliased_node_indices<mode_type>;
        static_assert(
            std::is_same_v<result,
                           std::tuple<std::integral_constant<std::size_t, 2>>>,
            "Incorrect result");
    }

    {
        using result = typename policy::alias_t<
            policy::short_name_t<traits::integral_constant<'a'>>>::
            template aliased_node_indices<mode_type>;
        static_assert(
            std::is_same_v<result,
                           std::tuple<std::integral_constant<std::size_t, 1>>>,
            "Incorrect result");
    }

    {
        using result = typename policy::alias_t<
            policy::short_name_t<traits::integral_constant<'b'>>>::
            template aliased_node_indices<mode_type>;
        static_assert(
            std::is_same_v<result,
                           std::tuple<std::integral_constant<std::size_t, 2>>>,
            "Incorrect result");
    }

    {
        using result = typename policy::alias_t<
            policy::short_name_t<traits::integral_constant<'a'>>,
            policy::long_name_t<S_("aaa")>>::
            template aliased_node_indices<mode_type>;
        static_assert(
            std::is_same_v<result,
                           std::tuple<std::integral_constant<std::size_t, 0>,
                                      std::integral_constant<std::size_t, 1>>>,
            "Incorrect result");
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
