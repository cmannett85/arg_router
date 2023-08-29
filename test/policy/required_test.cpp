// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/policy/required.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace arg_router::literals;

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
            [&](auto i) {
                using this_policy = std::tuple_element_t<i, typename stub_node::policies_type>;
                if constexpr (policy::has_missing_phase_method_v<this_policy, ValueType> &&
                              traits::is_specialisation_of_v<this_policy, policy::required_t>) {
                    result = this->this_policy::template missing_phase<ValueType>(parents...);
                }
            });

        return result;
    }
};
}  // namespace

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(required_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::required_t<>>, "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(missing_phase_test)
{
    const auto root =
        stub_node{stub_node{policy::long_name_t{"test"_S}, policy::required}, stub_node{}};

    auto f = [&](const auto& owner, auto ec) {
        try {
            owner.template missing_phase<int>(owner, root);
            BOOST_CHECK(!ec);
        } catch (multi_lang_exception& e) {
            BOOST_REQUIRE(ec);
            BOOST_CHECK_EQUAL(e.ec(), ec->ec());
            BOOST_CHECK_EQUAL(e.tokens(), ec->tokens());
        }
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{
                std::get<0>(root.children()),
                std::optional<multi_lang_exception>{
                    test::create_exception(error_code::missing_required_argument, {"--test"})}},
            std::tuple{std::get<1>(root.children()), std::optional<multi_lang_exception>{}}});
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(missing_phase_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include <vector>

using namespace arg_router;
using namespace arg_router::literals;

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
    void missing_phase(const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<1, typename stub_node::policies_type>;
        this->this_policy::template missing_phase<ValueType>(parents...);
    }
};
}  // namespace

int main() {
    const auto node = stub_node{policy::long_name_t{"test"_S},
                                policy::required};
    node.template missing_phase<int>();
    return 0;
}
    )",
        "Alias requires at least 1 parent");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
