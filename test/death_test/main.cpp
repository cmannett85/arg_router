
#include "arg_router/policy/validator.hpp"

int main() {
        arg_router::root_t<
            arg_router::flag_t<
                arg_router::policy::short_name_t<
                    arg_router::traits::integral_constant<'a'>>,
                arg_router::policy::long_name_t<S_("test")>,
                arg_router::policy::router<std::less<>>>>();
    return 0;
}
    