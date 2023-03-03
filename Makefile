MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
PROJECT_DIR := $(abspath $(dir $(MAKEFILE_PATH)))
BUILD_DIR := $(PROJECT_DIR)/build
PACKAGE_BASE := $(PROJECT_DIR)/package
PACKAGE_DIR := $(PACKAGE_BASE)/local
PACKAGE_UTIL := $(PACKAGE_BASE)/util
QT_VERSION := 6.4.2
QT_BASE_DIR := $(PROJECT_DIR)/qt/
QT_VERSION_BASE_DIR := $(PROJECT_DIR)/qt/$(QT_VERSION)/
QT_SRC_DIR := $(QT_VERSION_BASE_DIR)/Src/
QT_DIR := $(QT_VERSION_BASE_DIR)/gcc_64/
QT_COAP:=$(QT_SRC_DIR)/qtcoap

.PHONY: build

$(BUILD_DIR):
	mkdir -p $@

$(PACKAGE_DIR):
	mkdir -p $@

$(PACKAGE_UTIL):
	mkdir -p $@

$(QT_VERSION_BASE_DIR):
	mkdir -p $@

setup: $(QT_VERSION_BASE_DIR)
	aqt install-qt -O $(QT_BASE_DIR) linux desktop $(QT_VERSION) -m qtscxml
	aqt install-src -O $(QT_BASE_DIR) linux $(QT_VERSION) --archives qtcoap
	cd $(QT_COAP) && Qt6_DIR=$(QT_DIR) cmake .
	cd $(QT_COAP) && make
	cd $(QT_COAP) && cmake --install . --prefix $(QT_DIR)

build: $(BUILD_DIR)
	cd $(BUILD_DIR) && Qt6_DIR=$(QT_DIR) cmake .. 
	cd $(BUILD_DIR) && make


clean:
	rm -rf build package

full_clean: clean
	rm -rf $(QT_BASE_DIR)
	
