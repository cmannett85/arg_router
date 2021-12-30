#include "arg_router/policy/alias.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace std::string_literals;

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
            [&](auto i) {
                using this_policy =
                    std::tuple_element_t<i, typename stub_node::policies_type>;
                if constexpr (policy::has_pre_parse_phase_method_v<
                                  this_policy,
                                  Parents...> &&
                              traits::is_specialisation_of_v<this_policy,
                                                             policy::alias_t>) {
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

BOOST_AUTO_TEST_SUITE(alias_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::alias_t<>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_test)
{
    const auto root = stub_node{
        policy::long_name<S_("test_root")>,
        stub_node{policy::long_name<S_("test1")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::count<0>,
                            policy::alias(policy::long_name<S_("flag2")>)},
                  stub_node{policy::long_name<S_("flag2")>},
                  stub_node{policy::long_name<S_("flag3")>}},
        stub_node{policy::long_name<S_("test2")>,
                  stub_node{policy::long_name<S_("arg1")>,
                            policy::count<1>,
                            policy::alias(policy::long_name<S_("arg3")>)},
                  stub_node{policy::long_name<S_("arg2")>},
                  stub_node{policy::long_name<S_("arg3")>}},
        stub_node{policy::long_name<S_("test3")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::count<0>,
                            policy::alias(policy::long_name<S_("flag2")>,
                                          policy::long_name<S_("flag3")>)},
                  stub_node{policy::long_name<S_("flag2")>},
                  stub_node{policy::long_name<S_("flag3")>}},
        stub_node{policy::long_name<S_("test4")>,
                  stub_node{policy::long_name<S_("arg1")>,
                            policy::count<3>,
                            policy::alias(policy::long_name<S_("arg2")>,
                                          policy::long_name<S_("arg3")>)},
                  stub_node{policy::long_name<S_("arg2")>},
                  stub_node{policy::long_name<S_("arg3")>}}};

    auto f = [&](auto input_tokens,
                 const auto& owner,
                 const auto& test,
                 auto expected_result,
                 auto expected_tokens) {
        auto view = input_tokens.pending_view();
        const auto result =
            owner.pre_parse_phase(input_tokens, view, owner, test, root);
        BOOST_CHECK_EQUAL(result, expected_result);
        BOOST_CHECK_EQUAL(input_tokens, expected_tokens);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{
                parsing::token_list{},
                std::get<0>(std::get<0>(root.children()).children()),
                std::get<0>(root.children()),
                true,
                parsing::token_list{{parsing::prefix_type::LONG, "flag2"}}},
            std::tuple{parsing::token_list{},
                       std::get<1>(std::get<0>(root.children()).children()),
                       std::get<0>(root.children()),
                       false,
                       parsing::token_list{}},
            std::tuple{parsing::token_list{{parsing::prefix_type::NONE, "42"}},
                       std::get<0>(std::get<1>(root.children()).children()),
                       std::get<1>(root.children()),
                       true,
                       parsing::token_list{{parsing::prefix_type::NONE, "42"},
                                           {parsing::prefix_type::LONG, "arg3"},
                                           {parsing::prefix_type::NONE, "42"}}},
            std::tuple{
                parsing::token_list{},
                std::get<0>(std::get<2>(root.children()).children()),
                std::get<2>(root.children()),
                true,
                parsing::token_list{{parsing::prefix_type::LONG, "flag2"},
                                    {parsing::prefix_type::LONG, "flag3"}}},
            std::tuple{parsing::token_list{{parsing::prefix_type::NONE, "1"},
                                           {parsing::prefix_type::NONE, "2"},
                                           {parsing::prefix_type::NONE, "3"}},
                       std::get<0>(std::get<3>(root.children()).children()),
                       std::get<3>(root.children()),
                       true,
                       parsing::token_list{{parsing::prefix_type::NONE, "1"},
                                           {parsing::prefix_type::NONE, "2"},
                                           {parsing::prefix_type::NONE, "3"},
                                           {parsing::prefix_type::LONG, "arg2"},
                                           {parsing::prefix_type::NONE, "1"},
                                           {parsing::prefix_type::NONE, "2"},
                                           {parsing::prefix_type::NONE, "3"},
                                           {parsing::prefix_type::LONG, "arg3"},
                                           {parsing::prefix_type::NONE, "1"},
                                           {parsing::prefix_type::NONE, "2"},
                                           {parsing::prefix_type::NONE, "3"}}},
        });
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_too_small_view_test)
{
    const auto root =
        stub_node{policy::long_name<S_("root")>,
                  stub_node{policy::long_name<S_("arg1")>,
                            policy::count<2>,
                            policy::alias(policy::long_name<S_("arg2")>)},
                  stub_node{policy::long_name<S_("arg2")>},
                  stub_node{policy::long_name<S_("arg3")>}};

    auto tokens = parsing::token_list{{parsing::prefix_type::NONE, "42"}};
    auto view = tokens.pending_view();
    const auto& owner = std::get<0>(root.children());

    BOOST_CHECK_EXCEPTION(  //
        owner.pre_parse_phase(tokens, view, owner, root),
        parse_exception,
        [](const auto& e) {
            return e.what() == "Too few values for alias, needs 2: --arg1"s;
        });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(zero_aliases_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"

using namespace arg_router;

int main() {
    auto a = policy::alias();
    return 0;
}
    )",
        "At least one name needed for alias");
}

