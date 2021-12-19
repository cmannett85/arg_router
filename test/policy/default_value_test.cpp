#include "arg_router/policy/default_value.hpp"
#include "arg_router/tree_node.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

#include <string_view>

using namespace arg_router;
using namespace std::string_view_literals;

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
    std::optional<ValueType> missing_phase(const Parents&... parents) const
    {
        auto result = std::optional<ValueType>{};
        utility::tuple_type_iterator<typename stub_node::policies_type>(  //
            [&](auto /*i*/, auto ptr) {
                using this_policy = std::remove_pointer_t<decltype(ptr)>;
                if constexpr (policy::has_missing_phase_method_v<this_policy,
                                                                 ValueType,
                                                                 Parents...> &&
                              traits::is_specialisation_of_v<
                                  this_policy,
                                  policy::default_value>) {
                    result =
                        this->this_policy::template missing_phase<ValueType>(
                            parents...);
                }
            });

        return result;
    }
};
}  // namespace

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(default_value_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::default_value<int>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(constructor_and_get_test)
{
    auto f = [](auto input, auto is_ref) {
        using T = std::decay_t<decltype(input)>;

        const auto dv = policy::default_value<T>{input};
        BOOST_CHECK((std::is_same_v<typename decltype(dv)::value_type, T>));

        const auto result = dv.get_default_value();
        BOOST_CHECK(input == result);

        using result_type = decltype(std::declval<policy::default_value<T>>()
                                         .get_default_value());
        if (is_ref) {
            BOOST_CHECK((std::is_same_v<result_type, const T&>));
        } else {
            BOOST_CHECK((std::is_same_v<result_type, T>));
        }
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{42, false},
                       std::tuple{3.14, false},
                       std::tuple{"hello"sv, false},
                       // Should be bigger than any 'normal' L1 cache
                       std::tuple{std::array<char, 256>{}, true},
                   });
}

BOOST_AUTO_TEST_CASE(missing_phase_test)
{
    const auto root = stub_node{stub_node{policy::default_value{42}},
                                stub_node{policy::default_value{3.14}},
                                stub_node{}};

    auto f = [&](const auto& owner, auto expected_value) {
        using value_type =
            typename std::decay_t<decltype(expected_value)>::value_type;
        const auto result =
            owner.template missing_phase<value_type>(owner, root);
        BOOST_CHECK_EQUAL(result, expected_value);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::get<0>(root.children()), std::optional<int>{42}},
            std::tuple{std::get<1>(root.children()),
                       std::optional<double>{3.14}},
            std::tuple{std::get<2>(root.children()), std::optional<double>{}}});
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()