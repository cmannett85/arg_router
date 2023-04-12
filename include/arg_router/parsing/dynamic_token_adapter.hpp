// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/parsing/token_type.hpp"

namespace arg_router::parsing
{
/** An adaptor over the processed and unprocessed tokens.  This is used during the pre-parse phase.
 *
 * If an iterator element is read from that is beyond the end of the processed token container, then
 * one is returned from the equivalent position in the unprocessed token container (as if the two
 * containers were concatenated). Using iterator::set(value_type) will update the element pointed at
 * by the iterator, if the iterator is passed the end of the processed tokens container then
 * elements from the unprocessed container are transferred into it verbatim.
 *
 * This allows the pre-parse phase implementing policies to not concern themselves with managing the
 * processed/unprocessed containers.
 */
class dynamic_token_adapter
{
public:
    /** Value type. */
    using value_type = token_type;
    /** Size type. */
    using size_type = std::size_t;

    /** Iterator type. */
    class iterator
    {
    public:
        /** Difference type. */
        using difference_type = std::ptrdiff_t;
        /** Value type. */
        using value_type = dynamic_token_adapter::value_type;
        /** Pointer type. */
        using pointer = const value_type*;
        /** Reference type. */
        using reference = const value_type&;
        /** Iterator category. */
        using iterator_category = std::forward_iterator_tag;

        /** In-place increment operator.
         *
         * @param offset Increment offset (can be negative)
         * @return Reference to this
         */
        iterator& operator+=(difference_type offset) noexcept
        {
            i_ += offset;
            return *this;
        }

        /** In-place decrement operator.
         *
         * @param offset Decrement offset (can be negative)
         * @return Reference to this
         */
        iterator& operator-=(difference_type offset) noexcept { return (*this += -offset); }

        /** Increment operator.
         *
         * @param it
         * @param offset
         * @return Iterator
         */
        [[nodiscard]] friend iterator operator+(iterator it, difference_type offset) noexcept
        {
            return it += offset;
        }

        /** Decrement operator.
         *
         * @param it
         * @param offset
         * @return Iterator
         */
        [[nodiscard]] friend iterator operator-(iterator it, difference_type offset) noexcept
        {
            return it -= offset;
        }

        /** Increment operator.
         *
         * @param it
         * @param offset
         * @return Iterator
         */
        [[nodiscard]] friend iterator operator+(difference_type offset, iterator it) noexcept
        {
            return it += offset;
        }

        /** Decrement operator.
         *
         * @param it
         * @param offset
         * @return Iterator
         */
        [[nodiscard]] friend iterator operator-(difference_type offset, iterator it) noexcept
        {
            return it -= offset;
        }

        /** Difference operator.
         *
         * @note The owning containers are not compared, the resulting iterator
         * uses the owning container of @a lhs
         * @param lhs First instance
         * @param rhs Second instance
         * @return Iterator
         */
        [[nodiscard]] friend iterator operator-(iterator lhs, iterator rhs) noexcept
        {
            return {lhs.owner_, lhs.i_ - rhs.i_};
        }

        /** Less than operator.
         *
         * @note The owning containers are not compared
         * @param lhs First instance
         * @param rhs Second instance
         * @return True if the index represented by @a lhs is less than @a rhs
         */
        [[nodiscard]] friend bool operator<(iterator lhs, iterator rhs) noexcept
        {
            return lhs.i_ < rhs.i_;
        }

        /** Greater than operator.
         *
         * @note The owning containers are not compared
         * @param lhs First instance
         * @param rhs Second instance
         * @return True if the index represented by @a lhs is greater than
         * @a rhs
         */
        [[nodiscard]] friend bool operator>(iterator lhs, iterator rhs) noexcept
        {
            return rhs < lhs;
        }

        /** Less than or equal to operator.
         *
         * @note The owning containers are not compared
         * @param lhs First instance
         * @param rhs Second instance
         * @return True if the index represented by @a lhs is less than or
         * equal to @a rhs
         */
        [[nodiscard]] friend bool operator<=(iterator lhs, iterator rhs) noexcept
        {
            return !(lhs > rhs);
        }

        /** Greater than or equal to operator.
         *
         * @note The owning containers are not compared
         * @param lhs First instance
         * @param rhs Second instance
         * @return True if the index represented by @a lhs is greater than or
         * equal to @a rhs
         */
        [[nodiscard]] friend bool operator>=(iterator lhs, iterator rhs) noexcept
        {
            return !(lhs < rhs);
        }