BOOST_AUTO_TEST_CASE(all_params_must_be_policies_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto a = policy::alias(flag(policy::long_name<S_("flag1")>));
    return 0;
}
    )",
        "All parameters must be policies");
}

BOOST_AUTO_TEST_CASE(all_params_must_be_names_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto a = policy::alias(policy::display_name<S_("hello")>);
    return 0;
}
    )",
        "All parameters must provide a long and/or short form name");
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_0_parent_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/count.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(parsing::token_list& tokens,
                         utility::span<const parsing::token_type>& view,
                         const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<2, typename stub_node::policies_type>;
        this->this_policy::pre_parse_phase(tokens, view, parents...);
    }
};
}  // namespace

int main() {
    auto node = stub_node{policy::long_name<S_("node")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::count<0>,
                            policy::alias(policy::long_name<S_("flag2")>)},
                  stub_node{policy::long_name<S_("flag2")>},
                  stub_node{policy::long_name<S_("flag3")>}};
    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "flag1"}};
    auto view = tokens.pending_view();
    const auto& owner = std::get<0>(node.children());
    
    owner.pre_parse_phase(tokens, view);
    return 0;
}
    )",
        "Alias requires at least 2 parents");
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_needs_counts_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/count.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(parsing::token_list& tokens,
                         utility::span<const parsing::token_type>& view,
                         const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<1, typename stub_node::policies_type>;
        this->this_policy::pre_parse_phase(tokens, view, parents...);
    }
};
}  // namespace

int main() {
    auto node = stub_node{policy::long_name<S_("node")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::alias(policy::long_name<S_("flag2")>)},
                  stub_node{policy::long_name<S_("flag2")>},
                  stub_node{policy::long_name<S_("flag3")>}};
    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "flag1"}};
    auto view = tokens.pending_view();
    const auto& owner = std::get<0>(node.children());
    
    owner.pre_parse_phase(tokens, view, owner, node);
    return 0;
}
    )",
        "Node requires a count(), or minimum_count() and maximum_count() "
        "methods to use an alias");
}

BOOST_AUTO_TEST_CASE(pre_parse_phase_needs_fixed_count_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/count.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(parsing::token_list& tokens,
                         utility::span<const parsing::token_type>& view,
                         const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<3, typename stub_node::policies_type>;
        this->this_policy::pre_parse_phase(tokens, view, parents...);
    }
};
}  // namespace

int main() {
    auto node = stub_node{policy::long_name<S_("node")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::min_count<2>,
                            policy::max_count<3>,
                            policy::alias(policy::long_name<S_("flag2")>)},
                  stub_node{policy::long_name<S_("flag2")>},
                  stub_node{policy::long_name<S_("flag3")>}};
    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "flag1"}};
    auto view = tokens.pending_view();
    const auto& owner = std::get<0>(node.children());
    
    owner.pre_parse_phase(tokens, view, owner, node);
    return 0;
}
    )",
        "Node requires minimum_count() and maximum_count() to return the same "
        "value to use an alias");
}

