// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

/** @file
 *
 * Wrapper for including <TT>windows.h</TT> on Windows platforms. By default <TT>NOMINMAX</TT> and
 * <TT>WIN32_LEAN_AND_MEAN</TT> is defined before inclusion but these can be disabled by defining
 * <TT>AR_NO_*</TT> before if desired.
 */

#ifdef _WIN32
#    if !defined(AR_NO_NOMINMAX) && !defined(NOMINMAX)
#        define NOMINMAX
#    endif

#    if !defined(AR_NO_WIN32_LEAN_AND_MEAN) && !defined(WIN32_LEAN_AND_MEAN)
#        define WIN32_LEAN_AND_MEAN
#    endif

#    include "windows.h"
#endif
