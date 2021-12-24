
#include "arg_router/arg.hpp"
#include "arg_router/dependency/one_of.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
namespace ard = arg_router::dependency;

int main() {
    auto f = ard::one_of(arg<int>(policy::long_name<S_("arg1")>,
                                  policy::alias(policy::long_name<S_("arg2")>)),
                         arg<double>(policy::long_name<S_("arg2")>,
                                     policy::alias(policy::long_name<S_("arg1")>)));
    return 0;
}
    