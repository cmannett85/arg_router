// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>

#if defined(__linux__) || defined(__APPLE__)
#    include <sys/ioctl.h>
#elif _WIN32
#    include <windows.h>
#endif

/** Namespace for terminal utilities. */
namespace arg_router::utility::terminal
{
#ifdef UNIT_TEST_BUILD
extern std::size_t test_columns_value;
#endif

/** Returns the current number columns in the terminal.
 *
 * @return Column count
 */
inline std::size_t columns()
{
#ifdef UNIT_TEST_BUILD
    return test_columns_value;
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
