// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/exception.hpp"

#include <variant>

namespace arg_router::utility
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
    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    constexpr result(result_type value) noexcept : data_{std::move(value)} {}

    /** Exception constructor.
     *
     * @param ex Exception
     */
    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    constexpr result(ExceptionType ex) noexcept : data_{std::move(ex)} {}

    /** True if this instance holds a result.
     *
     * @return True if result
     */
    [[nodiscard]] constexpr bool has_result() const noexcept { return data_.index() == 0; }

    /** True if this instance holds an exception.
     *
     * @return True if exception
     */
    [[nodiscard]] constexpr bool has_error() const noexcept { return !has_result(); }

    /** True if this instance holds a result.
     *
     * @return True if result
     */
    [[nodiscard]] explicit constexpr operator bool() const noexcept { return has_result(); }

    /** True if this instance holds an exception.
     *
     * @return True if exception
     */
    [[nodiscard]] constexpr bool operator!() const noexcept { return has_error(); }

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
    [[nodiscard]] friend constexpr bool operator==(const result& lhs,
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
    [[nodiscard]] friend constexpr bool operator!=(const result& lhs,
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
     * As this method moves the value or exception out of the instance, it should not be called
     * again unless you know the value or exception is not move-constructible.
     * @return Result
     * @exception ExceptionType Thrown if the instance holds an exception
     */
    result_type extract()
    {
        if (has_result()) {
            return std::move(std::get<0>(data_));
        }
        throw std::move(std::get<1>(data_));
    }

    /** Returns the the result of this instance, or throw the exception if one is held.
     *
     * @return Result Either a copy of the result, or a const reference to it if the object is
     * larger than a word and trivially constructible
     * @exception ExceptionType Thrown if the instance holds an exception
     */
    [[nodiscard]] auto get() const
        -> std::conditional_t<(sizeof(std::size_t) >= sizeof(result_type)) &&
                                  std::is_copy_constructible_v<result_type>,
                              result_type,
                              const result_type&>
    {
        if (has_result()) {
            return std::get<0>(data_);
        }
        throw exception_type{std::get<1>(data_)};
    }

    /** Throws the exception if present, otherwise does nothing.
     *
     * @exception ExceptionType Thrown if the instance holds an exception
     */
    void throw_exception() const
    {
        if (has_error()) {
            throw exception_type{std::get<1>(data_)};
        }
    }

private:
    std::variant<result_type, exception_type> data_;
};
}  // namespace arg_router::utility
