/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include <utility>

namespace arg_router
{
namespace utility
{
/** Compile-time equivalent of <TT>std::optional</TT>.
 *
 * <TT>std::optional</TT> can be used in compile-time expressions, but not when
 * used as a argument, this class can do that but comes with severe runtime
 * limitations due to it.
 * 
 * An empty optional cannot be created and then populated later, nor cleared,
 * as that state is a part of its template parameters (a <TT>void</TT> type is
 * an empty compile_time_optional).
 * 
 * Like <TT>std::optional</TT> references cannot be stored internally, but it
 * can use a <TT>std::reference_wrapper</TT>.  Unlike <TT>std::optional</TT>
 * this type has a specialisation that makes the <TT>std::reference_wrapper</TT>
 * access transparent (i.e. you don't need to dereference twice).
 * 
 * @note The <TT>std::reference_wrapper</TT> specialisation is not constexpr
 * 
 * @tparam T Value type
 */
template <typename T>
class compile_time_optional
{
public:
    /** Value type. */
    using value_type = T;

    /** True if empty. */
    static constexpr bool empty = false;

    /** Constructor.
     *
     * @param val Value to copy into the optional
     */
    constexpr compile_time_optional(value_type val) noexcept :
        val_(std::move(val))
    {
    }

    /** Implicit bool conversion operator.
     *
     * @return True if not empty
     */
    constexpr operator bool() const { return !empty; }

    /** Dereference to member operator.
     *
     * @note Not available if empty
     * @return Pointer
     */
    constexpr const value_type* operator->() const noexcept { return &val_; }

    /** Dereference operator.
     *
     * @note Not available if empty
     * @return Reference
     */
    constexpr const value_type& operator*() const noexcept { return val_; }

    /** Non-const overload.
     *
     * @note Not available if empty
     * @return Pointer
     */
    value_type* operator->() noexcept { return &val_; }

    /** Non-const overload.
     *
     * @note Not available if empty
     * @return Reference
     */
    value_type& operator*() noexcept { return val_; }

private:
    value_type val_;
};

template <typename T>
class compile_time_optional<std::reference_wrapper<T>>
{
public:
    using value_type = T;

    static constexpr bool empty = false;

    constexpr operator bool() const { return !empty; }

    compile_time_optional(std::reference_wrapper<T> val) noexcept : ref_(val) {}

    const value_type* operator->() const noexcept { return &(ref_.get()); }

    const value_type& operator*() const noexcept { return ref_; }

    value_type* operator->() noexcept { return &(ref_.get()); }

    value_type& operator*() noexcept { return ref_; }

private:
    std::reference_wrapper<value_type> ref_;
};

template <>
class compile_time_optional<void>
{
public:
    static constexpr bool empty = true;

    constexpr operator bool() const { return !empty; }
};

// Deduction guide
template <typename T = void>
compile_time_optional() -> compile_time_optional<void>;
}  // namespace utility
}  // namespace arg_router
