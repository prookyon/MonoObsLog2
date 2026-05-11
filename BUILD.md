# General
Only tested build system is Cmake with MSVC and Vcpkg.

Building is only tested on Windows.

To find Qt6 installation:

`-DQt6_DIR=Q:/Qt/6.9.3/msvc2022_64/lib/cmake/Qt6`

# Dependencies

## Qt6
I am using the official Windows installer and select the MSVC version. `-DQt6_DIR` makes sure this version is used.
Also, the installation script in CMake uses the included windeployqt from there to create a clean package with all (Qt6) dependencies.

## libzip
Used for zipping backups. Downloaded from Vcpkg.

## supernovas
Used for all astrometric calculations (transit times, Moon positions, Sky plot positions etc.). ~~Download from Vcpkg.~~
Currently included by cloning git repo to libraries/ to be able to use latest version with some fixes. Could be used via Vcpkg after repo is updated there. 
It's C package so need to wrap the include in `extern "C"`.

## OpenXLSX
Used for Excel export. Download from Vcpkg.

## Qwt
Used for the polar plot (Qt6 internal one does not directly support custom text labels - and various recommended workarounds I just could not get working on a polar plot )
~~Download from Vcpkg.~~

Building qwt with vcpkg is pain. It pulls the whole Msys2 / Qt6 and rebuilds it. Plus builds just broke after latest MSVC update.
So now I'm building it per Qwt Windows install instructions from source zip (using qmake / nmake) and just link to it in Cmake.
Point CMake at that build with:

`-DQWT_ROOT=C:/path/to/qwt-6.3.0`

# Building deployment package
Build the **Release** configuration and install it to the default `deploy` folder:

`cmake --install cmake-build-release-msvc --config Release`

CLion's `Install` target runs the equivalent install script without a `--prefix` argument, so CMake defaults `CMAKE_INSTALL_PREFIX` to this repository's `deploy` folder.

To install somewhere else from the command line, pass an explicit prefix:

`cmake --install cmake-build-release-msvc --config Release --prefix "$PWD/deploy"`

The install step copies the executable and linked runtime dependencies, then runs Qt6 deployment for Qt libraries, plugins, and compiler runtime files.

