# Flatpack for Autodesk Fusion 360

An add-in for Fusion 360 that simplifies exporting flat faces to DXF and SVG files for laser cutting, CNC routing, and other fabrication processes.

![Demo](doc/demo.gif)

## Features

✨ **Simplified Workflow** - Export multiple faces without creating sketches  
📐 **Smart Orientation** - Automatically lays out parts efficiently  
🎨 **Color-Coded Cuts** - Inner holes and outer edges use different colors  
💾 **Persistent Settings** - Remembers your selections and settings per document  
🔄 **Curve Conversion** - Converts curves to line segments for better compatibility  
📦 **Batch Export** - All selected faces exported to a single file  

## Installation

### From Release (Recommended)

1. Download the latest release ZIP file for your platform (Windows or macOS)
2. Extract the ZIP file
3. In Fusion 360, go to **Utilities > Add-Ins > Add-Ins** tab
4. Click the green **+** icon next to "My Add-Ins"
5. Navigate to and select the extracted `Flatpack.bundle` folder
6. The add-in will appear in your list and automatically start

### Manual Installation

Alternatively, copy the `Flatpack.bundle` folder to your Fusion 360 add-ins directory:
- **Windows:** `%APPDATA%\Autodesk\Autodesk Fusion 360\API\AddIns\`
- **macOS:** `~/Library/Application Support/Autodesk/Autodesk Fusion 360/API/AddIns/`

Then restart Fusion 360 or click "Refresh" in the Add-Ins dialog.

## Usage

1. In Fusion 360, select **Utilities > Make > Export faces to DXF or SVG**
2. Select the flat faces you want to export (hold Ctrl/Cmd to select multiple)
3. Click "Select file..." to choose output location and filename
   - Use `.dxf` extension for DXF format
   - Use `.svg` extension for SVG format
4. Adjust the "Conversion tolerance" if needed (default: 0.01mm)
   - Smaller values = smoother curves, larger file size
   - Larger values = faster export, more angular curves
5. Click **OK** to export

The add-in remembers your face selections and settings, making it easy to re-export after design changes.

## Output Details

- **Outer edges** are exported in **color 1** (black in SVG, cyan in DXF)
- **Inner holes** are exported in **color 2** (red in SVG, yellow in DXF)
- Parts are automatically oriented horizontally for efficient material usage
- All parts are laid out in a single row with spacing

## Troubleshooting

**"No active Fusion design" error**  
Make sure you have a design document open (not just the Data Panel).

**Parts look angular instead of smooth**  
Decrease the "Conversion tolerance" value for smoother curves.

**Laser cutter doesn't recognize the file**  
Try switching between DXF and SVG formats. Some machines prefer one over the other.

## Support

- Report issues: [GitHub Issues](https://github.com/martinhansdk/Flatpack/issues)
- Email: [Add your support email here]

## License

This software is provided under the [MIT License](MIT.txt).

**This add-in comes with no warranty whatsoever. Use at your own risk.**

## Privacy

See [Privacy Policy](doc/privacy_policy.md) for details. In short: Flatpack only stores settings in your Fusion 360 document. No data is transmitted to external servers.

---

## Building from Source

### Prerequisites

- CMake 3.24 or later
- Ninja build system
- Visual Studio 2022 (Windows) or Xcode (macOS)
- Python 3.x
- Autodesk Fusion 360 installed

### Build Steps

```bash
git clone --recursive https://github.com/martinhansdk/Flatpack.git
cd Flatpack
mkdir build
cd build
cmake .. -GNinja
ninja package
```

The built add-in will be in the `build/` directory as a ZIP file.

### Dependencies

This project uses:
- **GLM** (OpenGL Mathematics) - automatically fetched via CMake
- **XDxfGen** - DXF generation library (submodule)
- **Fusion 360 C++ API** - provided by Fusion 360 installation

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues.

## Changelog

### Version 1.0.2
- Migrated from Boost to GLM for geometry operations
- Improved build system with CMake FetchContent
- Better CI/CD pipeline

### Version 1.0.1
- Initial public release

