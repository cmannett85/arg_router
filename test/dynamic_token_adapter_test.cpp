/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/dynamic_token_adapter.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(parsing_suite)

BOOST_AUTO_TEST_SUITE(dynamic_token_adapter_suite)

BOOST_AUTO_TEST_CASE(iterator_ops_test)
{
    auto processed = std::vector<parsing::token_type>{};
    auto unprocessed = std::vector<parsing::token_type>{
        {parsing::prefix_type::none, "--hello"},
        {parsing::prefix_type::none, "42"},
        {parsing::prefix_type::none, "-f"},
        {parsing::prefix_type::none, "goodbye"}};

    auto adapter = parsing::dynamic_token_adapter{processed, unprocessed};
    BOOST_CHECK(processed.empty());
    BOOST_CHECK_EQUAL(unprocessed.size(), 4);

    auto it = adapter.begin();
    BOOST_CHECK(processed.empty());
    BOOST_CHECK_EQUAL(unprocessed.size(), 4);
    BOOST_CHECK_EQUAL(
        *it,
        (parsing::token_type{parsing::prefix_type::none, "--hello"}));
    BOOST_CHECK_EQUAL(it->prefix, parsing::prefix_type::none);
    BOOST_CHECK_EQUAL(it->name, "--hello");
    BOOST_CHECK(it != adapter.end());

    ++it;
    BOOST_CHECK(processed.empty());
    BOOST_CHECK_EQUAL(unprocessed.size(), 4);
    BOOST_CHECK_EQUAL(*it,
                      (parsing::token_type{parsing::prefix_type::none, "42"}));
    BOOST_CHECK(it != adapter.end());

    BOOST_CHECK(adapter.begin() < it);
    BOOST_CHECK(adapter.begin() <= it);
    BOOST_CHECK(it <= it);
    BOOST_CHECK(it > adapter.begin());
    BOOST_CHECK(it >= adapter.begin());
    BOOST_CHECK(it >= it);

    it.set({parsing::prefix_type::long_, "test"});
    BOOST_CHECK_EQUAL(
        *it,
        (parsing::token_type{parsing::prefix_type::long_, "test"}));
    BOOST_CHECK_EQUAL(processed,
                      (std::vector<parsing::token_type>{
                          {parsing::prefix_type::none, "--hello"},
                          {parsing::prefix_type::long_, "test"}}));
    BOOST_CHECK_EQUAL(unprocessed,
                      (std::vector<parsing::token_type>{
                          {parsing::prefix_type::none, "-f"},
                          {parsing::prefix_type::none, "goodbye"}}));

    it += 2;
    BOOST_CHECK_EQUAL(
        *it,
        (parsing::token_type{parsing::prefix_type::none, "goodbye"}));
    {
        auto it2 = it - 2;
        BOOST_CHECK_EQUAL(
            *it2,
            (parsing::token_type{parsing::prefix_type::long_, "test"}));
    }

    it -= 2;
    BOOST_CHECK_EQUAL(
        *it,
        (parsing::token_type{parsing::prefix_type::long_, "test"}));
    {
        auto it2 = it + 2;
        BOOST_CHECK_EQUAL(
            *it2,
            (parsing::token_type{parsing::prefix_type::none, "goodbye"}));
    }

    ++it;
    BOOST_CHECK_EQUAL(*it,
                      (parsing::token_type{parsing::prefix_type::none, "-f"}));
    --it;
    BOOST_CHECK_EQUAL(
        *it,
        (parsing::token_type{parsing::prefix_type::long_, "test"}));

    // Increments/decrements
    {
        auto it2 = it++;
        BOOST_CHECK_EQUAL(
            *it,
            (parsing::token_type{parsing::prefix_type::none, "-f"}));
        BOOST_CHECK_EQUAL(
            *it2,
            (parsing::token_type{parsing::prefix_type::long_, "test"}));

        it2 = it--;
        BOOST_CHECK_EQUAL(
            *it,
            (parsing::token_type{parsing::prefix_type::long_, "test"}));
        BOOST_CHECK_EQUAL(
            *it2,
            (parsing::token_type{parsing::prefix_type::none, "-f"}));
    }

    BOOST_CHECK_EQUAL(it[3],
                      (parsing::token_type{parsing::prefix_type::none, "-f"}));

    it += 5;
    BOOST_CHECK(it == adapter.end());
    it += 1000;
    BOOST_CHECK(it == adapter.end());
    it -= 2000;
    BOOST_CHECK(it != adapter.end());
    BOOST_CHECK(it < adapter.begin());
}

