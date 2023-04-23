// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/policy/validator_rule_utilities.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
namespace arp = arg_router::policy;

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(validator_rule_utilities_suite)

BOOST_AUTO_TEST_CASE(find_index_of_rule_type_test)
{
    {
        constexpr auto pos = arp::validation::utility::find_index_of_rule_type<
            arp::validation::common_rules::despecialised_any_of_rule<arp::long_name_t,
                                                                     arp::short_name_t>,
            arp::validation::utility::default_rules>();
        static_assert(pos == 0, "Test failed");
    }

    {
        constexpr auto pos = arp::validation::utility::find_index_of_rule_type<
            arp::validation::common_rules::despecialised_any_of_rule<arg_t>,
            arp::validation::utility::default_rules>();
        static_assert(pos == 9, "Test failed");
    }

    {
        constexpr auto pos = arp::validation::utility::find_index_of_rule_type<
            arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
            arp::validation::utility::default_rules>();
        static_assert(pos == 19, "Test failed");
    }
}

BOOST_AUTO_TEST_CASE(insert_rule_test)
{
    {
        using new_rules = arp::validation::utility::insert_rule_t<
            0,
            arp::validation::rule_q<
                arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
                arp::validation::despecialised_unique_in_owner>,
            arp::validation::utility::default_rules>;
        static_assert(std::tuple_size_v<new_rules> ==
                          (std::tuple_size_v<arp::validation::utility::default_rules> + 1),
                      "Test failed");

        using inserted_rule = std::tuple_element_t<0, new_rules>;
        static_assert(std::is_same_v<
                          inserted_rule,
                          arp::validation::rule_q<
                              arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
                              arp::validation::despecialised_unique_in_owner>>,
                      "Test failed");
    }

    {
        using new_rules = arp::validation::utility::insert_rule_t<
            5,
            arp::validation::rule_q<
                arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
                arp::validation::despecialised_unique_in_owner>,
            arp::validation::utility::default_rules>;
        static_assert(std::tuple_size_v<new_rules> ==
                          (std::tuple_size_v<arp::validation::utility::default_rules> + 1),
                      "Test failed");

        using inserted_rule = std::tuple_element_t<5, new_rules>;
        static_assert(std::is_same_v<
                          inserted_rule,
                          arp::validation::rule_q<
                              arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
                              arp::validation::despecialised_unique_in_owner>>,
                      "Test failed");
    }

    {
        using new_rules = arp::validation::utility::insert_rule_t<
            std::tuple_size_v<arp::validation::utility::default_rules>,
            arp::validation::rule_q<
                arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
                arp::validation::despecialised_unique_in_owner>,
            arp::validation::utility::default_rules>;
        static_assert(std::tuple_size_v<new_rules> ==
                          (std::tuple_size_v<arp::validation::utility::default_rules> + 1),
                      "Test failed");

        using inserted_rule =
            std::tuple_element_t<std::tuple_size_v<arp::validation::utility::default_rules>,
                                 new_rules>;
        static_assert(std::is_same_v<
                          inserted_rule,
                          arp::validation::rule_q<
                              arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
                              arp::validation::despecialised_unique_in_owner>>,
                      "Test failed");
    }
}

