/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/policy/policy.hpp"
#include "arg_router/token_type.hpp"

#include <functional>
#include <string_view>

namespace arg_router
{
namespace policy
{
/** Provides the ability for an argument to have a user-provided value parser.
 * 
 * @tparam T Argument's value type i.e. the type the parser needs to return
 */
template <typename T>
class custom_parser
{
public:
    /** Alias of @a T. */
    using value_type = T;

    /** Parser signature. */
    using parser_type = std::function<value_type(std::string_view)>;

    /** Constructor.
     *
     * @param p Custom parser
     */
    constexpr explicit custom_parser(parser_type p) noexcept :
        parser_{std::move(p)}
    {
    }

    /** Parse @a str to produce an equivalent instance of value_type.
     *
     * @tparam ValueType Parsed value type, must be implicity convertible from
     * value_type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param token Token to parse
     * @param parents Parents instances pack
     * @return Parsed value
     * @exception parse_exception Thrown if parsing failed
     */
    template <typename ValueType, typename... Parents>
    [[nodiscard]] constexpr ValueType parse_phase(
        std::string_view token,
        [[maybe_unused]] const Parents&... parents) const
    {
        return parser_(token);
    }

private:
    parser_type parser_;
};

template <typename... Args>
struct is_policy<custom_parser<Args...>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
