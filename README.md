# Flatpack
An addin for Fusion 360 which provides a better way to export DXF and SVG files for laser cutting.

## Building
Install Visual Studio Community 2017.

Install boost 1.66.0 to "%APPDATA%\Autodesk\Autodesk Fusion 360\API\boost". In order to do this
I had to modify project-config.jam as explained at https://stackoverflow.com/questions/41464356/build-boost-with-msvc-14-1-vs2017-rc and run b2 like this:

    b2 toolset=msvc-14.0 address-model=64 install --prefix="%APPDATA%\Autodesk\Autodesk Fusion 360\API\boost"

Clone this project to "%APPDATA%\Autodesk\Autodesk Fusion 360\API\AddIns\Flatpackdev".