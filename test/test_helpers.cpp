#include "test_helpers.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/process/io.hpp>
#include <boost/process/search_path.hpp>
#include <boost/process/system.hpp>

#include <filesystem>
#include <fstream>

using namespace arg_router;
using namespace std::string_literals;
using namespace std::string_view_literals;
namespace fs = std::filesystem;
namespace bp = boost::process;

namespace
{
const auto main_file_suffix = fs::path{"test/death_test/main.cpp"};
const auto relative_build_dir = ".."s;
const auto target_name = "arg_router_death_test"s;
const auto repo_env = "AR_REPO_PATH"sv;

std::string_view get_project_repo()
{
    const auto path = std::getenv(repo_env.data());
    BOOST_REQUIRE_MESSAGE(
        path,
        "env var AR_REPO_PATH not set, set to the absolute repository path");

    return path;
}
}  // namespace

void test::death_test_compile(std::string_view code,
                              std::string_view expected_error)
{
    // Replace main stub with input
    const auto file_path = get_project_repo() / main_file_suffix;
    {
        auto stream = std::ofstream{file_path};
        BOOST_REQUIRE(stream);

        stream << code;
    }

    auto stream = bp::ipstream{};
    auto cmake = bp::child{bp::search_path("cmake"),
                           "--build",
                           relative_build_dir,
                           "--target",
                           target_name,
                           (bp::std_out & bp::std_err) > stream};

    auto output = ""s;
    {
        output.reserve(4096);
        for (auto line = ""s; std::getline(stream, line) && !line.empty();) {
            output += line + '\n';
        }
    }

    cmake.wait();
    BOOST_CHECK_NE(cmake.exit_code(), 0);
    if (cmake.exit_code() == 0) {
        return;
    }

    // Analyse output for expected error string
    const auto result = boost::algorithm::contains(output, expected_error);
    BOOST_CHECK(result);
    if (!result) {
        BOOST_TEST_MESSAGE("Output: " << output);
    }
}
