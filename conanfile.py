# Copyright (c) SunnyCase. All rights reserved.
# Licensed under the Apache license. See LICENSE file in the project root for full license information.
# pylint: disable=invalid-name, unused-argument, import-outside-toplevel

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.build import check_min_cppstd, cross_building
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout


class chinoConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "cmake_find_package", "cmake_paths"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "emulator": [True, False],
        "tests": [True, False]
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "emulator": True,
        "tests": False
    }

    @property
    def _min_cppstd(self):
        return 20
    
    def imports(self):
        pass

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        if self.options.emulator:
            self.requires('lyra/1.6.1')

    def build_requirements(self):
        pass

    def config_options(self):
        self.options['zlib-ng'].with_gzfileop = False
        self.options['zlib-ng'].with_optim = True
        
    def validate(self):
        if self.settings.compiler.get_safe("cppstd"):
            check_min_cppstd(self, self._min_cppstd)

        if self.settings.os == 'Windows':
            self.settings.compiler.toolset = 'ClangCL'

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["BUILD_TESTING"] = self.options.tests
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()