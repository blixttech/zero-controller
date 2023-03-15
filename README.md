# Desktop Application for Controlling Blixt Zero

## Introduction
This project provides a desktop application for controlling [Blixt Zero](https://blixt.tech/) devices.
Blixt Zero uses the [Constrained Application Protocol](https://tools.ietf.org/html/rfc7252) (CoAP) for controlling the devices remotely. 

**NOTE: This application is a work in progress. Therefore, features/functionalities are subjected for changes without prior notice.**

## Development workflow

### Linux
#### Build-environment setup

The software uses several tools to manage the build process

- [Conda](https://docs.conda.io/) (optional, but recommended)
    - Used ONLY as a Python environment manager.
    - Pro tip: If you are setting up development environment for the first time, install [Miniconda](https://docs.conda.io/en/latest/miniconda.html) to save storage space and your time. 
- [CMake](https://cmake.org/)
- aqt for installing Qt
- C++ development tools
    - minimum g++-9

Assuming, Conda and CMake are already installed, the following will setup the build environment.
This step will only have to be executed once.

```bash
# Create a Python environment for Conan 
conda create --name zero python=3 
conda activate zero
# (zero)$ Now, we are in the "zero" conda environment
pip install pip --upgrade

# install aqtinstall to manage Qt installations
pip install aqtinstall

# install all Qt and additional libraries
make setup
```

#### Building & Packing

- Release

    ```bash
    # (zero)$ We are in the "zero" conda environment
    make release
    ```

- Debug

    ```bash
    # (zero)$ We are in the "zero" conda environment
    make debug
    ```

The final executable is uploaded as AppImage. 
! This is only needed if you want to test the final AppImage and should usually be done on a Release build
To perform the AppImage build, just run
    ```bash
    # (zero)$ We are in the "zero" conda environment
    make pack
    ```

#### Running

To run the debug build:
```bash
# (qt)$ We are in the "qt" conda environment
./build/Debug/bin/zero-controller
```

### Windows

#### Build Environment Setup
- Install the [Visual Studio build tools] (https://visualstudio.microsoft.com/downloads/?q=build+tools)
- [Conda](https://docs.conda.io/) (optional, but recommended)
    - Used ONLY as a Python environment manager.
    - Pro tip: If you are setting up development environment for the first time, install [Miniconda](https://docs.conda.io/en/latest/miniconda.html) to save storage space and your time. 

Open a developer tools command prompt:
```cmd
# Create a Python environment for Conan 
conda create --name zero python=3 
conda activate zero
# (zero)$ Now, we are in the "zero" conda environment
pip install pip --upgrade

# install aqtinstall to manage Qt installations
pip install aqtinstall

# install all Qt and additional libraries
setup.bat
```

#### Building

Debug:
```cmd
build_debug.bat
```

Release:
```cmd
build_release.bat
```

Pack:
```cmd
pack.bat
```
