// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
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

    template <typename InputValueType, typename... Parents>
    bool validation_phase(const InputValueType& value, const Parents&... parents) const
    {
        auto hit = false;
        utility::tuple_type_iterator<typename stub_node::policies_type>(  //
            [&](auto i) {
                using this_policy = std::tuple_element_t<i, typename stub_node::policies_type>;
                if constexpr (policy::has_validation_phase_method_v<this_policy, InputValueType> &&
                              traits::is_specialisation_of_v<this_policy,
                                                             policy::min_max_value_ct>) {
                    this->this_policy::validation_phase(value, parents...);
                    hit = true;
                }
            });

        return hit;
    }
};
}  // namespace

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(min_max_value_ct_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(
        policy::is_policy_v<
            policy::min_max_value_ct<traits::integral_constant<0>, traits::integral_constant<1>>>,
        "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(has_validation_phase_test)
{
    static_assert(
        policy::has_validation_phase_method_v<
            policy::min_max_value_ct<traits::integral_constant<0>, traits::integral_constant<1>>,
            int>,
        "Phase method test has failed");
}

BOOST_AUTO_TEST_CASE(has_maximum_value_method_test)
{
    static_assert(
        traits::has_maximum_value_method_v<
            policy::min_max_value_ct<traits::integral_constant<0>, traits::integral_constant<1>>>,
        "Maximum value method test has failed");
}

BOOST_AUTO_TEST_CASE(has_not_maximum_value_method_test)
{
    static_assert(!traits::has_maximum_value_method_v<
                      policy::min_max_value_ct<traits::integral_constant<0>, void>>,
                  "Maximum value method test has failed");
}

BOOST_AUTO_TEST_CASE(has_not_minimum_value_method_test)
{
    static_assert(!traits::has_minimum_value_method_v<
                      policy::min_max_value_ct<void, traits::integral_constant<1>>>,
                  "Minimum value method test has failed");
}

BOOST_AUTO_TEST_CASE(validation_phase_test)
{
    auto f = [](auto node, auto value, auto expected_hit, auto ec) {
        try {
            const auto hit = node.validation_phase(value, node);
            BOOST_CHECK(!ec);
            BOOST_CHECK_EQUAL(hit, expected_hit);
        } catch (multi_lang_exception& e) {
            BOOST_REQUIRE(ec);
            BOOST_CHECK_EQUAL(e.ec(), ec->ec());
            BOOST_CHECK_EQUAL(e.tokens(), ec->tokens());
        }
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{stub_node{policy::min_max_value<1, 4>(), policy::long_name_t{"node"_S}},
                       2,
                       true,
                       std::optional<multi_lang_exception>{}},
            std::tuple{stub_node{policy::min_max_value<1, 4>(), policy::long_name_t{"node"_S}},
                       1,
                       true,
                       std::optional<multi_lang_exception>{}},
            std::tuple{stub_node{policy::min_max_value<1, 4>(), policy::long_name_t{"node"_S}},
                       4,
                       true,
                       std::optional<multi_lang_exception>{}},
            std::tuple{
                stub_node{policy::min_max_value<1, 4>(), policy::long_name_t{"node"_S}},
                0,
                true,
                std::optional<multi_lang_exception>{
                    test::create_exception(error_code::minimum_value_not_reached, {"--node"})}},
            std::tuple{
                stub_node{policy::min_max_value<1, 4>(), policy::long_name_t{"node"_S}},
                -5,
                true,
                std::optional<multi_lang_exception>{
                    test::create_exception(error_code::minimum_value_not_reached, {"--node"})}},
            std::tuple{stub_node{policy::min_max_value<1, 4>(), policy::long_name_t{"node"_S}},
                       6,
                       true,
                       std::optional<multi_lang_exception>{
                           test::create_exception(error_code::maximum_value_exceeded, {"--node"})}},

            std::tuple{stub_node{policy::min_value<2>(), policy::long_name_t{"node"_S}},  //
                       2,
                       true,
                       std::optional<multi_lang_exception>{}},
            std::tuple{stub_node{policy::min_value<2>(), policy::long_name_t{"node"_S}},  //
                       20,
                       true,
                       std::optional<multi_lang_exception>{}},
            std::tuple{
                stub_node{policy::min_value<2>(), policy::long_name_t{"node"_S}},
                1,
                true,
                std::optional<multi_lang_exception>{
                    test::create_exception(error_code::minimum_value_not_reached, {"--node"})}},

            std::tuple{stub_node{policy::max_value<2>(), policy::long_name_t{"node"_S}},  //
                       2,
                       true,
                       std::optional<multi_lang_exception>{}},
            std::tuple{stub_node{policy::max_value<2>(), policy::long_name_t{"node"_S}},  //
                       1,
                       true,
                       std::optional<multi_lang_exception>{}},
            std::tuple{stub_node{policy::max_value<2>(), policy::long_name_t{"node"_S}},
                       20,
                       true,
                       std::optional<multi_lang_exception>{
                           test::create_exception(error_code::maximum_value_exceeded, {"--node"})}},

            std::tuple{stub_node{policy::long_name_t{"node"_S}},
                       0,
                       false,
                       std::optional<multi_lang_exception>{}},
        });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(at_least_one_parent_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
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
    const auto node = stub_node{policy::long_name_t{"test"_S},
                                policy::min_max_value<1, 4>()};
    node.validation_phase(2);
    return 0;
}
    )",
        "Min/max value requires at least 1 parent");
}

