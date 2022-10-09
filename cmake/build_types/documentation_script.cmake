### Copyright (C) 2022 by Camden Mannett.
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

# Remove the badges
string(
    REGEX REPLACE
    "^(!\\[[a-zA-Z0-9 ]+\\]\\([a-zA-Z0-9:\\/\\/\\.%-_]+\\) ?)+\n\n"
    ""
    UPDATED_MD_DATA
    "${MD_DATA}"
)

if(NOT "${MD_DATA}" STREQUAL "${UPDATED_MD_DATA}")
    message(STATUS "Updating Doxygen README.md")
    file(WRITE ${NEW_API_MD_PATH} ${UPDATED_MD_DATA})
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
