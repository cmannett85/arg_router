### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

message(STATUS "Project revision: ${CMAKE_PROJECT_VERSION}")

# Update the version in version.hpp
path_prefixer(VERSION_FILE
    include/arg_router/version.hpp
)
set(VERSION_FILE_TMP
    "${CMAKE_BINARY_DIR}/cmake/versioning/version.hpp.tmp"
)
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/versioning/version.hpp.in"
    ${VERSION_FILE_TMP}
)

file(READ ${VERSION_FILE} VERSION_FILE_DATA)
file(READ ${VERSION_FILE_TMP} VERSION_FILE_TMP_DATA)
if(NOT "${VERSION_FILE_TMP_DATA}" STREQUAL "${VERSION_FILE_DATA}")
    message(STATUS "Updating version.hpp library version")
    file(RENAME ${VERSION_FILE_TMP} ${VERSION_FILE})
endif()

# Update the project version in the Doxyfile
file(READ "${CMAKE_SOURCE_DIR}/docs/Doxyfile" DOXYFILE_DATA)
string(
    REGEX REPLACE
    "PROJECT_NUMBER         = [0-9]*\\.[0-9]*\\.[0-9]*"
    "PROJECT_NUMBER         = ${CMAKE_PROJECT_VERSION}"
    UPDATED_DOXYFILE_DATA
    "${DOXYFILE_DATA}"
)

if(NOT "${DOXYFILE_DATA}" STREQUAL "${UPDATED_DOXYFILE_DATA}")
    message(STATUS "Updating Doxyfile library version")
    file(WRITE "${CMAKE_SOURCE_DIR}/docs/Doxyfile" ${UPDATED_DOXYFILE_DATA})
endif()

# Update the version in vcpkg.json
file(READ "${CMAKE_SOURCE_DIR}/vcpkg.json" VCPKG_JSON_DATA)
string(
    REGEX REPLACE
    "\"version-string\": \"[0-9]*\\.[0-9]*\\.[0-9]*\""
    "\"version-string\": \"${CMAKE_PROJECT_VERSION}\""
    UPDATED_VCPKG_JSON_DATA
    "${VCPKG_JSON_DATA}"
)

if(NOT "${VCPKG_JSON_DATA}" STREQUAL "${UPDATED_VCPKG_JSON_DATA}")
    message(STATUS "Updating vcpkg.json library version")
    file(WRITE "${CMAKE_SOURCE_DIR}/vcpkg.json" ${UPDATED_VCPKG_JSON_DATA})
endif()

# Update the version in conanfile.py
file(READ "${CMAKE_SOURCE_DIR}/conanfile.py" CONANFILE_DATA)
string(
    REGEX REPLACE
    "version = [0-9]*\\.[0-9]*\\.[0-9]*"
    "version = ${CMAKE_PROJECT_VERSION}"
    UPDATED_CONANFILE_DATA
    "${CONANFILE_DATA}"
)

if(NOT "${CONANFILE_DATA}" STREQUAL "${UPDATED_CONANFILE_DATA}")
    message(STATUS "Updating conanfile.py library version")
    file(WRITE "${CMAKE_SOURCE_DIR}/conanfile.py" ${UPDATED_CONANFILE_DATA})
endif()
