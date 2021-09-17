#include "arg_router/tree_node.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(tree_node_suite)

BOOST_AUTO_TEST_CASE(tree_node_test)
{
    auto tn =
        tree_node{policy::long_name<S_("hello")>, policy::short_name<'A'>};
    static_assert(tn.long_name() == "hello"sv, "Long name does not match");
    static_assert(tn.short_name() == 'A', "Short name does not match");
}

BOOST_AUTO_TEST_CASE(mixed_tree_node_types_test)
{
    using tn = tree_node<policy::long_name_t<S_("hello")>,
                         std::vector<int>,
                         tree_node<policy::long_name_t<S_("child")>>,
                         policy::short_name_t<traits::integral_constant<'A'>>>;
    static_assert(
        std::is_same_v<
            typename tn::parameters_type,
            std::tuple<policy::long_name_t<S_("hello")>,
                       std::vector<int>,
                       tree_node<policy::long_name_t<S_("child")>>,
                       policy::short_name_t<traits::integral_constant<'A'>>>>,
        "parameters_type does not match");

    static_assert(
        std::is_same_v<
            typename tn::policies_type,
            std::tuple<policy::long_name_t<S_("hello")>,
                       policy::short_name_t<traits::integral_constant<'A'>>>>,
        "policies_type does not match");

    static_assert(
        std::is_same_v<typename tn::children_type,
                       std::tuple<tree_node<policy::long_name_t<S_("child")>>>>,
        "children_type does not match");
}

BOOST_AUTO_TEST_CASE(only_policies_tree_node_types_test)
{
    using tn = tree_node<policy::long_name_t<S_("hello")>,
                         policy::short_name_t<traits::integral_constant<'A'>>>;
    static_assert(
        std::is_same_v<
            typename tn::parameters_type,
            std::tuple<policy::long_name_t<S_("hello")>,
                       policy::short_name_t<traits::integral_constant<'A'>>>>,
        "parameters_type does not match");

    static_assert(
        std::is_same_v<
            typename tn::policies_type,
            std::tuple<policy::long_name_t<S_("hello")>,
                       policy::short_name_t<traits::integral_constant<'A'>>>>,
        "policies_type does not match");

    static_assert(std::is_same_v<typename tn::children_type, std::tuple<>>,
                  "children_type does not match");
}

BOOST_AUTO_TEST_CASE(empty_tree_node_types_test)
{
    using tn = tree_node<>;
    static_assert(std::is_same_v<typename tn::parameters_type, std::tuple<>>,
                  "parameters_type does not match");

    static_assert(std::is_same_v<typename tn::policies_type, std::tuple<>>,
                  "policies_type does not match");

    static_assert(std::is_same_v<typename tn::children_type, std::tuple<>>,
                  "children_type does not match");
}

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(!is_tree_node_v<float>, "Should not be a tree_node");
    static_assert(!is_tree_node_v<std::vector<float>>,
                  "Should not be a tree_node");

    static_assert(is_tree_node_v<tree_node<float>>, "Should be a tree_node");
    static_assert(is_tree_node_v<tree_node<float, int, double>>,
                  "Should be a tree_node");
    static_assert(is_tree_node_v<tree_node<float, int, double>>,
                  "Should be a tree_node");
    static_assert(is_tree_node_v<tree_node<policy::long_name_t<S_("hello")>>>,
                  "Should be a tree_node");
    static_assert(is_tree_node_v<tree_node<policy::long_name_t<S_("hello")>,
                                           policy::long_name_t<S_("goodbye")>>>,
                  "Should be a tree_node");
}

BOOST_AUTO_TEST_SUITE_END()