BOOST_AUTO_TEST_CASE(remove_rule_test)
{
    {
        using new_rules =
            arp::validation::utility::remove_rule_t<0, arp::validation::utility::default_rules>;
        static_assert(std::tuple_size_v<new_rules> ==
                          (std::tuple_size_v<arp::validation::utility::default_rules> - 1),
                      "Test failed");

        using next_rule = std::tuple_element_t<0, new_rules>;
        static_assert(
            std::is_same_v<
                next_rule,
                arp::validation::rule_q<
                    arp::validation::common_rules::despecialised_any_of_rule<policy::none_name_t>,
                    arp::validation::despecialised_unique_in_owner,
                    arp::validation::policy_unique_from_owner_parent_to_mode_or_root<
                        arg_router::mode_t>,
                    arp::validation::policy_parent_must_not_have_policy<policy::long_name_t>,
                    arp::validation::policy_parent_must_not_have_policy<policy::short_name_t>,
                    arp::validation::policy_parent_must_not_have_policy<policy::display_name_t>>>,
            "Test failed");
    }

    {
        using new_rules =
            arp::validation::utility::remove_rule_t<9, arp::validation::utility::default_rules>;
        static_assert(std::tuple_size_v<new_rules> ==
                          (std::tuple_size_v<arp::validation::utility::default_rules> - 1),
                      "Test failed");

        using next_rule = std::tuple_element_t<11, new_rules>;
        static_assert(
            std::is_same_v<
                next_rule,
                arp::validation::rule_q<
                    arp::validation::common_rules::despecialised_any_of_rule<counting_flag_t>,
                    arp::validation::must_not_have_policies<policy::no_result_value,
                                                            policy::required_t,
                                                            policy::validation::validator>>>,
            "Test failed");
    }
}

BOOST_AUTO_TEST_CASE(remove_rule_by_type_test)
{
    {
        using new_rules = arp::validation::utility::remove_rule_by_type_t<
            arp::validation::common_rules::despecialised_any_of_rule<policy::long_name_t,
                                                                     policy::short_name_t>,
            arp::validation::utility::default_rules>;
        static_assert(std::tuple_size_v<new_rules> ==
                          (std::tuple_size_v<arp::validation::utility::default_rules> - 1),
                      "Test failed");

        using next_rule = std::tuple_element_t<0, new_rules>;
        static_assert(
            std::is_same_v<
                next_rule,
                arp::validation::rule_q<
                    arp::validation::common_rules::despecialised_any_of_rule<policy::none_name_t>,
                    arp::validation::despecialised_unique_in_owner,
                    arp::validation::policy_unique_from_owner_parent_to_mode_or_root<
                        arg_router::mode_t>,
                    arp::validation::policy_parent_must_not_have_policy<policy::long_name_t>,
                    arp::validation::policy_parent_must_not_have_policy<policy::short_name_t>,
                    arp::validation::policy_parent_must_not_have_policy<policy::display_name_t>>>,
            "Test failed");
    }

    {
        using new_rules = arp::validation::utility::remove_rule_by_type_t<
            arp::validation::common_rules::despecialised_any_of_rule<arg_t>,
            arp::validation::utility::default_rules>;
        static_assert(std::tuple_size_v<new_rules> ==
                          (std::tuple_size_v<arp::validation::utility::default_rules> - 1),
                      "Test failed");

        using next_rule = std::tuple_element_t<11, new_rules>;
        static_assert(
            std::is_same_v<
                next_rule,
                arp::validation::rule_q<
                    arp::validation::common_rules::despecialised_any_of_rule<counting_flag_t>,
                    arp::validation::must_not_have_policies<policy::no_result_value,
                                                            policy::required_t,
                                                            policy::validation::validator>>>,
            "Test failed");
    }

    {
        using new_rules = arp::validation::utility::remove_rule_by_type_t<
            arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
            arp::validation::utility::default_rules>;
        static_assert(std::tuple_size_v<new_rules> ==
                          (std::tuple_size_v<arp::validation::utility::default_rules>),
                      "Test failed");
    }
}

BOOST_AUTO_TEST_CASE(update_rule_test)
{
    {
        using new_rules = arp::validation::utility::update_rule_t<
            0,
            arp::validation::rule_q<
                arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
                arp::validation::despecialised_unique_in_owner>,
            arp::validation::utility::default_rules>;
        static_assert(std::tuple_size_v<new_rules> ==
                          (std::tuple_size_v<arp::validation::utility::default_rules>),
                      "Test failed");

        using updated_rule = std::tuple_element_t<0, new_rules>;
        static_assert(std::is_same_v<
                          updated_rule,
                          arp::validation::rule_q<
                              arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
                              arp::validation::despecialised_unique_in_owner>>,
                      "Test failed");
    }

    {
        using new_rules = arp::validation::utility::update_rule_t<
            8,
            arp::validation::rule_q<
                arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
                arp::validation::despecialised_unique_in_owner>,
            arp::validation::utility::default_rules>;
        static_assert(std::tuple_size_v<new_rules> ==
                          (std::tuple_size_v<arp::validation::utility::default_rules>),
                      "Test failed");

        using updated_rule = std::tuple_element_t<8, new_rules>;
        static_assert(std::is_same_v<
                          updated_rule,
                          arp::validation::rule_q<
                              arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
                              arp::validation::despecialised_unique_in_owner>>,
                      "Test failed");
    }
}

