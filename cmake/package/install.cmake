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
        DESTINATION ${INSTALL_AR_DIR})