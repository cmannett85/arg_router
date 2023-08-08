### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

# Can't use find_package as it tries to create a target which is not allowed in script mode
find_program(
    DOXYGEN_EXECUTABLE
    NAMES doxygen
    PATHS
        "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\doxygen_is1;Inno Setup: App Path]/bin"
        /Applications/Doxygen.app/Contents/Resources
        /Applications/Doxygen.app/Contents/MacOS
        /Applications/Utilities/Doxygen.app/Contents/Resources
        /Applications/Utilities/Doxygen.app/Contents/MacOS
    DOC "Doxygen documentation generation tool (http://www.doxygen.org)"
    REQUIRED
)

set(API_MD_PATH     "${ROOT}/README.md")
set(NEW_API_MD_PATH "${ROOT}/docs/README_API.md")

file(READ ${API_MD_PATH} MD_DATA)

# Remove the badges, by simply removing the first couple of lines
string(FIND "${MD_DATA}" "\n\n" NL_IDX)
if(${NL_IDX} EQUAL -1)
    message(FATAL "Cannot find README.md first newline")
endif()
math(EXPR NL_IDX "${NL_IDX} + 2")
string(SUBSTRING "${MD_DATA}" ${NL_IDX} -1 UPDATED_MD_DATA)

if(NOT "${MD_DATA}" STREQUAL "${UPDATED_MD_DATA}")
    message(STATUS "Updating Doxygen README.md")
    file(WRITE ${NEW_API_MD_PATH} "${UPDATED_MD_DATA}")
else()
    message(STATUS "README.md up to date")
endif()

execute_process(
    COMMAND           "${DOXYGEN_EXECUTABLE}" "${ROOT}/docs/Doxyfile"
    WORKING_DIRECTORY "${ROOT}/docs"
    RESULT_VARIABLE   DOXYGEN_RESULT
    ERROR_VARIABLE    DOXY_ERROR_STRING
)

if(NOT ${DOXYGEN_RESULT} EQUAL 0)
    message(FATAL_ERROR "Doxygen generation failed: ${DOXY_ERROR_STRING}")
endif()
