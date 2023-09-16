// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/parsing/dynamic_token_adapter.hpp"

namespace arg_router::parsing
{
dynamic_token_adapter::iterator dynamic_token_adapter::insert(iterator it, value_type value)
{
    // Only transfer up to the element before the target position, otherwise we transfer that
    // and then insert the new value, which isn't expected
    transfer(it - 1);
    auto processed_it = it.is_end() ? processed_->end() :  //
                                      processed_->begin() + it.i_;

    processed_->insert(processed_it, value);
    return {this, it.i_};
}

dynamic_token_adapter::iterator dynamic_token_adapter::erase(iterator it)
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

void dynamic_token_adapter::transfer(iterator it)
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
}  // namespace arg_router::parsing
