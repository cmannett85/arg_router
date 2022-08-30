### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

include(CMakePackageConfigHelpers)

set(INSTALL_BASE_DIR "include")
set(INSTALL_AR_DIR "${INSTALL_BASE_DIR}/arg_router")
set(CONFIG_FILE "${CMAKE_BINARY_DIR}/cmake/package/arg_router-config.cmake")
set(CONFIG_VERSION_FILE "${CMAKE_BINARY_DIR}/cmake/package/arg_router-config-version.cmake")

configure_package_config_file("${CMAKE_SOURCE_DIR}/cmake/package/arg_router-config.cmake.in"
                              ${CONFIG_FILE}
                              INSTALL_DESTINATION ${INSTALL_AR_DIR}
                              PATH_VARS INSTALL_BASE_DIR)
write_basic_package_version_file(${CONFIG_VERSION_FILE}
                                 COMPATIBILITY SameMajorVersion
                                 ARCH_INDEPENDENT)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/arg_router"
        DESTINATION ${INSTALL_BASE_DIR})
install(FILES ${CONFIG_FILE}
              ${CONFIG_VERSION_FILE}
              "${CMAKE_SOURCE_DIR}/README.md"
              "${CMAKE_SOURCE_DIR}/LICENSE"
        DESTINATION ${INSTALL_AR_DIR})

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
