/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/utility/result.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_SUITE(result_suite)

BOOST_AUTO_TEST_CASE(value_test)
{
    auto f = [](auto value) {
        auto r = utility::result<bool, std::runtime_error>{value};
        BOOST_CHECK(r.has_result());
        BOOST_CHECK(!r.has_error());
        BOOST_CHECK(r);
        BOOST_CHECK(!!r);

        const auto r_ptr = r.get_if();
        BOOST_CHECK(r_ptr);
        BOOST_CHECK_EQUAL(*r_ptr, value);

        const auto extracted = r.extract();
        BOOST_CHECK_EQUAL(extracted, value);

        r.throw_exception();
    };

    test::data_set(f,                  //
                   {std::tuple{true},  //
                    std::tuple{false}});
}

BOOST_AUTO_TEST_CASE(const_value_test)
{
    auto f = [](auto value) {
        const auto r = utility::result<bool, std::runtime_error>{value};
        BOOST_CHECK(r.has_result());
        BOOST_CHECK(!r.has_error());
        BOOST_CHECK(r);
        BOOST_CHECK(!!r);

        const auto r_ptr = r.get_if();
        BOOST_CHECK(r_ptr);
        BOOST_CHECK_EQUAL(*r_ptr, value);

        const auto extracted = r.get();
        BOOST_CHECK_EQUAL(extracted, value);

        r.throw_exception();
    };

    test::data_set(f,                  //
                   {std::tuple{true},  //
                    std::tuple{false}});
}

BOOST_AUTO_TEST_CASE(movable_value_test)
{
    auto f = [](auto value) {
        auto r = utility::result<std::unique_ptr<bool>, std::runtime_error>{
            std::make_unique<bool>(value)};
        BOOST_CHECK(r.has_result());
        BOOST_CHECK(!r.has_error());
        BOOST_CHECK(r);
        BOOST_CHECK(!!r);

        const auto r_ptr = r.get_if();
        BOOST_CHECK(r_ptr);
        BOOST_CHECK_EQUAL(**r_ptr, value);

        auto extracted = r.extract();
        BOOST_CHECK_EQUAL(*extracted, value);

        extracted = r.extract();
        BOOST_CHECK(extracted == nullptr);

        r.throw_exception();
    };

    test::data_set(f,                  //
                   {std::tuple{true},  //
                    std::tuple{false}});
}

BOOST_AUTO_TEST_CASE(exception_test)
{
    auto f = [](std::string message) {
        auto r = utility::result<bool, std::runtime_error>{std::runtime_error{message}};
        BOOST_CHECK(!r.has_result());
        BOOST_CHECK(r.has_error());
        BOOST_CHECK_EQUAL(static_cast<bool>(r), false);
        BOOST_CHECK(!r);

        const auto r_ptr = r.get_if();
        BOOST_CHECK(!r_ptr);

        BOOST_CHECK_EXCEPTION(r.extract(), std::runtime_error, [&](const auto& e) {
            return e.what() == message;
        });

        r = utility::result<bool, std::runtime_error>{std::runtime_error{message}};
        BOOST_CHECK_EXCEPTION(r.throw_exception(), std::runtime_error, [&](const auto& e) {
            return e.what() == message;
        });
    };

    test::data_set(f,                     //
                   {std::tuple{"test1"},  //
                    std::tuple{"test2"}});
}

BOOST_AUTO_TEST_CASE(equality_test)
{
    using result_type = utility::result<bool, std::runtime_error>;

    auto f = [](auto a, auto b, auto expected_result) {
        auto result = a == b;
        BOOST_CHECK_EQUAL(result, expected_result);

        result = a != b;
        BOOST_CHECK_EQUAL(result, !expected_result);
    };

    test::data_set(f,
                   std::tuple{
                       // Result pairs
                       std::tuple{result_type{true}, result_type{true}, true},
                       std::tuple{result_type{true}, result_type{false}, false},
                       std::tuple{result_type{false}, result_type{true}, false},
                       std::tuple{result_type{false}, result_type{false}, true},

                       // Mixed result/exceptions
                       std::tuple{result_type{true}, result_type{std::runtime_error{"foo"}}, false},
                       std::tuple{result_type{std::runtime_error{"foo"}}, result_type{true}, false},

                       // Exceptions
                       std::tuple{result_type{std::runtime_error{"foo"}},
                                  result_type{std::runtime_error{"bar"}},
                                  false},
                       std::tuple{result_type{std::runtime_error{"foo"}},
                                  result_type{std::runtime_error{"foo"}},
                                  false},
                   });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(same_result_and_exception_types_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/utility/result.hpp"

using namespace arg_router;

int main() {
    utility::result<bool, bool>{false};
    return 0;
}
    )",
        "Result and exception argument cannot be same type");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
