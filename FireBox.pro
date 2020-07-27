#-------------------------------------------------
#
# Project created by QtCreator 2011-05-26T17:27:40
#
#-------------------------------------------------

# Define version
__VERSION=66.4.1

#JamesMcCrae: define this when doing Oculus-submitted builds
#DEFINES += OCULUS_SUBMISSION_BUILD

DEFINES += __JANUS_VERSION=\\\"$$__VERSION\\\"
DEFINES += __JANUS_VERSION_COMPLETE=__JANUS_VERSION\\\".$$system(git --git-dir ./.git --work-tree . describe --always --tags --abbrev=7)\\\"
win32{
DEFINES += RIFT_ID=\\\"$$system(type riftid.txt)\\\"
}
!win32{
DEFINES += RIFT_ID=\\\"$$system(cat riftid.txt)\\\"
}

# ensure one "debug_and_release" in CONFIG, for clarity...
#debug_and_release {
#    CONFIG -= debug_and_release
#    CONFIG += debug_and_release
#}
# ensure one "debug" or "release" in CONFIG so they can be used as
#   conditionals instead of writing "CONFIG(debug, debug|release)"...
#CONFIG(debug, debug|release) {
#    CONFIG -= debug release
#    CONFIG += debug
#}
#CONFIG(release, debug|release) {
#    CONFIG -= debug release
#    CONFIG += release force_debug_info
# force_debug_info
#}

QT       += core opengl gui network xml script scripttools webengine webenginewidgets websockets

win32-msvc* {
    QMAKE_CXXFLAGS += /wd4251
    QMAKE_CXXFLAGS += /F 32000000
    QMAKE_LFLAGS += /STACK:32000000
    DEFINES += _VARIADIC_MAX=6
}

unix{
    QMAKE_CXXFLAGS += -Wl,--stack,32000000
    QMAKE_CXXFLAGS += -std=c++11 -Wno-unused-local-typedefs
}

CONFIG += c++11
CONFIG += qtnamespace
CONFIG += -opengl desktop -no-angle

TARGET = janusvr
TEMPLATE = app

SOURCES += \
    src/abstracthmdmanager.cpp \
    src/asset.cpp \
    src/assetghost.cpp \
    src/assetimage.cpp \
    src/assetimagedata.cpp \
    src/assetimagedataq.cpp \
    src/assetobject.cpp \
    src/assetrecording.cpp \
    src/assetscript.cpp \
    src/assetshader.cpp \
    src/assetskybox.cpp \
    src/assetsound.cpp \
    src/assetvideo.cpp \
    src/assetwebsurface.cpp \    
    src/assetwindow.cpp \
    src/audioutil.cpp \
    src/bookmarkmanager.cpp \    
    src/codeeditorwindow.cpp \
    src/contentimporter.cpp \
    src/controllermanager.cpp \
    src/cookiejar.cpp \    
    src/domnode.cpp \
    src/environment.cpp \
    src/filteredcubemapmanager.cpp \        
    src/game.cpp \
    src/gamepad.c \
    src/geom.cpp \
    src/glwidget.cpp \
    src/hierarchywindow.cpp \
    src/htmlpage.cpp \    
    src/leaphands.cpp \
    src/lightmanager.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/mathutil.cpp \
    src/mediaplayer.cpp \    
    src/multiplayermanager.cpp \
    src/navigationwindow.cpp \
    src/particlesystem.cpp \
    src/performancelogger.cpp \
    src/player.cpp \
    src/propertieswindow.cpp \
    src/renderer.cpp \
    src/room.cpp \
    src/roomobject.cpp \
    src/roomphysics.cpp \
    src/roomtemplate.cpp \
    src/scriptablevector.cpp \
    src/scriptablexmlhttprequest.cpp \
    src/scriptbuiltins.cpp \
    src/scriptvalueconversions.cpp \
    src/settingsmanager.cpp \
    src/settingswindow.cpp \
    src/socialwindow.cpp \
    src/soundmanager.cpp \
    src/spinanimation.cpp \
    src/textgeom.cpp \
    src/textureimportercmft.cpp \    
    src/textureimporterqimage.cpp \
    src/webasset.cpp \
    src/virtualmenu.cpp

