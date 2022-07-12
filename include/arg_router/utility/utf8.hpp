/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/utility/utf8_tables/double_width.hpp"
#include "arg_router/utility/utf8_tables/whitespace.hpp"
#include "arg_router/utility/utf8_tables/zero_width.hpp"

namespace arg_router
{
namespace utility
{
/** Namespace for UTF-8 encoded string functions.
 *
 * @note These are not true UTF-8 algorithms as they make the assumption that 1 code point is equal
 * to one user-perceived character (a.k.a. grapheme cluster) - this is @b not always true!  For true
 * UTF-8 support, you must link against ICU
 */
namespace utf8
{
namespace detail
{
template <std::size_t N>
[[nodiscard]] constexpr std::optional<code_point_range> find_range(
    const std::array<code_point_range, N>& table,
    code_point cp) noexcept
{
    auto first = table.begin();
    [[maybe_unused]] auto last = table.end();

    auto len = N;
    while (len > 0) {
        const auto step = len / 2;
        auto halfway = first + step;

        if (cp < halfway->first) {
            last = halfway;
            len = step;
        } else if (cp <= halfway->last) {
            return *halfway;
        } else {
            first = ++halfway;
            len -= step + 1;
        }
    }

    return {};
}
}  // namespace detail

/** Number of UTF-8 code points in the string.
 *
 * @note This is not the same as the number of grapheme clusters in @a str
 * @param str Input string
 * @return Number of code points
 */
[[nodiscard]] inline constexpr std::size_t num_code_points(std::string_view str) noexcept
{
    auto result = std::size_t{0};
    for (auto c : str) {
        result += (c & 0xC0) != 0x80;
    }

    return result;
}

/** Returns the size in bytes for the leading code point of @a str
 *
 * @param str Input string
 * @return Size in bytes, zero if @a str is empty
 */
[[nodiscard]] inline constexpr std::size_t code_point_size(std::string_view str) noexcept
{
    if (str.empty()) {
        return 0;
    }

    // The MSB determines if it's ASCII or UTF-8
    const auto first_byte = static_cast<std::uint8_t>(str[0]);
    if ((first_byte & 0b1000'0000) == 0) {
        // ASCII
        return 1;
    }

    // Can't think of a way to do this without branches
    if (first_byte < 0b1101'1111) {
        return 2;
    } else if (first_byte < 0b1110'1111) {
        return 3;
    }

    return 4;
}

/** Converts the code point @a str into the underlying numerical representation.
 *
 * Only up to the first four bytes of @a str are read.  Undefined behaviour if @a str is empty or
 * malformed.
 * @param str Code point
 * @return Number, or empty optional if @a str is empty or there are not enough bytes in @a str to
 * read the entire code point
 */
[[nodiscard]] inline constexpr std::optional<code_point> code_point_to_number(
    std::string_view str) noexcept
{
    const auto bytes_to_read = code_point_size(str);
    if (bytes_to_read == 0) {
        return {};
    }

    if (bytes_to_read == 1) {
        // ASCII
        return str[0];
    }

    if (str.size() < bytes_to_read) {
        return {};
    }

    constexpr auto subsequent_byte_mask = code_point{0b0011'1111};
    const auto first_byte = static_cast<std::uint8_t>(str[0]);

    auto result = first_byte & (0b0001'1111 >> (bytes_to_read - 2));
    for (auto i = 1u; i < bytes_to_read; ++i) {
        const auto subsequent_byte = static_cast<code_point>(str[i]);

        // Move the previous reads up to make space for the subsequent byte's data
        result <<= 6;
        result |= subsequent_byte & subsequent_byte_mask;
    }

    return result;
}

/** Forward iterator for a string's code points.
 */
class code_point_iterator
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
    constexpr code_point_iterator() noexcept = default;

    /** Constructor.
     *
     * If @a str is empty, then this will create an end iterator.
     * @param str String to iterator over.
     */
    constexpr explicit code_point_iterator(std::string_view str) noexcept : str_{str} {}

    /** Equality operator.
     *
     * @param other Instance to compare against
     * @return True if the start and end positions are the same
     */
    [[nodiscard]] constexpr bool operator==(code_point_iterator other) const noexcept
    {
        // If they are both empty, then they are considered both end iterators and therefore equal
        if ((str_.size() == 0) && (other.str_.size() == 0)) {
            return true;
        }
        return (str_.data() == other.str_.data()) && (str_.size() == other.str_.size());
    }

    /** Inequality operator.
     *
     * @param other Instance to compare against
     * @return False if the start and end positions are the same
     */
    [[nodiscard]] constexpr bool operator!=(code_point_iterator other) const noexcept
    {
        return !(*this == other);
    }

    /** Dereference operator.
     * 
     * @return Code point
     */
    [[nodiscard]] constexpr value_type operator*() const noexcept
    {
        const auto num_bytes = code_point_size(str_);
        return str_.substr(0, num_bytes);
    }

    /** Pre-increment the iterator by one code point.
     *
     * @return Reference to this after moving the iterator
     */
    constexpr code_point_iterator& operator++() noexcept
    {
        const auto num_bytes = code_point_size(str_);
        str_.remove_prefix(num_bytes);
        return *this;
    }

    /** Post-increment the iterator by one code point.
     *
     * @return Copy of this before moving the iterator
     */
    constexpr code_point_iterator operator++(int) noexcept
    {
        auto result = *this;

        ++(*this);
        return result;
    }

private:
    std::string_view str_;
};

/** A simple wrapper for a string view that allows code point iteration in range-for loops.
 */
class code_point_iterator_wrapper
{
public:
    /** Constructor.
     *
     * @param str Input string
     */
    constexpr explicit code_point_iterator_wrapper(std::string_view str) : str_{str} {}

    /** Returns a start iterator.
     *
     * @return Start iterator
     */
    [[nodiscard]] constexpr code_point_iterator begin() noexcept
    {
        return code_point_iterator{str_};
    }

    /** Returns an end iterator.
     *
     * @return End iterator
     */
    [[nodiscard]] constexpr code_point_iterator end() noexcept { return code_point_iterator{}; }

private:
    std::string_view str_;
};

/** Returns the start byte index into @a str corresponding to the UTF-8 code point @a i.
 *
 * @param i UTF-8 code point index
 * @param str Input string
 * @return Byte index, or empty optional if requested index was not found
 */
[[nodiscard]] inline constexpr std::optional<std::size_t> code_point_index_to_byte_index(
    std::size_t i,
    std::string_view str) noexcept
{
    auto cp = std::size_t{0};
    for (auto b = std::size_t{0}; b < str.size(); ++b) {
        if (((str[b] & 0xC0) != 0x80) && (cp++ == i)) {
            return b;
        }
    }

    return {};
}

/** True if the leading code point of @a str is one of the known whitespace characters.
 *
 * @param str Code point
 * @return True if whitespace, also false if @a str is empty or there are not enough bytes in @a str
 * to read the entire code point
 */
[[nodiscard]] inline constexpr bool is_whitespace(std::string_view str) noexcept
{
    const auto cp = code_point_to_number(str);
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
    for (auto cp : code_point_iterator_wrapper{str}) {
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
    for (auto cp_str : code_point_iterator_wrapper{str}) {
        const auto cp = code_point_to_number(cp_str);
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
    constexpr line_iterator() noexcept : max_columns_{0}, line_break_byte_{0} {}

    /** Constructor.
     *
     * @note If max_columns is zero then @a str is emptied making this iterator equivalent to
     * the end iterator - this is to prevent infinite loops
     * @param str Input string
     * @param max_columns Maximum width of each iteration in terminal columns
     */
    constexpr explicit line_iterator(std::string_view str, std::size_t max_columns) noexcept :
        str_{str}, max_columns_{max_columns}, line_break_byte_{0}
    {
        if (max_columns_ == 0) {
            str_ = std::string_view{};
        }
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
        if ((str_.size() == 0) && (other.str_.size() == 0)) {
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
            for (auto cp : code_point_iterator_wrapper{str_}) {
                if (!is_whitespace(cp)) {
                    break;
                }
                bytes += code_point_size(cp);
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
        for (auto cp : code_point_iterator_wrapper{str_}) {
            // Have we exceeded the terminal width?
            column += terminal_width(cp);
            if (column > max_columns_) {
                // Oh dear, the line has no whitespace in it to break on, so force break on the last
                // code  point
                if (line_break_column == 0) {
                    line_break_byte_ = bytes;
                } else {
                    line_break_byte_ = line_break_byte;
                }
                return;
            }

            bytes += code_point_size(cp);
            if (is_whitespace(cp)) {
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
};
}  // namespace utf8
}  // namespace utility
}  // namespace arg_router
