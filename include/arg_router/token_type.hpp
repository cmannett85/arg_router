#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace arg_router
{
/** Namespace containing types and functions to aid parsing. */
namespace parsing
{
/** Enum for the prefix type on a token. */
enum class prefix_type : std::uint8_t { LONG, SHORT, NONE };

/** Creates a string version of @a prefix.
 *
 * This uses config::long_prefix and config::short_prefix.
 * @param prefix Prefix type to convert
 * @return String version of @a prefix
 */
std::string_view to_string(prefix_type prefix);

/** Pair-like structure carrying the token's prefix type and the token itself
 * (stripped of prefix).
 */
struct token_type {
    /** Long form name constructor.
     *
     * @param p Prefix type
     * @param n Token name, stripped of prefix (if any)
     */
    constexpr token_type(prefix_type p, std::string_view n) : prefix{p}, name{n}
    {
    }

    /** Equality operator.
     *
     * @param other Instance to compare against
     * @return True if equal
     */
    bool operator==(const token_type& other) const
    {
        return prefix == other.prefix && name == other.name;
    }

    /** Inequality operator.
     *
     * @param other Instance to compare against
     * @return True if not equal
     */
    bool operator!=(const token_type& other) const { return !(*this == other); }

    prefix_type prefix;     ///< Prefix type
    std::string_view name;  ///< Token name, stripped of prefix (if any)
};

/** Creates a string representation of @a token, it effectively recreates the
 * original token on the command line.
 * 
 * @param token Token to convert
 * @return String representation of @a token
 */
std::string to_string(const token_type& token);

/** List of tokens.
 *  
 * This is basically a boost::container::devector but without the dependency
 * and... dumber.
 */
class token_list
{
    using base_type = std::vector<token_type>;

public:
    using value_type = base_type::value_type;
    using allocator_type = base_type::allocator_type;
    using size_type = base_type::size_type;
    using difference_type = base_type::difference_type;
    using reference = base_type::reference;
    using const_reference = base_type::const_reference;
    using pointer = base_type::pointer;
    using const_pointer = base_type::const_pointer;
    using iterator = base_type::iterator;
    using const_iterator = base_type::const_iterator;

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
    token_list(const token_list&) noexcept = default;
    token_list& operator=(token_list&&) noexcept = default;
    token_list& operator=(const token_list&) noexcept = default;

    /** Iterator to start of the container.
     *
     * @return Container start iterator
     */
    inline const_iterator begin() const noexcept
    {
        return data_.cbegin() + head_offset_;
    }

    /** Iterator to start of the container.
     *
     * @return Container start iterator
     */
    inline const_iterator cbegin() const noexcept { return begin(); }

    /** Iterator to one-past-the-end of the container.
     *
     * @return Container end iterator
     */
    inline const_iterator end() const noexcept { return data_.end(); }

    /** Iterator to one-past-the-end of the container.
     *
     * @return Container end iterator
     */
    inline const_iterator cend() const noexcept { return end(); }

    /** Pointer to the start of the array.
     *
     * @return Array pointer
     */
    inline const_pointer data() const noexcept { return &(*begin()); }

    /** Container size.
     * 
     * @return Number of elements in container
     */
    [[nodiscard]] inline size_type size() const noexcept
    {
        return data_.size() - head_offset_;
    }

    /** Maximum size of container.
     * 
     * @return Container size.
     */
    [[nodiscard]] inline size_type max_size() const noexcept
    {
        return data_.max_size() - head_offset_;
    }

    /** Returns true if empty.
     *
     * @return True if empty
     */
    [[nodiscard]] inline bool empty() const noexcept
    {
        return begin() == end();
    }

    /** Returns a reference to the first element.
     *
     * It is undefined behaviour if the container is empty.
     * @return Reference to the first element
     */
    [[nodiscard]] inline const_reference front() const { return *begin(); }

    /** Equality operator.
     *
     * @param other Instance to compre against
     * @return True if equal
     */
    [[nodiscard]] bool operator==(const token_list& other) const noexcept;

    /** Inequality operator.
     *
     * @param other Instance to compare against
     * @return True if not equal
     */
    [[nodiscard]] inline bool operator!=(const token_list& other) const noexcept
    {
        return !(*this == other);
    }

    /** Index operator.
     * 
     * @param index Index into container
     * @return Reference to element
     */
    [[nodiscard]] inline const_reference operator[](size_type index) const
    {
        return begin()[index];
    }

    /** Increase container capacity.
     *
     * @param new_cap New capacity
     */
    void reserve(size_type new_cap);

    /** Add new element to the end of the container.
     *
     * @param value Element to add
     */
    void push_back(const value_type& value);

    /** Create ne element in-place and add to the end of the container.
     *
     * @tparam Args Constructor argument types
     * @param args Element constructor arguments
     * @return Reference to newly constructed element
     */
    template <class... Args>
    const_reference emplace_back(Args&&... args)
    {
        return data_.emplace_back(std::forward<Args>(args)...);
    }

    /** Remove the first @a count elements from the start of the container.
     *
     * @note The elements are @em not destroyed, the begin() iterator is moved
     * instead
     * @param count Number of elements to remove
     */
    void pop_front(size_type count = 1);

    /** Insert elements into the container.
     *
     * @param pos Position to insert at
     * @param first Iterator to first elemenet to insert
     * @param last Iterator to one-past-the-end elemenet to insert
     * @return Iterator to one-past-the-end of the inserted sequence
     */
    template <typename InputIt>
    const_iterator insert(const_iterator pos, InputIt first, InputIt last)
    {
        // If there's enough space, try to reclaim the leading memory first
        const auto diff = std::distance(data_.cbegin(), begin());
        const auto input_count = std::distance(first, last);

        if ((pos == begin()) && (input_count <= diff)) {
            head_offset_ -= input_count;
            return std::move(first, last, data_.begin() + head_offset_);
        } else {
            return data_.insert(pos, first, last);
        }
    }

    /** Swap the container contents of this with @a other.
     *
     * @param other Instance to swap with
     */
    void swap(token_list& other) noexcept;

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

/** Creates a string representation of @a tokens.
 * 
 * @param tokens Tokens to convert
 * @return String representation of @a tokens
 */
std::string to_string(const token_list& tokens);

/** Analyse @a token and return a pair consisting of the prefix type and
 * @a token stripped of the token.
 *
 * @param token Token to analyse
 * @return Token type
 */
token_type get_token_type(std::string_view token);
}  // namespace parsing
}  // namespace arg_router
