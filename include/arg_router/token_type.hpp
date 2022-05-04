/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/utility/string_view_ops.hpp"

namespace arg_router
{
/** Namespace containing types and functions to aid parsing. */
namespace parsing
{
/** Enum for the prefix type on a token. */
enum class prefix_type : std::uint8_t {
    long_,   /// Long prefix
    short_,  /// Short prefix
    none     /// No prefix
};

/** Creates a string version of @a prefix.
 *
 * This uses config::long_prefix and config::short_prefix.
 * @param prefix Prefix type to convert
 * @return String version of @a prefix
 */
[[nodiscard]] constexpr std::string_view to_string(prefix_type prefix) noexcept
{
    switch (prefix) {
    case prefix_type::long_: return config::long_prefix;
    case prefix_type::short_: return config::short_prefix;
    default: return "";
    }
}

/** Pair-like structure carrying the token's prefix type and the token itself
 * (stripped of prefix).
 */
struct token_type {
    /** Long form name constructor.
     *
     * @param p Prefix type
     * @param n Token name, stripped of prefix (if any)
     */
    constexpr token_type(prefix_type p, std::string_view n) noexcept :
        prefix{p}, name{n}
    {
    }

    /** Equality operator.
     *
     * @param other Instance to compare against
     * @return True if equal
     */
    [[nodiscard]] constexpr bool operator==(
        const token_type& other) const noexcept
    {
        return prefix == other.prefix && name == other.name;
    }

    /** Inequality operator.
     *
     * @param other Instance to compare against
     * @return True if not equal
     */
    [[nodiscard]] constexpr bool operator!=(
        const token_type& other) const noexcept
    {
        return !(*this == other);
    }

    prefix_type prefix;     ///< Prefix type
    std::string_view name;  ///< Token name, stripped of prefix (if any)
};

/** Creates a string representation of @a token, it effectively recreates the
 * original token on the command line.
 * 
 * @param token Token to convert
 * @return String representation of @a token
 */
[[nodiscard]] inline string to_string(const token_type& token)
{
    using namespace utility::string_view_ops;
    return to_string(token.prefix) + token.name;
}

/** List of tokens.
 *  
 * This is similar in implementation to a boost::container::devector.  It
 * consists of the two views of the command line token array: pending and
 * processed.  Initially all tokens are pending and then as they processed by
 * the nodes, they mark the tokens as processed, which moves them to the back of
 * the processed view.
 * 
 * Of course we don't really keep two separate lists, there's only one, and
 * marking them as processed simply moves the pointer that separates the two
 * views of the container.
 */
class token_list
{
    using base_type = vector<token_type>;

public:
    /** token_type alias. */
    using value_type = base_type::value_type;
    /** Container allocator type. */
    using allocator_type = base_type::allocator_type;
    /** std::size_t alias. */
    using size_type = base_type::size_type;
    /** const token_type& alias. */
    using const_reference = base_type::const_reference;
    /** View type for the pending tokens. */
    using pending_view_type = span<const value_type>;

    /** View type for the processed tokens.
     *  
     * We have identical functioning but different types of views so people
     * can't pass a processed view iterator to the insert method.
     */
    class processed_view_type : public pending_view_type
    {
        using span::span;
    };

    /** Constructor.
     *  
     * @param alloc Allocator
     */
    explicit token_list(const allocator_type& alloc = allocator_type{}) noexcept
        :
        data_(alloc),
        head_offset_{0}
    {
    }

    /** Constructor.
     *  
     * @param init Initialiser list
     * @param alloc Allocator
     */
    token_list(std::initializer_list<value_type> init,
               const allocator_type& alloc = allocator_type{}) :
        data_(std::move(init), alloc),
        head_offset_{0}
    {
    }

    token_list(token_list&&) noexcept = default;
    token_list(const token_list&) = default;
    token_list& operator=(token_list&&) noexcept = default;
    token_list& operator=(const token_list&) = default;

    /** View of the tokens still to be processed.
     *
     * @return Pending view
     */
    [[nodiscard]] pending_view_type inline pending_view() const
    {
        return {data_.data() + head_offset_, data_.size() - head_offset_};
    }

    /** View of the tokens that have been processed.
     *
     * @return Pending view
     */
    [[nodiscard]] processed_view_type inline processed_view() const
    {
        return {data_.data(), head_offset_};
    }

    /** Maximum size of container.
     * 
     * @return Container size.
     */
    [[nodiscard]] inline size_type max_size() const noexcept
    {
        return data_.max_size() - head_offset_;
    }

