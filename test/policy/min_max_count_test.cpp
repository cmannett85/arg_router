#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;

namespace
{
template <typename... Policies>
class stub_node : public tree_node<Policies...>
{
public:
    constexpr explicit stub_node(Policies... policies) :
        tree_node<Policies...>{std::move(policies)...}
    {
    }

    template <typename... Parents>
    bool pre_parse_phase(parsing::token_list& tokens,
                         const Parents&... parents) const
    {
        auto hit = false;
        utility::tuple_type_iterator<typename stub_node::policies_type>(  //
            [&](auto i) {
                using this_policy =
                    std::tuple_element_t<i, typename stub_node::policies_type>;
                if constexpr (policy::has_pre_parse_phase_method_v<
                                  this_policy,
                                  Parents...> &&
                              traits::is_specialisation_of_v<
                                  this_policy,
                                  policy::min_max_count_t>) {
                    this->this_policy::pre_parse_phase(tokens, parents...);
                    hit = true;
                }
            });

        return hit;
    }
};
}  // namespace

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(min_max_count_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<
                      policy::min_max_count_t<traits::integral_constant<0>,
                                              traits::integral_constant<0>>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(min_count_test)
{
    static_assert(policy::fixed_count<42u>.minimum_count() == 42, "Fail");
    static_assert(policy::fixed_count<42u>.maximum_count() == 42, "Fail");

    static_assert(policy::fixed_count<5>.minimum_count() == 5, "Fail");
    static_assert(policy::fixed_count<5>.maximum_count() == 5, "Fail");
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_test)
{
    const auto root = stub_node{
        stub_node{policy::long_name<S_("node1")>, policy::min_count<1>},
        stub_node{policy::long_name<S_("node2")>, policy::min_count<2>},
        stub_node{policy::long_name<S_("node3")>}};

    auto f = [&](auto tokens,
                 const auto& owner,
                 auto expected_result,
                 std::string fail_message) {
        auto tokens_backup = tokens;
        try {
            const auto result = owner.pre_parse_phase(tokens, owner, root);
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK_EQUAL(result, expected_result);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }

        // Make sure input tokens is unchanged
        BOOST_CHECK_EQUAL(tokens, tokens_backup);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{
                parsing::token_list{{parsing::prefix_type::NONE, "42"},
                                    {parsing::prefix_type::LONG, "foo"},
                                    {parsing::prefix_type::NONE, "hello"}},
                std::get<0>(root.children()),
                true,
                ""},
            std::tuple{parsing::token_list{{parsing::prefix_type::NONE, "42"}},
                       std::get<1>(root.children()),
                       true,
                       "Minimum count not reached: --node2"},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::NONE, "42"},
                                    {parsing::prefix_type::LONG, "foo"},
                                    {parsing::prefix_type::NONE, "hello"}},
                std::get<1>(root.children()),
                true,
                ""},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::NONE, "42"},
                                    {parsing::prefix_type::NONE, "foo"},
                                    {parsing::prefix_type::LONG, "hello"}},
                std::get<2>(root.children()),
                false,
                ""}});
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(value_type_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/min_max_count.hpp"

struct my_type {};

int main() {
    const auto tmp = arg_router::policy::min_max_count_t<my_type, my_type>{};
    return 0;
}
    )",
        "MinType and MaxType must have a value_type");
}

BOOST_AUTO_TEST_CASE(integral_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/min_max_count.hpp"

struct my_type {
    using value_type = std::string;
};

int main() {
    const auto tmp = arg_router::policy::min_max_count_t<my_type, my_type>{};
    return 0;
}
    )",
        "MinType and MaxType must have a value_type that is implicitly "
        "convertible to std::size_t");
}

BOOST_AUTO_TEST_CASE(conversion_test)
{
    test::death_test_compile(
        R"(
#include <string>
#include "arg_router/policy/min_max_count.hpp"

struct my_type {
    using value_type = double;
};

int main() {
    const auto tmp = arg_router::policy::min_max_count_t<my_type, my_type>{};
    return 0;
}
    )",
        "MinType and MaxType value_types must be integrals");
}

BOOST_AUTO_TEST_CASE(min_count_positive_value_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/min_max_count.hpp"

using namespace arg_router;
int main() {
    const auto tmp = policy::min_max_count_t<traits::integral_constant<-5>,
                                             traits::integral_constant<5>>{};
    return 0;
}
    )",
        "MinType and MaxType must have a value that is a positive number");
}

BOOST_AUTO_TEST_CASE(max_count_positive_value_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/min_max_count.hpp"

using namespace arg_router;
int main() {
    const auto tmp = policy::min_max_count_t<traits::integral_constant<0>,
                                             traits::integral_constant<-5>>{};
    return 0;
}
    )",
        "MinType and MaxType must have a value that is a positive number");
}

BOOST_AUTO_TEST_CASE(valid_values_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/min_max_count.hpp"

using namespace arg_router;
int main() {
    const auto tmp = policy::min_max_count<5, 3>;
    return 0;
}
    )",
        "MinType must be less than or equal to MaxType");
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include <vector>

using namespace arg_router;
namespace
{
template <typename... Policies>
class stub_node : public tree_node<Policies...>
{
public:
    constexpr explicit stub_node(Policies... policies) :
        tree_node<Policies...>{std::move(policies)...}
    {
    }

    template <typename... Parents>
    void pre_parse_phase(parsing::token_list& tokens,
                         const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<1, typename stub_node::policies_type>;
        this->this_policy::pre_parse_phase(tokens, parents...);
    }
};
}  // namespace

int main() {
    const auto node = stub_node{policy::long_name<S_("test")>,
                                policy::fixed_count<1>};
    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "hello"}};
    node.pre_parse_phase(tokens);
    return 0;
}
    )",
        "Alias requires at least 1 parent");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
