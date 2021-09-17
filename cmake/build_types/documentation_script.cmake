# Can't use find_package as it tries to create a target which is not allowed in
# script mode
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
)

execute_process(
    COMMAND           "${DOXYGEN_EXECUTABLE}" ${ROOT}/docs/Doxyfile
    WORKING_DIRECTORY ${ROOT}/docs
    RESULT_VARIABLE   DOXYGEN_RESULT
)

if(NOT ${DOXYGEN_RESULT} STREQUAL "0")
    message(FATAL_ERROR "Doxygen generation failed")
endif()
