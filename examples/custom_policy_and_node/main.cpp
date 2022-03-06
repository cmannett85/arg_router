/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include <arg_router/arg_router.hpp>

#include <cstdlib>

namespace ar = arg_router;
namespace arp = ar::policy;
using namespace std::string_view_literals;

namespace
{
constexpr auto version = "v1.0.0"sv;
}

/* A ridiculous description policy implementation that appends a smiley face to
 * the user supplied description.
 *
 * Although contrived this demonstrates the static data type of policy, where as
 * long as the policy provides a common method name then it can be used by the
 * system.
 */
template <typename S>
class smiley_description_t
{
public:
    [[nodiscard]] constexpr static std::string_view description() noexcept
    {
        return S::template append_t<S_(" :)")>::get();
    }

private:
    static_assert(!S::empty(), "Descriptions must not be empty");
};

// Constant variable helper.
template <typename S>
constexpr auto smiley_description = smiley_description_t<S>{};

// Mark it as a policy
/// @cond
template <typename T>
struct arp::is_policy<smiley_description_t<T>> : std::true_type {
};
/// @endcond

/* Implements a validation phase to check that the parsed value is even, throws
 * if it isn't.
 */
template <typename ValueType>
class is_even
{
    static_assert(std::is_integral_v<ValueType>,
                  "ValueType must be an integer");

public:
    template <typename... Parents>
    void validation_phase(const ValueType& value, const Parents&...) const
    {
        if (value % 2 != 0) {
            throw ar::parse_exception{"Value not even: " +
                                      std::to_string(value)};
        }
    }
};

// Mark it as a policy
/// @cond
template <typename ValueType>
struct arp::is_policy<is_even<ValueType>> : std::true_type {
};
/// @endcond

/* This is really just a simplified positional_arg_t that only supports a single
 * argument.
 */
template <typename T, typename... Policies>
class single_positional_arg_t :
    public ar::tree_node<std::decay_t<decltype(arp::fixed_count<1>)>,
                         Policies...>
{
    static_assert(
        arp::is_all_policies_v<std::tuple<Policies...>>,
        "Positional args must only contain policies (not other nodes)");

    using parent_type =
        ar::tree_node<std::decay_t<decltype(arp::fixed_count<1>)>, Policies...>;

    static_assert(
        ar::traits::has_display_name_method_v<single_positional_arg_t>,
        "Positional arg must have a display name policy");
    static_assert(!ar::traits::has_long_name_method_v<single_positional_arg_t>,
                  "Positional arg must not have a long name policy");
    static_assert(!ar::traits::has_short_name_method_v<single_positional_arg_t>,
                  "Positional arg must not have a short name policy");
    static_assert(!ar::traits::has_none_name_method_v<single_positional_arg_t>,
                  "Positional arg must not have a none name policy");

public:
    using typename parent_type::policies_type;

    // You must expose the parsed type as a using declaration
    using value_type = T;

    /* This is a minor variation on the tree_node::default_leaf_help_data_type
     * type, we're just wrapping the display name in chevrons.
     */
    template <bool Flatten>
    class help_data_type
    {
        [[nodiscard]] constexpr static auto label_generator() noexcept
        {
            constexpr auto name_index = boost::mp11::mp_find_if<
                policies_type,
                ar::traits::has_display_name_method>::value;
            constexpr auto name =
                std::tuple_element_t<name_index, policies_type>::display_name();

            return S_("<"){} + S_(name){} + S_("> "){} +
                   parent_type::template default_leaf_help_data_type<
                       Flatten>::count_suffix();
        }

    public:
        using label = std::decay_t<decltype(label_generator())>;
        using description =
            typename parent_type::template default_leaf_help_data_type<
                Flatten>::description;
        using children = std::tuple<>;
    };

    constexpr explicit single_positional_arg_t(Policies... policies) noexcept :
        parent_type{arp::fixed_count<1>, std::move(policies)...}
    {
    }

    /* As a positional arg doesn't have a label token we have no name to match
     * against, but by using the triple arg overload of match(..) we gain
     * access to the result the mode has created so we can see if it has been 
     * set already.
     */
    template <typename Fn>
    constexpr bool match([[maybe_unused]] const ar::parsing::token_type& token,
                         const Fn& visitor,
                         const std::optional<T>& result) const
    {
        //  If the value has already been set, then we've already been used!
        if (result) {
            return false;
        }

        visitor(*this);
        return true;
    }

    template <typename... Parents>
    value_type parse(ar::parsing::token_list& tokens,
                     const Parents&... parents) const
    {
        // Pre-parse
        ar::utility::tuple_type_iterator<policies_type>([&](auto i) {
            using policy_type = std::tuple_element_t<i, policies_type>;
            if constexpr (arp::has_pre_parse_phase_method_v<policy_type>) {
                this->policy_type::pre_parse_phase(tokens, *this, parents...);
            }
        });

        // The fixed count guarantees there is a single token
        auto view = tokens.pending_view();
        auto result = parent_type::template parse<value_type>(view.front().name,
                                                              *this,
                                                              parents...);

        // Pop the value
        tokens.mark_as_processed();

        // Validation
        ar::utility::tuple_type_iterator<policies_type>([&](auto i) {
            using policy_type = std::tuple_element_t<i, policies_type>;
            if constexpr (arp::has_validation_phase_method_v<policy_type,
                                                             value_type>) {
                this->policy_type::validation_phase(result, *this, parents...);
            }
        });

        // No routing phase, it cannot be used as a top-level node

        return result;
    }

private:
    static_assert(
        !parent_type::template any_phases_v<value_type,
                                            arp::has_routing_phase_method>,
        "Single positional arg does not support policies with routing phases "
        "(e.g. router)");
};

