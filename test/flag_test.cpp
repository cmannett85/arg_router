#include "arg_router/flag.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(flag_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(is_tree_node_v<flag_t<>>, "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(policies_test)
{
    auto f = flag(policy::long_name<S_("hello")>, policy::short_name<'H'>);
    static_assert(f.long_name() == "hello"sv, "Long name test fail");
    static_assert(f.short_name() == "H", "Short name test fail");
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(policies_only_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = flag(
        policy::long_name<S_("hello")>,
        flag(policy::short_name<'b'>),
        policy::short_name<'H'>
    );
    return 0;
}
    )",
        "Flags must only contain policies (not other nodes)");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
