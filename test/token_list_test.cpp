#include "arg_router/token_type.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(token_list_suite)

BOOST_AUTO_TEST_CASE(default_constructor)
{
    const auto tl = parsing::token_list{};
    BOOST_CHECK(tl.empty());
    BOOST_CHECK_EQUAL(tl.size(), 0);
    BOOST_CHECK(tl.begin() == tl.cbegin());
    BOOST_CHECK(tl.begin() == tl.end());
    BOOST_CHECK(tl.end() == tl.cend());

    BOOST_CHECK_EQUAL(tl, parsing::token_list{});
}

BOOST_AUTO_TEST_CASE(init_constructor)
{
    const auto tl = parsing::token_list{{parsing::prefix_type::LONG, "long"},
                                        {parsing::prefix_type::SHORT, "s"},
                                        {parsing::prefix_type::NONE, "none"}};
    BOOST_CHECK(!tl.empty());
    BOOST_CHECK_EQUAL(tl.size(), 3);
    BOOST_CHECK(tl.begin() == tl.cbegin());
    BOOST_CHECK(tl.begin() != tl.end());
    BOOST_CHECK(tl.end() == tl.cend());

    BOOST_CHECK_EQUAL(
        tl[0],
        (parsing::token_type{parsing::prefix_type::LONG, "long"}));
    BOOST_CHECK_EQUAL(  //
        tl[1],
        (parsing::token_type{parsing::prefix_type::SHORT, "s"}));
    BOOST_CHECK_EQUAL(
        tl[2],
        (parsing::token_type{parsing::prefix_type::NONE, "none"}));

    BOOST_CHECK_EQUAL(
        *(tl.begin() + 0),
        (parsing::token_type{parsing::prefix_type::LONG, "long"}));
    BOOST_CHECK_EQUAL(  //
        *(tl.begin() + 1),
        (parsing::token_type{parsing::prefix_type::SHORT, "s"}));
    BOOST_CHECK_EQUAL(
        *(tl.begin() + 2),
        (parsing::token_type{parsing::prefix_type::NONE, "none"}));

    BOOST_CHECK_EQUAL(
        tl.front(),
        (parsing::token_type{parsing::prefix_type::LONG, "long"}));

    BOOST_CHECK_EQUAL(
        tl,
        (parsing::token_list{{parsing::prefix_type::LONG, "long"},
                             {parsing::prefix_type::SHORT, "s"},
                             {parsing::prefix_type::NONE, "none"}}));
}

BOOST_AUTO_TEST_CASE(push_back)
{
    auto tl = parsing::token_list{};
    BOOST_CHECK(tl.empty());
    BOOST_CHECK_EQUAL(tl.size(), 0);

    tl.push_back({parsing::prefix_type::LONG, "long"});
    BOOST_CHECK(!tl.empty());
    BOOST_CHECK_EQUAL(tl.size(), 1);

    tl.push_back({parsing::prefix_type::SHORT, "s"});
    BOOST_CHECK(!tl.empty());
    BOOST_CHECK_EQUAL(tl.size(), 2);

    BOOST_CHECK_EQUAL(
        tl,
        (parsing::token_list{{parsing::prefix_type::LONG, "long"},
                             {parsing::prefix_type::SHORT, "s"}}));
}

BOOST_AUTO_TEST_CASE(pop_front)
{
    auto tl = parsing::token_list{{parsing::prefix_type::LONG, "long"},
                                  {parsing::prefix_type::SHORT, "s"},
                                  {parsing::prefix_type::NONE, "none"}};
    BOOST_CHECK(!tl.empty());
    BOOST_CHECK_EQUAL(tl.size(), 3);

    tl.pop_front();
    BOOST_CHECK_EQUAL(tl.size(), 2);

    BOOST_CHECK_EQUAL(  //
        *(tl.begin() + 0),
        (parsing::token_type{parsing::prefix_type::SHORT, "s"}));
    BOOST_CHECK_EQUAL(
        *(tl.begin() + 1),
        (parsing::token_type{parsing::prefix_type::NONE, "none"}));
    BOOST_CHECK_EQUAL(  //
        tl[0],
        (parsing::token_type{parsing::prefix_type::SHORT, "s"}));
    BOOST_CHECK_EQUAL(
        tl[1],
        (parsing::token_type{parsing::prefix_type::NONE, "none"}));
    BOOST_CHECK_EQUAL(
        tl,
        (parsing::token_list{{parsing::prefix_type::SHORT, "s"},
                             {parsing::prefix_type::NONE, "none"}}));

    BOOST_CHECK_EQUAL(tl.front(),
                      (parsing::token_type{parsing::prefix_type::SHORT, "s"}));
}

