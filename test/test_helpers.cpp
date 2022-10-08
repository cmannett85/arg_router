// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test_helpers.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/process/io.hpp>
#include <boost/process/search_path.hpp>
#include <boost/process/system.hpp>

#include <bitset>
#include <filesystem>
#include <fstream>

using namespace arg_router;
using namespace arg_router::utility::string_view_ops;
using namespace std::string_literals;
using namespace std::string_view_literals;
namespace fs = std::filesystem;
namespace bp = boost::process;

namespace
{
static_assert(AR_DEATH_TEST_PARALLEL <= std::numeric_limits<std::uint8_t>::max(),
              "AR_DEATH_TEST_PARALLEL must be less than or equal to 255");

auto compile_mtx = std::mutex{};
auto general_mtx = std::mutex{};
auto pool_mtx = std::mutex{};
auto pool_cv = std::condition_variable{};
auto pool = std::bitset<AR_DEATH_TEST_PARALLEL>{};

// Threadsafe wrapper, otherwise it all gets very mangled...
void test_message(const std::string& message)
{
    auto lk = std::lock_guard{general_mtx};
    BOOST_TEST_MESSAGE(message);
}

std::string_view project_repo()
{
    auto lk = std::lock_guard{general_mtx};
    const auto path = std::getenv("AR_REPO_PATH");
    if (path == nullptr) {
        // If there's no env var, then fallback to the installation dir set at build time
        return AR_REPO_PATH;
    }

    return path;
}

fs::path main_file(std::uint8_t i)
{
    // This must match the DEATH_TEST_SRC variable in death_test.cmake
    return fs::path{"test/death_test/main_" + std::to_string(i) + ".cpp"};
}

std::string target_name(std::uint8_t i)
{
    return "arg_router_death_test_" + std::to_string(i);
}

std::uint8_t find_available_index()
{
    auto lk = std::unique_lock{pool_mtx};
    auto result = std::optional<std::uint8_t>{};

    // Find an available thread
    const auto search = [&]() {
        for (auto i = 0u; i < pool.size(); ++i) {
            if (!pool[i]) {
                result = i;
                return true;
            }
        }

        return false;
    };

    // If there isn't one, wait
    if (!search()) {
        pool_cv.wait(lk, search);
    }

    // Mark the thread in use
    pool[*result] = true;
    return *result;
}

void compile(std::uint8_t i,
             std::string_view code,
             std::string_view expected_error,
             std::string_view test_name = "")
{
    // Create source file
    const auto file_path = project_repo() / main_file(i);
    const auto parent_path = file_path.parent_path();

    if (!fs::exists(parent_path)) {
        auto ec = std::error_code{};
        BOOST_REQUIRE_MESSAGE(fs::create_directory(file_path.parent_path(), ec), ec.message());
    }
    {
        // The mutex is purely for Boost.Test, without it spurious failures occur
        auto lk = std::lock_guard{general_mtx};

        auto stream = std::ofstream{file_path};
        BOOST_REQUIRE_MESSAGE(stream, "Failed to open: " << file_path.string());

        stream << code;
    }

    auto stream = bp::ipstream{};
    auto cmake = bp::child{bp::search_path("cmake"),
                           "--build",
                           "..",
                           "--target",
                           target_name(i),
                           (bp::std_out & bp::std_err) > stream};

    auto output = ""s;
    {
        output.reserve(4096);
        for (auto line = ""s; std::getline(stream, line) && !line.empty();) {
            output += line + '\n';
        }
    }

    cmake.wait();
    fs::remove(file_path);

    BOOST_CHECK_NE(cmake.exit_code(), 0);
    if (cmake.exit_code() == 0) {
        if (!test_name.empty()) {
            test_message(test_name + " failed"sv);
        }
        return;
    }

    // Analyse output for expected error string
    const auto result = boost::algorithm::contains(output, expected_error);
    BOOST_CHECK(result);
    if (!result) {
        if (test_name.empty()) {
            test_message("Output: " + output);
        } else {
            test_message(test_name + " output: "sv + output);
        }
    }
}
}  // namespace

void test::death_test_compile(std::forward_list<death_test_info> tests)
{
    // Not re-entrant
    auto func_guard = std::lock_guard{compile_mtx};

    BOOST_TEST_MESSAGE("Parallel death tests");

    pool.reset();
    auto threads = std::vector<std::thread>(pool.size());

    while (!tests.empty()) {
        const auto i = find_available_index();
        const auto test = std::move(tests.front());
        tests.pop_front();

        auto& thread = threads[i];
        if (thread.joinable()) {
            thread.join();
        }

        thread = std::thread{[&, test = std::move(test), i]() {
            test_message("\tStarting "sv + test.test_name);

            try {
                compile(i, test.code, test.expected_error, test.test_name);
            } catch (std::exception& e) {
                BOOST_CHECK_MESSAGE(false, e.what());
            }

            // Notify find_available_index(), which will unblock it if it's waiting
            {
                auto lk = std::lock_guard{pool_mtx};
                pool[i] = false;
            }
            pool_cv.notify_one();
        }};
    }

    // Wait for any running tests to finish
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void test::death_test_compile(std::string_view code, std::string_view expected_error)
{
    compile(0, code, expected_error);
}
