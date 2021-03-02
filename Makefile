MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
PROJECT_DIR := $(abspath $(dir $(MAKEFILE_PATH)))
BUILD_DIR := $(PROJECT_DIR)/build/local
PACKAGE_BASE := $(PROJECT_DIR)/package
PACKAGE_DIR := $(PACKAGE_BASE)/local
PACKAGE_UTIL := $(PACKAGE_BASE)/util

APPIMG_TOOL_NAME := appimagetool-x86_64.AppImage
APPIMG_TOOL := $(PACKAGE_UTIL)/$(APPIMG_TOOL_NAME)

LINUXDPLY_TOOL_NAME := linuxdeploy-x86_64.AppImage
LINUXDPLY_TOOL := $(PACKAGE_UTIL)/$(LINUXDPLY_TOOL_NAME)

LINUXDPLYQT_TOOL_NAME := linuxdeploy-plugin-qt-x86_64.AppImage
LINUXDPLYQT_TOOL := $(PACKAGE_UTIL)/$(LINUXDPLYQT_TOOL_NAME)

$(BUILD_DIR):
	mkdir -p $@

$(PACKAGE_DIR):
	mkdir -p $@

$(PACKAGE_UTIL):
	mkdir -p $@

$(APPIMG_TOOL): URL = https://github.com/AppImage/AppImageKit/releases/download/continuous/$(APPIMG_TOOL_NAME)
$(APPIMG_TOOL):
	wget -P $(PACKAGE_UTIL) -c -nv $(URL) 
	chmod a+x $@

$(LINUXDPLY_TOOL): URL = https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/$(LINUXDPLY_TOOL_NAME)
$(LINUXDPLY_TOOL):
	wget -P $(PACKAGE_UTIL) -c -nv $(URL) 
	chmod a+x $@

$(LINUXDPLYQT_TOOL): URL = "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/$(LINUXDPLYQT_TOOL_NAME)"
$(LINUXDPLYQT_TOOL):
	wget -P $(PACKAGE_UTIL) -c -nv $(URL) 
	chmod a+x $@

PACKAGE_TOOLS := $(APPIMG_TOOL) $(LINUXDPLY_TOOL) $(LINUXDPLYQT_TOOL)

setup: $(BUILD_DIR) $(PACKAGE_DIR)

run_conan: setup
	conan install $(PROJECT_DIR) --install-folder $(BUILD_DIR) --profile $(CONAN_PROFILE)
	conan build $(PROJECT_DIR) --build-folder $(BUILD_DIR)

pack: $(PACKAGE_TOOLS)
	conan package $(PROJECT_DIR) --build-folder $(BUILD_DIR) --package-folder $(PACKAGE_DIR)
	NO_STRIP=1 $(LINUXDPLY_TOOL) --appdir=$(PACKAGE_DIR)/app.dir --plugin=qt --output appimage
#	rm -rf $(PACKAGE_DIR)/app.dir $(PACKAGE_DIR)/usr $(PACKAGE_DIR)/lib $(PACKAGE_DIR)/libxkbcommon*
#	$(LINUXDPLYQT_TOOL) --appdir=$(PACKAGE_DIR)/app.dir

release: CONAN_PROFILE = $(PROJECT_DIR)/conan/profile-linux-release run_conan pack

debug-light: CONAN_PROFILE = $(PROJECT_DIR)/conan/profile-linux-debug-light run_conan

debug: CONAN_PROFILE = $(PROJECT_DIR)/conan/profile-linux-debug-full
debug: run_conan

clean:
	rm -rf build package

	
	
