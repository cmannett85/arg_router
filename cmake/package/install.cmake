### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

include(CMakePackageConfigHelpers)

set(INSTALL_BASE_DIR "include")
set(INSTALL_AR_DIR "${INSTALL_BASE_DIR}/arg_router")
set(CONFIG_FILE "${CMAKE_BINARY_DIR}/cmake/package/arg_router-config.cmake")
set(CONFIG_VERSION_FILE "${CMAKE_BINARY_DIR}/cmake/package/arg_router-config-version.cmake")
set(TRANSLATION_GENERATOR_FILES
    "${CMAKE_SOURCE_DIR}/cmake/translation_generator/translation_generator.cmake"
    "${CMAKE_SOURCE_DIR}/cmake/translation_generator/translation_generator_script.cmake")
set(AR_CMAKE_PACKAGE_DIR "share/arg_router"
    CACHE STRING "Installation suffix of CMake package config files, defaults to share/arg_router")

install(TARGETS arg_router
        EXPORT AR_TARGETS)
install(EXPORT AR_TARGETS
        FILE arg_router.cmake
        NAMESPACE arg_router::
        DESTINATION "${AR_CMAKE_PACKAGE_DIR}")

configure_package_config_file("${CMAKE_SOURCE_DIR}/cmake/package/arg_router-config.cmake.in"
                              ${CONFIG_FILE}
                              INSTALL_DESTINATION "${AR_CMAKE_PACKAGE_DIR}"
                              PATH_VARS INSTALL_BASE_DIR)
write_basic_package_version_file(${CONFIG_VERSION_FILE}
                                 COMPATIBILITY SameMajorVersion
                                 ARCH_INDEPENDENT)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/arg_router"
        DESTINATION ${INSTALL_BASE_DIR})
install(FILES "${CMAKE_SOURCE_DIR}/README.md"
              "${CMAKE_SOURCE_DIR}/LICENSE"
        DESTINATION ${INSTALL_AR_DIR})
install(FILES "${CONFIG_FILE}"
              "${CONFIG_VERSION_FILE}"
              ${TRANSLATION_GENERATOR_FILES}
        DESTINATION ${AR_CMAKE_PACKAGE_DIR})

# CPack package configuration
set(CPACK_PACKAGE_VENDOR "Camden Mannett")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)
set(CPACK_RESOURCE_FILE_README ${CMAKE_SOURCE_DIR}/docs/README_API.md)
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}")
set(CPACK_GENERATOR ZIP DEB)
# .deb specific
set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_VENDOR})
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-dev (>= ${BOOST_VERSION})")
include(CPack)
