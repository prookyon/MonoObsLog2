# General
Only tested build system is Cmake with Msys2/Mingw64 and Vcpkg.
Some dependencies were easier to get working by installing as packages from Msys2 while others with Vcpkg.

Building is only tested on Windows.

In VSCode this would be needed to use the Windows Qt6 installation and Vcpkg:

`"cmake.configureArgs": ["-DQt6_DIR=Q:/Qt/6.10.1/mingw_64/lib/cmake/Qt6", "-DCMAKE_TOOLCHAIN_FILE=Q:/vcpkg/scripts/buildsystems/vcpkg.cmake"]`

In Clion they would go to CMake profile.

# Dependencies

## Qt6
I am using the official Windows installer and select the mingw32 version. `-DQt6_DIR` makes sure this version is used.
Also the install script in CMake uses the included windeployqt from there to create a clean package with all (Qt6) dependencies.

## libzip
Used for zipping backups. Downloaded from Vcpkg. I believe my Vcpkg install defaulted to building it using MSVC - this did not seem to matter.

## supernovas
Used for all astrometric calculations (transit times, Moon positions, Sky plot positions etc.). Download from Vcpkg. It's C package so again builds fine with MSVC - just need to wrap the include in `extern "C"`.

## OpenXLSX
Used for Excel export. Only got it working by building and installing manually in mingw64.

Also for some reason I needed to change:
`\mingw64\include\OpenXLSX\headers\XLXmlParser.hpp` so that one of the includes that was with full path just reads: `#include <pugixml.hpp>`. Based on comment the full path seems to be intentional there, but it did not work for me.

## Qwt
Used for the polar plot (Qt6 internal one does not directly support custom text labels - and various recommended workarounds I just could not get working on a polar plot )
Downloaded in mingw64 using pacman.

Needed to use `PkgConfig` and `pkg_search_module` for it in Cmake

# Building deployment package
For **Release** build only the Cmake script has install command that copies executable and some dependencies to `deploy` folder and then runs Qt6 `windeployqt` on the executable there.
Result should be something that can run on a fresh install of Windows 11 with only MS C Runtime install needed:
[Link to download](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170#latest-supported-redistributable-version)

It should be probably possible to get rid of that by not using MSVC to build the Vcpkg dependencies and building them also directly in mingw64, but I have not tried that yet.

