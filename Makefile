MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
PROJECT_DIR := $(abspath $(dir $(MAKEFILE_PATH)))
BUILD_BASE_DIR := $(PROJECT_DIR)/build

QT_VERSION := 6.4.2
QT_BASE_DIR := $(PROJECT_DIR)/qt/
QT_VERSION_BASE_DIR := $(PROJECT_DIR)/qt/$(QT_VERSION)/
QT_SRC_DIR := $(QT_VERSION_BASE_DIR)/Src/
QT_DIR := $(QT_VERSION_BASE_DIR)/gcc_64/
QT_COAP:=$(QT_SRC_DIR)/qtcoap

TOOLS_DIR:=$(PROJECT_DIR)/tools
PACKAGE_BASE_DIR := $(PROJECT_DIR)/artifacts
APP_DIR:=$(PACKAGE_BASE_DIR)/Release

VERSION=$(shell git describe --always --match v[0-9]* HEAD | cut -c2-)

.PHONY: build

$(QT_VERSION_BASE_DIR):
	mkdir -p $@

$(TOOLS_DIR):
	mkdir -p $@

setup-qt: $(QT_VERSION_BASE_DIR)
	aqt install-qt -O $(QT_BASE_DIR) linux desktop $(QT_VERSION) -m qtscxml
	aqt install-src -O $(QT_BASE_DIR) linux $(QT_VERSION) --archives qtcoap
	cd $(QT_COAP) && Qt6_DIR=$(QT_DIR) cmake .
	cd $(QT_COAP) && make
	cd $(QT_COAP) && cmake --install . --prefix $(QT_DIR)

setup-tools: $(TOOLS_DIR)
	wget -O $(TOOLS_DIR)/linuxdeploy https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
	wget -O $(TOOLS_DIR)/linuxdeploy-plugin-qt https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
	chmod ax+ $(TOOLS_DIR)/linuxdeploy
	chmod ax+ $(TOOLS_DIR)/linuxdeploy-plugin-qt

setup: setup-qt setup-tools


build: BUILD_DIR = $(BUILD_BASE_DIR)/$(BUILD_TYPE)
build: 
	Qt6_DIR=$(QT_DIR) cmake -B $(BUILD_DIR) -S . -D CMAKE_BUILD_TYPE=$(BUILD_TYPE) -D CMAKE_INSTALL_PREFIX=$(PACKAGE_BASE_DIR)/$(BUILD_TYPE)
	cd $(BUILD_DIR) && make


debug: BUILD_TYPE = Debug
debug: build

release: BUILD_TYPE = Release
release: build

pack:
	mkdir -p $(APP_DIR)/bin
	cp $(BUILD_BASE_DIR)/Release/src/zero-controller $(APP_DIR)/bin
	cp $(PROJECT_DIR)/app.dir/* $(APP_DIR)/
	NO_STRIP=1 QMAKE=$(QT_DIR)/bin/qmake VERSION=$(VERSION) \
	$(TOOLS_DIR)/linuxdeploy --appdir $(APP_DIR) \
		--executable $(APP_DIR)/bin/zero-controller \
		--plugin qt \
		--output appimage
		

	

clean:
	rm -rf $(BUILD_BASE_DIR) $(PACKAGE_BASE_DIR)

full_clean: clean
	rm -rf $(QT_BASE_DIR) $(TOOLS_DIR)
	
