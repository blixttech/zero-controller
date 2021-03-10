MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
PROJECT_DIR := $(abspath $(dir $(MAKEFILE_PATH)))
BUILD_DIR := $(PROJECT_DIR)/build/local
PACKAGE_BASE := $(PROJECT_DIR)/package
PACKAGE_DIR := $(PACKAGE_BASE)/local
PACKAGE_UTIL := $(PACKAGE_BASE)/util

$(BUILD_DIR):
	mkdir -p $@

$(PACKAGE_DIR):
	mkdir -p $@

$(PACKAGE_UTIL):
	mkdir -p $@

setup: $(BUILD_DIR) $(PACKAGE_DIR)

run_conan: setup
	conan install $(PROJECT_DIR) --install-folder $(BUILD_DIR) --profile $(CONAN_PROFILE)
	conan build $(PROJECT_DIR) --build-folder $(BUILD_DIR)

pack:
	conan package $(PROJECT_DIR) --build-folder $(BUILD_DIR) --package-folder $(PACKAGE_DIR)

release: CONAN_PROFILE = $(PROJECT_DIR)/conan/profile-linux-release
release: run_conan pack

debug-light: CONAN_PROFILE = $(PROJECT_DIR)/conan/profile-linux-debug-light
debug-light: run_conan

debug: CONAN_PROFILE = $(PROJECT_DIR)/conan/profile-linux-debug-full
debug: run_conan

clean:
	rm -rf build package

	
	