HEADERS += \
    src/abstracthmdmanager.h \
    src/asset.h \
    src/assetghost.h \
    src/assetimage.h \
    src/assetimagedata.h \
    src/assetimagedataq.h \
    src/assetobject.h \
    src/assetrecording.h \
    src/assetscript.h \
    src/assetshader.h \
    src/assetskybox.h \
    src/assetsound.h \
    src/assetvideo.h \    
    src/assetwebsurface.h \    
    src/assetwindow.h \
    src/audioutil.h \
    src/bookmarkmanager.h \
    src/codeeditorwindow.h \
    src/contentimporter.h \
    src/controllermanager.h \
    src/cookiejar.h \    
    src/domnode.h \
    src/environment.h \
    src/filteredcubemapmanager.h \        
    src/game.h \
    src/gamepad.h \
    src/geom.h \
    src/glwidget.h \
    src/hierarchywindow.h \
    src/htmlpage.h \
    src/leaphands.h \
    src/lightmanager.h \
    src/mainwindow.h \
    src/mathutil.h \
    src/mediaplayer.h \    
    src/multiplayermanager.h \
    src/navigationwindow.h \
    src/particlesystem.h \
    src/performancelogger.h \
    src/player.h \
    src/propertieswindow.h \
    src/renderer.h \
    src/room.h \
    src/roomobject.h \
    src/roomphysics.h \
    src/roomtemplate.h \
    src/scriptablevector.h \
    src/scriptablexmlhttprequest.h \
    src/scriptbuiltins.h \
    src/scriptvalueconversions.h \
    src/settingsmanager.h \
    src/settingswindow.h \
    src/socialwindow.h \
    src/soundmanager.h \
    src/spinanimation.h \
    src/textgeom.h \
    src/textureimportercmft.h \    
    src/textureimporterqimage.h \    
    src/webasset.h \
    src/virtualmenu.h

# Vive support (if not an Oculus Submission)
!contains(DEFINES, OCULUS_SUBMISSION_BUILD) {
    win32:HEADERS += src/vivemanager.h
    win32:SOURCES += src/vivemanager.cpp
}

unix:HEADERS += src/vivemanager.h
unix:SOURCES += src/vivemanager.cpp

win32:LIBS += -L"$$PWD/dependencies/windows/"
unix:LIBS += -L"$$PWD/dependencies/linux/"

# cmft: Cross-platform open-source command-line cubemap filtering tool.
win32:INCLUDEPATH += "./resources/cmft/dependency/bx/include/compat/msvc"
INCLUDEPATH += "./resources/cmft/dependency"
INCLUDEPATH += "./resources/cmft/dependency/bx/include"
INCLUDEPATH += "./resources/cmft/dependency/bx/3rdparty"
INCLUDEPATH += "./resources/cmft/dependency/cl/include"
INCLUDEPATH += "./resources/cmft/dependency/dm/include"
INCLUDEPATH += "./resources/cmft/dependency/stb"
INCLUDEPATH += "./resources/cmft/include"
LIBS += -L"$$PWD/resources/cmft/lib"

CONFIG(debug) {
    win32:LIBS += -lcmftDebug
    unix:LIBS += -lcmftRelease
}
CONFIG(release) {
    LIBS += -lcmftRelease
}

# gli
INCLUDEPATH += "./resources/gli"
INCLUDEPATH += "./resources/glm"

# half
INCLUDEPATH += "./resources/half_1.12/include"

#OVR Platform on Windows
win32:HEADERS += src/riftmanager.h
win32:SOURCES += src/riftmanager.cpp

# OVR Platform SDK (essential for Oculus Home build)
win32:SOURCES += "./resources/OVRPlatformSDK_v1.24.0/Windows/OVR_PlatformLoader.cpp"
win32:INCLUDEPATH += "./resources/OVRPlatformSDK_v1.24.0/Include" #note that Windows version is built against latest version (0.8+) of LibOVR
win32:LIBS += -L"$$PWD/resources/OVRPlatformSDK_v1.24.0/Windows"
win32:LIBS += -lLibOVRPlatform64_1

