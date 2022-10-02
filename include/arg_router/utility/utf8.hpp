/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/utility/utf8/double_width.hpp"
#include "arg_router/utility/utf8/grapheme_cluster_break.hpp"
#include "arg_router/utility/utf8/line_break.hpp"
#include "arg_router/utility/utf8/whitespace.hpp"
#include "arg_router/utility/utf8/zero_width.hpp"

#include <string_view>

/** Namespace for UTF-8 encoded string functions. */
namespace arg_router::utility::utf8
{
namespace detail
{
template <std::size_t N>
[[nodiscard]] constexpr std::optional<code_point::range> find_range(
    const std::array<code_point::range, N>& table,
    code_point::type cp) noexcept
{
    auto first = table.begin();

    auto len = N;
    while (len > 0) {
        const auto step = len / 2;
        auto halfway = first + step;

        if (cp < halfway->first()) {
            len = step;
        } else if (cp <= halfway->last()) {
            return *halfway;
        } else {
            first = ++halfway;
            len -= step + 1;
        }
    }

    return {};
}
}  // namespace detail

/** Forward iterator for a string's grapheme clusters a.k.a. user-perceived characters.
 */
class iterator
{
public:
    /** Difference in bytes between two iterator positions. */
    using difference_type = std::string_view::difference_type;
    /** Dereferenced type. */
    using value_type = std::string_view;
    /** Pointer type. */
    using pointer = const value_type*;
    /** Reference type/ */
    using reference = const value_type&;
    /** Iterator category. */
    using iterator_category = std::forward_iterator_tag;

    /** A simple wrapper for a string view that allows code point iteration in range-for loops.
     *
     * @code
     * for (auto cp : iterator::range(str)) {
     *     // Do stuff
     * }
     * @endcode
     */
    class range_t
    {
    public:
        friend class iterator;

        /** Returns a start iterator.
         *
         * @return Start iterator
         */
        [[nodiscard]] constexpr iterator begin() noexcept { return iterator{str_}; }

        /** Returns an end iterator.
         *
         * @return End iterator
         */
        [[nodiscard]] constexpr static iterator end() noexcept { return iterator{}; }

    private:
        constexpr explicit range_t(std::string_view str) noexcept : str_{str} {}

        std::string_view str_;
    };

    /** Returns an object that can be used in range-for loops.
     *
     * @param str Input string
     * @return Range object
     */
    [[nodiscard]] constexpr static range_t range(std::string_view str) noexcept
    {
        return range_t{str};
    }

    /** Default constructor.
     *
     * Represents the end iterator.
     */
    constexpr iterator() noexcept : current_{0}, trailing_window_{} {}

    /** Constructor.
     *
     * If @a str is empty, then this will create an end iterator.
     * @param str String to iterator over.
     */
    constexpr explicit iterator(std::string_view str) noexcept :
        current_{0}, str_{str}, trailing_window_{}
    {
        fill_trailing_window();
        update_current();
    }

    /** Equality operator.
     *
     * @param other Instance to compare against
     * @return True if the start and end positions are the same
     */
    [[nodiscard]] constexpr bool operator==(iterator other) const noexcept
    {
        // If they are both empty, then they are considered both end iterators and therefore equal
        if (str_.empty() && other.str_.empty()) {
            return true;
        }
        return (str_.data() == other.str_.data()) && (str_.size() == other.str_.size());
    }

    /** Inequality operator.
     *
     * @param other Instance to compare against
     * @return False if the start and end positions are the same
     */
    [[nodiscard]] constexpr bool operator!=(iterator other) const noexcept
    {
        return !(*this == other);
    }

    /** Dereference operator.
     *
     * @return Grapheme cluster
     */
    [[nodiscard]] constexpr value_type operator*() const noexcept
    {
        return str_.substr(0, current_);
    }

    /** Pre-increment the iterator by one code point.
     *
     * @return Reference to this after moving the iterator
     */
    constexpr iterator& operator++() noexcept
    {
        // Remove the leading cluster
        str_.remove_prefix(current_);

        // Find the end of the new one
        update_current();

        return *this;
    }

    /** Post-increment the iterator by one code point.
     *
     * @return Copy of this before moving the iterator
     */
    constexpr iterator operator++(int) noexcept
    {
        auto result = *this;

        ++(*this);
        return result;
    }

private:
    constexpr static auto trailing_window_size = std::size_t{AR_UTF8_TRAILING_WINDOW_SIZE};

    [[nodiscard]] constexpr static grapheme_cluster_break_class extract_class(
        code_point::type cp) noexcept
    {
        auto result = grapheme_cluster_break_class::any;
        const auto range = detail::find_range(grapheme_cluster_break_table, cp);
        if (range) {
            result = static_cast<grapheme_cluster_break_class>(range->meta());
        }

        return result;
    }

