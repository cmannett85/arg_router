// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/traits.hpp"

namespace arg_router::utility
{
namespace detail
{
template <typename Fn, typename Tuple, std::size_t... I>
auto apply_impl(Fn&& f, Tuple&& t, [[maybe_unused]] std::integer_sequence<std::size_t, I...> i)
{
    return f(std::get<I>(std::forward<Tuple>(t))...);
}
}  // namespace detail

template <typename Fn, typename Tuple>
auto apply(Fn&& f, Tuple&& t)
{
    static_assert(traits::is_tuple_like_v<std::decay_t<Tuple>>, "Tuple must be a tuple-like type");

    return detail::apply_impl(std::forward<Fn>(f),
                              std::forward<Tuple>(t),
                              std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
}
}  // namespace arg_router::utility
