#include "arg_router/policy/max_count.hpp"
#include "arg_router/tree_node.hpp"

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
                         utility::span<const parsing::token_type>& view,
                         const Parents&... parents) const
    {
        auto hit = false;
        utility::tuple_type_iterator<typename stub_node::policies_type>(  //
            [&](auto /*i*/, auto ptr) {
                using this_policy = std::remove_pointer_t<decltype(ptr)>;
                if constexpr (policy::has_pre_parse_phase_method_v<
                                  this_policy,
                                  Parents...> &&
                              traits::is_specialisation_of_v<
                                  this_policy,
                                  policy::max_count_t>) {
                    this->this_policy::pre_parse_phase(tokens,
                                                       view,
                                                       parents...);
                    hit = true;
                }
            });

        return hit;
    }
};
}  // namespace

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(max_count_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(
        policy::is_policy_v<policy::max_count_t<traits::integral_constant<42>>>,
        "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(max_count_test)
{
    static_assert(policy::max_count<42u>.maximum_count() == 42, "Fail");
    static_assert(policy::max_count<5>.maximum_count() == 5, "Fail");
    static_assert(policy::max_count<0>.maximum_count() == 0, "Fail");
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_test)
{
    const auto root = stub_node{stub_node{policy::max_count<1>},
                                stub_node{policy::max_count<2>},
                                stub_node{}};

    auto f = [&](auto input_tokens,
                 const auto& owner,
                 auto expected_result,
                 auto expected_view) {
        auto tokens_backup = input_tokens;
        auto view = utility::span<const parsing::token_type>{input_tokens};
        const auto result =
            owner.pre_parse_phase(input_tokens, view, owner, root);
        BOOST_CHECK_EQUAL(result, expected_result);

        BOOST_REQUIRE_EQUAL(view.size(), expected_view.size());
        for (auto i = 0u; i < expected_view.size(); ++i) {
            BOOST_CHECK_EQUAL(view[i], expected_view[i]);
        }

        // Make sure input tokens is unchanged
        BOOST_CHECK_EQUAL(input_tokens, tokens_backup);
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
                parsing::token_list{{parsing::prefix_type::NONE, "42"}}},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::NONE, "42"},
                                    {parsing::prefix_type::NONE, "foo"},
                                    {parsing::prefix_type::LONG, "hello"}},
                std::get<1>(root.children()),
                true,
                parsing::token_list{{parsing::prefix_type::NONE, "42"},
                                    {parsing::prefix_type::NONE, "foo"}}},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::NONE, "42"},
                                    {parsing::prefix_type::NONE, "foo"},
                                    {parsing::prefix_type::LONG, "hello"}},
                std::get<2>(root.children()),
                false,
                parsing::token_list{{parsing::prefix_type::NONE, "42"},
                                    {parsing::prefix_type::NONE, "foo"},
                                    {parsing::prefix_type::LONG, "hello"}}}});
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(value_type_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/max_count.hpp"

struct my_type {};

int main() {
    const auto tmp = arg_router::policy::max_count_t<my_type>{};
    return 0;
}
    )",
        "T must have a value_type");
}

BOOST_AUTO_TEST_CASE(integral_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/max_count.hpp"

struct my_type {
    using value_type = std::string;
};

int main() {
    const auto tmp = arg_router::policy::max_count_t<my_type>{};
    return 0;
}
    )",
        "T must have a value_type that is implicitly convertible to "
        "std::size_t");
}

BOOST_AUTO_TEST_CASE(conversion_test)
{
    test::death_test_compile(
        R"(
#include <string>
#include "arg_router/policy/max_count.hpp"

struct my_type {
    using value_type = double;
};

int main() {
    const auto tmp = arg_router::policy::max_count_t<my_type>{};
    return 0;
}
    )",
        "T must be an integral type");
}

BOOST_AUTO_TEST_CASE(greater_than_or_equal_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/max_count.hpp"

using namespace arg_router;
int main() {
    const auto tmp = policy::max_count_t<traits::integral_constant<-5>>{};
    return 0;
}
    )",
        "T must have a value_type that is a positive number");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
