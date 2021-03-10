import os
import os.path
import stat
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

    linuxdeploy = "package/utils/linuxdeploy-x86_64.AppImage"

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
        if not os.path.isfile(self.linuxdeploy):
            tools.download("https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage", self.linuxdeploy, overwrite=True)
            st = os.stat(self.linuxdeploy)
            os.chmod(self.linuxdeploy, st.st_mode | stat.S_IEXEC)

        linuxdeploy_qt = "package/utils/linuxdeploy-plugin-qt-x86_64.AppImage"
        if not os.path.isfile(linuxdeploy_qt):
            tools.download("https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage", linuxdeploy_qt, overwrite=True)
            st = os.stat(linuxdeploy_qt)
            os.chmod(linuxdeploy_qt, st.st_mode | stat.S_IEXEC)

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

    def _package_linux(self):
        self.copy("*", dst="app.dir", src="app.dir")
        self.copy("*", dst=os.path.join("app.dir", "usr", "bin"), src="bin")

        linuxdeploy_args = []
        linuxdeploy_args.append("--appdir")
        linuxdeploy_args.append(os.path.join(self.package_folder, "app.dir"))
        linuxdeploy_args.append("--plugin")
        linuxdeploy_args.append("qt")
        linuxdeploy_args.append("--output")
        linuxdeploy_args.append("appimage")
        
        deploy_env = {"NO_STRIP": "1"}
        with tools.environment_append(deploy_env):
            dplyExec = os.path.join(self.recipe_folder, self.linuxdeploy)
            self.run(dplyExec + " %s" % " ".join(linuxdeploy_args), run_environment=True)

        
