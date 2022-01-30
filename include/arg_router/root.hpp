#pragma once

#include "arg_router/parsing.hpp"
#include "arg_router/policy/no_result_value.hpp"
#include "arg_router/tree_node.hpp"

#include <sstream>
#include <utility>
#include <variant>

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
class root_t : public tree_node<Params...>
{
    using parent_type = tree_node<Params...>;

public:
    using typename parent_type::policies_type;
    using typename parent_type::children_type;

private:
    static_assert(!parent_type::template any_phases_v<
                      bool,  // Type doesn't matter, as long as it isn't void
                      policy::has_pre_parse_phase_method,
                      policy::has_parse_phase_method,
                      policy::has_validation_phase_method,
                      policy::has_routing_phase_method,
                      policy::has_missing_phase_method>,
                  "Root does not support policies with any parsing phases");

    static_assert(!traits::has_long_name_method_v<parent_type> &&
                      !traits::has_short_name_method_v<parent_type> &&
                      !traits::has_display_name_method_v<parent_type> &&
                      !traits::has_none_name_method_v<parent_type> &&
                      !traits::has_description_method_v<parent_type>,
                  "Root cannot have name or description policies");

    constexpr static auto validator_index =
        algorithm::find_specialisation_v<policy::validation::validator,
                                         policies_type>;
    static_assert(validator_index != std::tuple_size_v<policies_type>,
                  "Root must have a validator policy, use "
                  "policy::validation::default_validator unless you have "
                  "created a custom one");

    static_assert(std::tuple_size_v<children_type> >= 1,
                  "Root must have at least one child");

    template <typename Child>
    struct router_checker {
        constexpr static bool has_router =
            !std::is_void_v<typename Child::template phase_finder_t<
                policy::has_routing_phase_method>>;

        constexpr static bool value =
            policy::has_no_result_value_v<Child> || has_router;
    };
    static_assert(
        boost::mp11::mp_all_of<children_type, router_checker>::value,
        "All root children must have routers, unless they have no value");

    // Find the help node
    constexpr static auto help_index =
        boost::mp11::mp_find_if<children_type,
                                traits::has_generate_help_method>::value;

public:
    /** Validator type. */
    // Initially I wanted the default_validator to be used if one isn't user
    // specified, but you get into a circular dependency as the validator needs
    // the root first
    using validator_type = std::tuple_element_t<validator_index, policies_type>;

    /** Help data type. */
    template <bool Flatten>
    class help_data_type
    {
    public:
        using label = S_("");
        using description = S_("");
        using children = typename tree_node<Params...>::template  //
            default_leaf_help_data_type<Flatten>::all_children_help;
    };

    /** Constructor.
     *
     * @param params Policy and child instances
     */
    constexpr explicit root_t(Params... params) :
        parent_type{std::move(params)...}
    {
        validator_type::template validate<std::decay_t<decltype(*this)>>();
    }

    /** Parse the command line arguments.
     *
     * @param argc Number of arguments
     * @param argv Array of char pointers to the command line tokens
     * @exception parse_exception Thrown if parsing has failed
     */
    void parse(int argc, char* argv[]) const
    {
        // Trying to handle collapsed short form tokens without expanding them
        // is very painful...  Shame about the heap allocation though, hopefully
        // SSO will eliminate it for most cases
        auto tokens =
            parsing::expand_arguments(argc, const_cast<const char**>(argv));

        // Find the initial child, if there are no tokens then create an empty
        // one - an anonymous mode will still be found
        const auto first_token =
            tokens.pending_view().empty() ?
                parsing::token_type{parsing::prefix_type::NONE, ""} :
                tokens.pending_view().front();
        const auto found_child =
            parent_type::find(first_token,
                              [&](auto /*i*/, const auto& child) {  //
                                  child.parse(tokens, *this);
                              });
        if (!found_child) {
            if (tokens.pending_view().empty()) {
                throw parse_exception{"No arguments passed"};
            } else {
                throw parse_exception{"Unknown argument",
                                      tokens.pending_view().front()};
            }
        }
    }

    /** Generates a root-level help string and writes it into @a stream.
     *
     * @param stream Output stream to write into
     */
    void help(std::ostream& stream) const
    {
        if constexpr (help_index < std::tuple_size_v<children_type>) {
            std::get<help_index>(this->children())
                .template generate_help<root_t>(stream);
        }
    }

    /** Overload that writes into std::string and returns it.
     *
     * @return String holding the help output
     */
    std::string help() const
    {
        auto stream = std::stringstream{};
        help(stream);

        return stream.str();
    }
};

/** Constructs a root_t with the given policies and children.
 *
 * This is used for similarity with arg_t.
 * @tparam Params Policies and child node types for the mode
 * @param params Pack of policy and child node instances
 * @return Root instance
 */
template <typename... Params>
constexpr root_t<Params...> root(Params... params)
{
    return root_t{std::move(params)...};
}
}  // namespace arg_router
