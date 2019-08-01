# Define environment variables
export QT_FRAMEWORK_PATH=/Users/mccrae/Qt/5.5/clang_64/lib
export QT_OTHER_FRAMEWORK_PATH=/Users/mccrae/Desktop/firebox/janus-vr/resources/qtpdf/lib/mac
export QT_BIN_PATH=/Users/mccrae/Qt/5.5/clang_64/bin
export CERTIFICATE="Janus VR Inc."
export FRAMEWORKS="QtCore QtGui QtPrintSupport QtWidgets QtDBus QtMultimedia QtMultimediaWidgets QtNetwork QtOpenGL QtPositioning QtQml QtQuick QtScript QtScriptTools QtSensors QtSql QtWebChannel QtWebKit QtWebKitWidgets QtWebSockets QtXml"
export OTHER_FRAMEWORKS="QtPdf"
export BAD_FRAMEWORKS="QtNetwork QtPrintSupport QtPdf"

# Call itself
./deploy.sh ./build-janusvr_subdirs-Desktop_Qt_5_5_1_clang_64bit2-Release/janusvr.app