# janus-vr

Janus VR source code is released with a GPLv3 license:

https://www.gnu.org/licenses/gpl.html

Additional deployment notes:

- Before building, copy and rename the following: riftid.txt.sample
to riftid.txt. Update these files with specific app IDs (for publishing 
to platforms like Oculus Store).

Building for Windows
===================

1) Download and install MSVC 2017 community version (64-bit).  
   Do not forget to check the "C++ development box".

2) Download and install Qt 5.13.0
   https://download.qt.io/official_releases/qt/5.13/5.13.0/

   Make sure to select the release compatible with 64-bit MSVC 2017,
   and during installation select msvc2017-64bit, QtWebEngine and 
   QtScript.

3) Clone this janus repo

4) Open the project in Qt Creator by pointing it towards
   $CONTAINING_DIR\janus\Firebox.pro

   (At this point, the project should build successfully.)

5) Copy the assets and dependencies/windows/* to your build folder, 
   (as specified in Qt Creator project settings), where the generated 
   janusvr.exe is located.

   - If your build is a release build, you should now be able to run your 
     executable.
   - If your build is a debug build, copy and paste the Qt debug dlls 
   	 to the same location. Those can be found where you installed Qt
     (e.g. C:\Qt\5.13.0\msvc2017\bin)

   Qt Creator will look for those files in a different location, so in 
   order to run from inside Qt Creator, you will also need to paste 
   those things in the folder containing janusvr.exe (one level up from 
   where you just pasted).

6. For debugging on Windows when compiling with MSVC 2017, you will need 
   to install additional Debugging Tools for Windows, which are packaged 
   with the Windows Driver Kit: 

https://developer.microsoft.com/en-us/windows/hardware/windows-driver-kit

Building for Linux (Ubuntu 18.04 LTS)
================================================

=== Automated script ===

1) Clone the repository using Git:
	- `git clone https://github.com/janusvr/janus`

2) Change directory into source directory and fetch submodules needed:
  - `cd janus`
  - `git submodule init`
  - `git submodule update`

3) Run the automated build script:
	- `./build-janusvr-linux.sh`

4) Once completed, your new JanusVR build can be found inside `dist/linux/` in the root of your source repo.
   To run Janus, just type `dist/linux/janusvr -render 2d`

=== Via command line ===

For now, to compile manually please read the instructions found in the automated build script that
is located in the root of the source repository (`build-janusvr-linux.sh`).

(Optional, for reference) You may need to install a collection of packages for Qt5 development, assimp, 
bullet3, etc:

   - sudo apt-get install build-essential
   - sudo apt-get install qtcreator
   - sudo apt-get install qt5-default
   - sudo apt-get install libqt5websockets5-dev
   - sudo apt-get install libqt5webengine5-dev
   - sudo apt-get install libassimp-dev libassimp4
   - sudo apt-get install libbullet-dev libbullet2.87
   - sudo apt-get install libopus

Then, compile Janus with:

   - qmake -makefile
   - make
 
=== Via Qt Creator ===

1)  Load up Firebox.pro and configure build profiles (debug, release, etc.)

2)  Build

Common issues and fixes
=======================

- If you encounter mysterious linking errors after having just added
  a class, try cleaning the project, running qmake, and rebuilding.
- Note, Linux: if updating cmft, edit main.lua and comment out strip(), 
  otherwise the generated libcmftRelease.a will not work
   