// Create a helper function to ease CTAD.
template <typename T, typename... Policies>
[[nodiscard]] constexpr single_positional_arg_t<T, Policies...>
single_positional_arg(Policies... policies) noexcept
{
    return single_positional_arg_t<T, std::decay_t<Policies>...>{
        std::move(policies)...};
}

// Take a copy of the default rules, as we're adding new types we just need to
// add to them
using original_rules =
    typename decltype(arp::validation::default_validator)::rules_type;

// We can't have two description types in a single node, that's just confusing
using smiley_rules = boost::mp11::mp_push_front<
    original_rules,
    arp::validation::rule_q<arp::validation::common_rules::
                                despecialised_any_of_rule<smiley_description_t>,
                            arp::validation::despecialised_unique_in_owner,
                            arp::validation::policy_parent_must_not_have_policy<
                                arp::description_t>>>;

// is_even doesn't need it's own rule as the generic policy one suffices

// As single_positional_arg_t is just a slightly less generic version of
// positional_arg_t, we can just add it to the despecialised_any_of_rule list
// for positional_arg_t.  First we need to find it...
constexpr auto pos_rule_index = boost::mp11::mp_find_if_q<
    smiley_rules,
    boost::mp11::mp_bind<
        std::is_same,
        arp::validation::common_rules::despecialised_any_of_rule<
            ar::positional_arg_t>,
        boost::mp11::mp_bind<boost::mp11::mp_first, boost::mp11::_1>>>::value;
static_assert(pos_rule_index < std::tuple_size_v<smiley_rules>,
              "Cannot find positional_arg_t rule");

using pos_rule = boost::mp11::mp_replace_at_c<
    boost::mp11::mp_at_c<smiley_rules, pos_rule_index>,
    0,
    arp::validation::common_rules::despecialised_any_of_rule<
        ar::positional_arg_t,
        single_positional_arg_t>>;
using new_rules =
    boost::mp11::mp_replace_at_c<smiley_rules, pos_rule_index, pos_rule>;

using my_validator = arp::validation::validator_from_tuple<new_rules>;

int main(int argc, char* argv[])
{
    ar::root(
        my_validator{},
        ar::help(arp::long_name<S_("help")>,
                 arp::short_name<'h'>,
                 arp::program_name<S_("is_even")>,
                 arp::program_version<S_(version)>,
                 smiley_description<S_("Display this help and exit")>),
        ar::flag(arp::long_name<S_("version")>,
                 smiley_description<S_("Output version information and exit")>,
                 arp::router{[](bool) {
                     std::cout << version << std::endl;
                     std::exit(EXIT_SUCCESS);
                 }}),
        ar::mode(
            single_positional_arg<int>(arp::required,
                                       arp::display_name<S_("Value")>,
                                       smiley_description<S_("Value to read")>,
                                       is_even<int>{}),
            arp::router{[](int) {}}))
        .parse(argc, argv);

    return EXIT_SUCCESS;
}