BOOST_AUTO_TEST_CASE(single_aliased_must_not_be_in_owner_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/count.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(parsing::token_list& tokens,
                         utility::span<const parsing::token_type>& view,
                         const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<2, typename stub_node::policies_type>;
        this->this_policy::pre_parse_phase(tokens, view, parents...);
    }
};
}  // namespace

int main() {
    auto node = stub_node{policy::long_name<S_("node")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::count<1>,
                            policy::alias(policy::long_name<S_("flag1")>)}};
    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "flag1"}};
    auto view = tokens.pending_view();
    const auto& owner = std::get<0>(node.children());
    
    owner.pre_parse_phase(tokens, view, owner, node);
    return 0;
}
    )",
        "Alias names cannot appear in owner");
}

BOOST_AUTO_TEST_CASE(multiple_aliased_must_not_be_in_owner_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/count.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(parsing::token_list& tokens,
                         utility::span<const parsing::token_type>& view,
                         const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<2, typename stub_node::policies_type>;
        this->this_policy::pre_parse_phase(tokens, view, parents...);
    }
};
}  // namespace

int main() {
    auto node = stub_node{policy::long_name<S_("node")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::count<1>,
                            policy::alias(policy::long_name<S_("flag2")>,
                                          policy::long_name<S_("flag1")>)}};
    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "flag1"}};
    auto view = tokens.pending_view();
    const auto& owner = std::get<0>(node.children());
    
    owner.pre_parse_phase(tokens, view, owner, node);
    return 0;
}
    )",
        "Alias names cannot appear in owner");
}

BOOST_AUTO_TEST_CASE(one_matching_node_per_alias_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/count.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(parsing::token_list& tokens,
                         utility::span<const parsing::token_type>& view,
                         const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<2, typename stub_node::policies_type>;
        this->this_policy::pre_parse_phase(tokens, view, parents...);
    }
};
}  // namespace

int main() {
    auto node = stub_node{policy::long_name<S_("node")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::count<0>,
                            policy::alias(policy::long_name<S_("flag4")>)},
                  stub_node{policy::long_name<S_("flag2")>},
                  stub_node{policy::long_name<S_("flag3")>}};
    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "flag1"}};
    auto view = tokens.pending_view();
    const auto& owner = std::get<0>(node.children());
    
    owner.pre_parse_phase(tokens, view, owner, node);
    return 0;
}
    )",
        "There must one matching node per alias entry");
}

BOOST_AUTO_TEST_CASE(multiple_alias_matching_single_node_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/count.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(parsing::token_list& tokens,
                         utility::span<const parsing::token_type>& view,
                         const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<2, typename stub_node::policies_type>;
        this->this_policy::pre_parse_phase(tokens, view, parents...);
    }
};
}  // namespace

int main() {
    auto node = stub_node{policy::long_name<S_("node")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::count<0>,
                            policy::alias(policy::long_name<S_("flag3")>,
                                          policy::short_name<'f'>)},
                  stub_node{policy::long_name<S_("flag2")>},
                  stub_node{policy::long_name<S_("flag3")>,
                            policy::short_name<'f'>}};
    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "flag1"}};
    auto view = tokens.pending_view();
    const auto& owner = std::get<0>(node.children());
    
    owner.pre_parse_phase(tokens, view, owner, node);
    return 0;
}
    )",
        "There must one matching node per alias entry");
}

BOOST_AUTO_TEST_CASE(cyclic_dependency_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/count.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

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
    void pre_parse_phase(parsing::token_list& tokens,
                         utility::span<const parsing::token_type>& view,
                         const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<2, typename stub_node::policies_type>;
        this->this_policy::pre_parse_phase(tokens, view, *this, parents...);
    }
};
}  // namespace

int main() {
    auto node = stub_node{policy::long_name<S_("node")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::count<0>,
                            policy::alias(policy::long_name<S_("flag3")>)},
                  stub_node{policy::long_name<S_("flag2")>},
                  stub_node{policy::long_name<S_("flag3")>,
                            policy::count<0>,
                            policy::alias(policy::long_name<S_("flag1")>)}};
    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "flag1"}};
    auto view = tokens.pending_view();
    const auto& owner = std::get<0>(node.children());
    
    owner.pre_parse_phase(tokens, view, node);
    return 0;
}
    )",
        "An aliased node cannot be an alias too");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
