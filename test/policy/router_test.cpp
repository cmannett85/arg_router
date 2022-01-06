#include "arg_router/policy/router.hpp"
#include "arg_router/tree_node.hpp"

#include "test_helpers.hpp"

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

    template <typename... Args>
    bool routing_phase(Args&&... args) const
    {
        auto hit = false;
        utility::tuple_type_iterator<typename stub_node::policies_type>(  //
            [&](auto i) {
                using this_policy =
                    std::tuple_element_t<i, typename stub_node::policies_type>;
                if constexpr (policy::has_routing_phase_method_v<this_policy> &&
                              traits::is_specialisation_of_v<this_policy,
                                                             policy::router>) {
                    this->this_policy::routing_phase(
                        std::forward<Args>(args)...);
                    hit = true;
                }
            });

        return hit;
    }
};
}  // namespace

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(router_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::router<std::function<void()>>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(type_test)
{
    {
        using router_type = policy::router<std::function<void()>>;
        BOOST_CHECK((std::is_same_v<typename router_type::callable_type,
                                    std::function<void()>>));
    }

    {
        using router_type = policy::router<std::function<double()>>;
        BOOST_CHECK((std::is_same_v<typename router_type::callable_type,
                                    std::function<double()>>));
    }

    {
        using router_type = policy::router<std::function<double(float, int)>>;
        BOOST_CHECK((std::is_same_v<typename router_type::callable_type,
                                    std::function<double(float, int)>>));
    }
}

BOOST_AUTO_TEST_CASE(routing_phase_test)
{
    auto fn_hit = false;
    auto f = [&](auto fn, auto tokens, auto fail_message, auto... args) {
        fn_hit = false;
        auto r = stub_node{policy::router{std::move(fn)}};

        try {
            r.routing_phase(tokens, args...);
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK(fn_hit);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{[&]() { fn_hit = true; }, parsing::token_list{}, ""s},
            std::tuple{[&](auto val) {
                           fn_hit = true;
                           BOOST_CHECK_EQUAL(val, 13.6);
                       },
                       parsing::token_list{},
                       ""s,
                       13.6},
            std::tuple{[&](auto... val) {
                           static_assert(sizeof...(val) == 5,
                                         "Incorrect pack size");
                           auto val_tuple = std::tuple{val...};
                           fn_hit = true;
                           BOOST_CHECK_EQUAL(std::get<0>(val_tuple), 1);
                           BOOST_CHECK_EQUAL(std::get<1>(val_tuple), 2);
                           BOOST_CHECK_EQUAL(std::get<2>(val_tuple), 3);
                           BOOST_CHECK_EQUAL(std::get<3>(val_tuple), 4);
                           BOOST_CHECK_EQUAL(std::get<4>(val_tuple), 5);
                       },
                       parsing::token_list{},
                       ""s,
                       1,
                       2,
                       3,
                       4,
                       5},
            std::tuple{
                [&]() { fn_hit = true; },
                parsing::token_list{{parsing::prefix_type::LONG, "test1"}},
                "Unhandled arguments: --test1"s},
            std::tuple{
                [&]() { fn_hit = true; },
                parsing::token_list{{parsing::prefix_type::LONG, "test1"},
                                    {parsing::prefix_type::SHORT, "t"}},
                "Unhandled arguments: --test1, -t"s},
        });
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