    /** Equality operator.
     *
     * @note Both the pending and processed tokens are compared
     * @param other Instance to compare against
     * @return True if equal
     */
    [[nodiscard]] inline bool operator==(const token_list& other) const noexcept
    {
        return (head_offset_ == other.head_offset_) && (data_ == other.data_);
    }

    /** Inequality operator.
     *
     * @note Both the pending and processed tokens are compared
     * @param other Instance to compare against
     * @return True if not equal
     */
    [[nodiscard]] inline bool operator!=(const token_list& other) const noexcept
    {
        return !(*this == other);
    }

    /** Increase container capacity.
     *
     * @param new_cap New capacity
     */
    inline void reserve(size_type new_cap) { data_.reserve(new_cap); }

    /** Add new element to the end of the pending view.
     *
     * @param value Element to add
     */
    inline void push_back_pending(const value_type& value)
    {
        data_.push_back(value);
    }

    /** Create one element in-place and add to the end of the container.
     *
     * @tparam Args Constructor argument types
     * @param args Element constructor arguments
     * @return Reference to newly constructed element
     */
    template <class... Args>
    const_reference emplace_pending(Args&&... args)
    {
        return data_.emplace_back(std::forward<Args>(args)...);
    }

    /** Marks the first @a count elements in the pending view as processed.
     *
     * The elements are then moved to the back of the processed view.
     * @param count Number of elements to remove, the value is clamped by the
     * pending view size
     */
    inline void mark_as_processed(size_type count = 1)
    {
        head_offset_ += std::min(count, pending_view().size());
    }

    /** Insert elements into the container.
     *
     * @param pos Position to insert at
     * @param first Iterator to first elemenet to insert
     * @param last Iterator to one-past-the-end elemenet to insert
     * @return Iterator to one-past-the-end of the inserted sequence
     */
    template <typename InputIt>
    pending_view_type::const_iterator  //
    insert_pending(pending_view_type::const_iterator pos,
                   InputIt first,
                   InputIt last)
    {
        // If there's enough space, try to reclaim the leading memory first
        const auto input_count =
            static_cast<std::size_t>(std::distance(first, last));

        if ((pos == pending_view().begin()) && (input_count <= head_offset_)) {
            const auto result = pending_view().begin();
            head_offset_ -= input_count;
            std::move(first, last, data_.begin() + head_offset_);
            return result;
        } else {
            // We have to do this nonsense because the vector may reallocate
            // due to the insertion which would invalidate pos+input_count
            const auto offset = std::distance(pending_view().begin(), pos);
            data_.insert(data_.begin() + head_offset_ + offset, first, last);
            return pending_view().begin() + offset + input_count;
        }
    }

