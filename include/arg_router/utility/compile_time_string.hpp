#pragma once

#include "arg_router/traits.hpp"

#include <boost/mp11/algorithm.hpp>
#include <boost/preprocessor/config/limits.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>

#include <array>

namespace arg_router
{
namespace utility
{
/** Compile time string.
 *
 * @tparam Cs Pack of chars
 */
template <char... Cs>
class compile_time_string
{
public:
    /** Array of characters as a type. */
    using array_type = std::tuple<traits::integral_constant<Cs>...>;

    /** Returns the string data as a view.
     *
     * @return View of the string data
     */
    constexpr static std::string_view get() { return {sv_.data(), sv_.size()}; }

private:
    constexpr static auto sv_ = std::array<char, sizeof...(Cs)>{Cs...};
};

namespace cts_detail
{
template <int N>
constexpr char get(const char (&str)[N], std::size_t i)
{
    return i < N ? str[i] : '\0';
}

constexpr char get(std::string_view str, std::size_t i)
{
    return i < str.size() ? str[i] : '\0';
}

// Required so that the extra nulls S_ adds can be removed before defining the
// compile_time_string.  Otherwise any compiler warnings you hit are just walls
// of null template args...
template <char... Cs>
struct builder {
    struct is_null_char {
        template <typename T>
        using fn = std::is_same<std::integral_constant<char, '\0'>, T>;
    };

    using strip_null =
        boost::mp11::mp_remove_if_q<boost::mp11::mp_list_c<char, Cs...>,
                                    is_null_char>;

    template <char... StrippedCs>
    static auto list_to_string(boost::mp11::mp_list_c<char, StrippedCs...>)
    {
        return compile_time_string<StrippedCs...>{};
    }

    using type = decltype(list_to_string(strip_null{}));
};

#define AR_STR_CHAR(z, n, tok) \
    BOOST_PP_COMMA_IF(n) arg_router::utility::cts_detail::get(tok, n)

#define AR_STR_N(n, tok)                               \
    typename arg_router::utility::cts_detail::builder< \
        BOOST_PP_REPEAT(n, AR_STR_CHAR, tok)>::type
}  // namespace cts_detail
}  // namespace utility
}  // namespace arg_router

/** Macro that represents the type of a compile-time string, useful for policies
 * that require a compile string.
 *
 * There is no requirement to use this, it just makes definitions easier to
 * read.
 * @note The size limit is set by using the AR_MAX_CTS_SIZE define (defaults to
 * 128).  Increasing this will not increase the size of your program, but will
 * increase build time as the preprocessor and compiler have to do more work
 */
#define S_(tok) AR_STR_N(AR_MAX_CTS_SIZE, tok)