BOOST_AUTO_TEST_CASE(mintype_and_maxtype_cannot_both_be_void_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
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
};

struct bad {
    using value_type = double;
};
}  // namespace

int main() {
    const auto node = stub_node{policy::long_name_t{"test"_S},
                                policy::min_max_value_ct<void, void>()};
    return 0;
}
    )",
        "MinType and MaxType cannot both be void");
}

BOOST_AUTO_TEST_CASE(mintype_must_have_value_type_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
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
};
}  // namespace

int main() {
    const auto node = stub_node{policy::long_name_t{"test"_S},
                                policy::min_max_value_ct<int, void>()};
    return 0;
}
    )",
        "MinType must have a value_type");
}

BOOST_AUTO_TEST_CASE(maxtype_must_have_value_type_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
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
};
}  // namespace

int main() {
    const auto node = stub_node{policy::long_name_t{"test"_S},
                                policy::min_max_value_ct<void, int>()};
    return 0;
}
    )",
        "MaxType must have a value_type");
}

BOOST_AUTO_TEST_CASE(mintype_must_be_integrals_or_enums_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
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
};

struct bad {
    using value_type = double;
};
}  // namespace

int main() {
    const auto node = stub_node{policy::long_name_t{"test"_S},
                                policy::min_max_value_ct<bad, void>()};
    return 0;
}
    )",
        "MinType value_type must be integrals or enums");
}

BOOST_AUTO_TEST_CASE(maxtype_must_be_integrals_or_enums_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
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
};

struct bad {
    using value_type = double;
};
}  // namespace

int main() {
    const auto node = stub_node{policy::long_name_t{"test"_S},
                                policy::min_max_value_ct<void, bad>()};
    return 0;
}
    )",
        "MaxType value_type must be integrals or enums");
}

BOOST_AUTO_TEST_CASE(min_must_be_less_than_max_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
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
};
}  // namespace

int main() {
    const auto node = stub_node{policy::long_name_t{"test"_S},
                                policy::min_max_value<4, 1>()};
    return 0;
}
    )",
        "MinType must be less than or equal to MaxType");
}

BOOST_AUTO_TEST_CASE(mintype_and_maxtype_must_have_same_value_type_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
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
};
}  // namespace

int main() {
    const auto node = stub_node{policy::long_name_t{"test"_S},
                                policy::min_max_value<0u, 3>()};
    return 0;
}
    )",
        "MinType and MaxType must have the same value_type");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
