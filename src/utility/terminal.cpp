// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/utility/terminal.hpp"

#if defined(__linux__) || defined(__APPLE__)
#    include <sys/ioctl.h>
#elif _WIN32
#    if !defined(AR_NO_NOMINMAX) && !defined(NOMINMAX)
#        define NOMINMAX
#    endif

#    if !defined(AR_NO_WIN32_LEAN_AND_MEAN) && !defined(WIN32_LEAN_AND_MEAN)
#        define WIN32_LEAN_AND_MEAN
#    endif

#    include "windows.h"
#endif

namespace arg_router::utility::terminal
{
#ifdef UNIT_TEST_BUILD
[[nodiscard]] std::size_t& test_columns_value() noexcept
{
    static std::size_t value;
    return value;
}
#endif

std::size_t columns() noexcept
{
#ifdef UNIT_TEST_BUILD
    return test_columns_value();
#else
#    if defined(__linux__) || defined(__APPLE__)
    // NOLINTBEGIN(*-member-init, *-vararg)
    struct winsize w;
    if (::ioctl(0, TIOCGWINSZ, &w) != 0) {
        return 0;
    }
    // NOLINTEND(*-member-init, *-vararg)
    return w.ws_col;
#    elif _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &csbi)) {
        return 0;
    }
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#    endif
#endif
}
}  // namespace arg_router::utility::terminal
