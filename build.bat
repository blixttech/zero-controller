@echo off
set btype=%1

set BDIR=build\zero-controller\%btype%

cmake -G Ninja -B %BDIR% -S . -D CMAKE_BUILD_TYPE=%btype% -DCMAKE_TOOLCHAIN_FILE=c:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build %BDIR%
cmake --install %BDIR% --prefix %~dp0\artifact\%btype%


