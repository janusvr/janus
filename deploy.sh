#!/bin/bash
# Script name: deploy.sh

# Following environment variables must be defined:
# - QT_FRAMEWORK_PATH
# - QT_BIN_PATH
# - CERTIFICATE
# - FRAMEWORKS
# - BAD_FRAMEWORKS


# retrieve bundle name from first parameter
BUNDLE_NAME=$1

# Run QT tool to deploy
${QT_BIN_PATH}/macdeployqt $BUNDLE_NAME

# FIX ISSUE 6
# Please note that Qt5 frameworks have incorrect layout after SDK build, so this isn't just a problem with `macdeployqt` but whole framework assembly part.
# Present
#   QtCore.framework/
#       Contents/
#           Info.plist
#       QtCore    -> Versions/Current/QtCore
#       Versions/
#           Current -> 5
#           5/
#               QtCore
# After macdeployqt
#   QtCore.framework/
#       Resources/
#       Versions/
#           5/
#               QtCore
#
# Expected
#   QtCore.framework/
#       QtCore    -> Versions/Current/QtCore
#       Resources -> Versions/Current/Resources
#       Versions/
#           Current -> 5
#           5/
#               QtCore
#               Resources/
#                   Info.plist
# So in order to comply with expected layout: https://developer.apple.com/library/mac/documentation/MacOSX/Conceptual/BPFrameworks/Concepts/FrameworkAnatomy.html

for CURRENT_FRAMEWORK in ${FRAMEWORKS}; do
echo "Processing framework: ${CURRENT_FRAMEWORK}"

echo "Deleting existing resource folder"
rm ${BUNDLE_NAME}/Contents/Frameworks/${CURRENT_FRAMEWORK}.framework/Resources

echo "create resource folder"
mkdir ${BUNDLE_NAME}/Contents/Frameworks/${CURRENT_FRAMEWORK}.framework/Versions/5/Resources

echo "create copy resource file"
cp ${QT_FRAMEWORK_PATH}/${CURRENT_FRAMEWORK}.framework/Versions/5/Resources/Info.plist $BUNDLE_NAME/Contents/Frameworks/${CURRENT_FRAMEWORK}.framework/Versions/5/Resources/

echo "create symbolic links"
ln -f -s 5                                     ${BUNDLE_NAME}/Contents/Frameworks/${CURRENT_FRAMEWORK}.framework/Versions/Current
ln -f -s Versions/Current/${CURRENT_FRAMEWORK} ${BUNDLE_NAME}/Contents/Frameworks/${CURRENT_FRAMEWORK}.framework/${CURRENT_FRAMEWORK}
ln -f -s Versions/Current/Resources            ${BUNDLE_NAME}/Contents/Frameworks/${CURRENT_FRAMEWORK}.framework/Resources
done

# FIX ISSUE 7
echo "***** Correct Frameworks Info.plist file*****"

for CURRENT_FRAMEWORK in ${BAD_FRAMEWORKS}; do
echo "Correcting bad framework Info.plist: ${CURRENT_FRAMEWORK}"
TMP=$(sed 's/_debug//g' ${BUNDLE_NAME}/Contents/Frameworks/${CURRENT_FRAMEWORK}.framework/Resources/Info.plist)
echo "$TMP" > ${BUNDLE_NAME}/Contents/Frameworks/${CURRENT_FRAMEWORK}.framework/Resources/Info.plist
done

# SIGNING FIXED FRAMEWORK
CODESIGN_OPTIONS="--verbose=4"

echo "******* Signing Frameworks ***********"
for CURRENT_FRAMEWORK in ${FRAMEWORKS}; do
echo "Signing framework: ${CURRENT_FRAMEWORK}"
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "$CERTIFICATE" $BUNDLE_NAME/Contents/Frameworks/${CURRENT_FRAMEWORK}.framework
done

for CURRENT_FRAMEWORK in ${OTHER_FRAMEWORKS}; do
echo "Signing framework: ${CURRENT_FRAMEWORK}"
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "$CERTIFICATE" $BUNDLE_NAME/Contents/Frameworks/${CURRENT_FRAMEWORK}.framework
done

# Sign PlugIns
echo "******* Signing PlugIns ***********"
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/Frameworks/libassimp.4.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/Frameworks/libcrypto.1.0.0.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/Frameworks/libHalf.12.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/Frameworks/libIex-2_2.12.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/Frameworks/libIexMath-2_2.12.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/Frameworks/libIlmImf-2_2.22.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/Frameworks/libIlmThread-2_2.12.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/Frameworks/libImath-2_2.12.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/Frameworks/libmpg123.0.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/Frameworks/libmysqlclient.18.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/Frameworks/libogg.0.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/Frameworks/libopenal.1.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/Frameworks/libopus.0.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/Frameworks/libpq.5.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/Frameworks/libssl.1.0.0.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/Frameworks/libvorbis.0.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/Frameworks/libvorbisfile.3.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/Frameworks/libLeap.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/audio/libqtaudio_coreaudio.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/bearer/libqcorewlanbearer.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/bearer/libqgenericbearer.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/imageformats/libqdds.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/imageformats/libqgif.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/imageformats/libqicns.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/imageformats/libqico.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/imageformats/libqjp2.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/imageformats/libqjpeg.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/imageformats/libqdds.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/imageformats/libqmng.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/imageformats/libqdds.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/imageformats/libqtga.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/imageformats/libqtiff.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/imageformats/libqdds.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/imageformats/libqwbmp.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/imageformats/libqwebp.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/mediaservice/libqavfcamera.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/mediaservice/libavfmediaplayer.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/mediaservice/libqtmedia_audioengine
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/platforms/libqcocoa.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/printsupport/libcocoaprintersupport.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/sqldrivers/libsqlite.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/sqldrivers/libqsqlmysql.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/sqldrivers/libqsqlodbc.dylib
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "${CERTIFICATE}" ${BUNDLE_NAME}/Contents/PlugIns/sqldrivers/libqsqlpsql.dylib

# Sign bundle itself
echo "******* Signing Bundle ***********"
codesign --deep --force --verify ${CODESIGN_OPTIONS} --sign "$CERTIFICATE" $BUNDLE_NAME

# Verify

echo "******* Verify Bundle ***********"
codesign --deep --force --verify ${CODESIGN_OPTIONS} $BUNDLE_NAME


echo "******* Verify Bundle using dpctl ***********"
spctl -a -vvvv $BUNDLE_NAME