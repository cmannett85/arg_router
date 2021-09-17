# Taken from https://stackoverflow.com/a/50104093/498437
#

find_package(Git REQUIRED)

set(VERSION_FILE_TMP ${CMAKE_CURRENT_SOURCE_DIR}/cmake/versioning/version.hpp.tmp)

execute_process(
    OUTPUT_VARIABLE   GIT_REV
    COMMAND           ${GIT_EXECUTABLE} rev-parse -q HEAD
    WORKING_DIRECTORY ${ORIG_BASE_PATH}
)

string(STRIP ${GIT_REV} GIT_REV)
message(STATUS "Project revision: ${PROJECT_VERSION}.${GIT_REV}")

configure_file(
    ${ORIG_BASE_PATH}/cmake/versioning/version.hpp.in
    ${VERSION_FILE_TMP}
)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VERSION_FILE_TMP} ${VERSION_FILE}
)

# Update the project version in the Doxyfile too
file(READ ${ORIG_BASE_PATH}/docs/Doxyfile DOXYFILE_DATA)

string(
    REGEX REPLACE
    "PROJECT_NUMBER         = [0-9]*\\.[0-9]*\\.[0-9]*"
    "PROJECT_NUMBER         = ${PROJECT_VERSION}"
    UPDATED_DOXYFILE_DATA
    "${DOXYFILE_DATA}"
)

if(NOT "${DOXYFILE_DATA}" STREQUAL "${UPDATED_DOXYFILE_DATA}")
    message(STATUS "Updating Doxyfile project version")
    file(WRITE ${ORIG_BASE_PATH}/docs/Doxyfile ${UPDATED_DOXYFILE_DATA})
endif()   