    /** Swap the container contents of this with @a other.
     *
     * @param other Instance to swap with
     */
    inline void swap(token_list& other) noexcept
    {
        using std::swap;

        swap(data_, other.data_);
        swap(head_offset_, other.head_offset_);
    }

private:
    base_type data_;
    size_type head_offset_;
};

/** Global swap function for token_list.
 *
 * @param lhs First instance
 * @param rhs Second instance
 */
inline void swap(token_list& lhs, token_list& rhs) noexcept
{
    lhs.swap(rhs);
}

namespace detail
{
template <typename ViewType>
[[nodiscard]] constexpr bool token_list_view_equality(ViewType lhs,
                                                      ViewType rhs) noexcept
{
    // For some insane reason, span doesn't have equality operators
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename ViewType>
[[nodiscard]] string token_list_view_to_string(ViewType view)
{
    auto str = string{""};
    for (auto i = 0u; i < view.size(); ++i) {
        str += to_string(view[i]);
        if (i != (view.size() - 1)) {
            str += ", ";
        }
    }
    return str;
}
}  // namespace detail

/** Equality operator.
 *
 * @param lhs Pending view
 * @param rhs Pending view
 * @return True if lexicographically equal
 */
[[nodiscard]] inline bool operator==(token_list::pending_view_type lhs,
                                     token_list::pending_view_type rhs) noexcept
{
    return detail::token_list_view_equality(lhs, rhs);
}

/** Equality operator.
 *
 * Compares against the pending view of @a rhs.
 * @param lhs Pending view
 * @param rhs Token list
 * @return True if lexicographically equal
 */
[[nodiscard]] inline bool operator==(token_list::pending_view_type lhs,
                                     const token_list& rhs) noexcept
{
    return lhs == rhs.pending_view();
}

/** Inequality operator.
 *
 * Compares against the pending view of @a rhs.
 * @param lhs Pending view
 * @param rhs Token list
 * @return True if lexicographically equal
 */
[[nodiscard]] inline bool operator!=(token_list::pending_view_type lhs,
                                     const token_list& rhs) noexcept
{
    return !(lhs == rhs);
}

/** Equality operator.
 *
 * Compares against the pending view of @a lhs.
 * @param lhs Token list
 * @param rhs Pending view
 * @return True if lexicographically equal
 */
[[nodiscard]] inline bool operator==(const token_list& lhs,
                                     token_list::pending_view_type rhs) noexcept
{
    return rhs == lhs;
}

/** Inequality operator.
 *
 * Compares against the pending view of @a lhs.
 * @param lhs Token list
 * @param rhs Pending view
 * @return True if lexicographically equal
 */
[[nodiscard]] inline bool operator!=(const token_list& lhs,
                                     token_list::pending_view_type rhs) noexcept
{
    return !(lhs == rhs);
}

/** Equality operator.
 *
 * @param lhs Pending view
 * @param rhs Pending view
 * @return True if lexicographically equal
 */
[[nodiscard]] inline bool operator==(
    token_list::processed_view_type lhs,
    token_list::processed_view_type rhs) noexcept
{
    return detail::token_list_view_equality(lhs, rhs);
}

/** Equality operator.
 *
 * Compares against the processed view of @a rhs.
 * @param lhs Pending view
 * @param rhs Token list
 * @return True if lexicographically equal
 */
[[nodiscard]] inline bool operator==(token_list::processed_view_type lhs,
                                     const token_list& rhs) noexcept
{
    return lhs == rhs.processed_view();
}

/** Inequality operator.
 *
 * Compares against the processed view of @a rhs.
 * @param lhs Pending view
 * @param rhs Token list
 * @return True if lexicographically equal
 */
[[nodiscard]] inline bool operator!=(token_list::processed_view_type lhs,
                                     const token_list& rhs) noexcept
{
    return !(lhs == rhs);
}

/** Equality operator.
 *
 * Compares against the processed view of @a lhs.
 * @param lhs Token list
 * @param rhs Pending view
 * @return True if lexicographically equal
 */
[[nodiscard]] inline bool operator==(
    const token_list& lhs,
    token_list::processed_view_type rhs) noexcept
{
    return rhs == lhs;
}

/** Inequality operator.
 *
 * Compares against the processed view of @a lhs.
 * @param lhs Token list
 * @param rhs Pending view
 * @return True if lexicographically equal
 */
[[nodiscard]] inline bool operator!=(
    const token_list& lhs,
    token_list::processed_view_type rhs) noexcept
{
    return !(lhs == rhs);
}

/** Creates a string representation of @a view.
 * 
 * @param view Pending tokens to convert
 * @return String representation of @a view
 */
[[nodiscard]] inline string to_string(const token_list::pending_view_type& view)
{
    return detail::token_list_view_to_string(view);
}

/** Creates a string representation of @a view.
 * 
 * @param view Processed tokens to convert
 * @return String representation of @a view
 */
[[nodiscard]] inline string to_string(
    const token_list::processed_view_type& view)
{
    return detail::token_list_view_to_string(view);
}

/** Creates a string representation of @a view.
 * 
 * @param view Processed tokens to convert
 * @return String representation of @a view
 */
[[nodiscard]] inline string to_string(const vector<token_type>& view)
{
    return detail::token_list_view_to_string(view);
}

/** Creates a string representation of the pending view of @a tokens.
 * 
 * @param tokens Tokens to convert
 * @return String representation of @a tokens
 */
[[nodiscard]] inline string to_string(const token_list& tokens)
{
    return to_string(tokens.pending_view());
}

/** Analyse @a token and return a pair consisting of the prefix type and
 * @a token stripped of the token.
 *
 * @param token Token to analyse
 * @return Token type
 */
[[nodiscard]] inline token_type get_token_type(std::string_view token)
{
    using namespace config;

    if (token.substr(0, long_prefix.size()) == long_prefix) {
        token.remove_prefix(long_prefix.size());
        return {prefix_type::long_, token};
    } else if (token.substr(0, short_prefix.size()) == short_prefix) {
        token.remove_prefix(short_prefix.size());
        return {prefix_type::short_, token};
    } else {
        return {prefix_type::none, token};
    }
}
}  // namespace parsing
}  // namespace arg_router
