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
Used for all astrometric calculations (transit times, Moon positions, Sky plot positions etc.). Download from Vcpkg. It's C package so need to wrap the include in `extern "C"`.

## OpenXLSX
Used for Excel export. Download from Vcpkg.

## Qwt
Used for the polar plot (Qt6 internal one does not directly support custom text labels - and various recommended workarounds I just could not get working on a polar plot )
Download from Vcpkg.

# Building deployment package
Install is only defined for **Release** build. It copies executable and some dependencies to `deploy` folder and then runs Qt6 `windeployqt` on the executable there.
Result should be something that can run on a fresh installation of Windows 11 with only MS C Runtime installation needed:
[Link to download](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170#latest-supported-redistributable-version)
`windeployqt` seems to put the installer in the `deploy` folder also

