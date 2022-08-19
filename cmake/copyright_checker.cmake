### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

find_package (Python3 REQUIRED COMPONENTS Interpreter)

execute_process(COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/scripts/copyright_checker.py"
                presence
                "${CMAKE_SOURCE_DIR}"
                RESULT_VARIABLE COPYRIGHT_PASS)
if (NOT COPYRIGHT_PASS EQUAL 0)
    message(FATAL_ERROR "Copyright check failure")
endif()
