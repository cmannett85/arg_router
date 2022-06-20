### Copyright (C) 2022 by Camden Mannett.  All rights reserved. 

if(DOCS_ONLY)
    message(STATUS "Documentation-only build")
endif()

set(DOCS_FOR_IDE
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_types/documentation_script.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/related_pages/architecture.doxy
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/related_pages/configuration.doxy
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/related_pages/examples.doxy
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/related_pages/help.doxy
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/related_pages/images/node_relationships.dia
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/related_pages/images/node_relationships.svg
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/related_pages/nodes.doxy
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/related_pages/policies.doxy
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/related_pages/validation.doxy
)

set(README_API_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/docs/README_API.md
)

set(DOCS_COMMAND
    ${CMAKE_COMMAND} -DROOT=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_types/documentation_script.cmake
)

if (DOCS_ONLY)
    add_custom_target(documentation ALL
        COMMAND ${DOCS_COMMAND}
        SOURCES ${DOCS_FOR_IDE}
    )
else()
    add_custom_target(documentation
        COMMAND ${DOCS_COMMAND}
        SOURCES ${DOCS_FOR_IDE}
    )
endif()
add_dependencies(documentation copyright_checker gen_version)

# We have touch the generated API readme because CPack configuration will fail if it is not there.
# set_source_files_properties cannot be used in a script so that's set here too
file(TOUCH ${README_API_PATH})
set_source_files_properties(${README_API_PATH} PROPERTIES GENERATED TRUE)
