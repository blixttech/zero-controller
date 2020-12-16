import os
import sys
import shutil
import pathlib
import re
from conans import ConanFile, CMake, tools
from conans.errors import ConanException


class BCBControllerDesktopConan(ConanFile):
    name = "bcbcontroller-desktop"
    description = "Desktop application for controlling Blixt Circuit Breaker"
    url = "https://github.com/blixttech/bcbcontroller-desktop"
    homepage = "https://github.com/blixttech/bcbcontroller-desktop"
    license = "GPL-3.0-only"

    exports_sources = ["src/*", "app.dir/*", "resources/*", "CMakeLists.txt"]
    generators = "cmake", "virtualrunenv"

    settings = "os", "compiler", "build_type", "arch"

    _git_is_dirty = False
    _git_commit = "unknown"
    _cmake = None

    def set_version(self):
        git = tools.Git(folder=self.recipe_folder)
        if not self.version:
            output = git.run("describe --all").splitlines()[0].strip()
            self.version = re.sub("^.*/v?|^v?", "", output)
        output = git.run("diff --stat").splitlines()
        self._git_is_dirty = True if output else False
        self._git_commit = git.run("rev-parse HEAD").splitlines()[0].strip()

        self.output.info("Version: %s, Commit: %s, Is_dirty: %s" %
                         (self.version, self._git_commit, self._git_is_dirty))

    def requirements(self):
        self.requires("qt/5.14.2@bincrafters/stable")
        self.requires("qtsvg/5.14.2@blixt/stable")
        self.requires("qtcoap/5.14.2@blixt/stable")

    def _configure_cmake(self):
        if not self._cmake:
            self._cmake = CMake(self)
            self._cmake.definitions["USE_CONAN_BUILD_INFO"] = "ON"
            self._cmake.definitions["SOURCE_VERSION"] = self.version
            self._cmake.definitions["SOURCE_COMMIT"] = self._git_commit
            self._cmake.definitions["SOURCE_DIRTY"] = self._git_is_dirty
            del self._cmake.definitions["CMAKE_EXPORT_NO_PACKAGE_REGISTRY"]
            self._cmake.configure()
        return self._cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

        if self.settings.os == "Linux":
            self._package_linux()
        else:
            raise ConanException("Packaging is not supported for %s" % self.settings.os)

    def deploy(self):
        if self.settings.os == "Linux":
            self._deploy_linux()
        else:
            raise ConanException("Deploying is not supported for %s" % self.settings.os)

    def _package_linux(self):
        self.copy("*", dst="app.dir", src="app.dir")
        self.copy("*", dst=os.path.join("app.dir", "usr", "bin"), src="bin")

    def _deploy_linux(self):
        self.copy("*", dst="app.dir", src="app.dir")
