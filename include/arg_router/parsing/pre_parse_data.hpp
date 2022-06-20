/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/parsing/parse_target.hpp"

namespace arg_router
{
namespace parsing
{
namespace detail
{
struct always_returns_true {
    template <typename... Args>
    constexpr bool operator()(Args&&...) const noexcept
    {
        return true;
    }
};
}  // namespace detail

/** Base class for pre_parse_data.
 *  
 * This is only used for pre_parse_data.
 * @tparam Validator Validation checker type, see derived class documentation
 * for more info
 * @tparam HasTarget True if this instance contains a parse_target reference
 */
template <typename Validator, bool HasTarget>
class pre_parse_data_base
{
public:
    /** True if this instance contains a parse_target reference */
    static constexpr auto has_target = HasTarget;

    /** Returns the arg list reference.
     *
     * @return Arg list
     */
    [[nodiscard]] vector<parsing::token_type>& args() noexcept { return args_; }

    /** Const overload.
     *
     * @return Arg list
     */
    [[nodiscard]] const vector<parsing::token_type>& args() const noexcept
    {
        return args_;
    }

    /** Returns the validator reference.
     *
     * @return Validator
     */
    [[nodiscard]] const Validator& validator() const noexcept
    {
        return validator_;
    }

protected:
    pre_parse_data_base(
        vector<parsing::token_type>& args,
        const Validator& validator =
            [](const auto&...) { return true; }) noexcept :
        args_{args},
        validator_{validator}
    {
    }

private:
    std::reference_wrapper<vector<parsing::token_type>> args_;
    std::reference_wrapper<const Validator> validator_;
};

/** A simple wrapper struct over non-parent data used by the node's
 * <TT>pre_parse(..)</TT> method.
 *
 * As each tree_node derived type must reimplement the <TT>pre_parse(..)</TT>
 * method to at least add a reference to themselves, then overloads cause a lot
 * of extra boilerplate.  By wrapping the arg variations for each of those
 * overloads into another type (this one), the user will only need to change a 
 * single overload with any variation compile-time switchable.
 * 
 * There are two specialisations of pre_parse_data, one that carries a
 * parse_target reference and one that doesn't.  The difference is invisible at
 * construction, but changes how it is used:
 * @code
 * auto args = std::vector<parsing::token_type>{"-f"};
 * auto ppd = parsing::pre_parse_data{args};
 * ...
 * auto target = parsing::parse_target{node, parents...};
 * if constexpr (ppd.has_target) { // In this example, this is false
 *     target = ppd.target();      // Must be wrapped in if, as target() does
 *                                 // not exist is non-target constructed
 *                                 // version
 * }
 * @endcode
 * 
 * @tparam Validator Validation checker type, see specialisation documentation
 * for more info
 */
template <typename Validator, bool>
class pre_parse_data;

/** Without parse_target reference specialisation.
 *  
 * The @a Validator instance is called just before the args list is updated by
 * the <TT>pre_parse(..)</TT> method, and allows the caller to run a custom
 * verification the on the method's node and parents arguments. @a Validator
 * must be equivalent to:
 * @code
 * struct my_validator {
 *     template <typename Node, typename... Parents>
 *     bool operator()(const Node&, const Parents&...);
 * };
 * @endcode
 * If the validator returns true then the result is kept.
 * 
 * @tparam Validator Validation checker type
 */
template <typename Validator>
class pre_parse_data<Validator, false> :
    public pre_parse_data_base<Validator, false>
{
public:
    /** Constructor.
     *  
     * @param args Unprocessed tokens
     * @param validator Validator instance, defaults to always returning true
     */
    pre_parse_data(
        vector<parsing::token_type>& args,
        const Validator& validator = detail::always_returns_true{}) noexcept :
        pre_parse_data_base<Validator, false>{args, validator}
    {
    }
};

/** With parse_target reference specialisation.
 *  
 * The @a Validator instance is called just before the args list is updated by
 * the <TT>pre_parse(..)</TT> method, and allows the caller to run a custom
 * verification the on the method's node and parents arguments. @a Validator
 * must be equivalent to:
 * @code
 * struct my_validator {
 *     template <typename Node, typename... Parents>
 *     bool operator()(const Node&, const Parents&...);
 * };
 * @endcode
 * If the validator returns true then the result is kept.
 * 
 * @tparam Validator Validation checker
 */
template <typename Validator>
class pre_parse_data<Validator, true> :
    public pre_parse_data_base<Validator, true>
{
public:
    /** Constructor.
     * 
     * @param args Unprocessed tokens
     * @param target Processed parse target from parent
     * @param validator Validator instance, defaults to always returning true
     */
    pre_parse_data(
        vector<parsing::token_type>& args,
        const parse_target& target,
        const Validator& validator = detail::always_returns_true{}) noexcept :
        pre_parse_data_base<Validator, true>{args, validator},
        target_{target}
    {
    }

    /** Returns the processed parse target from parent reference.
     *
     * @return Processed parse target
     */
    [[nodiscard]] const parse_target& target() const noexcept
    {
        return target_;
    }

private:
    std::reference_wrapper<const parse_target> target_;
};

// Deduction guides
template <typename... T>
pre_parse_data(vector<parsing::token_type>&)
    -> pre_parse_data<detail::always_returns_true, false>;

template <typename T>
pre_parse_data(vector<parsing::token_type>&, const T&)
    -> pre_parse_data<T, false>;

template <typename... T>
pre_parse_data(vector<parsing::token_type>&, const parse_target&)
    -> pre_parse_data<detail::always_returns_true, true>;

template <typename T>
pre_parse_data(vector<parsing::token_type>&, const parse_target&, const T&)
    -> pre_parse_data<T, true>;
}  // namespace parsing
}  // namespace arg_router
