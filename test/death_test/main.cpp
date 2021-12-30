
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/count.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
namespace
{
template <typename... Policies>
class stub_node : public tree_node<Policies...>
{
public:
    constexpr explicit stub_node(Policies... policies) :
        tree_node<Policies...>{std::move(policies)...}
    {
    }

    template <typename... Parents>
    void pre_parse_phase(parsing::token_list& tokens,
                         utility::span<const parsing::token_type>& view,
                         const Parents&... parents) const
    {
        using this_policy =
            std::tuple_element_t<2, typename stub_node::policies_type>;
        this->this_policy::pre_parse_phase(tokens, view, parents...);
    }
};
}  // namespace

int main() {
    auto node = stub_node{policy::long_name<S_("node")>,
                  stub_node{policy::long_name<S_("flag1")>,
                            policy::count<0>,
                            policy::alias(policy::long_name<S_("flag2")>)},
                  stub_node{policy::long_name<S_("flag2")>},
                  stub_node{policy::long_name<S_("flag3")>}};
    auto tokens = parsing::token_list{{parsing::prefix_type::LONG, "flag1"}};
    auto view = tokens.pending_view();
    const auto& owner = std::get<0>(node.children());
    
    owner.pre_parse_phase(tokens, view);
    return 0;
}
    