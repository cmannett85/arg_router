// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <arg_router/arg_router.hpp>

#include <cstdlib>

namespace ar = arg_router;
namespace arp = ar::policy;
using namespace std::string_view_literals;
using namespace ar::literals;

namespace
{
constexpr auto version = std::span{"v1.0.0"};
}  // namespace

/* A ridiculous description policy implementation that appends a smiley face to the user supplied
 * description.
 *
 * Although contrived this demonstrates the static data form of policy, where as long as the policy
 * provides a common method name then it can be used by the system.
 */
template <typename S>
class smiley_description_t
{
public:
    constexpr explicit smiley_description_t([[maybe_unused]] S str = {}) noexcept {}

    [[nodiscard]] constexpr static std::string_view description() noexcept
    {
        return S::template append_t<ar::str<" 🙂">>::get();
    }

private:
    static_assert(!S::empty(), "Descriptions must not be empty");
};

// Mark it as a policy
/// @cond
template <typename T>
struct arp::is_policy<smiley_description_t<T>> : std::true_type {
};
/// @endcond

// Implements a validation phase to check that the parsed value is even, throws if it isn't
template <typename ValueType>
class is_even
{
    static_assert(std::is_integral_v<ValueType>, "ValueType must be an integer");

public:
    template <typename... Parents>
    void validation_phase(const ValueType& value, [[maybe_unused]] const Parents&... parents) const
    {
        if (value % 2 != 0) {
            throw ar::parse_exception{"Value not even: " + std::to_string(value)};
        }
    }
};

// Mark it as a policy
/// @cond
template <typename ValueType>
struct arp::is_policy<is_even<ValueType>> : std::true_type {
};
/// @endcond

// This is really just a simplified positional_arg_t that only supports a single argument
template <typename T, typename... Policies>
class single_positional_arg_t :
    public ar::multi_arg_base_t<T,
                                1,
                                arp::min_max_count_t<ar::traits::integral_constant<std::size_t{1}>,
                                                     ar::traits::integral_constant<std::size_t{1}>>,
                                Policies...>
{
    using parent_type =
        ar::multi_arg_base_t<T,
                             1,
                             arp::min_max_count_t<ar::traits::integral_constant<std::size_t{1}>,
                                                  ar::traits::integral_constant<std::size_t{1}>>,
                             Policies...>;

    static_assert(ar::traits::has_display_name_method_v<single_positional_arg_t>,
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
    using value_type = typename parent_type::value_type;

    /* This is a minor variation on the tree_node::default_leaf_help_data_type type, we're just
     * wrapping the display name in chevrons.
     */
    template <bool Flatten>
    class help_data_type
    {
        [[nodiscard]] constexpr static auto label_generator() noexcept
        {
            constexpr auto name = single_positional_arg_t::display_name();

            // This is unfortunately necessary due to C++ language limitations, we can't pass
            // a std::string_view directly to ar::str
            constexpr auto name_span = std::span<const char, name.size()>{name};
            return "<"_S + ar::str<name_span>{} + "> "_S +
                   parent_type::template default_leaf_help_data_type<Flatten>::count_suffix();
        }

    public:
        using label = std::decay_t<decltype(label_generator())>;
        using description =
            typename parent_type::template default_leaf_help_data_type<Flatten>::description;
        using children = std::tuple<>;
    };

    constexpr explicit single_positional_arg_t(Policies... policies) noexcept :
        parent_type{arp::min_max_count_t<ar::traits::integral_constant<std::size_t{1}>,
                                         ar::traits::integral_constant<std::size_t{1}>>{},
                    std::move(policies)...}
    {
    }

    /* Forward the request onto the parent's definition.  As there is no polymorphism in the
     * library, we have to create this stub so the <TT>*this</TT> expression evaluates to the
     * correct type
     */
    template <typename Validator, bool HasTarget, typename... Parents>
    [[nodiscard]] std::optional<ar::parsing::parse_target> pre_parse(
        ar::parsing::pre_parse_data<Validator, HasTarget> pre_parse_data,
        const Parents&... parents) const

    {
        return parent_type::pre_parse(pre_parse_data, *this, parents...);
    }

    template <typename... Parents>
    [[nodiscard]] value_type parse(ar::parsing::parse_target target,
                                   const Parents&... parents) const
    {
        return parent_type::parse(target, *this, parents...);
    }

private:
    static_assert(!parent_type::template any_phases_v<value_type, arp::has_routing_phase_method>,
                  "Single positional arg does not support policies with routing phases "
                  "(e.g. router)");
};

// Create a helper function to ease CTAD.  The string_to_policy::convert function isn't required
// but allows string-to-policy conversion to as intended
template <typename T, typename... Policies>
[[nodiscard]] constexpr auto single_positional_arg(Policies... policies) noexcept
{
    return std::apply(
        [](auto... converted_policies) {
            return single_positional_arg_t<T, std::decay_t<decltype(converted_policies)>...>{
                std::move(converted_policies)...};
        },
        ar::utility::string_to_policy::convert<
            ar::utility::string_to_policy::first_text_mapper<arp::display_name_t>,
            ar::utility::string_to_policy::second_text_mapper<smiley_description_t>>(
            std::move(policies)...));
}

// Take a copy of the default rules, as we're adding new types we just need to add to them
using original_rules = typename decltype(arp::validation::default_validator)::rules_type;

// We can't have two description types in a single node, that's just confusing
using smiley_rules = arp::validation::utility::insert_rule_t<
    0,
    arp::validation::rule_q<
        arp::validation::common_rules::despecialised_any_of_rule<smiley_description_t>,
        arp::validation::despecialised_unique_in_owner,
        arp::validation::policy_parent_must_not_have_policy<arp::description_t>>,
    arp::validation::utility::default_rules>;

// is_even doesn't need it's own rule as the generic policy one suffices

// As single_positional_arg_t is just a slightly less generic version of positional_arg_t, we can
// just add it to the despecialised_any_of_rule list for positional_arg_t.  First we need to find
// it...
using new_rules = arp::validation::utility::add_to_rule_types_by_rule_t<
    arp::validation::common_rules::despecialised_any_of_rule<ar::positional_arg_t>,
    single_positional_arg_t,
    smiley_rules>;

using my_validator = arp::validation::validator_from_tuple<new_rules>;

int main(int argc, char* argv[])
{
    // Help and flag are built-in and therefore will use policy::description_t if passed a
    // description string, so we have to be explicit
    ar::root(
        my_validator{},
        ar::help("help"_S,
                 "h"_S,
                 arp::program_name_t{"is_even"_S},
                 arp::program_version<ar::str<version>>,
                 arp::program_addendum_t{"An example program for arg_router."_S},
                 smiley_description_t{"Display this help and exit"_S}),
        ar::flag("version"_S,
                 smiley_description_t{"Output version information and exit"_S},
                 arp::router{[](bool) {
                     std::cout << version.data() << std::endl;
                     std::exit(EXIT_SUCCESS);
                 }}),
        ar::mode(
            single_positional_arg<int>(arp::required, "Value"_S, "Value to read"_S, is_even<int>{}),
            arp::router{[](int value) { std::cout << "Value: " << value << std::endl; }}))
        .parse(argc, argv);

    return EXIT_SUCCESS;
}
