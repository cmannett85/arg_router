/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/utility/tuple_iterator.hpp"

#include <variant>

/** Namespace for types and functions relating to runtime multi-language support.
 */
namespace arg_router::multi_lang
{
/** A wrapper around a root node that provides multi-language support.
 *
 * The wrapper relies on the use of string_selector (or @ref SM_) to provide compile-time
 * selection of language variants for strings.  The wrapper simply creates a root variant for the
 * given ISO language code.  If the given ISO language code does not match any of the supporting
 * ones, then the first supported language is used as a fallback.
 *
 * @code
 * namespace ar = arg_router;
 * namespace arp = ar::policy;
 *
 * ar::multi_lang::root_wrapper<S_("en_GB"), S_("fr"), S_("es")>{
 *     ar::multi_lang::iso_locale(std::locale("").name()),
 *     [&](auto I) {
 *         return ar::root(my_validator{},
 *             ar::help(arp::long_name<SM_(I, "help", "aider", "ayuda")>,
 *                      arp::short_name<'h'>,
 *                      arp::program_name<S_("is_even")>,
 *                      arp::program_version<S_(version)>,
 *                      arp::program_addendum<SM_(I, "An example program for arg_router.",
 *                                                   "Un exemple de programme pour arg_router.",
 *                                                   "Un programa de ejemplo para arg_router")>,
 *             ...
 * @endcode
 * As implied by the above example, the supported language is given by index to the function object
 * that returns the root instance.  This means that each use of string_selector must have the same
 * number of language options, <I>in the same order</I>.  The latter requirement is especially
 * important as this cannot be checked by arg_router.
 *
 * @a Fn is a function object used to return a given root type from a supported language index, its
 * signature must match:
 * @code
 * template <typename IntegralConstant>
 * auto operator()(IntegralConstant I) {
 *     ...
 * };
 * @endcode
 *
 * The SupportedISOLanguageCodes parameters can be any set of unique strings, but as they are
 * compared against an input that would @em usually come from a <TT>std::locale()</TT> call, then
 * ISO language codes are least troublesome and easiest to read.
 *
 * @tparam Fn Function object type that accepts an integral constant (the supported language index)
 * and returns a root instance
 * @tparam SupportedISOLanguageCodes The supported ISO language codes as compile time strings
 */
template <typename Fn, typename... SupportedISOLanguageCodes>
class root_wrapper_t
{
    constexpr static auto num_supported_codes = sizeof...(SupportedISOLanguageCodes);
    static_assert(num_supported_codes > 1, "Must be more than one language provided");

    using supported_codes = std::tuple<SupportedISOLanguageCodes...>;
    static_assert(num_supported_codes == std::tuple_size_v<boost::mp11::mp_unique<supported_codes>>,
                  "Supported ISO language codes must be unique");

    template <std::size_t... I>
    constexpr static auto indicies_to_variant(std::index_sequence<I...>) -> std::variant<
        std::decay_t<decltype(std::declval<Fn>()(traits::integral_constant<I>{}))>...>;

    using root_variant_t =
        decltype(indicies_to_variant(std::make_index_sequence<num_supported_codes>{}));
    using supported_iso_languages = std::tuple<SupportedISOLanguageCodes...>;

public:
    /** Constructor
     *
     * @param iso_language The runtime language selection, if it doesn't match any of
     * SupportedISOLanguageCodes, then the first language in SupportedISOLanguageCodes is used
     * @param f Function object that returns the root instance for a given supported language index
     */
    explicit root_wrapper_t(std::string_view iso_language, Fn&& f)
    {
        utility::tuple_type_iterator<boost::mp11::mp_iota_c<num_supported_codes>>([&](auto I) {
            if (root_ ||
                (iso_language != std::tuple_element_t<I, supported_iso_languages>::get())) {
                return;
            }

            root_.emplace(root_variant_t{f(I)});
        });

        // No match, so fall back to the default (the first language)
        if (!root_) {
            root_.emplace(root_variant_t{f(traits::integral_constant<std::size_t{0}>{})});
        }
    }

    /** Calls the parse method on the selected root.
     *
     * @param argc Number of arguments
     * @param argv Array of char pointers to the command line tokens
     */
    void parse(int argc, char** argv) const
    {
        std::visit([&](const auto& root) { root.parse(argc, argv); }, *root_);
    }

    /** Calls the help method on the selected root.
     *
     * @param stream Output stream to write into
     */
    void help(std::ostream& stream) const
    {
        std::visit([&](const auto& root) { root.help(stream); }, *root_);
    }

    /** Calls the help method on the selected root.
     *
     * @return String holding the help output
     */
    [[nodiscard]] string help() const
    {
        return std::visit([](const auto& root) { root.help(); }, *root_);
    }

private:
    // Only optional due to the delayed initialisation
    std::optional<root_variant_t> root_;
};

/** Convenience function that returns a root_wrapper_t.
 *
 * Allows the user to define the supported ISO languages in the template parameters but has @a Fn
 * deduced from the input.
 * @param iso_language The runtime language selection, if it doesn't match any of
 * SupportedISOLanguageCodes, then the first language in SupportedISOLanguageCodes is used
 * @param f Function object that returns the root instance for a given supported language index
 * @return root_wrapper_t instance
 */
template <typename... SupportedISOLanguageCodes, typename Fn>
[[nodiscard]] auto root_wrapper(std::string_view iso_language, Fn&& f)
{
    return root_wrapper_t<Fn, SupportedISOLanguageCodes...>{iso_language, std::forward<Fn>(f)};
}
}  // namespace arg_router::multi_lang