BOOST_AUTO_TEST_CASE(update_rule_by_type_test)
{
    {
        using new_rules = arp::validation::utility::update_rule_by_type_t<
            arp::validation::common_rules::despecialised_any_of_rule<policy::long_name_t,
                                                                     policy::short_name_t>,
            arp::validation::rule_q<
                arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
                arp::validation::despecialised_unique_in_owner>,
            arp::validation::utility::default_rules>;
        static_assert(std::tuple_size_v<new_rules> ==
                          (std::tuple_size_v<arp::validation::utility::default_rules>),
                      "Test failed");

        using updated_rule = std::tuple_element_t<0, new_rules>;
        static_assert(std::is_same_v<
                          updated_rule,
                          arp::validation::rule_q<
                              arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
                              arp::validation::despecialised_unique_in_owner>>,
                      "Test failed");
    }

    {
        using new_rules = arp::validation::utility::update_rule_by_type_t<
            arp::validation::common_rules::despecialised_any_of_rule<arg_t>,
            arp::validation::rule_q<
                arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
                arp::validation::despecialised_unique_in_owner>,
            arp::validation::utility::default_rules>;
        static_assert(std::tuple_size_v<new_rules> ==
                          (std::tuple_size_v<arp::validation::utility::default_rules>),
                      "Test failed");

        using updated_rule = std::tuple_element_t<9, new_rules>;
        static_assert(std::is_same_v<
                          updated_rule,
                          arp::validation::rule_q<
                              arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
                              arp::validation::despecialised_unique_in_owner>>,
                      "Test failed");
    }
}

BOOST_AUTO_TEST_CASE(add_to_rule_types_test)
{
    {
        using new_rules = arp::validation::utility::
            add_to_rule_types_t<0, std::vector, arp::validation::utility::default_rules>;
        static_assert(std::tuple_size_v<new_rules> ==
                          (std::tuple_size_v<arp::validation::utility::default_rules>),
                      "Test failed");

        using updated_rule = std::tuple_element_t<0, new_rules>;
        static_assert(
            std::is_same_v<
                updated_rule,
                arp::validation::rule_q<
                    arp::validation::common_rules::despecialised_any_of_rule<policy::long_name_t,
                                                                             policy::short_name_t,
                                                                             std::vector>,
                    arp::validation::despecialised_unique_in_owner,
                    arp::validation::policy_unique_from_owner_parent_to_mode_or_root<
                        arg_router::mode_t>>>,
            "Test failed");
    }

    {
        using new_rules = arp::validation::utility::
            add_to_rule_types_t<9, std::vector, arp::validation::utility::default_rules>;
        static_assert(std::tuple_size_v<new_rules> ==
                          (std::tuple_size_v<arp::validation::utility::default_rules>),
                      "Test failed");

        using updated_rule = std::tuple_element_t<9, new_rules>;
        static_assert(
            std::is_same_v<
                updated_rule,
                arp::validation::rule_q<
                    arp::validation::common_rules::despecialised_any_of_rule<arg_t, std::vector>,
                    arp::validation::must_not_have_policies<policy::multi_stage_value,
                                                            policy::no_result_value,
                                                            policy::validation::validator>>>,
            "Test failed");
    }
}

