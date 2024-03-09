$QTVERSION="6.4.2"
$spath=Split-Path $MyInvocation.MyCommand.Path -Parent
$QTBASEDIR=$spath + "\qt"
$QTVERDIR=$QTBASEDIR + "\" + $QTVERSION + "\"
aqt install-qt -O $QTBASEDIR windows desktop $QTVERSION win64_msvc2019_64 -m qtscxml
aqt install-src -O $QTBASEDIR windows $QTVERSION --archives qtcoap

$Global:Qt6_DIR=$QTVERDIR + "\msvc2019_64\"

$QTCOAPSRC=$QTVERDIR + "\Src\qtcoap"
echo "Configuring QtCOAP Release"
cmake -G Ninja -B build\qtcoap -S $QTCOAPSRC -D CMAKE_PREFIX_PATH=$Qt6_DIR -D CMAKE_BUILD_TYPE=Release
echo "Building QtCOAP"
cmake --build build\qtcoap
echo "Installating QtCOAP"
cmake --install build\qtcoap


$QWTSRC=$QTVERDIR + "\Src\qwt"
$QWTVERSION="6.2.0"
$QWTFILE="qwt-" + $QWTVERSION + ".zip"
$QWTDIR=$QWTSRC + "\qwt-" + $QWTVERSION

echo "Creating"
echo $QWTSRC
echo "Directory"
mkdir $QWTSRC

echo "Downloading Qwt"
wget -O $QWTSRC + "\" + $QWTFILE https://sourceforge.net/projects/qwt/files/qwt/$QWTVERSION/$QWTFILE
unzip $QWTSRC + "\" + "$QWTFILE
copy $spath\config\qwtconfig.pri $QWTDIR
cd $QWTDIR && qmake qwt.pro
cd $QWTDIR && make
cd $QWTDIR && make install


echo "Setup Complete"

