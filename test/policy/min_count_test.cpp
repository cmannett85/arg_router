#include "arg_router/policy/min_count.hpp"
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
    bool validation_phase(const ValueType& value,
                          const Parents&... parents) const
    {
        auto hit = false;
        utility::tuple_type_iterator<typename stub_node::policies_type>(  //
            [&](auto /*i*/, auto ptr) {
                using this_policy = std::remove_pointer_t<decltype(ptr)>;
                if constexpr (stub_node::
                                  template policy_has_validation_phase_method_v<
                                      this_policy,
                                      ValueType,
                                      Parents...> &&
                              traits::is_specialisation_of_v<
                                  this_policy,
                                  policy::min_count_t>) {
                    this->this_policy::validation_phase(value, parents...);
                    hit = true;
                }
            });

        return hit;
    }
};
}  // namespace

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(min_count_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(
        policy::is_policy_v<policy::min_count_t<traits::integral_constant<42>>>,
        "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(min_count_test)
{
    static_assert(policy::min_count<42u>.minimum_count() == 42, "Fail");
    static_assert(policy::min_count<5>.minimum_count() == 5, "Fail");
    static_assert(policy::min_count<0>.minimum_count() == 0, "Fail");
}

BOOST_AUTO_TEST_CASE(validation_phase_test)
{
    const auto root = stub_node{
        stub_node{policy::long_name<S_("test1")>, policy::min_count<1>},
        stub_node{policy::long_name<S_("test2")>, policy::min_count<3>},
        stub_node{}};

    auto f = [&](auto value,
                 const auto& owner,
                 auto expected_result,
                 std::string fail_message) {
        try {
            const auto result = owner.validation_phase(value, owner, root);
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK_EQUAL(result, expected_result);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    test::data_set(
        f,
        std::tuple{std::tuple{std::vector{42},  //
                              std::get<0>(root.children()),
                              true,
                              ""},
                   std::tuple{std::vector{42, 84, 96},
                              std::get<0>(root.children()),
                              true,
                              ""},
                   std::tuple{std::vector{42},
                              std::get<1>(root.children()),
                              true,
                              "Minimum count not reached: --test2"},
                   std::tuple{std::vector{42, 84, 96},
                              std::get<1>(root.children()),
                              true,
                              ""},
                   std::tuple{std::vector{42},
                              std::get<2>(root.children()),
                              false,
                              ""},
                   std::tuple{42, std::get<0>(root.children()), false, ""}});
}

BOOST_AUTO_TEST_SUITE(death_suite)
BOOST_AUTO_TEST_CASE(value_type_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/min_count.hpp"

struct my_type {};

int main() {
    const auto tmp = arg_router::policy::min_count_t<my_type>{};
    return 0;
}
    )",
        "T must have a value_type");
}

BOOST_AUTO_TEST_CASE(integral_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/min_count.hpp"

struct my_type {
    using value_type = std::string;
};

int main() {
    const auto tmp = arg_router::policy::min_count_t<my_type>{};
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
#include "arg_router/policy/min_count.hpp"

struct my_type {
    using value_type = double;
};

int main() {
    const auto tmp = arg_router::policy::min_count_t<my_type>{};
    return 0;
}
    )",
        "T must be an integral type");
}

BOOST_AUTO_TEST_CASE(greater_than_or_equal_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/min_count.hpp"

using namespace arg_router;
int main() {
    const auto tmp = policy::min_count_t<traits::integral_constant<-5>>{};
    return 0;
}
    )",
        "T must have a value_type that is a positive number");
}

BOOST_AUTO_TEST_CASE(validation_phase_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_count.hpp"
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
    void validation_phase(const ValueType& value,
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
                                policy::min_count<1>};
    node.validation_phase(std::vector{42});
    return 0;
}
    )",
        "Alias requires at least 1 parent");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
