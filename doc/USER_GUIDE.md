# Flatpack User Guide

Complete guide to using Flatpack for Autodesk Fusion 360.

## Table of Contents

1. [Installation](#installation)
2. [Getting Started](#getting-started)
3. [Exporting Faces](#exporting-faces)
4. [Understanding the Output](#understanding-the-output)
5. [Advanced Settings](#advanced-settings)
6. [Tips & Tricks](#tips--tricks)
7. [Troubleshooting](#troubleshooting)

## Installation

### Requirements

- Autodesk Fusion 360 (any recent version)
- Windows 10/11 or macOS 10.15+
- Administrative rights to install add-ins

### Installation Steps

1. **Download** the latest release for your platform from the [releases page](https://github.com/martinhansdk/Flatpack/releases)

2. **Extract** the ZIP file to a temporary location

3. **Open Fusion 360** and go to the **Utilities** workspace

4. Click **Add-Ins** in the toolbar, then select the **Add-Ins** tab

5. Click the green **+** button next to "My Add-Ins"

6. Browse to the extracted `Flatpack.bundle` folder and select it

7. The add-in will appear in the list with a checkbox to enable it

8. Ensure "Run on Startup" is checked so it loads automatically

### Verifying Installation

After installation, you should see a new command:
- Location: **Utilities > Make > Export faces to DXF or SVG**

If you don't see this command, try:
- Restarting Fusion 360
- Clicking "Refresh" in the Add-Ins dialog
- Checking that the add-in is enabled in the Add-Ins list

## Getting Started

### Your First Export

Let's export a simple rectangular face:

1. **Create or open a design** with at least one flat face

2. **Switch to the Utilities workspace** (if not already there)

3. **Click Make > Export faces to DXF or SVG**

4. **Select a face** by clicking on it in the viewport
   - The face will be highlighted when selected
   - You can select multiple faces by holding Ctrl (Windows) or Cmd (Mac)

5. **Choose an output file:**
   - Click the "Select file..." button
   - Choose a location and enter a filename
   - Use `.dxf` extension for DXF files or `.svg` for SVG files

6. **Click OK** to export

The add-in will create your file with the selected faces laid out and ready for fabrication!

## Exporting Faces

### Selecting Faces

**Single Face:**
- Simply click on the face you want to export

**Multiple Faces:**
- Hold Ctrl (Windows) or Cmd (Mac) and click additional faces
- All selected faces will be highlighted in blue

**Face Requirements:**
- Faces must be planar (flat)
- Non-planar faces (curved surfaces) will be rejected with an error

**Best Practices:**
- Select all faces that will be cut from the same material
- Group similar thicknesses together in one export
- Create separate exports for different materials or thicknesses

### File Format Selection

**DXF Format (.dxf):**
- ✅ Widely supported by CAM software
- ✅ Preserves color information
- ✅ Good for CNC, laser cutters, waterjet
- ❌ Larger file sizes

**SVG Format (.svg):**
- ✅ Smaller file sizes
- ✅ Can be opened in web browsers for preview
- ✅ Works with many laser cutters
- ❌ Some CAM software has limited SVG support

**Recommendation:** Try DXF first, fall back to SVG if your machine doesn't recognize DXF.

## Understanding the Output

### Layout

Parts are automatically:
- Rotated to minimize width (longest edge becomes horizontal)
- Arranged in a single horizontal row
- Separated by 5mm spacing

### Color Coding

**Color 1 (Outer Edges):**
- DXF: Cyan (color code 1)
- SVG: Black stroke
- These are the outer boundary lines of each part

**Color 2 (Inner Holes):**
- DXF: Yellow (color code 2)
- SVG: Red stroke
- These are internal cutouts, pockets, or holes

**Why Color Coding?**
Most fabrication software allows you to set different cutting parameters per color:
- Cut inner holes first to prevent material shifting
- Use different speeds/power for detail work
- Easily identify what to cut vs. what to engrave

### Curve Conversion

Circles, arcs, ellipses, and splines are converted to straight line segments:
- Default tolerance: 0.01mm
- Smaller tolerance = more segments = smoother curves
- Larger tolerance = fewer segments = faster cutting but more angular

The tolerance controls the maximum distance between the true curve and the approximated line segments.

## Advanced Settings

### Conversion Tolerance

**What it does:**
Controls how accurately curves are converted to line segments.

**When to adjust:**

**Decrease tolerance (e.g., 0.001mm) if:**
- Curves look too angular
- You need very smooth circles or arcs
- Working with precision parts
- File size is not a concern

**Increase tolerance (e.g., 0.1mm) if:**
- File sizes are too large
- Cutting takes too long
- You're working with rough/prototype parts
- Your machine struggles with many segments

**Recommended values:**
- High precision: 0.001mm - 0.005mm
- Normal use: 0.01mm (default)
- Fast/rough cuts: 0.05mm - 0.1mm

### Persistent Settings

Flatpack stores the following in your Fusion 360 document:
- Selected faces (reselects them next time)
- Output filename
- Conversion tolerance setting

This means after making design changes, you can simply:
1. Open the export dialog
2. Click OK (settings are already there)
3. Your file is updated!

## Tips & Tricks

### Efficient Workflow

**Set it and forget it:**
1. Set up your export once with all the right faces and filename
2. As you modify your design, just re-run the export command
3. All settings are remembered automatically

**Multiple exports:**
- Create separate exports for different materials (wood vs. acrylic)
- Create separate exports for different thicknesses (3mm vs. 6mm)
- Each export's settings are saved independently

### Working with Complex Designs

**Too many faces?**
- Split into multiple exports by component
- Group related parts together

**Overlapping parts in output?**
- The add-in lays parts horizontally in a row
- If parts overlap, check that all faces are truly planar
- Consider exporting fewer faces per file

**Need specific orientation?**
- The add-in automatically rotates for optimal layout
- If you need specific orientation, create sketches instead

### Material Optimization

While Flatpack doesn't do true nesting (arranging parts to minimize waste), you can:
- Preview the output in your CAM software
- Manually arrange parts for better material usage
- Use dedicated nesting software after export

## Troubleshooting

### "No active Fusion design" Error

**Problem:** The dialog appears but immediately shows an error.

**Solution:**
- Make sure you have a design document open
- Switch to the Design workspace (not Drawing or Animation)
- Save your design at least once

### "Selection is not a face" Error

**Problem:** You selected something but get an error.

**Solution:**
- Make sure you're clicking on a face, not an edge or body
- The face must be planar (flat)
- Try clicking in the center of the face

### "Face selection is not valid" Error

**Problem:** Can't proceed after selecting faces.

**Solution:**
- Make sure at least one face is selected
- Check that all selected faces are planar
- Try deselecting all and reselecting

### Curves Look Angular

**Problem:** Circles and arcs have visible flat segments.

**Solution:**
- Decrease the "Conversion tolerance" value
- Try 0.001mm for very smooth curves
- Note: This increases file size and processing time

### File Won't Open in CAM Software

**Problem:** Your laser cutter or CAM software rejects the file.

**Solution:**
1. Try the other format (DXF vs. SVG)
2. Check your software's supported file formats
3. Try opening in a CAD program first to verify the file
4. Some software needs specific DXF versions - try saving as DXF R12 if possible

### Parts Are the Wrong Size

**Problem:** Exported parts are scaled incorrectly.

**Solution:**
- DXF units are in millimeters
- SVG units are in centimeters
- Check your CAM software's import units setting
- Fusion 360's document units don't affect export units

### Add-in Doesn't Appear in Menu

**Problem:** Can't find "Export faces to DXF or SVG" command.

**Solution:**
1. Go to Utilities > Add-Ins > Add-Ins tab
2. Check that Flatpack is in the list and enabled
3. Click "Refresh" if needed
4. Restart Fusion 360
5. Check that "Run on Startup" is enabled
6. Reinstall the add-in if necessary

### Slow Export Performance

**Problem:** Export takes a long time.

**Solution:**
- Increase the conversion tolerance (less detail)
- Reduce the number of faces exported at once
- Simplify complex curves in your design
- Close other applications to free up memory

## Getting Help

If you encounter issues not covered here:

1. **Check the [GitHub Issues](https://github.com/martinhansdk/Flatpack/issues)** - Your problem may already be reported
2. **Create a new issue** with:
   - Your operating system and version
   - Fusion 360 version
   - Steps to reproduce the problem
   - Screenshots if applicable
3. **Email support:** [Add your support email]

## Privacy & Data

Flatpack only stores export settings in your Fusion 360 document's attributes. No data is sent to external servers or the add-in author. See the [Privacy Policy](privacy_policy.md) for full details.

---

**Version:** 1.0.2  
**Last Updated:** February 2026  
**License:** MIT License
