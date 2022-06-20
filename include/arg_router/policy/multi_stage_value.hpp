/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/policy/policy.hpp"

#include <functional>
#include <optional>

namespace arg_router
{
namespace policy
{
/** Indicates how a node's parsed value should be merged in some way with the
 * parent's result value.
 *
 * Node's should derive from this when they appear multiple times
 * non-contiguously in the command line.  The parent mode-like type will merge,
 * in a user defined way, the values returned by each into a single result that
 * will be passed to the routing phase.
 * 
 * An example of this is the counting_flag, it parses a bool but that needs to
 * be added to a total count maintained by the parent.  This is achieved by
 * having the node's <TT>value_type</TT> set to std::size_t, the parse return
 * type a bool, and this type's merge function increment the total count.
 * @tparam ResultType Node's value_type
 * @tparam ValueType Type returned from the node's parse function
 */
template <typename ResultType, typename ValueType>
class multi_stage_value
{
public:
    /** Merge Callable type. */
    using merge_fn =
        std::function<void(std::optional<ResultType>&, ValueType&&)>;

    /** Constructor.
     *
     * @param fn Merge function object
     */
    constexpr explicit multi_stage_value(merge_fn fn) noexcept :
        fn_{std::move(fn)}
    {
    }

    /** Public API to merge the parsed value into the parent's result value.
     *
     * @param result Parent's result value.  If empty, will need initialising
     * @param value Parsed value
     * @return Void
     */
    constexpr void merge(std::optional<ResultType>& result,
                         ValueType&& value) const
    {
        fn_(result, std::forward<ValueType>(value));
    }

private:
    merge_fn fn_;
};

template <typename ResultType, typename ValueType>
struct is_policy<multi_stage_value<ResultType, ValueType>> : std::true_type {
};

/** Evaulates to true if @a T derives from a multi_stage_value specialisation.
 *
 * @tparam T Type to test
 */
template <typename T>
struct has_multi_stage_value {
private:
    template <typename... Ts>
    constexpr static std::true_type test(const multi_stage_value<Ts...>*);

    constexpr static std::false_type test(...);

public:
    constexpr static bool value = decltype(test(std::declval<T*>()))::value;
};

/** Helper variable for has_multi_stage_value.
 *
 * @tparam T Type to test
 */
template <typename T>
constexpr bool has_multi_stage_value_v = has_multi_stage_value<T>::value;
}  // namespace policy
}  // namespace arg_router
