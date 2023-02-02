# Flatpack for Autodesk Fusion 360
An addin for Fusion 360 which provides a better way to export DXF and SVG files for laser cutting.

The built in workflow for exporting DXF files from Fusion is cumbersome to use. This addin improves this:

  - No need to align faces so they can be turned into a sketch for export
  - All faces are exported into the same output file
  - Curves are converted to short line segments - avoids problems with laser software that doesn't understand curves
  - Holes in parts are given a different color than the outer edges. This makes it easy to cut the holes first.
  - The selected faces, the output file name and other settings are stored in the document which makes it easy to re-export the data after making design changes

![Demo](doc/demo.gif)

Simply choose Utilities > Make > Export faces to DXF or SVG. The select the faces you want to export and a file name. Optionally, set the accuracy with which curved line segments are converted into straight line segments. Then press OK.

**This addin comes with no warranty whatsoever. Use at your own risk. See also the [software license](MIT.txt).**

## Building the extension yourself
To install, simply download an installer from the releases. If you prefer to build the addin yourself, do as follows:

Install Visual Studio Community 2022.

Install boost 1.81.0 to "%APPDATA%\Autodesk\Autodesk Fusion 360\API\boost". In order to do this
run the following from a x64 Native Tools Command Prompt for Visual Studio 2022 inside the unzipped boost folder:

    bootstrap.bat
    b2 install --prefix="%APPDATA%\Autodesk\Autodesk Fusion 360\API\boost"

Install [cmake](https://cmake.org) and [Ninja](https://ninja-build.org/) and make sure both are on your PATH.

Clone this project recursively to your hard drive, then open a x64 Native Tools Command Prompt and cd to your project. Build the extension as follows:

    makedir build
    cd build
    cmake .. -GNinja
    ninja

