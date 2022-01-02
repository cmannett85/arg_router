
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
namespace ard = arg_router::dependency;

int main() {
    const auto of = ard::one_of(
        arg<int>(policy::long_name<S_("arg1")>),
        arg<double>(policy::long_name<S_("arg2")>),
        policy::required,
        policy::custom_parser<std::variant<int, double>>{[](std::string_view) {
            return std::variant<int, double>{}; }});
    return 0;
}
    