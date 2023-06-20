// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/basic_types.hpp"

#include <string_view>

namespace arg_router::utility
{
/** A view that can own the memory pointed at by the view if required.
 *
 * This is a view until the user needs to modify the string, at which point the string data is
 * copied into internal storage.
 */
// NOLINTNEXTLINE(hicpp-special-member-functions)
class dynamic_string_view
{
public:
    using value_type = string::value_type;                    ///<! Value type
    using allocator_type = string::allocator_type;            ///<! Allocator type
    using size_type = string::size_type;                      ///<! Size type
    using difference_type = string::difference_type;          ///<! Difference type
    using const_reference = string::const_reference;          ///<! Reference type
    using const_pointer = string::const_pointer;              ///<! Pointer type
    using const_iterator = std::string_view::const_iterator;  ///<! Iterator type

    /** Default constructor.
     *
     * View is assigned from @a sv and internal storage is empty.
     * @param sv Initial view data, or empty
     */
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    dynamic_string_view(std::string_view sv = {}) noexcept : view_{sv} {}

    /** String literal constructor.
     *
     * View is assigned from @a str and internal storage is empty.
     * @param str Initial view data, or empty
     */
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    dynamic_string_view(const char* str) noexcept : view_{str} {}

    /** String move constructor.
     *
     * @a str is moved into internal storage.  View is set to point to the internal storage.
     * @param str Initial string data
     */
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    dynamic_string_view(string&& str) noexcept : str_{std::move(str)} { update_view(); }

    /** Copy constructor.
     *
     * If @a other is just a view, this instance will be duplicate.  Otherwise the internal storage
     * is copied too and the view updated to match
     * @param other Instance to copy from
     */
    dynamic_string_view(const dynamic_string_view& other) : view_{other.view_}, str_{other.str_}
    {
        if (!str_.empty()) {
            update_view();
        }
    }

    /** Move constructor.
     *
     * @param other Instance to move from
     */
    dynamic_string_view(dynamic_string_view&& other) noexcept : dynamic_string_view{}
    {
        swap(*this, other);
    }

    /** Assignment operator.
     *
     * @param other Instance to copy or move from
     * @return Reference to this
     */
    dynamic_string_view& operator=(dynamic_string_view other)
    {
        swap(*this, other);
        return *this;
    }

    /** Copy the view data into internal storage and update the view to match.
     *
     * Does nothing if already in internal storage.
     */
    void convert_to_internal_storage()
    {
        if (str_.empty()) {
            str_ = view_;
            update_view();
        }
    }

    /** Implicit conversion operator for std::string_view.
     *
     * @return The view
     */
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    [[nodiscard]] operator std::string_view() const noexcept { return view_; }

    /** Equality operator.
     *
     * @param other Instance to compare against
     * @return True if the same
     */
    [[nodiscard]] bool operator==(const dynamic_string_view& other) const noexcept
    {
        return view_ == other.view_;
    }

    /** Inequality operator.
     *
     * @param other Instance to compare against
     * @return True if the same
     */
    [[nodiscard]] bool operator!=(const dynamic_string_view& other) const noexcept
    {
        return !(*this == other);
    }

    /** In-place concatenation operator.
     *
     * @tparam T Type of @a other
     * @param other String-like instance to append
     * @return Reference to this
     */
    template <typename T,
              typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, dynamic_string_view>>>
    dynamic_string_view& operator+=(T&& other)
    {
        convert_to_internal_storage();
        str_ += std::forward<T>(other);
        update_view();

        return *this;
    }

    dynamic_string_view& operator+=(const dynamic_string_view& other)
    {
        convert_to_internal_storage();
        str_ += other.view_;
        update_view();

        return *this;
    }

    /** @return Number of characters */
    [[nodiscard]] size_type size() const noexcept { return view_.size(); }
    /** @return Number of characters in internal storage */
    [[nodiscard]] size_type internal_storage_size() const noexcept { return str_.size(); }
    /** @return True if the view is empty */
    [[nodiscard]] bool empty() const noexcept { return view_.empty(); }

    /** @return Iterator to beginning of the view */
    [[nodiscard]] const_iterator begin() const noexcept { return view_.begin(); }
    /** @return Iterator to one-past-the-end of the view */
    [[nodiscard]] const_iterator end() const noexcept { return view_.end(); }

    /** Swaps @a a and @a b.
     *
     * @param a First instance
     * @param b Second instance
     */
    friend void swap(dynamic_string_view& a, dynamic_string_view& b)
    {
        using std::swap;

        swap(a.view_, b.view_);
        swap(a.str_, b.str_);

        if (!a.str_.empty()) {
            a.update_view();
        }
        if (!b.str_.empty()) {
            b.update_view();
        }
    }

private:
    void update_view() { view_ = str_; }

    std::string_view view_;
    string str_;
};

/** Textual streaming operator.
 *
 * @param stream Output stream
 * @param dsv Instance
 * @return @a stream
 */
inline std::ostream& operator<<(std::ostream& stream, const dynamic_string_view& dsv)
{
    return stream << static_cast<std::string_view>(dsv);
}

/** Concatenation operator.
 *
 * Copies @a a and then @a b into internal storage and updates the view to match.
 * @tparam T Type of @a a
 * @param a dynamic_string_view instance
 * @param b String-like type
 * @return dynamic_string_view consisting of @a a with @a b appended
 */
template <typename T>
[[nodiscard]] inline dynamic_string_view operator+(dynamic_string_view a, T&& b)
{
    a += std::forward<T>(b);
    return a;
}
}  // namespace arg_router::utility
