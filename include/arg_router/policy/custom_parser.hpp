#pragma once

#include "arg_router/policy/policy.hpp"

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

    /** Parse @a str to produce a matching instance of @a T.
     *
     * @param str String to parse
     * @return Representation of @a str
     * @exception std::invalid_argument Should be thrown in case of parse
     * failure
     */
    value_type parse(std::string_view str) const { return parser_(str); }

private:
    parser_type parser_;
};

template <typename... Args>
struct is_policy<custom_parser<Args...>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
