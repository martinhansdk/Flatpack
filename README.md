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

## Quick Start

1. **Install:** Download the latest [release](https://github.com/martinhansdk/Flatpack/releases), extract, and add to Fusion 360 via **Utilities > Add-Ins**
2. **Use:** Select flat faces → **Utilities > Make > Export faces to DXF or SVG** → Choose filename → OK
3. **Done:** Your parts are exported and ready for fabrication!

📖 **[Read the complete User Guide](doc/USER_GUIDE.md)** for detailed instructions, troubleshooting, and tips.

## Documentation

- **[User Guide](doc/USER_GUIDE.md)** - Complete usage instructions and troubleshooting
- **[Privacy Policy](doc/privacy_policy.md)** - Data handling and privacy information
- **[Manual Test Procedure](test_manual_procedure.md)** - Pre-release testing checklist
- **[Menu Integration Test](test_menu_integration.md)** - Verify menu locations

## Support

- **Issues:** [GitHub Issues](https://github.com/martinhansdk/Flatpack/issues)
- **Email:** GitHub Issues only (no personal email required)

## License

This software is provided under the [MIT License](MIT.txt).

**This add-in comes with no warranty whatsoever. Use at your own risk.**

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
- Enhanced documentation

### Version 1.0.1
- Initial public release
