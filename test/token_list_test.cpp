/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/token_type.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(token_list_suite)

BOOST_AUTO_TEST_CASE(default_constructor_test)
{
    const auto tl = parsing::token_list{};
    BOOST_CHECK(tl.pending_view().empty());
    BOOST_CHECK(tl.processed_view().empty());

    BOOST_CHECK_EQUAL(tl, parsing::token_list{});
}

BOOST_AUTO_TEST_CASE(init_constructor_test)
{
    const auto tl = parsing::token_list{{parsing::prefix_type::LONG, "long"},
                                        {parsing::prefix_type::SHORT, "s"},
                                        {parsing::prefix_type::NONE, "none"}};
    BOOST_CHECK(!tl.pending_view().empty());
    BOOST_CHECK(tl.processed_view().empty());
    BOOST_CHECK_EQUAL(tl.pending_view().size(), 3);

    BOOST_CHECK_EQUAL(
        tl.pending_view()[0],
        (parsing::token_type{parsing::prefix_type::LONG, "long"}));
    BOOST_CHECK_EQUAL(  //
        tl.pending_view()[1],
        (parsing::token_type{parsing::prefix_type::SHORT, "s"}));
    BOOST_CHECK_EQUAL(
        tl.pending_view()[2],
        (parsing::token_type{parsing::prefix_type::NONE, "none"}));

    BOOST_CHECK_EQUAL(
        tl.pending_view().front(),
        (parsing::token_type{parsing::prefix_type::LONG, "long"}));

    BOOST_CHECK_EQUAL(
        tl,
        (parsing::token_list{{parsing::prefix_type::LONG, "long"},
                             {parsing::prefix_type::SHORT, "s"},
                             {parsing::prefix_type::NONE, "none"}}));
}

BOOST_AUTO_TEST_CASE(add_pending_test)
{
    auto tl = parsing::token_list{};
    BOOST_CHECK(tl.pending_view().empty());
    BOOST_CHECK(tl.processed_view().empty());

    tl.add_pending({parsing::prefix_type::LONG, "long"});
    BOOST_CHECK(!tl.pending_view().empty());
    BOOST_CHECK(tl.processed_view().empty());
    BOOST_CHECK_EQUAL(tl.pending_view().size(), 1);

    tl.add_pending({parsing::prefix_type::SHORT, "s"});
    BOOST_CHECK(!tl.pending_view().empty());
    BOOST_CHECK(tl.processed_view().empty());
    BOOST_CHECK_EQUAL(tl.pending_view().size(), 2);
}

BOOST_AUTO_TEST_CASE(mark_as_processed_test)
{
    auto tl = parsing::token_list{{parsing::prefix_type::LONG, "long"},
                                  {parsing::prefix_type::SHORT, "s"},
                                  {parsing::prefix_type::NONE, "none"}};
    BOOST_CHECK(!tl.pending_view().empty());
    BOOST_CHECK(tl.processed_view().empty());
    BOOST_CHECK_EQUAL(tl.pending_view().size(), 3);

    tl.mark_as_processed();
    BOOST_CHECK_EQUAL(tl.pending_view().size(), 2);
    BOOST_CHECK_EQUAL(tl.processed_view().size(), 1);

    BOOST_CHECK_EQUAL(  //
        tl.pending_view()[0],
        (parsing::token_type{parsing::prefix_type::SHORT, "s"}));
    BOOST_CHECK_EQUAL(
        tl.pending_view()[1],
        (parsing::token_type{parsing::prefix_type::NONE, "none"}));
    BOOST_CHECK_EQUAL(  //
        tl.processed_view()[0],
        (parsing::token_type{parsing::prefix_type::LONG, "long"}));
    BOOST_CHECK_EQUAL(
        tl.pending_view(),
        (parsing::token_list{{parsing::prefix_type::SHORT, "s"},
                             {parsing::prefix_type::NONE, "none"}}));
}