        /** Pre-increment operator.
         *
         * @return Reference to this
         */
        iterator& operator++() noexcept { return (*this += 1); }

        /** Post-increment operator.
         *
         * @return Iterator before the increment
         */
        iterator operator++(int) noexcept
        {
            auto tmp = *this;
            *this += 1;
            return tmp;
        }

        /** Pre-decrement operator.
         *
         * @return Reference to this
         */
        iterator& operator--() noexcept { return (*this -= 1); }

        /** Post-decrement operator.
         *
         * @return Iterator before the decrement
         */
        iterator operator--(int) noexcept
        {
            auto tmp = *this;
            *this -= 1;
            return tmp;
        }

        /** Dereference operator.
         *
         * @note Undefined if iterator is out of bounds
         * @return Reference to element
         */
        [[nodiscard]] reference operator*() const noexcept
        {
            if (i_ >= static_cast<difference_type>(processed().size())) {
                const auto i = i_ - processed().size();
                return unprocessed()[i];
            }

            return processed()[i_];
        }

        /** Structure dereference operator.
         *
         * @note Undefined if iterator is out of bounds
         * @return Pointer to element
         */
        [[nodiscard]] pointer operator->() const noexcept { return &(*(*this)); }

        /** Offset and deference operator.
         *
         * @note Undefined if iterator is out of bounds
         * @param offset Offset
         * @return Reference to element
         */
        [[nodiscard]] reference operator[](difference_type offset) const noexcept
        {
            return *(*this + offset);
        }

        /** Equality operator.
         *
         * @note The owner's address is used in the comparison, so two identical
         * underlying containers will still return not equal
         * @param other Instance to compare against
         * @return True if equal
         */
        [[nodiscard]] bool operator==(iterator other) const noexcept
        {
            const auto is_this_end = is_end();
            const auto is_other_end = other.is_end();

            // Both end() iterators
            if (is_this_end && is_other_end) {
                return true;
            }
            if (is_this_end || is_other_end) {
                return false;
            }

            return (owner_ == other.owner_) && (i_ == other.i_);
        }

        /** Inequality operator.
         *
         * @note The owner's address is used in the comparison, so two identical
         * underlying containers will still return not equal
         * @param other Instance to compare against
         * @return True if not equal
         */
        [[nodiscard]] bool operator!=(iterator other) const noexcept { return !(*this == other); }

        /** Updates the element with @a value.
         *
         * If the iterator is beyond the end of the processed tokens vector,
         * then the elements up to and including the iterator are transferred
         * from the command line token container to the processed container.
         * Then the last transferred element is updated (i.e. the new end last
         * element of the processed container.
         * @note Undefined if iterator is out of bounds
         * @param value New value
         */
        void set(value_type value)
        {
            owner_->transfer(*this);
            processed()[i_] = value;
        }

    private:
        friend class dynamic_token_adapter;

        iterator() : owner_{nullptr}, i_{0} {}

        iterator(dynamic_token_adapter* owner, difference_type i) : owner_{owner}, i_{i} {}

        [[nodiscard]] bool is_end() const noexcept
        {
            return !owner_ || (i_ >= static_cast<difference_type>(owner_->size()));
        }

        [[nodiscard]] vector<token_type>& processed() const { return *(owner_->processed_); }

        [[nodiscard]] vector<token_type>& unprocessed() const { return *(owner_->unprocessed_); }

        dynamic_token_adapter* owner_;
        difference_type i_;
    };

    /** Constructor.
     *
     * @param processed Processed tokens container
     * @param unprocessed Unprocessed tokens container
     */
    dynamic_token_adapter(vector<token_type>& processed, vector<token_type>& unprocessed) :
        processed_{&processed}, unprocessed_{&unprocessed}
    {
        // Perform a reserve for all the tokens.  There is a resonable chance that more processed
        // than unprocessed tokens will be needed (due to short-form expansion, value separation,
        // etc.), but this should still dramatically reduce the number of allocations needed
        unprocessed_->reserve(size());
    }

    /** Equality operator.
     *
     * @note The underlying containers' addresses are used in the comparison, so two separate but
     * identical instances will return not equal
     * @param other Instance to compare against
     * @return True if equal
     */
    [[nodiscard]] bool operator==(dynamic_token_adapter other) const
    {
        return (processed_ == other.processed_) && (unprocessed_ == other.unprocessed_);
    }

