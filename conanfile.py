import os
import sys
import shutil
import pathlib
import re
from conans import ConanFile, CMake, tools
from conans.errors import ConanException
from conans.client import generators
from conans.client.run_environment import RunEnvironment
from conans.client.tools.oss import OSInfo


class CustomRunEnvGenerator(generators.VirtualRunEnvGenerator):

    def __init__(self, conanfile):
        super(CustomRunEnvGenerator, self).__init__(conanfile)
        run_env = RunEnvironment(conanfile)
        self.env = run_env.vars
        os_info = OSInfo()
        if os_info.is_posix:
            self.env['FONTCONFIG_PATH'] = "/etc/fonts"


class BZControllerConan(ConanFile):
    name = "bzcontroller"
    description = "Desktop application for controlling Blixt Zero"
    url = "https://github.com/blixttech/bzcontroller"
    homepage = "https://github.com/blixttech/bzcontroller"
    license = "GPL-3.0-only"

    exports_sources = ["src/*", "app.dir/*", "resources/*", "CMakeLists.txt"]
    generators = "cmake", "CustomRunEnvGenerator"

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
        #self.requires("linuxdeploy-bundle-qt/continuous@bincreators/stable")

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
        print ("HERE")
        self.copy("*", dst="app.dir", src="app.dir")
        self.copy("*", dst=os.path.join("app.dir", "usr", "bin"), src="bin")

    def _deploy_linux(self):
        self.copy("*", dst="app.dir", src="app.dir")
        
        print ("HERE")
        linuxdeploy_args = []
        linuxdeploy_args.append("--appdir")
        linuxdeploy_args.append(os.path.join(self.install_folder, "app.dir"))
        linuxdeploy_args.append("--plugin")
        linuxdeploy_args.append("qt")
        # libxkbcommon* libraries from conan causes problems in Qt when loading keyboard information.
        # Looks like libxkbcommon* libraries are included in most of the Linux distributions.
        # So we have to delete libxkbcommon* libraries discovered by linuxdeploy prior to create the
        # appimage.
        # Therefore, we create the appimage in a later step using linuxdeploy-plugin-appimage.   
        appimage_args = []
        appimage_args.append("--appdir")
        appimage_args.append(os.path.join(self.install_folder, "app.dir"))

        deploy_env = {"NO_STRIP": "1"}
        deploy_env["OUTPUT"] = "{0}-{1}.AppImage".format(self.name, self.settings.arch)
        with tools.environment_append(deploy_env):
            self.run("linuxdeploy %s" % " ".join(linuxdeploy_args), run_environment=True)
            self.run("rm -rf %s" % os.path.join(self.install_folder, 
                                                "app.dir", 
                                                "usr", 
                                                "lib", 
                                                "libxkbcommon*"), 
                    run_environment=True) 
            self.run("linuxdeploy-plugin-appimage %s" % " ".join(appimage_args), run_environment=True)

        
