#include "arg_router/policy/min_max_value.hpp"
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

    template <typename InputValueType, typename... Parents>
    bool validation_phase(const InputValueType& value,
                          const Parents&... parents) const
    {
        auto hit = false;
        utility::tuple_type_iterator<typename stub_node::policies_type>(  //
            [&](auto i) {
                using this_policy =
                    std::tuple_element_t<i, typename stub_node::policies_type>;
                if constexpr (policy::has_validation_phase_method_v<
                                  this_policy,
                                  InputValueType,
                                  Parents...> &&
                              traits::is_specialisation_of_v<
                                  this_policy,
                                  policy::min_max_value>) {
                    this->this_policy::validation_phase(value, parents...);
                    hit = true;
                }
            });

        return hit;
    }
};
}  // namespace

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(min_max_value_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::min_max_value<int>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(constructor_test)
{
    auto f = [](auto min, auto max) {
        const auto p = policy::min_max_value{min, max};
        BOOST_CHECK_EQUAL(min, p.minimum_value());
        BOOST_CHECK_EQUAL(max, p.maximum_value());
    };

    test::data_set(
        f,
        std::tuple{std::tuple{0, 3},
                   std::tuple{1, 3},
                   std::tuple{2.1, 10.4},
                   std::tuple{std::vector{1, 2, 3}, std::vector{4, 5, 6}},
                   std::tuple{std::vector{1, 2}, std::vector{4, 5, 6}}});
}

BOOST_AUTO_TEST_CASE(validation_phase_test)
{
    auto f =
        [](auto node, auto value, auto expected_hit, std::string fail_message) {
            try {
                const auto hit = node.validation_phase(value, node);
                BOOST_CHECK(fail_message.empty());
                BOOST_CHECK_EQUAL(hit, expected_hit);
            } catch (parse_exception& e) {
                BOOST_CHECK_EQUAL(e.what(), fail_message);
            }
        };

    test::data_set(
        f,
        std::tuple{
            std::tuple{stub_node{policy::min_max_value{1, 4},
                                 policy::long_name<S_("node")>},
                       2,
                       true,
                       ""},
            std::tuple{stub_node{policy::min_max_value{1, 4},
                                 policy::long_name<S_("node")>},
                       1,
                       true,
                       ""},
            std::tuple{stub_node{policy::min_max_value{1, 4},
                                 policy::long_name<S_("node")>},
                       4,
                       true,
                       ""},
            std::tuple{stub_node{policy::min_max_value{1, 4},
                                 policy::long_name<S_("node")>},
                       0,
                       true,
                       "Minimum value not reached: --node"},
            std::tuple{stub_node{policy::min_max_value{1, 4},
                                 policy::long_name<S_("node")>},
                       -5,
                       true,
                       "Minimum value not reached: --node"},
            std::tuple{stub_node{policy::min_max_value{1, 4},
                                 policy::long_name<S_("node")>},
                       6,
                       true,
                       "Maximum value exceeded: --node"},

            std::tuple{stub_node{policy::min_max_value{std::vector{5, 6},
                                                       std::vector{1, 3, 4, 2},
                                                       [](auto&& a, auto&& b) {
                                                           return a.size() <
                                                                  b.size();
                                                       }},
                                 policy::long_name<S_("node")>},
                       std::vector{3, 4, 5},
                       true,
                       ""},
            std::tuple{stub_node{policy::min_max_value{std::vector{5, 6},
                                                       std::vector{1, 3, 4, 2},
                                                       [](auto&& a, auto&& b) {
                                                           return a.size() <
                                                                  b.size();
                                                       }},
                                 policy::long_name<S_("node")>},
                       std::vector<int>{},
                       true,
                       "Minimum value not reached: --node"},
            std::tuple{stub_node{policy::min_max_value{std::vector{5, 6},
                                                       std::vector{1, 3, 4, 2},
                                                       [](auto&& a, auto&& b) {
                                                           return a.size() <
                                                                  b.size();
                                                       }},
                                 policy::long_name<S_("node")>},
                       std::vector{5},
                       true,
                       "Minimum value not reached: --node"},
            std::tuple{stub_node{policy::min_max_value{std::vector{5, 6},
                                                       std::vector{1, 3, 4, 2},
                                                       [](auto&& a, auto&& b) {
                                                           return a.size() <
                                                                  b.size();
                                                       }},
                                 policy::long_name<S_("node")>},
                       std::vector{1, 2, 3, 4, 5},
                       true,
                       "Maximum value exceeded: --node"},

            std::tuple{stub_node{policy::long_name<S_("node")>}, 0, false, ""},
        });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(validation_phase_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
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

    template <typename InputValueType, typename... Parents>
    void validation_phase(const InputValueType& value,
                          const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<1, typename stub_node::policies_type>;
        this->this_policy::validation_phase(value, parents...);
    }
};
}  // namespace

int main() {
    const auto node = stub_node{policy::long_name<S_("test")>,
                                policy::min_max_value{1, 4}};
    node.validation_phase(2);
    return 0;
}
    )",
        "Min/max value requires at least 1 parent");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
