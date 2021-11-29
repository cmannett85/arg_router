#include "arg_router/policy/required.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"

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
                                  policy::required_t>) {
                    this->this_policy::post_parse_phase(value, parents...);
                    hit = true;
                }
            });

        return hit;
    }
};
}  // namespace

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(required_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::required_t<>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(post_parse_phase_test)
{
    const auto root =
        stub_node{stub_node{policy::long_name<S_("test")>, policy::required},
                  stub_node{}};

    auto f = [&](auto value,
                 const auto& owner,
                 auto expected_result,
                 std::string fail_message) {
        try {
            const auto result = owner.post_parse_phase(value, owner, root);
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK_EQUAL(result, expected_result);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    test::data_set(f,
                   std::tuple{std::tuple{std::optional<int>{42},  //
                                         std::get<0>(root.children()),
                                         true,
                                         ""},
                              std::tuple{std::optional<int>{},
                                         std::get<0>(root.children()),
                                         true,
                                         "Missing required argument: --test"},
                              std::tuple{std::optional<int>{},
                                         std::get<1>(root.children()),
                                         false,
                                         ""}});
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(post_parse_phase_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
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

    template <typename ValueType, typename... Parents>
    void post_parse_phase(std::optional<ValueType>& value,
                          const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<1, typename stub_node::policies_type>;
        this->this_policy::post_parse_phase(value, parents...);
    }
};
}  // namespace

int main() {
    const auto node = stub_node{policy::long_name<S_("test")>,
                                policy::required};
    auto value = std::optional<int>{42};
    node.post_parse_phase(value);
    return 0;
}
    )",
        "Alias requires at least 1 parent");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
