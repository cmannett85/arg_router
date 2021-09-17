if(DOCS_ONLY)
    message(STATUS "Documentation-only build")
endif()

set(DOCS_FOR_IDE
     ${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_types/documentation_script.cmake
)

add_custom_target(documentation
    COMMAND ${CMAKE_COMMAND} -DROOT=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_types/documentation_script.cmake
    SOURCES ${DOCS_FOR_IDE} ${FOR_IDE}
)
add_dependencies(documentation gen_version)
