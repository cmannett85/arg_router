#include "arg_router/utility/tree_recursor.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

namespace
{
struct test_Fn {
    template <typename Current, typename... Parents>
    constexpr static void fn()
    {
        using parents_type = std::tuple<Parents...>;

        if constexpr (std::is_same_v<
                          Current,
                          std::decay_t<decltype(
                              policy::validation::default_validator)>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<
                        root<std::decay_t<decltype(
                                 policy::validation::default_validator)>,
                             flag<policy::description_t<S_("test1")>,
                                  policy::long_name_t<S_("test")>>,
                             flag<policy::description_t<S_("test2")>,
                                  policy::short_name_t<
                                      traits::integral_constant<'a'>>>>>>);
        } else if constexpr (std::is_same_v<
                                 Current,
                                 policy::description_t<S_("test1")>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<
                        flag<policy::description_t<S_("test1")>,
                             policy::long_name_t<S_("test")>>,
                        root<std::decay_t<decltype(
                                 policy::validation::default_validator)>,
                             flag<policy::description_t<S_("test1")>,
                                  policy::long_name_t<S_("test")>>,
                             flag<policy::description_t<S_("test2")>,
                                  policy::short_name_t<
                                      traits::integral_constant<'a'>>>>>>);
        } else if constexpr (std::is_same_v<Current,
                                            policy::long_name_t<S_("test")>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<
                        flag<policy::description_t<S_("test1")>,
                             policy::long_name_t<S_("test")>>,
                        root<std::decay_t<decltype(
                                 policy::validation::default_validator)>,
                             flag<policy::description_t<S_("test1")>,
                                  policy::long_name_t<S_("test")>>,
                             flag<policy::description_t<S_("test2")>,
                                  policy::short_name_t<
                                      traits::integral_constant<'a'>>>>>>);
        } else if constexpr (std::is_same_v<
                                 Current,
                                 policy::description_t<S_("test2")>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<
                        flag<policy::description_t<S_("test2")>,
                             policy::short_name_t<
                                 traits::integral_constant<'a'>>>,
                        root<std::decay_t<decltype(
                                 policy::validation::default_validator)>,
                             flag<policy::description_t<S_("test1")>,
                                  policy::long_name_t<S_("test")>>,
                             flag<policy::description_t<S_("test2")>,
                                  policy::short_name_t<
                                      traits::integral_constant<'a'>>>>>>);
        } else if constexpr (std::is_same_v<
                                 Current,
                                 policy::short_name_t<
                                     traits::integral_constant<'a'>>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<
                        flag<policy::description_t<S_("test2")>,
                             policy::short_name_t<
                                 traits::integral_constant<'a'>>>,
                        root<std::decay_t<decltype(
                                 policy::validation::default_validator)>,
                             flag<policy::description_t<S_("test1")>,
                                  policy::long_name_t<S_("test")>>,
                             flag<policy::description_t<S_("test2")>,
                                  policy::short_name_t<
                                      traits::integral_constant<'a'>>>>>>);
        } else if constexpr (std::is_same_v<
                                 Current,
                                 flag<policy::description_t<S_("test1")>,
                                      policy::long_name_t<S_("test")>>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<
                        root<std::decay_t<decltype(
                                 policy::validation::default_validator)>,
                             flag<policy::description_t<S_("test1")>,
                                  policy::long_name_t<S_("test")>>,
                             flag<policy::description_t<S_("test2")>,
                                  policy::short_name_t<
                                      traits::integral_constant<'a'>>>>>>);
        } else if constexpr (std::is_same_v<
                                 Current,
                                 flag<policy::description_t<S_("test2")>,
                                      policy::short_name_t<
                                          traits::integral_constant<'a'>>>>) {
            static_assert(
                std::is_same_v<
                    parents_type,
                    std::tuple<
                        root<std::decay_t<decltype(
                                 policy::validation::default_validator)>,
                             flag<policy::description_t<S_("test1")>,
                                  policy::long_name_t<S_("test")>>,
                             flag<policy::description_t<S_("test2")>,
                                  policy::short_name_t<
                                      traits::integral_constant<'a'>>>>>>);
        } else if constexpr (
            std::is_same_v<Current,
                           root<std::decay_t<decltype(
                                    policy::validation::default_validator)>,
                                flag<policy::description_t<S_("test1")>,
                                     policy::long_name_t<S_("test")>>,
                                flag<policy::description_t<S_("test2")>,
                                     policy::short_name_t<
                                         traits::integral_constant<'a'>>>>>) {
            static_assert(std::is_same_v<parents_type, std::tuple<>>);
        }
    }
};
}  // namespace

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_CASE(tree_recursor_test)
{
    using Root =
        root<std::decay_t<decltype(policy::validation::default_validator)>,
             flag<policy::description_t<S_("test1")>,
                  policy::long_name_t<S_("test")>>,
             flag<policy::description_t<S_("test2")>,
                  policy::short_name_t<traits::integral_constant<'a'>>>>;

    utility::tree_recursor<test_Fn, Root>();
}

BOOST_AUTO_TEST_SUITE_END()