BOOST_AUTO_TEST_CASE(add_to_rule_types_by_rule_test)
{
    {
        using new_rules = arp::validation::utility::add_to_rule_types_by_rule_t<
            arp::validation::common_rules::despecialised_any_of_rule<policy::long_name_t,
                                                                     policy::short_name_t>,
            std::vector,
            arp::validation::utility::default_rules>;
        static_assert(std::tuple_size_v<new_rules> ==
                          (std::tuple_size_v<arp::validation::utility::default_rules>),
                      "Test failed");

        using updated_rule = std::tuple_element_t<0, new_rules>;
        static_assert(
            std::is_same_v<
                updated_rule,
                arp::validation::rule_q<
                    arp::validation::common_rules::despecialised_any_of_rule<policy::long_name_t,
                                                                             policy::short_name_t,
                                                                             std::vector>,
                    arp::validation::despecialised_unique_in_owner,
                    arp::validation::policy_unique_from_owner_parent_to_mode_or_root<
                        arg_router::mode_t>>>,
            "Test failed");
    }

    {
        using new_rules = arp::validation::utility::add_to_rule_types_by_rule_t<
            arp::validation::common_rules::despecialised_any_of_rule<arg_t>,
            std::vector,
            arp::validation::utility::default_rules>;
        static_assert(std::tuple_size_v<new_rules> ==
                          (std::tuple_size_v<arp::validation::utility::default_rules>),
                      "Test failed");

        using updated_rule = std::tuple_element_t<9, new_rules>;
        static_assert(
            std::is_same_v<
                updated_rule,
                arp::validation::rule_q<
                    arp::validation::common_rules::despecialised_any_of_rule<arg_t, std::vector>,
                    arp::validation::must_not_have_policies<policy::multi_stage_value,
                                                            policy::no_result_value,
                                                            policy::validation::validator>>>,
            "Test failed");
    }
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({{R"(
#include "arg_router/policy/validator_rule_utilities.hpp"

using namespace arg_router;
namespace arp = arg_router::policy;

int main() {
    using new_rules = arp::validation::utility::insert_rule_t<
        100,
        arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
        arp::validation::utility::default_rules>;
    return 0;
}
    )",
                               "I must be less than or equal Rules size",
                               "insert_rule_test"},
                              {
                                  R"(
#include "arg_router/policy/validator_rule_utilities.hpp"

using namespace arg_router;
namespace arp = arg_router::policy;

int main() {
    using new_rules = arp::validation::utility::remove_rule_t<
        100,
        arp::validation::utility::default_rules>;
    return 0;
}
    )",
                                  "I must be less than Rules size",
                                  "remove_rule_test"},
                              {
                                  R"(
#include "arg_router/policy/validator_rule_utilities.hpp"

using namespace arg_router;
namespace arp = arg_router::policy;

int main() {
    using new_rules = arp::validation::utility::update_rule_t<
        100,
        arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
        arp::validation::utility::default_rules>;
    return 0;
}
    )",
                                  "I must be less than Rules size",
                                  "update_rule_test"},
                              {
                                  R"(
#include "arg_router/policy/validator_rule_utilities.hpp"

using namespace arg_router;
namespace arp = arg_router::policy;

int main() {
    using new_rules = arp::validation::utility::update_rule_by_type_t<
        arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
        arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
        arp::validation::utility::default_rules>;
    return 0;
}
    )",
                                  "RuleType cannot be found",
                                  "update_rule_by_type_test"},
                              {
                                  R"(
#include "arg_router/policy/validator_rule_utilities.hpp"

using namespace arg_router;
namespace arp = arg_router::policy;

int main() {
    using new_rules = arp::validation::utility::
            add_to_rule_types_t<100, std::vector, arp::validation::utility::default_rules>;
    return 0;
}
    )",
                                  "I must be less than Rules size",
                                  "add_to_rule_types_test"},
                              {
                                  R"(
#include "arg_router/policy/validator_rule_utilities.hpp"

using namespace arg_router;
namespace arp = arg_router::policy;

int main() {
    using new_rules = arp::validation::utility::add_to_rule_types_by_rule_t<
            arp::validation::common_rules::despecialised_any_of_rule<std::vector>,
            std::vector,
            arp::validation::utility::default_rules>;
    return 0;
}
    )",
                                  "RuleType cannot be found",
                                  "add_to_rule_types_by_rule_test"}});
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