BOOST_AUTO_TEST_CASE(partial_start_test)
{
    auto processed = std::vector<parsing::token_type>{
        {parsing::prefix_type::none, "--hello"},
        {parsing::prefix_type::none, "42"}};
    auto unprocessed = std::vector<parsing::token_type>{
        {parsing::prefix_type::none, "-f"},
        {parsing::prefix_type::none, "goodbye"}};

    auto adapter = parsing::dynamic_token_adapter{processed, unprocessed};
    BOOST_CHECK_EQUAL(processed.size(), 2);
    BOOST_CHECK_EQUAL(unprocessed.size(), 2);

    auto it = adapter.begin();
    BOOST_CHECK_EQUAL(processed.size(), 2);
    BOOST_CHECK_EQUAL(unprocessed.size(), 2);
    BOOST_CHECK(it != adapter.end());

    it += 2;
    it.set({parsing::prefix_type::long_, "test"});
    BOOST_CHECK_EQUAL(processed,
                      (std::vector<parsing::token_type>{
                          {parsing::prefix_type::none, "--hello"},
                          {parsing::prefix_type::none, "42"},
                          {parsing::prefix_type::long_, "test"}}));
    BOOST_CHECK_EQUAL(unprocessed,
                      (std::vector<parsing::token_type>{
                          {parsing::prefix_type::none, "goodbye"}}));
    BOOST_CHECK(it != adapter.end());
    BOOST_CHECK((it + 2) == adapter.end());
}

BOOST_AUTO_TEST_CASE(end_iterator_test)
{
    auto processed = std::vector<parsing::token_type>{};
    auto unprocessed = std::vector<parsing::token_type>{
        {parsing::prefix_type::none, "--hello"},
        {parsing::prefix_type::none, "42"},
        {parsing::prefix_type::none, "-f"},
        {parsing::prefix_type::none, "goodbye"}};

    auto adapter = parsing::dynamic_token_adapter{processed, unprocessed};

    auto end_it = adapter.end();
    BOOST_CHECK(processed.empty());
    BOOST_CHECK_EQUAL(unprocessed.size(), 4);

    // No-op
    ++end_it;
    BOOST_CHECK(processed.empty());
    BOOST_CHECK_EQUAL(unprocessed.size(), 4);
    BOOST_CHECK(end_it == adapter.end());
    end_it++;
    BOOST_CHECK(processed.empty());
    BOOST_CHECK_EQUAL(unprocessed.size(), 4);
    BOOST_CHECK(end_it == adapter.end());

    // Can reach the end
    auto it = adapter.begin() + 4;
    BOOST_CHECK(processed.empty());
    BOOST_CHECK_EQUAL(unprocessed.size(), 4);
    BOOST_CHECK(it == adapter.end());
    BOOST_CHECK(it == end_it);
}

BOOST_AUTO_TEST_CASE(loop_test)
{
    auto processed = std::vector<parsing::token_type>{};
    auto unprocessed = std::vector<parsing::token_type>{
        {parsing::prefix_type::none, "--hello"},
        {parsing::prefix_type::none, "42"},
        {parsing::prefix_type::none, "-f"},
        {parsing::prefix_type::none, "goodbye"}};

    auto adapter = parsing::dynamic_token_adapter{processed, unprocessed};

    auto i = 0u;
    for (auto token : adapter) {
        BOOST_CHECK_EQUAL(token, unprocessed[i++]);
    }
    BOOST_CHECK(processed.empty());
    BOOST_CHECK_EQUAL(unprocessed.size(), 4);
}

