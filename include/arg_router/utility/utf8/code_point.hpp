/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

/** Namespace for UTF-8 code point types and functions. */
namespace arg_router::utility::utf8::code_point
{
/** Code point type. */
using type = std::uint32_t;

/** Defines an @em inclusive contiguous range of code points, plus some meta data specific to the
 * table type it comes from.
 */
class range
{
public:
    /** Constructor.
     *
     * @note Only the first 21 bits of a code point are valid Unicode data, and so that is all that
     * is stored in here
     * @param first First code point in range
     * @param last Inclusive last code point in range
     * @param meta Metadata, only the first 6 bits are used.  Zero will be set if unused
     */
    constexpr range(type first, type last, std::uint8_t meta = 0) noexcept : data_{}
    {
        // array::fill is not constexpr until C++20...
        for (auto& b : data_) {
            b = 0;
        }

        // Can't use reinterpret_cast in constexpr function, so we have to do it the old fashioned
        // way!
        // [0-20]  Start code point
        // [21-41] End code point
        // [42-47] Metadata
        // NOLINTBEGIN(readability-magic-numbers)
        data_[0] = first & 0xFF;
        data_[1] = (first >> 8) & 0xFF;
        data_[2] = (first >> 16) & 0x1F;

        data_[2] |= (last & 0x7) << 5;
        data_[3] = (last >> 3) & 0xFF;
        data_[4] = (last >> 11) & 0xFF;
        data_[5] = (last >> 19) & 0x3;

        data_[5] |= (meta & 0x3F) << 2;
        // NOLINTEND(readability-magic-numbers)
    }

    /** First code point in range.
     *
     * @return Code point
     */
    [[nodiscard]] constexpr type first() const noexcept
    {
        // NOLINTBEGIN(readability-magic-numbers)
        auto value = type{data_[0]};
        value |= data_[1] << 8;
        value |= (data_[2] & 0x1F) << 16;
        // NOLINTEND(readability-magic-numbers)

        return value;
    }

    /** Inclusive last code point in range.
     *
     * @return Code point
     */
    [[nodiscard]] constexpr type last() const noexcept
    {
        // NOLINTBEGIN(readability-magic-numbers)
        type value = (data_[2] >> 5) & 0x7;
        value |= data_[3] << 3;
        value |= data_[4] << 11;
        value |= (data_[5] & 0x3) << 19;
        // NOLINTEND(readability-magic-numbers)

        return value;
    }

    /** Meta data.
     *
     * @return Meta data, zero if unset
     */
    [[nodiscard]] constexpr std::uint8_t meta() const noexcept
    {
        // NOLINTNEXTLINE
        return (data_[5] >> 2) & 0x3F;
    }

    /** Less than operator.
     *
     * @param other Instance to compare against
     * @return True if this is less than @a other
     */
    constexpr bool operator<(range other) const noexcept
    {
        if (first() == other.first()) {
            return last() < other.last();
        }
        return first() < other.first();
    }

    /** Less than operator for a single code point.
     *
     * @param cp Instance to compare against
     * @return True if the start of this range is less than @a cp
     */
    constexpr bool operator<(type cp) const noexcept { return first() < cp; }

private:
    static constexpr auto bytes_per_cp = std::size_t{6};
    std::array<std::uint8_t, bytes_per_cp> data_;
};

/** Number of UTF-8 code points in the string.
 *
 * @param str Input string
 * @return Number of code points
 */
[[nodiscard]] inline constexpr std::size_t count(std::string_view str) noexcept
{
    // Only the leading code point byte (applies to ASCII too) will not lead with 0x80
    constexpr auto high_2_bit_mask = std::uint8_t{0xC0};
    constexpr auto high_bit = std::uint8_t{0x80};

    auto result = std::size_t{0};
    for (auto c : str) {
        result += (c & high_2_bit_mask) != high_bit;
    }

    return result;
}

/** Returns the size in bytes for the leading code point of @a str
 *
 * @param str Input string
 * @return Size in bytes, zero if @a str is empty
 */
[[nodiscard]] inline constexpr std::size_t size(std::string_view str) noexcept
{
    if (str.empty()) {
        return 0;
    }

    const auto first_byte = static_cast<std::uint8_t>(str[0]);

    // The MSB determines if it's ASCII or UTF-8
    constexpr auto is_ascii_mask = std::uint8_t{0b1000'0000};
    if ((first_byte & is_ascii_mask) == 0) {
        // ASCII
        return 1;
    }

    // Can't think of a way to do this without branches
    constexpr auto max_2_byte_header = std::uint8_t{0b1101'1111};
    if (first_byte < max_2_byte_header) {
        return 2;
    }

    constexpr auto max_3_byte_header = std::uint8_t{0b1110'1111};
    if (first_byte < max_3_byte_header) {
        return 3;
    }

    return 4;
}

/** Decodes the leading code point of @a str into the underlying numerical representation.
 *
 * Only up to the first four bytes of @a str are read.  Undefined behaviour if @a str is malformed.
 * @param str Code point
 * @return Number, or empty optional if @a str is empty or there are not enough bytes in @a str to
 * read the entire code point
 */
[[nodiscard]] inline constexpr std::optional<type> decode(std::string_view str) noexcept
{
    const auto bytes_to_read = size(str);
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

    constexpr auto subsequent_byte_data_bits = 6;
    constexpr auto subsequent_byte_mask = type{(1 << subsequent_byte_data_bits) - 1};
    constexpr auto maximum_first_byte_data_mask = type{0b0001'1111};

    const auto first_byte = static_cast<std::uint8_t>(str[0]);

    auto result = first_byte & (maximum_first_byte_data_mask >> (bytes_to_read - 2));
    for (auto i = 1u; i < bytes_to_read; ++i) {
        const auto subsequent_byte = static_cast<type>(str[i]);

        // Move the previous reads up to make space for the subsequent byte's data
        result <<= subsequent_byte_data_bits;
        result |= subsequent_byte & subsequent_byte_mask;
    }

    return result;
}

/** Forward iterator for a string's code points.
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
     * for (auto cp : code_point_iterator::range(str)) {
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
        [[nodiscard]] static constexpr iterator end() noexcept { return iterator{}; }

    private:
        constexpr explicit range_t(std::string_view str) noexcept : str_{str} {}

        std::string_view str_;
    };

    /** Returns an object that can be used in range-for loops.
     *
     * @param str Input string
     * @return Range object
     */
    [[nodiscard]] static constexpr range_t range(std::string_view str) noexcept
    {
        return range_t{str};
    }

    /** Default constructor.
     *
     * Represents the end iterator.
     */
    constexpr iterator() noexcept = default;

    /** Constructor.
     *
     * If @a str is empty, then this will create an end iterator.
     * @param str String to iterator over.
     */
    constexpr explicit iterator(std::string_view str) noexcept : str_{str} {}

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
     * @return Code point
     */
    [[nodiscard]] constexpr value_type operator*() const noexcept
    {
        const auto num_bytes = code_point::size(str_);
        return str_.substr(0, num_bytes);
    }

    /** Pre-increment the iterator by one code point.
     *
     * @return Reference to this after moving the iterator
     */
    constexpr iterator& operator++() noexcept
    {
        const auto num_bytes = code_point::size(str_);
        str_.remove_prefix(num_bytes);
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
    std::string_view str_;
};
}  // namespace arg_router::utility::utf8::code_point
