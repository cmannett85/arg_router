### Copyright (C) 2022 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

path_prefixer(VERSION_FILE
    include/arg_router/version.hpp
)

set_source_files_properties(${VERSION_FILE} PROPERTIES GENERATED TRUE)

find_package(Git REQUIRED)
execute_process(
    COMMAND           "${GIT_EXECUTABLE}" rev-parse -q HEAD
    OUTPUT_VARIABLE   GIT_REV
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

string(STRIP ${GIT_REV} GIT_REV)
message(STATUS "Project revision: ${CMAKE_PROJECT_VERSION}.${GIT_REV}")

set(VERSION_FILE_TMP
    "${CMAKE_BINARY_DIR}/cmake/versioning/version.hpp.tmp"
)
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/versioning/version.hpp.in"
    ${VERSION_FILE_TMP}
)
if(NOT "${VERSION_FILE_TMP}" STREQUAL "${VERSION_FILE}")
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
    message(STATUS "Updating Doxyfile project version")
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
    message(STATUS "Updating vcpkg.json version")
    file(WRITE "${CMAKE_SOURCE_DIR}/vcpkg.json" ${UPDATED_VCPKG_JSON_DATA})
endif()
