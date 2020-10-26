#!/bin/bash -E

INSTALLDEPS=true
BUILDOVR=true
BUILDAI=true
QUIET=
CQUIET=
VERBOSE=false
NPROC=
TIMEBUILD=false
HELP=false
BUILD_DIR="dist/linux/"
WITHDEBUG=
QDB="CONFIG-=force_debug_info CONFIG-=debug_and_release CONFIG-=debug CONFIG+=release"
MDB=
UV=$(lsb_release -r -s)
BUILDFAIL=false
pkglist=

while test $# -gt 0
do
	case "$1" in 
		-d | --nodepinst ) INSTALLDEPS=false
			;;
		-o | --nobuildovr ) BUILDOVR=false
			;;
		-a | --nobuildai ) BUILDAI=false
			;;
		-b | --debug ) WITHDEBUG=true; QDB="CONFIG+=force_debug_info"
			;;
		-v | --verbose ) VERBOSE=true
			;;
		-t | --timebuild ) TIME=false
			;;
		-t | --threads ) NPROC="$2"
			;;
		-h | --help ) HELP=true
			;;
		--* ) echo "unknown argument '$1'"
			;;
		*) NPROC=$1
			;;
	esac
	shift
done



if [[ $UV == "18.04" ]];
then
	pkglist=(
		cmake
		qtcreator
		qt5-default
		build-essential
		mesa-common-dev
		git
		git-lfs
		build-essential
		libqt5websockets5-dev
		libbullet-dev
		libopenal-dev
		libopus-dev
		libvorbis-dev
		libudev-dev
		libvlc-dev
		libopenexr-dev
		libudev-dev
		vlc
		zlib1g-dev
		qt5-default
		qtscript5-dev
		mesa-common-dev
		libilmbase-dev
		qtbase5-private-dev
		libqt5webengine5
		libqt5webenginecore5
		libqt5webenginewidgets5
		qtwebengine5-dev
		libqt5scripttools5
		qtscript-tools
		)
elif [[ $UV == "20.04" ]];
then
	pkglist=(
		cmake
		qtcreator
		qt5-default
		build-essential
		mesa-common-dev
		git
		git-lfs
		build-essential
		libqt5websockets5-dev
		libbullet-dev
		libopenal-dev
		libopus-dev
		libvorbis-dev
		libudev-dev
		libvlc-dev
		libopenexr-dev
		libudev-dev
		vlc
		zlib1g-dev
		qt5-default
		qtscript5-dev
		mesa-common-dev
		libilmbase-dev
		qtbase5-private-dev
		libqt5webengine5
		libqt5webenginecore5
		libqt5webenginewidgets5
		qtwebengine5-dev
		libqt5scripttools5
		)
else
	echo "not ubuntu"
fi

install_dependencies () {
	echo -e "\n[*] Installing prerequisite OS packages for Ubuntu $UV"
	echo -e "    ( this can be skipped by using the -d or --nodepinst flag )"
	sudo apt install ${pkglist[@]}
}

header_text () {
	echo "########################################################"
	echo " Welcome to the magical Linux Auto Compiler for JanusVR "
	echo "########################################################"
}

footer_text () {
	echo -e "\n[*] Done! Please run 'janusvr' from $BUILD_DIR"
	echo -e "    ( Use -h or --help for build options )"
}

build_janus () {
	echo -e "\n[*] Building JanusVR Native binary distribution using $NPROC threads. Please wait..."
	if [ "$TIMEBUILD" = true ]
	then
		qmake FireBox.pro -spec linux-g++ CONFIG+=release $QDB
		time { make -j$NPROC $QUIET; }
	else
		qmake FireBox.pro -spec linux-g++ CONFIG+=release $QDB
		make -j$NPROC $QUIET;
	fi
}

remove_build_folder () {
	echo -e "\n[*] Deleting build dir $BUILD_DIR"
	rm -rf $BUILD_DIR
}

create_build_folder () {
	echo -e "\n[*] Creating directory for build distribution in $BUILD_DIR..."
	mkdir -p $BUILD_DIR
}

copy_janus_binary_and_assets () {
	cp janusvr $BUILD_DIR
	ln -s $(pwd)/assets $(pwd)/$BUILD_DIR/assets
	cp -r dependencies/linux/* $BUILD_DIR
}

bullet_patch () {
	echo -e "\n[#] PATCH 1: Adding depedencies from OS to distribution directory"
	cp /usr/lib/x86_64-linux-gnu/libBulletDynamics.so.2.87	$BUILD_DIR
	cp /usr/lib/x86_64-linux-gnu/libBulletCollision.so.2.87	$BUILD_DIR
	cp /usr/lib/x86_64-linux-gnu/libLinearMath.so.2.87		$BUILD_DIR
}

build_assimp () {
	echo -e "\n[*] Building ASSet IMPorter v5.0.1"
	echo -e "    ( this can be skipped by using the -a or --nobuildai flag )"
	cd resources/assimp/
	cmake .
	make -j $NPROC
	cd ../../
}

copy_assimp () {
	echo -e "\n[*] Copying assimp to $BUILD_DIR"
	cp resources/assimp/lib/libassimp.so.5.0.0 $BUILD_DIR
	cd $BUILD_DIR
	ln -s libassimp.so.5.0.0 libassimp.so.5
	ln -s libassimp.so.5.0.0 libassimp.so
	cd ../../
}

build_openvr () {
	echo -e "\n[*] Building OpenVR v1.12.5"
	echo -e "    ( this can be skipped by using the -o or --nobuildovr flag )"
	cd resources/openvr-1.12.5/
	cmake .
	make -j $NPROC
	cd ../../
}

copy_openvr () {
	echo -e "[*] Copying OpenVR to $BUILD_DIR"
	cp resources/openvr-1.12.5/bin/linux64/libopenvr_api.so $BUILD_DIR
}

display_help_text () {
	echo " Usage: $0 [options] [arguments]\n"
	echo " -d   --nodepinst     Skip dependency installation"
	echo " -o   --nobuildovr    Skip build and install of OpenVR"
	echo " -a   --nobuildai     Skip build and install of AssImp"
	echo " -b   --debug         Include debug symbols"
	echo " -v   --verbose       Verbose display of build process"
	echo " -t   --threads N     Number of threads to use in build process"
	echo " -h   --help          Display this help page"
}

main () {
	if [ "$HELP" = true ]
	then
		display_help_text
	fi

	if [ "$HELP" = false ]
	then
		if [ "$INSTALLDEPS" = true ]
		then
			install_dependencies
		fi
		remove_build_folder
		create_build_folder
	fi

	if [ "$VERBOSE" = false ] && [ "$HELP" = false ]
	then
		QUIET=" -s "
		CQUIET=""
	fi
	
	if [ "$HELP" = false ]
	then
		

		bullet_patch
		#create_library_build_folders

		if [ "$BUILDAI" = true ]
		then
			build_assimp
		fi
		copy_assimp
		if [ "$BUILDOVR" = true ]
		then
			build_openvr
		fi
		copy_openvr
	fi

	if [ ! -z "$NPROC" ]
	then
		if [ "$HELP" = false ]
		then
			build_janus
			copy_janus_binary_and_assets
					
		fi
	else
		if [ "$HELP" = false ]
		then
			NPROC=$(nproc)
			build_janus
			copy_janus_binary_and_assets
					
		fi
	fi
}

if [ "$HELP" = false ]
then
	header_text
fi

main

if [ "$HELP" = false ]
then
	footer_text
fi