# LibOVR
win32:INCLUDEPATH += "./resources/LibOVR_v1.24.0/Include" #note that Windows version is built against latest version (0.8+) of LibOVR
CONFIG(debug) {
    win32:LIBS += -L"$$PWD/resources/LibOVR_v1.24.0/Lib/Windows/x64/Debug/VS2015"
}
CONFIG(release) {
    win32:LIBS += -L"$$PWD/resources/LibOVR_v1.24.0/Lib/Windows/x64/Release/VS2015"
}
win32:LIBS += -llibOVR

# openVR (note that we only include it if OCULUS_SUBMISSION_BUILD is not defined)
!contains(DEFINES, OCULUS_SUBMISSION_BUILD) {
win32:INCLUDEPATH += "./resources/openvr-1.12.5/headers"
win32:LIBS += -L"$$PWD/resources/openvr-1.12.5/lib/win64"
win32:LIBS += -lopenvr_api
}
unix:INCLUDEPATH += "./resources/openvr-1.12.5/headers"
unix:LIBS += -L"$$PWD/resources/openvr-1.12.5/lib/linux64"
unix:LIBS += -lopenvr_api

# OpenAL
INCLUDEPATH += "./resources/openal-soft-1.20.1/include"
win32:LIBS += -L"$$PWD/resources/openal-soft-1.20.1/libs/Win64"
win32:LIBS += -lOpenAL32

# OpenSSL
INCLUDEPATH +="./resources/openssl-1.1.1g/x64/include"
win32:LIBS += -L"$$PWD/resources/openssl-1.1.1g/x64/lib"
win32:LIBS += -llibcrypto -llibssl
OPENSSL_LIBS ='-L"$$PWD/resources/openssl-1.1.1g/x64/lib" -llibcrypto -llibssl'
CONFIG += openssl-linked

# Opus
INCLUDEPATH +="./resources/opus-1.3.1/include"
CONFIG(debug) {
    win32:LIBS += -L"$$PWD/resources/opus-1.3.1/win32/VS2015/x64/debug"
}
CONFIG(release) {
    win32:LIBS += -L"$$PWD/resources/opus-1.3.1/win32/VS2015/x64/release"
}
LIBS += -lopus

# VLC
win32:INCLUDEPATH +="./resources/libvlc/include"
win32:LIBS += -L"$$PWD/resources/libvlc/libs"
win32:LIBS += -llibvlc -llibvlccore
unix:LIBS += -lvlc

# Bullet Physics - on Linux, this is 2.87 and installed via libbullet-dev
win32:INCLUDEPATH +="./resources/bullet3/src"
win32:LIBS += -L"$$PWD/resources/bullet3/lib/Release"
win32:LIBS += -lBulletDynamics -lBulletCollision -lLinearMath

unix:INCLUDEPATH += "/usr/include/bullet"
unix:LIBS += -lBulletDynamics -lBulletCollision -lLinearMath

# Assimp
INCLUDEPATH += "./resources/assimp/include"

CONFIG(debug) {
    LIBS += -L"$$PWD/resources/assimp/lib/Debug"
    win32:LIBS += -lassimp-vc140-mt -lIrrXML
}
CONFIG(release) {
    LIBS += -L"$$PWD/resources/assimp/lib/Release"
    win32:LIBS += -lassimp-vc140-mt -lIrrXML
}
LIBS += -L"$$PWD/resources/assimp/lib"
unix:LIBS += -lassimp

# Generic Windows libs
win32:LIBS += -lopengl32 -lglu32 -ladvapi32 -lwinmm

# Other Linux libs (need to be separately installed via apt, etc.)
unix:LIBS += -lX11 -ludev -lGLU -lrt -ldl -lopenal -lz #-lXrandr -lXinerama
unix{
   QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"
}

#icon
win32:RC_FILE = janusvr.rc

RESOURCES +=

# OpenGL
DEFINES += OPENGL