BOOST_AUTO_TEST_CASE(mark_as_processed_n_test)
{
    auto tl = parsing::token_list{{parsing::prefix_type::LONG, "long"},
                                  {parsing::prefix_type::SHORT, "s"},
                                  {parsing::prefix_type::NONE, "none"}};
    BOOST_CHECK(!tl.pending_view().empty());
    BOOST_CHECK(tl.processed_view().empty());
    BOOST_CHECK_EQUAL(tl.pending_view().size(), 3);

    tl.mark_as_processed(2);
    BOOST_CHECK_EQUAL(tl.pending_view().size(), 1);
    BOOST_CHECK_EQUAL(tl.processed_view().size(), 2);

    BOOST_CHECK_EQUAL(
        tl.pending_view()[0],
        (parsing::token_type{parsing::prefix_type::NONE, "none"}));
    BOOST_CHECK_EQUAL(  //
        tl.processed_view()[0],
        (parsing::token_type{parsing::prefix_type::LONG, "long"}));
    BOOST_CHECK_EQUAL(  //
        tl.processed_view()[1],
        (parsing::token_type{parsing::prefix_type::SHORT, "s"}));

    tl.mark_as_processed();
    BOOST_CHECK(tl.pending_view().empty());
    BOOST_CHECK(!tl.processed_view().empty());
    BOOST_CHECK_EQUAL(tl.processed_view().size(), 3);
}

BOOST_AUTO_TEST_CASE(repeated_mark_as_processed_test)
{
    auto tl = parsing::token_list{{parsing::prefix_type::LONG, "long"},
                                  {parsing::prefix_type::SHORT, "s"},
                                  {parsing::prefix_type::NONE, "none"}};
    BOOST_CHECK(!tl.pending_view().empty());
    BOOST_CHECK(tl.processed_view().empty());
    BOOST_CHECK_EQUAL(tl.pending_view().size(), 3);

    tl.mark_as_processed();
    tl.mark_as_processed();
    tl.mark_as_processed();
    BOOST_CHECK(tl.pending_view().empty());
    BOOST_CHECK(!tl.processed_view().empty());
    BOOST_CHECK_EQUAL(tl.processed_view().size(), 3);

    // Should be a no-op
    tl.mark_as_processed();
    BOOST_CHECK(tl.pending_view().empty());
    BOOST_CHECK(!tl.processed_view().empty());
    BOOST_CHECK_EQUAL(tl.processed_view().size(), 3);
}

BOOST_AUTO_TEST_CASE(insert_pending_test)
{
    auto t1 = parsing::token_list{};

    auto t2 = parsing::token_list{{parsing::prefix_type::LONG, "long"},
                                  {parsing::prefix_type::SHORT, "s"},
                                  {parsing::prefix_type::NONE, "none"}};
    t1.insert_pending(t1.pending_view().begin(),
                      t2.pending_view().begin(),
                      t2.pending_view().end());
    BOOST_CHECK_EQUAL(t1, t2);

    t1.insert_pending(t1.pending_view().end(),
                      t2.pending_view().begin(),
                      t2.pending_view().end());
    BOOST_CHECK_EQUAL(
        t1,
        (parsing::token_list{{parsing::prefix_type::LONG, "long"},
                             {parsing::prefix_type::SHORT, "s"},
                             {parsing::prefix_type::NONE, "none"},
                             {parsing::prefix_type::LONG, "long"},
                             {parsing::prefix_type::SHORT, "s"},
                             {parsing::prefix_type::NONE, "none"}}));

    t1.mark_as_processed(3);
    BOOST_CHECK_EQUAL(t1.pending_view().size(), 3);
    BOOST_CHECK_EQUAL(t1.processed_view().size(), 3);

    // Reclaims the head space...
    t1.insert_pending(t1.pending_view().begin(),
                      t2.pending_view().begin(),
                      t2.pending_view().end());
    BOOST_CHECK_EQUAL(
        t1,
        (parsing::token_list{{parsing::prefix_type::LONG, "long"},
                             {parsing::prefix_type::SHORT, "s"},
                             {parsing::prefix_type::NONE, "none"},
                             {parsing::prefix_type::LONG, "long"},
                             {parsing::prefix_type::SHORT, "s"},
                             {parsing::prefix_type::NONE, "none"}}));
    BOOST_CHECK_EQUAL(t1.pending_view().size(), 6);
    BOOST_CHECK(t1.processed_view().empty());
}

BOOST_AUTO_TEST_CASE(swap_test)
{
    auto t1 = parsing::token_list{};
    auto t2 = parsing::token_list{{parsing::prefix_type::LONG, "long"},
                                  {parsing::prefix_type::SHORT, "s"},
                                  {parsing::prefix_type::NONE, "none"}};

    swap(t1, t2);
    BOOST_CHECK_EQUAL(
        t1,
        (parsing::token_list{{parsing::prefix_type::LONG, "long"},
                             {parsing::prefix_type::SHORT, "s"},
                             {parsing::prefix_type::NONE, "none"}}));
    BOOST_CHECK_EQUAL(t2, parsing::token_list{});
}

BOOST_AUTO_TEST_SUITE_END()
