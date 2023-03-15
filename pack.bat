@echo off
cd %~dp0\artifact\Release\bin

windeployqt zero-controller.exe --release --compiler-runtime
copy C:\Windows\System32\concrt140.dll . 
copy C:\Windows\System32\vccorlib140.dll .
copy C:\Windows\System32\msvcp140.dll .
copy C:\Windows\System32\vcruntime140.dll .
7z a C:\zero-controller.zip *          