    [[nodiscard]] constexpr bool should_break(grapheme_cluster_break_class next_class) noexcept
    {
        for (auto rule : no_break_rules::grapheme_cluster<trailing_window_size>) {
            if (rule(trailing_window_, next_class)) {
                return false;
            }
        }

        return true;
    }

    constexpr void fill_trailing_window() noexcept
    {
        // array::fill(..) is only constexpr in C++20
        for (auto& p : trailing_window_) {
            p = grapheme_cluster_break_class::any;
        }
    }

    constexpr void rotate_trailing_window() noexcept
    {
        // Right rotate the window, std::rotate isn't constexpr in C++17
        auto it = trailing_window_.rbegin();
        while (true) {
            auto prev = it++;
            if (it == trailing_window_.rend()) {
                break;
            }
            *prev = *it;
        }
    }

    constexpr void update_current() noexcept
    {
        current_ = 0;
        if (str_.empty()) {
            return;
        }

        // Iterate over each code point and its neighbour, pass their break classes into the rule
        // checker
        for (auto it = code_point::iterator{str_}; it != code_point::iterator{}; ++it) {
            current_ += code_point::size(*it);

            // If this code point is malformed, then just skip it
            const auto this_cp = code_point::decode(*it);
            if (!this_cp) {
                continue;
            }

            rotate_trailing_window();
            trailing_window_.front() = extract_class(*this_cp);

            // If there is a following code point, use it to update the next_break_class
            auto next_break_class = grapheme_cluster_break_class::any;
            {
                auto next_it = it;
                ++next_it;
                if (next_it != code_point::iterator{}) {
                    const auto next_cp = code_point::decode(*next_it);
                    if (next_cp) {
                        next_break_class = extract_class(*next_cp);
                    }
                }
            }

            if (should_break(next_break_class)) {
                break;
            }
        }
    }

    std::size_t current_;
    std::string_view str_;
    std::array<grapheme_cluster_break_class, trailing_window_size> trailing_window_;
};

/** Number of UTF-8 grapheme clusters in the string.
 *
 * @param str Input string
 * @return Number of grapheme clusters
 */
[[nodiscard]] inline constexpr std::size_t count(std::string_view str) noexcept
{
    return std::distance(iterator(str), iterator());
}

/** True if the leading code point of @a str is one of the known whitespace characters.
 *
 * @param str Code point
 * @return True if whitespace, also false if @a str is empty or there are not enough bytes in @a str
 * to read the entire code point
 */
[[nodiscard]] inline constexpr bool is_whitespace(std::string_view str) noexcept
{
    const auto cp = code_point::decode(str);
    if (!cp) {
        return false;
    }

    return !!detail::find_range(whitespace_table, *cp);
}

/** Returns true if @a str contains whitespace.
 *
 * @param str Input string
 * @return True if whitespace is present
 */
[[nodiscard]] inline constexpr bool contains_whitespace(std::string_view str) noexcept
{
    for (auto cp : code_point::iterator::range(str)) {
        if (is_whitespace(cp)) {
            return true;
        }
    }

    return false;
}

/** Returns the terminal width (i.e. number columns) required by @a str.
 *
 * This is equivalent to https://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c, but constexpr.
 * @param str Input string
 * @return Terminal width
 */
[[nodiscard]] inline constexpr std::size_t terminal_width(std::string_view str) noexcept
{
    auto width = std::size_t{0};
    for (auto cp_str : code_point::iterator::range(str)) {
        const auto cp = code_point::decode(cp_str);
        if (!cp) {
            continue;
        }

        if (detail::find_range(double_width_table, *cp)) {
            width += 2;
        } else if (!detail::find_range(zero_width_table, *cp)) {
            width += 1;
        }
    }

    return width;
}

/** Iterates over a string view, dereferencing to a substring that is equal or less than the max
 * terminal column count, breaking on whitespace.
 */
class line_iterator
{
public:
    /** Difference in bytes between two iterator positions. */
    using difference_type = std::string_view::difference_type;
    /** Dereferenced type. */
    using value_type = std::string_view;
    /** Pointer type. */
    using pointer = const value_type*;
    /** Reference type/ */
    using reference = const value_type&;
    /** Iterator category. */
    using iterator_category = std::forward_iterator_tag;

    /** Default constructor.
     *
     * Represents the end iterator.
     */
    constexpr line_iterator() noexcept : max_columns_{0}, line_break_byte_{0}, trailing_window_{} {}

    /** Constructor.
     *
     * @note If max_columns is zero then @a str is emptied making this iterator equivalent to
     * the end iterator - this is to prevent infinite loops
     * @param str Input string
     * @param max_columns Maximum width of each iteration in terminal columns
     */
    constexpr explicit line_iterator(std::string_view str, std::size_t max_columns) noexcept :
        str_{str}, max_columns_{max_columns}, line_break_byte_{0}, trailing_window_{}
    {
        if (max_columns_ == 0) {
            str_ = std::string_view{};
        }

        fill_trailing_window();
        consume();
    }

