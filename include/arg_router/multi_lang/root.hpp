// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/multi_lang/translation.hpp"
#include "arg_router/utility/tuple_iterator.hpp"

#include <variant>

namespace arg_router::multi_lang
{
/** Provides multi-language support by instantiating an arg_router::root_t for a given language ID.
 *
 * This relies on the use of multi_lang::translation to provide compile-time selection of language
 * variants for strings.  The user needs to specialise multi_lang::translation for DefaultLanguageID
 * and each supported ID in SupportedLanguageIDs.
 *
 * @code
 * namespace ar = arg_router;
 * namespace arp = ar::policy;
 *
 * namespace ar::multi_lang
 * {
 * template <>
 * class translation<S_("en_GB")>
 * {
 * public:
 *     using force = S_("force");
 *     using force_description = S_("Force overwrite existing files");
 *     ...
 * };
 *
 * template <>
 * class translation<S_("fr")>
 * {
 * public:
 *     using force = S_("forcer");
 *     using force_description = S_("Forcer l'écrasement des fichiers existants");
 *     ...
 * };
 *
 * template <>
 * class translation<S_("ja")>
 * {
 * public:
 *     using force = S_("強制");
 *     using force_description = S_("既存のファイルを強制的に上書きする");
 *     ...
 * };
 * } // namespace ar::multi_lang
 *
 * ar::multi_lang::root<S_("en_GB"), S_("fr"), S_("ja")>(  //
 *     ar::multi_lang::iso_locale(std::locale("").name()),
 *     [&](auto tr_) {
 *         using tr = decltype(tr_);
 *
 *         const auto common_args = ar::list{
 *             ar::flag(arp::long_name<typename tr::force>,
 *                      arp::short_name<'f'>,
 *                      arp::description<typename tr::force_description>),
 *         ...
 * @endcode
 * As shown by the above example, the supported translation object is given to the function object
 * that returns the root instance.  This means that each use of translation specialisation must have
 * the same number of language-specific compile-time strings.
 *
 * @a RootFactory is a function object used to return a given root type from a supported language
 * ID, its signature must be equivalent to:
 * @code
 * template <typename ID>
 * auto operator()(multi_lang::translation<ID> I) {
 *     ...
 * };
 * @endcode
 *
 * The DefaultLanguageID and SupportedLanguageIDs parameters can be any set of unique compile-time
 * strings, but as they are compared against an input that would @em usually come from
 * <TT>std::locale()</TT>, then ISO language codes are least troublesome and easiest to read.
 *
 * @tparam RootFactory Function object type that accepts a multi_lang::translation specialisation
 * and returns a root instance
 * @tparam DefaultLanguageID The default language ID as a compile time string, this is used if the
 * runtime input code does not match this or any of SupportedLanguageIDs
 * @tparam SupportedLanguageIDs The supported language IDs as compile time strings
 */
template <typename RootFactory, typename DefaultLanguageID, typename... SupportedLanguageIDs>
class root_t
{
    using supported_language_ids = std::tuple<DefaultLanguageID, SupportedLanguageIDs...>;
    constexpr static auto num_supported_ids = std::tuple_size_v<supported_language_ids>;

    static_assert(num_supported_ids > 1, "Must be more than one language supported");
    static_assert(num_supported_ids ==
                      std::tuple_size_v<boost::mp11::mp_unique<supported_language_ids>>,
                  "Supported languages must be unique");

    using root_variant_t =
        std::variant<std::decay_t<decltype(std::declval<RootFactory>()(
                         std::declval<translation<DefaultLanguageID>>()))>,
                     std::decay_t<decltype(std::declval<RootFactory>()(
                         std::declval<translation<SupportedLanguageIDs>>()))>...>;

public:
    /** Constructor
     *
     * @param language_id The runtime language selection, if it doesn't match any of the
     * SupportedLanguageIDs, then DefaultLanguageID is used
     * @param f Function object that returns the root instance for a given supported language
     */
    explicit root_t(std::string_view language_id, const RootFactory& f)
    {
        utility::tuple_type_iterator<supported_language_ids>([&](auto I) {
            using id = std::tuple_element_t<I, supported_language_ids>;

            if (root_ || (language_id != id::get())) {
                return;
            }

            root_.emplace(root_variant_t{f(translation<id>{})});
        });

        // No match, so fall back to the default (the first language)
        if (!root_) {
            root_.emplace(root_variant_t{f(translation<DefaultLanguageID>{})});
        }
    }

    /** Calls the parse method on the selected root.
     *
     * The first element is @em not expected to be the executable name.
     * @param args Vector of tokens
     * @exception parse_exception Thrown if parsing has failed
     */
    void parse(vector<parsing::token_type> args) const
    {
        std::visit([&](const auto& root) { root.parse(std::move(args)); }, *root_);
    }

    /** Calls the parse method on the selected root.
     *
     * The first element is @em not expected to be the executable name.
     * @note The strings must out live the parse process as they are not copied.
     * @tparam Iter Iterator type to <TT>std::string_view</TT> convertible elements
     * @param begin Iterator to the first element
     * @param end Iterator to the one-past-the-end element
     */
    template <typename Iter, typename = std::enable_if_t<!std::is_same_v<std::decay_t<Iter>, int>>>
    void parse(Iter begin, Iter end) const
    {
        std::visit([&](const auto& root) { root.parse(begin, end); }, *root_);
    }

    /** Calls the parse method on the selected root.
     *
     * The first element is @em not expected to be the executable name.
     * @note This does not take part in overload resolution if @a c is a
     * <TT>vector\<parsing::token_type\></TT>
     * @tparam Container
     * @param c Elements to parse
     */
    template <typename Container,
              typename = std::enable_if_t<
                  !std::is_same_v<std::decay_t<Container>, vector<parsing::token_type>>>>
    void parse(const Container& c) const
    {
        std::visit([&](const auto& root) { root.parse(c); }, *root_);
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

/** Convenience function that returns a root_t.
 *
 * Allows the user to define the supported languages IDs in the template parameters but has @a Fn
 * deduced from the input.
 *
 * @tparam DefaultLanguageID The default language ID as a compile time string, this is used if the
 * runtime input code does not match this or any of SupportedLanguageIDs
 * @tparam SupportedLanguageIDs The supported language IDs as compile time strings
 * @tparam RootFactory Function object type that accepts a multi_lang::translation specialisation
 * and returns a root instance
 * @param language_id The runtime language selection, if it doesn't match any of the
 * SupportedLanguageIDs, then DefaultLanguageID is used
 * @param f Function object that returns the root instance for a given supported language
 * @return root_t instance
 */
template <typename DefaultLanguageID, typename... SupportedLanguageIDs, typename RootFactory>
auto root(std::string_view language_id, const RootFactory& f)
{
    return root_t<RootFactory, DefaultLanguageID, SupportedLanguageIDs...>{language_id, f};
}
}  // namespace arg_router::multi_lang
