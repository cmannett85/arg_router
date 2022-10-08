// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/flatten_help.hpp"
#include "arg_router/policy/no_result_value.hpp"
#include "arg_router/tree_node.hpp"

#include <utility>
#include <variant>

namespace arg_router
{
namespace policy::validation
{
template <typename... Rules>
class validator;
}  // namespace policy::validation

/** The root of the parse tree.
 *
 * @tparam Params The top-level policies and child node types
 */
template <typename... Params>
class root_t : public tree_node<std::decay_t<Params>...>
{
    using parent_type = tree_node<std::decay_t<Params>...>;

public:
    using typename parent_type::policies_type;
    using typename parent_type::children_type;

private:
    static_assert(
        !parent_type::template any_phases_v<bool,  // Type doesn't matter, as long as it isn't void
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
        algorithm::find_specialisation_v<policy::validation::validator, policies_type>;
    static_assert(validator_index != std::tuple_size_v<policies_type>,
                  "Root must have a validator policy, use "
                  "policy::validation::default_validator unless you have "
                  "created a custom one");

    static_assert(std::tuple_size_v<children_type> >= 1, "Root must have at least one child");

    template <typename Child>
    struct router_checker {
        constexpr static bool has_router = !std::is_void_v<
            typename Child::template phase_finder_t<policy::has_routing_phase_method>>;

        constexpr static bool value = policy::has_no_result_value_v<Child> || has_router;
    };
    static_assert(boost::mp11::mp_all_of<children_type, router_checker>::value,
                  "All root children must have routers, unless they have no value");

    // Find the help node
    constexpr static auto help_index =
        boost::mp11::mp_find_if<children_type, traits::has_generate_help_method>::value;

public:
    /** Validator type. */
    // Initially I wanted the default_validator to be used if one isn't user specified, but you get
    // into a circular dependency as the validator needs the root first
    using validator_type = std::tuple_element_t<validator_index, policies_type>;

    /** Help data type. */
    template <bool Flatten>
    class help_data_type
    {
    public:
        using label = S_("");
        using description = S_("");
        using children = typename tree_node<std::decay_t<Params>...>::template  //
            default_leaf_help_data_type<Flatten>::all_children_help;
    };

    /** Constructor.
     *
     * @param params Policy and child instances
     */
    constexpr explicit root_t(Params... params) noexcept : parent_type{std::move(params)...}
    {
        validator_type::template validate<std::decay_t<decltype(*this)>>();
    }

    /** Parse the command line arguments.
     *
     * @param argc Number of arguments
     * @param argv Array of char pointers to the command line tokens
     * @exception parse_exception Thrown if parsing has failed
     */
    void parse(int argc, char** argv) const
    {
        // Skip the program name
        auto args = vector<parsing::token_type>{};
        args.reserve(argc - 1);
        for (auto i = 1; i < argc; ++i) {
            args.emplace_back(parsing::prefix_type::none, argv[i]);
        }

        // Take a copy of the front token for the error messages
        const auto front_token = args.empty() ?  //
                                     parsing::token_type{parsing::prefix_type::none, ""} :
                                     args.front();

        // Find a matching child
        auto match = std::optional<parsing::parse_target>{};
        utility::tuple_iterator(
            [&](auto /*i*/, const auto& child) {
                // Skip any remaining children if one has been found
                if (!match && (match = child.pre_parse(parsing::pre_parse_data{args}, *this))) {
                    if (!args.empty()) {
                        throw parse_exception{"Unhandled arguments", args};
                    }
                    (*match)();
                }
            },
            this->children());
        if (!match) {
            if (front_token.name.empty()) {
                throw parse_exception{"No arguments passed"};
            }
            throw parse_exception{"Unknown argument", front_token};
        }
    }

    /** Generates a root-level help string and writes it into @a stream.
     *
     * @param stream Output stream to write into
     */
    void help(std::ostream& stream) const
    {
        if constexpr (help_index < std::tuple_size_v<children_type>) {
            using help_type = std::tuple_element_t<help_index, children_type>;
            constexpr auto flatten =
                algorithm::has_specialisation_v<policy::flatten_help_t,
                                                typename help_type::policies_type>;

            std::get<help_index>(this->children())
                .template generate_help<root_t, help_type, flatten>(stream);
        }
    }

    /** Overload that writes into a string and returns it.
     *
     * @return String holding the help output
     */
    [[nodiscard]] string help() const
    {
        auto stream = ostringstream{};
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
[[nodiscard]] constexpr auto root(Params... params) noexcept
{
    return root_t{std::move(params)...};
}
}  // namespace arg_router
