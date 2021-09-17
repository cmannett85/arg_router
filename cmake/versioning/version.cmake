set(VERSION_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/arg_router/version.hpp
)

set(VERSION_FOR_IDE
     ${CMAKE_CURRENT_SOURCE_DIR}/cmake/versioning/version.cmake
     ${CMAKE_CURRENT_SOURCE_DIR}/cmake/versioning/version.hpp.in
     ${CMAKE_CURRENT_SOURCE_DIR}/cmake/versioning/version_script.cmake
)

set_source_files_properties(${VERSION_FILE} PROPERTIES GENERATED TRUE)

add_custom_target(gen_version
    ALL
    COMMAND ${CMAKE_COMMAND} -DORIG_BASE_PATH=${CMAKE_CURRENT_SOURCE_DIR} -DPROJECT_VERSION=${PROJECT_VERSION} -DVERSION_FILE=${VERSION_FILE} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/versioning/version_script.cmake
    SOURCES ${VERSION_FOR_IDE}
)
