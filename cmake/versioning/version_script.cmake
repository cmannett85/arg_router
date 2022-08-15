### Copyright (C) 2022 by Camden Mannett.  All rights reserved. 

# Taken from https://stackoverflow.com/a/50104093/498437
#

include("${ORIG_BASE_PATH}/cmake/path_prefixer.cmake")

find_package(Git REQUIRED)

execute_process(
    OUTPUT_VARIABLE   GIT_REV
    COMMAND           "${GIT_EXECUTABLE}" rev-parse -q HEAD
    WORKING_DIRECTORY ${ORIG_BASE_PATH}
)

string(STRIP ${GIT_REV} GIT_REV)
message(STATUS "Project revision: ${PROJECT_VERSION}.${GIT_REV}")

path_prefixer(VERSION_FILE_TMP cmake/versioning/version.hpp.tmp)
configure_file(
    "${ORIG_BASE_PATH}/cmake/versioning/version.hpp.in"
    ${VERSION_FILE_TMP}
)

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${VERSION_FILE_TMP} ${VERSION_FILE}
)

# Update the project version in the Doxyfile too
file(READ "${ORIG_BASE_PATH}/docs/Doxyfile" DOXYFILE_DATA)

string(
    REGEX REPLACE
    "PROJECT_NUMBER         = [0-9]*\\.[0-9]*\\.[0-9]*"
    "PROJECT_NUMBER         = ${PROJECT_VERSION}"
    UPDATED_DOXYFILE_DATA
    "${DOXYFILE_DATA}"
)

if(NOT "${DOXYFILE_DATA}" STREQUAL "${UPDATED_DOXYFILE_DATA}")
    message(STATUS "Updating Doxyfile project version")
    file(WRITE "${ORIG_BASE_PATH}/docs/Doxyfile" ${UPDATED_DOXYFILE_DATA})
endif()   
