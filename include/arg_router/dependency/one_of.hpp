#pragma once

#include "arg_router/config.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/policy/no_result_value.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include <variant>

namespace arg_router
{
/** The dependency namespace carries nodes and policies that define dependency
 * relationships between other nodes.
 */
namespace dependency
{
namespace detail
{
// Collect the child names and concetenate into a compile time string for the
// one_of display name
template <typename... Params>
struct one_of_generate_string_of_child_names {
    template <typename Prefix, typename String>
    using prepend_prefix = boost::mp11::mp_flatten<
        boost::mp11::mp_insert_c<String, 0, typename Prefix::array_type>>;

    template <typename Child>
    struct build_name {
        // This is clunky, but it's still more readable than doing it with bare
        // types
        constexpr static auto build()
        {
            using policies = typename Child::policies_type;
            constexpr auto num_policies = std::tuple_size_v<policies>;

            // Find display name
            constexpr auto display_index =
                boost::mp11::mp_find_if<policies,
                                        traits::has_display_name_method>::value;
            if constexpr (display_index >= num_policies) {
                // Find long name
                constexpr auto long_index = boost::mp11::
                    mp_find_if<policies, traits::has_long_name_method>::value;
                if constexpr (long_index >= num_policies) {
                    // Find short name
                    constexpr auto short_index = boost::mp11::mp_find_if<
                        policies,
                        traits::has_short_name_method>::value;

                    static_assert(short_index < num_policies,
                                  "All one_of children must be named");

                    return S_(config::short_prefix){} +
                           S_((std::tuple_element_t<short_index,
                                                    policies>::short_name())){};
                } else {
                    return S_(config::long_prefix){} +
                           S_((std::tuple_element_t<long_index,
                                                    policies>::long_name())){};
                }
            } else {
                return S_((std::tuple_element_t<display_index,
                                                policies>::display_name())){};
            }
        }

        using type = typename std::decay_t<decltype(build())>::array_type;
    };

    using children_type =
        boost::mp11::mp_filter<is_tree_node, std::tuple<Params...>>;

    // The gist is that for each child we get its name (prepended with an
    // appropriate prefix) and concatenate with a comma separator.  And then
    // add a helpful prefix to the whole thing
    using string_array = prepend_prefix<
        S_("One of: "),
        boost::mp11::mp_join<
            boost::mp11::mp_transform_q<
                boost::mp11::mp_bind<
                    traits::get_type,
                    boost::mp11::mp_bind<build_name, boost::mp11::_1>>,
                children_type>,
            traits::integral_constant<','>>>;

    using type = utility::convert_to_cts_t<string_array>;
};
}  // namespace detail

/** Groups child nodes such that any @em one can be used on the command line.
 *
 * The value_type is a variant of all the policies' value_types that are not
 * derived from policy::no_result_value.  If that list is only a single type,
 * then it collapses into just that type (i.e. it won't be a variant containing
 * a single type).
 * @tparam Params Policies and child node types for the mode
 */
template <typename... Params>
class one_of_t :
    public tree_node<policy::display_name_t<
                         typename detail::one_of_generate_string_of_child_names<
                             Params...>::type>,
                     Params...>
{
    // C++ really needs a super_type...
    using parent_type =
        tree_node<policy::display_name_t<
                      typename detail::one_of_generate_string_of_child_names<
                          Params...>::type>,
                  Params...>;

    static_assert((std::tuple_size_v<typename one_of_t::children_type> >= 2),
                  "one_of must have at least one two child nodes");
    static_assert(!traits::has_long_name_method_v<one_of_t>,
                  "one_of must not have a long name policy");
    static_assert(!traits::has_short_name_method_v<one_of_t>,
                  "one_of must not have a short name policy");
    static_assert(!traits::has_none_name_method_v<one_of_t>,
                  "one_of must not have a none name policy");

    using basic_value_type = boost::mp11::mp_rename<
        boost::mp11::mp_transform<
            traits::get_value_type,
            boost::mp11::mp_remove_if<typename parent_type::children_type,
                                      policy::has_no_result_value>>,
        std::variant>;

    static_assert(std::variant_size_v<basic_value_type> >= 1,
                  "one_of must have at least one child with a value_type");

public:
    using typename parent_type::children_type;
    using typename parent_type::policies_type;

    /** A variant of the child value_types, or just the value_type if there is
     * only a single child.
     */
    using value_type =
        std::conditional_t<(std::variant_size_v<basic_value_type> == 1),
                           std::variant_alternative_t<0, basic_value_type>,
                           basic_value_type>;

    static_assert(
        boost::mp11::mp_any_of_q<
            policies_type,
            boost::mp11::mp_bind<policy::has_missing_phase_method,
                                 boost::mp11::_1,
                                 value_type>>::value,
        "one_of must have a missing phase method, a policy::required or "
        "policy::default_value are commonly used");

    /** Help data type. */
    template <bool Flatten>
    class help_data_type
    {
    public:
        template <typename Child, typename Prefix>
        struct prefixer {
            using label = typename Prefix::template append_t<
                typename Child::template help_data_type<Flatten>::label>;
            using description =
                typename Child::template help_data_type<Flatten>::description;
            using children = std::tuple<>;
        };

    private:
        template <typename Child>
        using first_prefixer = prefixer<Child, S_("┌ ")>;

        template <typename Child>
        using middle_prefixer = prefixer<Child, S_("├ ")>;

        template <typename Child>
        using last_prefixer = prefixer<Child, S_("└ ")>;

    public:
        using label = S_("One of:");
        using description = S_("");
        using children = boost::mp11::mp_replace_at_c<
            boost::mp11::mp_replace_first<
                boost::mp11::mp_transform<middle_prefixer, children_type>,
                first_prefixer<boost::mp11::mp_first<children_type>>>,
            std::tuple_size_v<children_type> - 1,
            last_prefixer<boost::mp11::mp_back<children_type>>>;
    };

    /** Constructor.
     *
     * @param params Policy and child instances
     */
    constexpr explicit one_of_t(Params... params) :
        parent_type{std::tuple_element_t<0, policies_type>{},
                    std::move(params)...}
    {
    }

    /** Returns true and calls @a visitor if any of the children's match methods
     * return true.
     * 
     * @a visitor needs to be equivalent to:
     * @code
     * [](const auto& node) { ... }
     * @endcode
     * <TT>node</TT> will be a reference to the first matching child.
     * @tparam Fn Visitor type
     * @param token Command line token to match
     * @param visitor Visitor instance
     * @return Match result
     */
    template <typename Fn>
    constexpr bool match(const parsing::token_type& token,
                         const Fn& visitor) const
    {
        auto found = false;
        utility::tuple_iterator(
            [&](auto /*i*/, const auto& child) {
                if (!found && child.match(token, visitor)) {
                    found = true;
                }
            },
            this->children());

        return found;
    }

private:
    static_assert(
        !parent_type::template any_phases_v<value_type,
                                            policy::has_pre_parse_phase_method,
                                            policy::has_parse_phase_method,
                                            policy::has_validation_phase_method,
                                            policy::has_routing_phase_method>,
        "one_of does not support policies with pre-parse, parse, validation, "
        "or routing phases; as it delegates those to its children");
};

/** Constructs a one_of_t with the given policies and children.
 *
 * This is used for similarity with arg_t.
 * @tparam Params Policies and child node types for the mode
 * @param params Pack of policy and child node instances
 * @return Mode instance
 */
template <typename... Params>
[[nodiscard]] constexpr one_of_t<Params...> one_of(Params... params)
{
    return one_of_t{std::move(params)...};
}
}  // namespace dependency
}  // namespace arg_router
