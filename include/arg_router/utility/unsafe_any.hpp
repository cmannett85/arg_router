/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/config.hpp"

#include <cstring>
#include <functional>

namespace arg_router::utility
{
/** Type erasure type similar to <TT>std::any</TT> but has no type checking safety features.
 *
 * @tparam SmallObjectOptimisationSize Size in bytes of the small object optimisation limit.
 * Holding objects larger will incur heap allocation. Defaults to word size
 */
template <std::size_t SmallObjectOptimisationSize = sizeof(std::size_t)>
// NOLINTNEXTLINE(*-special-member-functions)
class unsafe_any_t
{
    using ptr_type = void*;
    using aligned_storage_type = std::aligned_storage_t<SmallObjectOptimisationSize>;

    template <typename T>
    static constexpr bool use_internal_storage = (sizeof(T) <= sizeof(aligned_storage_type));

public:
    /** Default constructor.
     *
     * Calling get() will result in undefined behaviour.
     */
    constexpr unsafe_any_t() = default;

    /** Constructs an unsafe_any_t from @a value.
     *
     * This constructor only takes part in overload resolution if @a T fits inside the internal
     * storage.
     * @tparam T Type to construct from
     * @param value Value to initialise from, will move construct if an rvalue is passed in
     */
    template <typename T,
              typename = std::enable_if_t<use_internal_storage<T> &&
                                          !std::is_same_v<std::decay_t<T>, unsafe_any_t>>>
    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    unsafe_any_t(T&& value) noexcept
    {
        using value_type = std::decay_t<T>;

        // Build in buffer
        new (&storage_.buffer) value_type(std::forward<T>(value));

        copier_ = [](const storage_type& storage) -> unsafe_any_t {
            return *reinterpret_cast<const value_type*>(&storage.buffer);
        };
        destroyer_ = [](storage_type& storage) {
            auto ptr = reinterpret_cast<value_type*>(&storage.buffer);
            ptr->~value_type();
        };
    }

    /** Constructs an unsafe_any_t from @a value.
     *
     * This constructor only takes part in overload resolution if @a T does not fit inside the
     * internal storage.
     * @tparam T Type to construct from
     * @tparam Allocator Allocator type, only used when not using internal storage
     * @param value Value to initialise from, will move construct if an rvalue is passed in
     * @param alloc Allocator instance
     */
    template <typename T,
              typename Allocator = config::allocator<std::decay_t<T>>,
              typename = std::enable_if_t<!use_internal_storage<T> &&
                                          !std::is_same_v<std::decay_t<T>, unsafe_any_t>>>
    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    unsafe_any_t(T&& value, Allocator alloc = Allocator{})
    {
        using value_type = std::decay_t<T>;

        // Build using memory from allocator
        storage_.ptr = alloc.allocate(1);
        new (storage_.ptr) value_type(std::forward<T>(value));

        copier_ = [](const storage_type& storage) -> unsafe_any_t {
            return *reinterpret_cast<const value_type*>(storage.ptr);
        };
        destroyer_ = [alloc = std::move(alloc)](storage_type& storage) mutable noexcept {
            auto s_ptr = reinterpret_cast<value_type*>(storage.ptr);

            std::allocator_traits<Allocator>::destroy(alloc, s_ptr);
            alloc.deallocate(s_ptr, 1);
            storage.ptr = nullptr;
        };
    }

    /** Move constructor.
     *
     * @note @a other is left in an empty state (i.e. has_value() returns false).
     * @param other Instance to move from
     */
    unsafe_any_t(unsafe_any_t&& other) noexcept { swap(*this, other); }

    /** Copy constructor.
     *
     * @param other Instance to copy from
     */
    unsafe_any_t(const unsafe_any_t& other)
    {
        auto new_any = other.copier_(other.storage_);
        swap(*this, new_any);
    }

    /** Assignment operator.
     *
     * @param other Instance to copy from
     * @return Reference to this
     */
    unsafe_any_t& operator=(unsafe_any_t other) noexcept
    {
        swap(*this, other);
        return *this;
    }

    /** Destructor.
     */
    ~unsafe_any_t() noexcept
    {
        if (destroyer_) {
            destroyer_(storage_);
        }
    }

    /** Returns true if the any is holding a value.
     *
     * This is false if the instance has been moved from.
     * @return True if the any is holding a value
     */
    [[nodiscard]] bool has_value() const noexcept { return !!destroyer_; }

    /** Returns a reference to the held object.
     *
     * Undefined behaviour if @a T is not the held type.
     * @tparam T Type to return
     * @return Held object
     */
    template <typename T, typename DecayType = std::decay_t<T>>
    [[nodiscard]] std::decay_t<T>& get() noexcept
    {
        using value_type = std::decay_t<T>;

        if constexpr (use_internal_storage<value_type>) {
            return *reinterpret_cast<value_type*>(&storage_.buffer);
        } else {
            return *reinterpret_cast<value_type*>(storage_.ptr);
        }
    }

    /** Const overload.
     *
     * Undefined behaviour if @a T is not the held type.
     * @tparam T Type to return
     * @return A reference to held object if it is larger than <TT>std::size_t</TT> and copy
     * constructible, otherwise it returns a copy
     */
    template <typename T>
    [[nodiscard]] auto get() const noexcept
        -> std::conditional_t<(sizeof(T) <= sizeof(std::size_t)) && std::is_copy_constructible_v<T>,
                              std::decay_t<T>,
                              const std::decay_t<T>&>
    {
        using value_type = std::decay_t<T>;

        if constexpr (use_internal_storage<value_type>) {
            return *reinterpret_cast<const value_type*>(&storage_.buffer);
        } else {
            return *reinterpret_cast<const value_type*>(storage_.ptr);
        }
    }

    /** Swap function.
     *
     * @param a First instance
     * @param b Second instance
     */
    friend void swap(unsafe_any_t& a, unsafe_any_t& b) noexcept
    {
        using std::swap;

        swap(a.storage_, b.storage_);
        swap(a.copier_, b.copier_);
        swap(a.destroyer_, b.destroyer_);
    }

private:
    union storage_type {
        constexpr storage_type() noexcept : ptr{nullptr} {}

        ptr_type ptr;
        aligned_storage_type buffer;
    };

    storage_type storage_;
    std::function<unsafe_any_t(const storage_type&)> copier_;
    std::function<void(storage_type&)> destroyer_;
};

/** Typedef for an unsafe_any_t with internal storage big enough to fit a <TT>std::string_view</TT>.
 */
using unsafe_any = unsafe_any_t<sizeof(std::string_view)>;
}  // namespace arg_router::utility
