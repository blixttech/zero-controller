# Desktop Application for Controlling Blixt Zero

## Introduction
This project provides a desktop application for controlling [Blixt Zero](https://blixt.tech/) devices.
Blixt Zero uses the [Constrained Application Protocol](https://tools.ietf.org/html/rfc7252) (CoAP) for controlling the devices remotely. 

**NOTE: This application is a work in progress. Therefore, features/functionalities are subjected for changes without prior notice.**

## Development workflow (Linux)

The development workflow of this project centers on [Conan](https://conan.io/) and [CMake](https://cmake.org/) due to the convenience of managing dependencies.

### Prerequisites

- [Conda](https://docs.conda.io/) (optional, but recommended)
    - Used ONLY as a Python environment manager.
    - Pro tip: If you are setting up development environment for the first time, install [Miniconda](https://docs.conda.io/en/latest/miniconda.html) to save storage space and your time. 
- [Conan](https://conan.io/)
- [CMake](https://cmake.org/)
- C++ development tools
    - minimum g++-7 

### Build environment setup
    
Assuming, [Conda](https://docs.conda.io/) is already installed, 

```bash
# Create a Python environment for Conan 
conda create --name conan python=3 
source activate conan
# (conan)$ Now, we are in the "conan" conda environment
pip install pip --upgrade
pip install conan
# Adding required conan package repositories
conan remote add blixttech-bintray https://api.bintray.com/conan/blixttech/conan-packages
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
```

### Building

There are three build types for the Linux build:
- Release

  The final executable binary and the dependent libraries are built in **release mode**.  

- Light debug

  Only the final executable binary is built in **debug mode** and the dependent libraries are built in **release mode**.
  Though mixing debug and release binaries/libraries sometimes considered as a bad practice (impossible to do with Microsoft Visual C++), it offers great advantages in terms of storage and performance.  

- Full debug

  The final executable binary and the dependent libraries are built in **debug mode**.

The program is build using the provided Makefile as follows.


- Release

    ```bash
    # (conan)$ We are in the "conan" conda environment
    make release
    ```

- Light debug

    ```bash
    # (conan)$ We are in the "conan" conda environment
    make debug-light
    ```

- Full debug

    ```bash
    # (conan)$ We are in the "conan" conda environment
    make debug
    ```

The final executable is uploaded as AppImage. To perform the AppImage build, just run
    ```bash
    # (conan)$ We are in the "conan" conda environment
    make pack
    ```

### Running


```bash
# (conan)$ We are in the "conan" conda environment
source build/local/activate_run.sh
# (conanrunenv) (conan)$ We are in the "conan" conda & conan's virtual run environment
./build/local/bin/zero-controller
```

As this project uses [CMake](https://cmake.org/), successive rebuilds do not require invoking the Makefile again. Instead, ``make`` can be used as follows to rebuild if the dependencies, build steps and related environmental variables are not changed in the [conanfile.py](conanfile.py) file. 

```bash
# (conanrunenv) (conan)$ We are in the "conan" conda & conan's virtual run environment
cd build/local
make
```
