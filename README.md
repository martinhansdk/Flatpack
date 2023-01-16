# Flatpack
An addin for Fusion 360 which provides a better way to export DXF and SVG files for laser cutting.

## Building
Install Visual Studio Community 2022.

Install boost 1.81.0 to "%APPDATA%\Autodesk\Autodesk Fusion 360\API\boost". In order to do this
run the following from a x64 Native Tools Command Prompt for Visual Studio 2022 inside the unzipped boost folder:

    bootstrap.bat
    b2 install --prefix="%APPDATA%\Autodesk\Autodesk Fusion 360\API\boost"

Install WiX:

    dotnet tool install --global wix --version 4.0.0-rc.1

Install the [Wix extension for Visual Studio](https://marketplace.visualstudio.com/items?itemName=WixToolset.WixToolsetVisualStudio2022Extension).

Clone this project recursively to "%APPDATA%\Autodesk\Autodesk Fusion 360\API\AddIns\Flatpackdev".