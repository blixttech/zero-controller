@echo off
cd C:\vcpkg
tree /f /a 

cd %~dp0\artifact\Release\bin

windeployqt zero-controller.exe --release --compiler-runtime
copy %Qt6_DIR%\lib\qwt.dll .
copy C:\Windows\System32\concrt140.dll . 
copy C:\Windows\System32\vccorlib140.dll .
copy C:\Windows\System32\msvcp140.dll .
copy C:\Windows\System32\vcruntime140.dll .
copy C:\vcpkg\packages\protobuf_x64-windows\lib\libprotobuf.dll .

7z a C:\zero-controller-win64.zip *          

