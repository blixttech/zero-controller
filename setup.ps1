$QTVERSION="6.4.2"
$spath=Split-Path $MyInvocation.MyCommand.Path -Parent
$QTBASEDIR=$spath + "\qt"
$QTVERDIR=$QTBASEDIR + "\" + $QTVERSION + "\"
aqt install-qt -O $QTBASEDIR windows desktop $QTVERSION win64_msvc2019_64 -m qtscxml
aqt install-src -O $QTBASEDIR windows $QTVERSION --archives qtcoap

$Global:Qt6_DIR=$QTVERDIR + "\msvc2019_64\"

$QTCOAPSRC=$QTVERDIR + "\Src\qtcoap"
echo "Configuring QtCOAP"
cmake -G Ninja -B build\qtcoap $QTCOAPSRC -D CMAKE_PREFIX_PATH=$Qt6_DIR
echo "Building QtCOAP"
cmake --build build\qtcoap
echo "Installating QtCOAP"
cmake --install build\qtcoap

echo "Setup Complete"

