# Copyright (C) 2023 by Camden Mannett.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

import os
from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMakeDeps, CMakeToolchain, CMake


class arg_routerRecipe(ConanFile):
    name = "arg_router"
    version = "1.2.2"

    license = "BSL-1.0"
    author = "Camden Mannett"
    url = "https://github.com/cmannett85/arg_router"
    description = "C++ command line argument parsing and routing."
    topics = ("cpp", "command-line", "argument-parser", "libraries")

    settings = "build_type", "compiler"
    default_options = {"boost/*:header_only": True}
    package_type = "header-library"
    generators = "CMakeDeps", "CMakeToolchain"

    exports_sources = "CMakeLists.txt", "README.md", "LICENSE", "cmake/*", "include/*", "docs/*", "README.md"
    no_copy_source = True

    def requirements(self):
        self.requires("boost/[>=1.74.0]")

    def validate(self):
        check_min_cppstd(self, 17)

    def layout(self):
        self.folders.build = "."
        self.folders.generators = "."
        self.cpp.source.includedirs = ["include"]

    def build(self):
        cmake = CMake(self)
        cmake.configure(variables={"INSTALLATION_ONLY": True})
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.builddirs.append(os.path.join("share", "arg_router"))
        self.cpp_info.set_property("cmake_find_mode", "none")

    def package_id(self):
        # build_type and compiler are needed for the Conan's CMake tools but are not actually used
        self.info.settings.clear()
