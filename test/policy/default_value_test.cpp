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
    bool post_parse_phase(std::optional<ValueType>& value,
                          const Parents&... parents) const
    {
        auto hit = false;
        utility::tuple_type_iterator<typename stub_node::policies_type>(  //
            [&](auto /*i*/, auto ptr) {
                using this_policy = std::remove_pointer_t<decltype(ptr)>;
                if constexpr (stub_node::
                                  template policy_has_post_parse_phase_method_v<
                                      this_policy,
                                      ValueType,
                                      Parents...> &&
                              traits::is_specialisation_of_v<
                                  this_policy,
                                  policy::default_value>) {
                    this->this_policy::post_parse_phase(value, parents...);
                    hit = true;
                }
            });

        return hit;
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

BOOST_AUTO_TEST_CASE(post_parse_phase_test)
{
    const auto root = stub_node{stub_node{policy::default_value{42}},
                                stub_node{policy::default_value{3.14}},
                                stub_node{}};

    auto f = [&](auto input_value,
                 const auto& owner,
                 auto expected_result,
                 auto expected_value) {
        const auto result = owner.post_parse_phase(input_value, owner, root);
        BOOST_CHECK_EQUAL(result, expected_result);
        BOOST_CHECK_EQUAL(input_value, expected_value);
    };

    test::data_set(f,
                   std::tuple{std::tuple{std::optional<int>{},
                                         std::get<0>(root.children()),
                                         true,
                                         std::optional<int>{42}},
                              std::tuple{std::optional<double>{},
                                         std::get<1>(root.children()),
                                         true,
                                         std::optional<double>{3.14}},
                              std::tuple{std::optional<double>{1.1},
                                         std::get<1>(root.children()),
                                         true,
                                         std::optional<double>{1.1}},
                              std::tuple{std::optional<double>{},
                                         std::get<2>(root.children()),
                                         false,
                                         std::optional<double>{}}});
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
