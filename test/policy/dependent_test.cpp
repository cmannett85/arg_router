/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/policy/dependent.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
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
    using value_type = bool;

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
                                  this_policy> &&
                              traits::is_specialisation_of_v<
                                  this_policy,
                                  policy::dependent_t>) {
                    this->this_policy::pre_parse_phase(tokens, parents...);
                    hit = true;
                }
            });

        return hit;
    }
};

template <std::size_t I, std::size_t... Is, typename T>
const auto& get_node(const T& parent)
{
    const auto& child = std::get<I>(parent.children());

    if constexpr (sizeof...(Is) > 0) {
        return get_node<Is...>(child);
    } else {
        return child;
    }
}

template <std::size_t I, std::size_t... Is, typename T>
auto get_parents(const T& parent)
{
    auto result = std::tuple{std::ref(get_node<I, Is...>(parent))};

    if constexpr (sizeof...(Is) > 0) {
        // All this because you can't resize a tuple in std...
        using index_tuple = boost::mp11::mp_pop_back<
            std::tuple<traits::integral_constant<I>,
                       traits::integral_constant<Is>...>>;
        return std::apply(
            [&](auto... NewIs) {
                return std::tuple_cat(result, get_parents<NewIs...>(parent));
            },
            index_tuple{});
    } else {
        return result;
    }
}
}  // namespace

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(dependent_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::dependent_t<>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_test)
{
    const auto root = stub_node{
        policy::long_name<S_("test_root")>,
        stub_node{policy::long_name<S_("test1")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::dependent(policy::long_name<S_("flag2")>)},
                  stub_node{policy::long_name<S_("flag2")>},
                  stub_node{policy::long_name<S_("flag3")>},
                  policy::router{[](bool, bool, bool) {}}},
        stub_node{policy::long_name<S_("test2")>,
                  stub_node{policy::long_name<S_("one_of")>,
                            stub_node{policy::long_name<S_("flag1")>,
                                      policy::dependent(
                                          policy::long_name<S_("flag2")>)},
                            stub_node{policy::long_name<S_("flag3")>}},
                  stub_node{policy::long_name<S_("flag2")>},
                  policy::router{[](bool, bool, bool) {}}},
        stub_node{policy::long_name<S_("test3")>,
                  stub_node{policy::long_name<S_("one_of")>,
                            stub_node{policy::long_name<S_("flag1")>,
                                      policy::dependent(
                                          policy::long_name<S_("flag2")>)},
                            stub_node{policy::long_name<S_("flag2")>}},
                  stub_node{policy::long_name<S_("flag3")>},
                  policy::router{[](bool, bool, bool) {}}},
        stub_node{policy::long_name<S_("test4")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::dependent(policy::long_name<S_("flag2")>),
                            policy::dependent(policy::long_name<S_("flag3")>)},
                  stub_node{policy::long_name<S_("flag2")>},
                  stub_node{policy::long_name<S_("flag3")>},
                  policy::router{[](bool, bool, bool) {}}},
        stub_node{policy::long_name<S_("test5")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::dependent(policy::long_name<S_("flag2")>)},
                  stub_node{policy::long_name<S_("flag2")>,
                            policy::dependent(policy::long_name<S_("flag3")>)},
                  stub_node{policy::long_name<S_("flag3")>},
                  policy::router{[](bool, bool, bool) {}}}};

    auto f = [&](auto tokens,
                 const auto parents_tuple,
                 const auto processed_count,
                 const auto expected_result,
                 const std::string fail_message) {
        tokens.mark_as_processed(processed_count);
        auto tokens_bkup = tokens;

        try {
            const auto result = std::apply(
                [&](auto&&... parents) {
                    return std::get<0>(parents_tuple)
                        .get()
                        .pre_parse_phase(tokens, (parents.get())..., root);
                },
                parents_tuple);

            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK_EQUAL(result, expected_result);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }

        BOOST_CHECK_EQUAL(tokens, tokens_bkup);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{
                parsing::token_list{{parsing::prefix_type::LONG, "flag2"},
                                    {parsing::prefix_type::LONG, "flag3"}},
                get_parents<0, 0>(root),
                0,
                true,
                ""},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::LONG, "flag2"},
                                    {parsing::prefix_type::LONG, "flag3"}},
                get_parents<0, 1>(root),
                0,
                false,
                ""},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::LONG, "flag3"},
                                    {parsing::prefix_type::LONG, "flag2"}},
                get_parents<0, 0>(root),
                0,
                true,
                ""},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::LONG, "flag3"}},
                get_parents<0, 0>(root),
                0,
                true,
                "Dependent argument missing: --flag2"},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::LONG, "flag2"},
                                    {parsing::prefix_type::LONG, "flag3"}},
                get_parents<0, 0>(root),
                1,
                true,
                ""},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::LONG, "flag2"},
                                    {parsing::prefix_type::LONG, "flag3"}},
                get_parents<1, 0, 0>(root),
                0,
                true,
                ""},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::LONG, "flag2"},
                                    {parsing::prefix_type::LONG, "flag3"}},
                get_parents<2, 0, 0>(root),
                0,
                true,
                ""},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::LONG, "flag2"},
                                    {parsing::prefix_type::LONG, "flag3"}},
                get_parents<3, 0>(root),
                0,
                true,
                ""},
            std::tuple{
                parsing::token_list{{parsing::prefix_type::LONG, "flag2"},
                                    {parsing::prefix_type::LONG, "flag3"}},
                get_parents<4, 0>(root),
                0,
                true,
                ""},
        });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(zero_depends_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/dependent.hpp"

using namespace arg_router;

int main() {
    auto a = policy::dependent();
    return 0;
}
    )",
        "At least one name needed for dependent");
}

