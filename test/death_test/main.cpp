
#include "arg_router/flag.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto m = root(policy::router{[](std::string_view) { return true; }},
                        flag(policy::long_name<S_("hello")>));
    return 0;
}
    