BOOST_AUTO_TEST_CASE(pop_front_n)
{
    auto tl = parsing::token_list{{parsing::prefix_type::LONG, "long"},
                                  {parsing::prefix_type::SHORT, "s"},
                                  {parsing::prefix_type::NONE, "none"}};
    BOOST_CHECK(!tl.empty());
    BOOST_CHECK_EQUAL(tl.size(), 3);

    tl.pop_front(2);
    BOOST_CHECK_EQUAL(tl.size(), 1);

    BOOST_CHECK_EQUAL(
        *tl.begin(),
        (parsing::token_type{parsing::prefix_type::NONE, "none"}));
    BOOST_CHECK_EQUAL(
        tl[0],
        (parsing::token_type{parsing::prefix_type::NONE, "none"}));
    BOOST_CHECK_EQUAL(
        tl,
        (parsing::token_list{{parsing::prefix_type::NONE, "none"}}));

    BOOST_CHECK_EQUAL(
        tl.front(),
        (parsing::token_type{parsing::prefix_type::NONE, "none"}));

    tl.pop_front();
    BOOST_CHECK(tl.empty());
    BOOST_CHECK_EQUAL(tl.size(), 0);
    BOOST_CHECK(tl.begin() == tl.cbegin());
    BOOST_CHECK(tl.begin() == tl.end());
    BOOST_CHECK(tl.end() == tl.cend());

    BOOST_CHECK_EQUAL(tl, parsing::token_list{});
}

BOOST_AUTO_TEST_CASE(repeated_pop_front)
{
    auto tl = parsing::token_list{{parsing::prefix_type::LONG, "long"},
                                  {parsing::prefix_type::SHORT, "s"},
                                  {parsing::prefix_type::NONE, "none"}};
    BOOST_CHECK(!tl.empty());
    BOOST_CHECK_EQUAL(tl.size(), 3);

    tl.pop_front();
    tl.pop_front();
    tl.pop_front();

    BOOST_CHECK(tl.empty());
    BOOST_CHECK_EQUAL(tl.size(), 0);
    BOOST_CHECK(tl.begin() == tl.end());
    BOOST_CHECK_EQUAL(tl, parsing::token_list{});

    // Should be a no-op
    tl.pop_front();
    BOOST_CHECK(tl.empty());
    BOOST_CHECK_EQUAL(tl.size(), 0);
    BOOST_CHECK(tl.begin() == tl.end());
    BOOST_CHECK_EQUAL(tl, parsing::token_list{});
}

BOOST_AUTO_TEST_CASE(insert)
{
    auto t1 = parsing::token_list{};

    auto t2 = parsing::token_list{{parsing::prefix_type::LONG, "long"},
                                  {parsing::prefix_type::SHORT, "s"},
                                  {parsing::prefix_type::NONE, "none"}};
    t1.insert(t1.begin(), t2.begin(), t2.end());
    BOOST_CHECK_EQUAL(t1, t2);

    t1.insert(t1.end(), t2.begin(), t2.end());
    BOOST_CHECK_EQUAL(
        t1,
        (parsing::token_list{{parsing::prefix_type::LONG, "long"},
                             {parsing::prefix_type::SHORT, "s"},
                             {parsing::prefix_type::NONE, "none"},
                             {parsing::prefix_type::LONG, "long"},
                             {parsing::prefix_type::SHORT, "s"},
                             {parsing::prefix_type::NONE, "none"}}));

    t1.pop_front(3);
    BOOST_CHECK_EQUAL(t1, t2);

    // Reclaims the head space...
    t1.insert(t1.begin(), t2.begin(), t2.end());
    BOOST_CHECK_EQUAL(
        t1,
        (parsing::token_list{{parsing::prefix_type::LONG, "long"},
                             {parsing::prefix_type::SHORT, "s"},
                             {parsing::prefix_type::NONE, "none"},
                             {parsing::prefix_type::LONG, "long"},
                             {parsing::prefix_type::SHORT, "s"},
                             {parsing::prefix_type::NONE, "none"}}));

    t1.pop_front(3);
    BOOST_CHECK_EQUAL(t1, t2);
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
