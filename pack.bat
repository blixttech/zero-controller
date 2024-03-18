@echo off

cd %~dp0\artifact\Release\bin

windeployqt zero-controller.exe --release --compiler-runtime
copy %Qt6_DIR%\lib\qwt.dll .
copy C:\Windows\System32\concrt140.dll . 
copy C:\Windows\System32\vccorlib140.dll .
copy C:\Windows\System32\msvcp140.dll .
copy C:\Windows\System32\vcruntime140.dll .

copy C:\vcpkg\buildtrees\protobuf\x64-windows-rel\libprotobuf.dll .

7z a C:\zero-controller-win64.zip *          

