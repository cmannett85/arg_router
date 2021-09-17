#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/parsing.hpp"
#include "arg_router/utility/string_view_ops.hpp"

#include <utility>

namespace arg_router
{
namespace policy
{
namespace validation
{
template <typename... Rules>
class validator;
}
}  // namespace policy

/** The root of the parse tree.
 *
 * @tparam Params The top-level policies and child node types
 */
template <typename... Params>
class root : public tree_node<Params...>
{
public:
    using typename tree_node<Params...>::policies_type;

private:
    constexpr static auto validator_index =
        algorithm::find_specialisation_v<policy::validation::validator,
                                         policies_type>;
    static_assert(validator_index != std::tuple_size_v<policies_type>,
                  "Root must have a validator policy, use "
                  "policy::validation::default_validator unless you have "
                  "created a custom one");

public:
    /** Validator type. */
    // Initially I wanted the default_validator to be used if one isn't user
    // specified, but you get into a circular dependency as the validator needs
    // the root first
    using validator_type = std::tuple_element_t<validator_index, policies_type>;

    /** Constructor.
     *
     * @param params Policy and child instances
     */
    constexpr explicit root(Params... params) :
        tree_node<Params...>{std::move(params)...}
    {
        validator_type::template validate<std::decay_t<decltype(*this)>>();
    }

    /** Parse the command line arguments.
     *
     * @param argc Number of arguments
     * @param argv Array of char pointers to the command line tokens
     * @exception std::invalid_argument Thrown if parsing has failed
     */
    void parse(int argc, char* argv[]) const
    {
        using namespace utility::string_view_ops;

        // Trying to handle collapsed short form tokens without expanding them
        // is very painful...  Shame about the heap allocation though
        auto tokens =
            parsing::expand_arguments(argc - 1,
                                      const_cast<const char**>(argv + 1));

        // Match the initial token, the first token determines what type the
        // result tuple will be
        if (tokens.empty()) {
            throw std::invalid_argument{
                "Anonymous mode support not added yet!"};
        }

        const auto [prefix, stripped] =
            parsing::get_prefix_type(tokens.front());
        auto found_child = false;

        switch (prefix) {
        case parsing::prefix_type::LONG:
            found_child = parsing::visit_child<parsing::prefix_type::LONG>(
                this->children(),
                stripped,
                [&](auto i, const auto& child, auto result) {
                    handle_top_level<i>(child, result, std::move(tokens));
                });
            break;
        case parsing::prefix_type::SHORT:
            found_child = parsing::visit_child<parsing::prefix_type::SHORT>(
                this->children(),
                stripped,
                [&](auto i, const auto& child, auto result) {
                    handle_top_level<i>(child, result, std::move(tokens));
                });
            break;
        case parsing::prefix_type::NONE:
            found_child = parsing::visit_child<parsing::prefix_type::NONE>(
                this->children(),
                stripped,
                [&](auto i, const auto& child, auto result) {
                    handle_top_level<i>(child, result, std::move(tokens));
                });
            break;
        }

        if (!found_child) {
            throw std::invalid_argument{"Unknown argument: " + tokens.front()};
        }
    }

private:
    template <std::size_t ChildIndex, typename Child>
    static void handle_top_level(Child&& child,
                                 parsing::match_result m_result,
                                 std::deque<std::string> tokens)
    {
        if (m_result.matched) {
            tokens.pop_front();
        }

        // Build results tuple
        using router_args_t = parsing::build_router_args_t<std::decay_t<Child>>;
        auto router_args = router_args_t{};

        if constexpr (std::tuple_size_v<router_args_t> > 1) {
            // Handle bottom level, as this means we have children
        } else {
            static_assert(
                std::tuple_size_v<router_args_t> == 1,
                "Dev: No children but somehow has multiple value_types");

            if (m_result.has_argument) {
                // Parse when we support args
            } else {
                // We have hit the flag, so set the value to true
                std::get<0>(router_args) = true;

                // If there are any other tokens, then throw as they are
                // unhandled
                if (!tokens.empty()) {
                    throw std::invalid_argument{"Unhandled argument: " +
                                                tokens.front()};
                }
            }

            // Call the router, we should not get here without one as it is a
            // rule
            std::apply(child, std::move(router_args));
        }
    }
};
}  // namespace arg_router
