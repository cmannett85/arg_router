### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

function(path_prefixer VAR)
    foreach(SOURCE ${ARGN})
        list(APPEND NEW_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE}")
    endforeach()

    set(${VAR} "${NEW_SOURCES}" PARENT_SCOPE)
endfunction()
