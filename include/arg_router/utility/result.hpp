/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/exception.hpp"

#include <variant>

namespace arg_router
{
namespace utility
{
/** Result or exception wrapper.
 *  
 * @tparam ValueType Result type
 * @tparam ExceptionType Exception type
 */
template <typename ResultType, typename ExceptionType>
class result
{
    static_assert(!std::is_same_v<ResultType, ExceptionType>,
                  "Result and exception argument cannot be same type");

public:
    /** Result type. */
    using result_type = std::decay_t<ResultType>;
    /** Exception type. */
    using exception_type = ExceptionType;

    /** Result constructor.
     *
     * @param value Result value
     */
    constexpr result(result_type value) noexcept : data_{std::move(value)} {}

    /** Exception constructor.
     *
     * @param ex Exception
     */
    constexpr result(ExceptionType ex) noexcept : data_{std::move(ex)} {}

    /** True if this instance holds a result.
     *
     * @return True if result
     */
    [[nodiscard]] constexpr bool has_result() const noexcept
    {
        return data_.index() == 0;
    }

    /** True if this instance holds an exception.
     *
     * @return True if exception
     */
    [[nodiscard]] constexpr bool has_error() const noexcept
    {
        return !has_result();
    }

    /** True if this instance holds a result.
     *
     * @return True if result
     */
    [[nodiscard]] constexpr operator bool() const noexcept
    {
        return has_result();
    }

    /** True if this instance holds an exception.
     *
     * @return True if exception
     */
    [[nodiscard]] constexpr bool operator!() const noexcept
    {
        return has_error();
    }

    /** Equality operator
     *
     * @param other Instance to compare against
     * @return True if the same value, false if either carry exceptions
     */
    [[nodiscard]] constexpr bool operator==(const result& other) const noexcept
    {
        if (has_error() || other.has_error()) {
            return false;
        }
        return std::get<0>(data_) == std::get<0>(other.data_);
    }

    /** Inequality operator
     *
     * @param other Instance to compare against
     * @return True if not the same value, true if either carry exceptions
     */
    [[nodiscard]] constexpr bool operator!=(const result& other) const noexcept
    {
        return !(*this == other);
    }

    /** Result type equality operator
     *
     * @param lhs Instance to compare against
     * @param rhs Value instance to compare against
     * @return True if the same value, false if either carry exceptions
     */
    [[nodiscard]] friend constexpr bool operator==(
        const result& lhs,
        const result_type& rhs) noexcept
    {
        if (lhs.has_error()) {
            return false;
        }
        return std::get<0>(lhs.data_) == rhs;
    }

    /** Result type inequality operator
     *
     * @param lhs Instance to compare against
     * @param rhs Value instance to compare against
     * @return True if not the same value, false if either carry exceptions
     */
    [[nodiscard]] friend constexpr bool operator!=(
        const result& lhs,
        const result_type& rhs) noexcept
    {
        return !(lhs == rhs);
    }

    /** Result type equality operator
     *
     * @param lhs Value Instance to compare against
     * @param rhs Instance to compare against
     * @return True if the same value, false if either carry exceptions
     */
    [[nodiscard]] friend constexpr bool operator==(const result_type& lhs,
                                                   const result& rhs) noexcept
    {
        return rhs == lhs;
    }

    /** Result type inequality operator
     *
     * @param lhs Instance to compare against
     * @param rhs Value instance to compare against
     * @return True if not the same value, false if either carry exceptions
     */
    [[nodiscard]] friend constexpr bool operator!=(const result_type& lhs,
                                                   const result& rhs) noexcept
    {
        return !(lhs == rhs);
    }

    /** Returns a pointer to the result, or nullptr if this instance holds an
     * exception.
     *
     * @return Pointer to the result, or nullptr
     */
    [[nodiscard]] result_type* get_if() noexcept
    {
        return has_result() ? &std::get<0>(data_) : nullptr;
    }

    /** Const overload
     *
     * @return Pointer to the result, or nullptr
     */
    [[nodiscard]] const result_type* get_if() const noexcept
    {
        return has_result() ? &std::get<0>(data_) : nullptr;
    }

    /** Move the result of this instance, or throw the exception if one is held.
     *
     * As this method moves the value out of the instance, it should
     * not be called again unless you know the value is not move-constructible.
     * @return Result
     * @exception ExceptionType Thrown if the instance holds an exception
     */
    result_type extract()
    {
        if (has_result()) {
            return std::move(std::get<0>(data_));
        }
        throw std::get<1>(data_);
    }

    /** Const overload.
     *
     * As this is a const-overload, the value is copied out of the instance,
     * so this method may be called more than once.
     * @return Result Either a copy of the result, or a const reference to it if
     * the object is too big for the L1 cache
     * @exception ExceptionType Thrown if the instance holds an exception
     */
    auto extract() const
        -> std::conditional_t<config::l1_cache_size() >= sizeof(result_type),
                              result_type,
                              const result_type&>
    {
        if (has_result()) {
            return std::get<0>(data_);
        }
        throw std::get<1>(data_);
    }

    /** Throws the exception if present, otherwise does nothing.
     *
     * @exception ExceptionType Thrown if the instance holds an exception
     */
    void throw_exception() const
    {
        if (has_error()) {
            throw std::get<1>(data_);
        }
    }

private:
    std::variant<result_type, exception_type> data_;
};
}  // namespace utility
}  // namespace arg_router