    /** Returns the maximum terminal column count passed to the constructor.
     *
     * @return Max column count
     */
    [[nodiscard]] constexpr std::size_t max_columns() const noexcept { return max_columns_; }

    /** Equality operator.
     *
     * @param other Instance to compare against
     * @return True if equal
     */
    [[nodiscard]] constexpr bool operator==(line_iterator other) const noexcept
    {
        // If they are both empty, then they are considered both end iterators and therefore equal
        if (str_.empty() && other.str_.empty()) {
            return true;
        }
        return (str_.data() == other.str_.data()) && (str_.size() == other.str_.size()) &&
               (max_columns_ == other.max_columns_) && (line_break_byte_ == other.line_break_byte_);
    }

    /** Inequality operator.
     *
     * @param other Instance to compare against
     * @return False if equal
     */
    [[nodiscard]] constexpr bool operator!=(line_iterator other) const noexcept
    {
        return !(*this == other);
    }

    /** Dereference operator.
     *
     * @return Whole line
     */
    [[nodiscard]] constexpr value_type operator*() const noexcept
    {
        return str_.substr(0, line_break_byte_);
    }

    constexpr line_iterator& operator++() noexcept
    {
        consume();
        return *this;
    }

    /** Post-increment the iterator by line.
     *
     * @return Copy of this before moving the iterator
     */
    constexpr line_iterator operator++(int) noexcept
    {
        auto result = *this;

        ++(*this);
        return result;
    }

private:
    constexpr static auto trailing_window_size = std::size_t{AR_UTF8_TRAILING_WINDOW_SIZE};

    [[nodiscard]] constexpr static line_break_class extract_class(code_point::type cp) noexcept
    {
        auto result = line_break_class::any;
        const auto range = detail::find_range(line_break_table, cp);
        if (range) {
            result = static_cast<line_break_class>(range->meta());
        }

        return result;
    }

    [[nodiscard]] constexpr bool should_break(line_break_class next_class) noexcept
    {
        for (auto rule : no_break_rules::line_break<trailing_window_size>) {
            if (rule(trailing_window_, next_class)) {
                return false;
            }
        }

        return true;
    }

    constexpr void fill_trailing_window() noexcept
    {
        // array::fill(..) is only constexpr in C++20
        for (auto& p : trailing_window_) {
            p = line_break_class::any;
        }
    }

    constexpr void rotate_trailing_window() noexcept
    {
        // Right rotate the window, std::rotate isn't constexpr in C++17
        auto it = trailing_window_.rbegin();
        while (true) {
            auto prev = it++;
            if (it == trailing_window_.rend()) {
                break;
            }
            *prev = *it;
        }
    }

    constexpr void consume() noexcept
    {
        if (str_.empty()) {
            return;
        }

        // Start by consuming the previous line
        if (line_break_byte_ != 0) {
            str_.remove_prefix(line_break_byte_);
            line_break_byte_ = 0;

            // Consuming any leading whitespace
            auto bytes = std::size_t{0};
            for (auto cp : code_point::iterator::range(str_)) {
                if (!is_whitespace(cp)) {
                    break;
                }
                bytes += code_point::size(cp);
            }
            str_.remove_prefix(bytes);

            if (str_.empty()) {
                return;
            }
        }

        // Iterate over the code points until you reach or exceed the column limit, setting the
        // break markers at each whitespace code point
        auto column = std::size_t{0};
        auto bytes = std::size_t{0};
        auto line_break_column = std::size_t{0};
        auto line_break_byte = std::size_t{0};
        for (auto it = code_point::iterator{str_}; it != code_point::iterator{}; ++it) {
            // Have we exceeded the terminal width?
            column += terminal_width(*it);
            if (column > max_columns_) {
                if (line_break_column == 0) {
                    // Oh dear, the line has no whitespace in it to break on, so force break on the
                    // last code point
                    line_break_byte_ = bytes;
                } else {
                    line_break_byte_ = line_break_byte;
                }
                return;
            }

            bytes += code_point::size(*it);

            // If this code point is malformed, then just skip it
            const auto this_cp = code_point::decode(*it);
            if (!this_cp) {
                continue;
            }

            rotate_trailing_window();
            trailing_window_.front() = extract_class(*this_cp);

            // If there is a following code point, use it to update the next_break_class
            auto next_break_class = line_break_class::any;
            {
                auto next_it = it;
                ++next_it;
                if (next_it != code_point::iterator{}) {
                    const auto next_cp = code_point::decode(*next_it);
                    if (next_cp) {
                        next_break_class = extract_class(*next_cp);
                    }
                }
            }

            if (should_break(next_break_class)) {
                line_break_column = column;
                line_break_byte = bytes;
            }
        }

        // We haven't hit the max column, so consume the whole string
        line_break_byte_ = str_.size();
    }

    std::string_view str_;
    std::size_t max_columns_;
    std::size_t line_break_byte_;
    std::array<line_break_class, trailing_window_size> trailing_window_;
};
}  // namespace arg_router::utility::utf8
