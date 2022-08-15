### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

find_package (Python3 REQUIRED COMPONENTS Interpreter)

# Creates a copyright_checker target
function(create_copyright_checker_target)
    cmake_parse_arguments(ARGS "" "NAME" "SOURCES" ${ARGN})

    add_custom_target(${ARGS_NAME}
        COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/scripts/copyright_checker.py"
            presence
            "${CMAKE_CURRENT_SOURCE_DIR}"
        SOURCES ${ARGS_SOURCES}
    )
endfunction()
