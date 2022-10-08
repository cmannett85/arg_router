### Copyright (C) 2022 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

function(path_prefixer VAR)
    foreach(SOURCE ${ARGN})
        list(APPEND NEW_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE}")
    endforeach()

    set(${VAR} "${NEW_SOURCES}" PARENT_SCOPE)
endfunction()