BOOST_AUTO_TEST_CASE(all_params_must_be_policies_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/policy/dependent.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto a = policy::dependent(flag(policy::long_name<S_("flag1")>));
    return 0;
}
    )",
        "All parameters must be policies");
}

BOOST_AUTO_TEST_CASE(all_params_must_be_names_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/dependent.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto a = policy::dependent(policy::display_name<S_("hello")>);
    return 0;
}
    )",
        "All parameters must provide a long and/or short form name");
}

BOOST_AUTO_TEST_CASE(cannot_find_parent_node_empty_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/dependent.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

namespace
{
template <typename... Policies>
class stub_node : public tree_node<Policies...>
{
public:
    using value_type = bool;

    constexpr explicit stub_node(Policies... policies) :
        tree_node<Policies...>{std::move(policies)...}
    {
    }

    template <typename... Parents>
    void pre_parse_phase(parsing::token_list& tokens,
                         const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<0, typename stub_node::policies_type>;
        this->this_policy::pre_parse_phase(tokens, *this, parents...);
    }
};
}  // namespace

int main() {
    const auto root = stub_node{
        policy::dependent(policy::long_name<S_("flag2")>)};

    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "flag2"},
                                      {parsing::prefix_type::LONG, "flag3"}};
    root.pre_parse_phase(tokens);
    return 0;
}
    )",
        "Cannot find parent mode");
}

BOOST_AUTO_TEST_CASE(cannot_find_parent_node_missing_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/dependent.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

namespace
{
template <typename... Policies>
class stub_node : public tree_node<Policies...>
{
public:
    using value_type = bool;

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
        this->this_policy::pre_parse_phase(tokens, *this, parents...);
    }
};
}  // namespace

int main() {
    const auto root =
        stub_node{policy::long_name<S_("mode")>,
              stub_node{policy::long_name<S_("flag1")>,
                        policy::dependent(policy::long_name<S_("flag2")>)},
              stub_node{policy::long_name<S_("flag2")>},
              stub_node{policy::long_name<S_("flag3")>}};

    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "flag2"},
                                      {parsing::prefix_type::LONG, "flag3"}};
    const auto& owner = std::get<0>(root.children());

    owner.pre_parse_phase(tokens, root);
    return 0;
}
    )",
        "Cannot find parent mode");
}