    /** Inequality operator.
     *
     * @note The underlying containers' addresses are used in the comparison, so two separate but
     * identical instances will return not equal
     * @param other Instance to compare against
     * @return True if not equal
     */
    [[nodiscard]] bool operator!=(dynamic_token_adapter other) const { return !(*this == other); }

    /** Returns an iterator to the beginning of the processed container.
     *
     * If the processed container is empty, this will initialise it by moving an entry from the
     * front of the raw command line token container into it, using prefix_type::none.
     * @return Begin iterator
     */
    [[nodiscard]] iterator begin() { return {this, 0}; }

    /** Returns a one-past-the-end iterator.
     *
     * @return End iterator
     */
    [[nodiscard]] static iterator end() { return {}; }

    /** Returns the count of all tokens, processed and unprocessed.
     *
     * @return Number of components
     */
    [[nodiscard]] size_type size() const { return processed_->size() + unprocessed_->size(); }

    /** @return True if there are no processed or unprocessed tokens.
     */
    [[nodiscard]] bool empty() const { return processed_->empty() && unprocessed_->empty(); }

    /** The underlying processed container.
     *
     * @return Processed container reference
     */
    [[nodiscard]] vector<value_type>& processed() { return *processed_; }

    /** The underlying unprocessed container.
     *
     * @return Unprocessed container reference
     */
    [[nodiscard]] vector<value_type>& unprocessed() { return *unprocessed_; }

    /** Inserts @a token at position @a it.
     *
     * @note No checking is performed that the iterator was created from the same adapter
     * @param it Position to insert at
     * @param value Value to insert
     * @return Iterator to inserted value
     */
    iterator insert(iterator it, value_type value)
    {
        // Only transfer up to the element before the target position, otherwise we transfer that
        // and then insert the new value, which isn't expected
        transfer(it - 1);
        auto processed_it = it.is_end() ? processed_->end() :  //
                                          processed_->begin() + it.i_;

        processed_->insert(processed_it, value);
        return {this, it.i_};
    }

    /** Inserts the value in the range [ @a first, @a last ) at position @a it.
     *
     * @param it Position to start inserting at
     * @param first Iterator to first instance in range to insert
     * @param last One-past-the-end iterator of range to insert
     * @return Iterator to first of the range inserted
     */
    template <typename Iter>
    iterator insert(iterator it, Iter first, Iter last)
    {
        // Only transfer up to the element before the target position, otherwise we transfer that
        // and then insert the new value, which isn't expected
        transfer(it - 1);
        auto processed_it = it.is_end() ? processed_->end() :  //
                                          processed_->begin() + it.i_;

        processed_->insert(processed_it, first, last);
        return {this, it.i_};
    }

    /** Erases the element at @a it.
     *
     * Does not perform any transfer between the process and unprocessed sides.
     * @param it Element to remove.  If one-past-the-end iterator, this method is a no-op
     * @return Element following the one removed
     */
    iterator erase(iterator it)
    {
        // If the iterator is an end(), it's a no-op
        if (it.is_end()) {
            return it;
        }

        const auto processed_size = static_cast<iterator::difference_type>(processed_->size());
        if (it.i_ < processed_size) {
            processed_->erase(processed_->begin() + it.i_);
        } else {
            const auto offset = it.i_ - processed_size;
            unprocessed_->erase(unprocessed_->begin() + offset);
        }

        return it;
    }

    /** Transfer elements from the raw command line token container to processed one up to and
     * including the one represented by @a it.
     *
     * If @a it is before or within the processed container, then this is a no-op.
     * @param it Iterator, and the preceding elements too, to transfer
     */
    void transfer(iterator it)
    {
        // If the iterator is an end(), then consume all the unprocessed tokens
        if (it.is_end()) {
            it.i_ = size() - 1;
        }

        if ((it.i_ < 0) || (it.i_ < static_cast<iterator::difference_type>(processed_->size()))) {
            return;
        }

        const auto count = (it.i_ + 1) - processed_->size();
        processed_->insert(processed_->end(), unprocessed_->begin(), unprocessed_->begin() + count);
        unprocessed_->erase(unprocessed_->begin(), unprocessed_->begin() + count);
    }

private:
    vector<token_type>* processed_;
    vector<token_type>* unprocessed_;
};
}  // namespace arg_router::parsing
