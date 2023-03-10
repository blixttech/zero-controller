@echo off

set QTVERSION=6.4.2
set QTBASEDIR=%~dp0\qt
set QTVERDIR=%QTBASEDIR%\%QTVERSION%\
aqt install-qt -O %QTBASEDIR% windows desktop %QTVERSION% win64_msvc2019_64 -m qtscxml
aqt install-src -O %QTBASEDIR% windows %QTVERSION% --archives qtcoap

set Qt6_DIR=%QTVERDIR%\msvc2019_64\
set QTCOAPSRC=%QTVERDIR%\Src\qtcoap
echo "Configuring QtCOAP"
cmake -G Ninja -B build\qtcoap %QTCOAPSRC%
echo "Building QtCOAP"
cmake --build build\qtcoap
echo "Installating QtCOAP"
cmake --install build\qtcoap

echo "Setup Complete"