BOOST_AUTO_TEST_CASE(cyclic_dependency_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/dependent.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

namespace
{
template <typename... Policies>
class stub_node : public tree_node<Policies...>
{
public:
    using value_type = bool;

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
        this->this_policy::pre_parse_phase(tokens, *this, parents...);
    }
};
}  // namespace

int main() {
    const auto root =
        stub_node{policy::long_name<S_("mode")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::dependent(policy::long_name<S_("flag2")>)},
                  stub_node{policy::long_name<S_("flag2")>,
                            policy::dependent(policy::long_name<S_("flag3")>)},
                  stub_node{policy::long_name<S_("flag3")>,
                            policy::dependent(policy::long_name<S_("flag1")>)},
                  policy::router{[](bool, bool, bool) {}}};

    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "flag2"},
                                      {parsing::prefix_type::LONG, "flag3"}};
    const auto& owner = std::get<0>(root.children());

    owner.pre_parse_phase(tokens, root);
    return 0;
}
    )",
        "Cyclic dependency detected");
}

BOOST_AUTO_TEST_CASE(missing_target_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/dependent.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

namespace
{
template <typename... Policies>
class stub_node : public tree_node<Policies...>
{
public:
    using value_type = bool;

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
        this->this_policy::pre_parse_phase(tokens, *this, parents...);
    }
};
}  // namespace

int main() {
    const auto root =
        stub_node{policy::long_name<S_("mode")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::dependent(policy::long_name<S_("flag4")>)},
                  stub_node{policy::long_name<S_("flag2")>},
                  stub_node{policy::long_name<S_("flag3")>},
                  policy::router{[](bool, bool, bool) {}}};

    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "flag2"},
                                      {parsing::prefix_type::LONG, "flag3"}};
    const auto& owner = std::get<0>(root.children());

    owner.pre_parse_phase(tokens, root);
    return 0;
}
    )",
        "Number of found modes must match depends policy count");
}

BOOST_AUTO_TEST_CASE(duplicate_targets_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/dependent.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

namespace
{
template <typename... Policies>
class stub_node : public tree_node<Policies...>
{
public:
    using value_type = bool;

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
        this->this_policy::pre_parse_phase(tokens, *this, parents...);
    }
};
}  // namespace

int main() {
    const auto root =
        stub_node{policy::long_name<S_("mode")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::dependent(policy::long_name<S_("flag2")>,
                                              policy::long_name<S_("flag2")>)},
                  stub_node{policy::long_name<S_("flag2")>},
                  stub_node{policy::long_name<S_("flag3")>},
                  policy::router{[](bool, bool, bool) {}}};

    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "flag2"},
                                      {parsing::prefix_type::LONG, "flag3"}};
    const auto& owner = std::get<0>(root.children());

    owner.pre_parse_phase(tokens, root);
    return 0;
}
    )",
        "Number of found modes must match depends policy count");
}

BOOST_AUTO_TEST_CASE(duplicate_target_different_name_types_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/dependent.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

namespace
{
template <typename... Policies>
class stub_node : public tree_node<Policies...>
{
public:
    using value_type = bool;

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
        this->this_policy::pre_parse_phase(tokens, *this, parents...);
    }
};
}  // namespace

int main() {
    const auto root =
        stub_node{policy::long_name<S_("mode")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::dependent(policy::long_name<S_("flag2")>,
                                              policy::short_name<'a'>)},
                  stub_node{policy::long_name<S_("flag2")>,
                            policy::short_name<'a'>},
                  stub_node{policy::long_name<S_("flag3")>},
                  policy::router{[](bool, bool, bool) {}}};

    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "flag2"},
                                      {parsing::prefix_type::LONG, "flag3"}};
    const auto& owner = std::get<0>(root.children());

    owner.pre_parse_phase(tokens, root);
    return 0;
}
    )",
        "Node dependency list must be unique, do you have short and long names "
        "from the same node?");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
