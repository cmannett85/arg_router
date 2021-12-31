
#include "arg_router/policy/router.hpp"
#include "arg_router/tree_node.hpp"

using namespace arg_router;

template <typename... Params>
class stub_node : public tree_node<Params...>
{
public:
    using tree_node<Params...>::parse;

    constexpr explicit stub_node(Params... params) :
        tree_node<Params...>{std::move(params)...}
    {
    }

    using tree_node<Params...>::routing_phase;
};

int main() {
    auto f = stub_node{
        policy::router{[](auto...) {}},
        policy::router{[](auto...) {}}
    };
    f.routing_phase(parsing::token_list{});
    return 0;
}
    