BOOST_AUTO_TEST_CASE(insertion_test)
{
    auto processed = std::vector<parsing::token_type>{};
    auto unprocessed = std::vector<parsing::token_type>{
        {parsing::prefix_type::none, "--hello"},
        {parsing::prefix_type::none, "42"},
        {parsing::prefix_type::none, "-f"},
        {parsing::prefix_type::none, "goodbye"}};

    auto adapter = parsing::dynamic_token_adapter{processed, unprocessed};
    auto it = adapter.begin() + 2;

    auto result = adapter.insert(
        it,
        (parsing::token_type{parsing::prefix_type::long_, "foo"}));
    BOOST_CHECK(it == result);
    BOOST_CHECK_EQUAL(processed,
                      (std::vector<parsing::token_type>{
                          {parsing::prefix_type::none, "--hello"},
                          {parsing::prefix_type::none, "42"},
                          {parsing::prefix_type::long_, "foo"}}));
    BOOST_CHECK_EQUAL(unprocessed,
                      (std::vector<parsing::token_type>{
                          {parsing::prefix_type::none, "-f"},
                          {parsing::prefix_type::none, "goodbye"}}));

    it = adapter.end();
    adapter.insert(it,
                   (parsing::token_type{parsing::prefix_type::long_, "bar"}));
    BOOST_CHECK_EQUAL(processed,
                      (std::vector<parsing::token_type>{
                          {parsing::prefix_type::none, "--hello"},
                          {parsing::prefix_type::none, "42"},
                          {parsing::prefix_type::long_, "foo"},
                          {parsing::prefix_type::none, "-f"},
                          {parsing::prefix_type::none, "goodbye"},
                          {parsing::prefix_type::long_, "bar"}}));
    BOOST_CHECK(unprocessed.empty());
}

BOOST_AUTO_TEST_CASE(transfer_test)
{
    auto f = [](auto processed,
                auto unprocessed,
                auto offset,
                auto expected_processed,
                auto expected_unprocessed) {
        auto adapter = parsing::dynamic_token_adapter{processed, unprocessed};
        adapter.transfer(adapter.begin() + offset);
        BOOST_CHECK_EQUAL(processed, expected_processed);
        BOOST_CHECK_EQUAL(unprocessed, expected_unprocessed);
    };

    test::data_set(f,
                   {
                       std::tuple{
                           std::vector<parsing::token_type>{},
                           std::vector<parsing::token_type>{
                               {parsing::prefix_type::none, "--hello"},
                               {parsing::prefix_type::none, "42"},
                               {parsing::prefix_type::none, "-f"},
                               {parsing::prefix_type::none, "goodbye"}},
                           1,
                           std::vector<parsing::token_type>{
                               {parsing::prefix_type::none, "--hello"},
                               {parsing::prefix_type::none, "42"}},
                           std::vector<parsing::token_type>{
                               {parsing::prefix_type::none, "-f"},
                               {parsing::prefix_type::none, "goodbye"}},
                       },
                       std::tuple{
                           std::vector<parsing::token_type>{},
                           std::vector<parsing::token_type>{
                               {parsing::prefix_type::none, "--hello"},
                               {parsing::prefix_type::none, "42"},
                               {parsing::prefix_type::none, "-f"},
                               {parsing::prefix_type::none, "goodbye"}},
                           4,
                           std::vector<parsing::token_type>{
                               {parsing::prefix_type::none, "--hello"},
                               {parsing::prefix_type::none, "42"},
                               {parsing::prefix_type::none, "-f"},
                               {parsing::prefix_type::none, "goodbye"}},
                           std::vector<parsing::token_type>{},
                       },
                       std::tuple{
                           std::vector<parsing::token_type>{},
                           std::vector<parsing::token_type>{
                               {parsing::prefix_type::none, "--hello"},
                               {parsing::prefix_type::none, "42"},
                               {parsing::prefix_type::none, "-f"},
                               {parsing::prefix_type::none, "goodbye"}},
                           4000,
                           std::vector<parsing::token_type>{
                               {parsing::prefix_type::none, "--hello"},
                               {parsing::prefix_type::none, "42"},
                               {parsing::prefix_type::none, "-f"},
                               {parsing::prefix_type::none, "goodbye"}},
                           std::vector<parsing::token_type>{},
                       },
                       std::tuple{
                           std::vector<parsing::token_type>{},
                           std::vector<parsing::token_type>{},
                           4000,
                           std::vector<parsing::token_type>{},
                           std::vector<parsing::token_type>{},
                       },
                       std::tuple{
                           std::vector<parsing::token_type>{},
                           std::vector<parsing::token_type>{
                               {parsing::prefix_type::none, "--hello"},
                               {parsing::prefix_type::none, "42"},
                               {parsing::prefix_type::none, "-f"},
                               {parsing::prefix_type::none, "goodbye"}},
                           0,
                           std::vector<parsing::token_type>{
                               {parsing::prefix_type::none, "--hello"}},
                           std::vector<parsing::token_type>{
                               {parsing::prefix_type::none, "42"},
                               {parsing::prefix_type::none, "-f"},
                               {parsing::prefix_type::none, "goodbye"}},
                       },
                   });
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
