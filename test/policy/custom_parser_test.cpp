#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/global_parser.hpp"
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

    template <typename ValueType, typename... Parents>
    std::optional<ValueType> parse_phase(std::string_view token,
                                         const Parents&... parents) const
    {
        auto result = std::optional<ValueType>{};
        utility::tuple_type_iterator<typename stub_node::policies_type>(  //
            [&](auto i) {
                using this_policy =
                    std::tuple_element_t<i, typename stub_node::policies_type>;
                if constexpr (policy::has_parse_phase_method_v<this_policy,
                                                               ValueType,
                                                               Parents...> &&
                              traits::is_specialisation_of_v<
                                  this_policy,
                                  policy::custom_parser>) {
                    BOOST_CHECK(!result);
                    result = this->this_policy::template parse_phase<ValueType>(
                        token,
                        parents...);
                }
            });

        return result;
    }
};
}  // namespace

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(custom_parser_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::custom_parser<int>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(parse_phase_test)
{
    const auto root = stub_node{
        stub_node{policy::custom_parser<int>{
            [](auto str) { return parser<int>::parse(str); }}},
        stub_node{policy::custom_parser<std::string_view>{
            [](auto str) { return parser<std::string_view>::parse(str); }}},
        stub_node{}};

    auto f = [&](auto token, const auto& owner, auto expected_value) {
        using value_type =
            typename std::decay_t<decltype(expected_value)>::value_type;
        const auto result =
            owner.template parse_phase<value_type>(token, owner, root);
        BOOST_CHECK_EQUAL(result, expected_value);
    };

    test::data_set(f,
                   std::tuple{std::tuple{"42",
                                         std::get<0>(root.children()),
                                         std::optional<int>{42}},
                              std::tuple{"42",
                                         std::get<1>(root.children()),
                                         std::optional<std::string_view>{"42"}},
                              std::tuple{"42",
                                         std::get<2>(root.children()),
                                         std::optional<std::string_view>{}}});
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
