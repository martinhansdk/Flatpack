# Flatpack Manual Test Procedure

Pre-release testing checklist for the Flatpack add-in.

**Run before each release to verify functionality.**

---

## Prerequisites

- [ ] Flatpack add-in installed from latest build
- [ ] Fusion 360 open with a blank design

---

## Test 1: Basic DXF Export

**Setup:**
1. Create a sketch on XY plane
2. Draw a 100mm x 50mm rectangle
3. Extrude it 10mm

**Test Steps:**
1. Select the top face of the box
2. Go to **Utilities > Make > Export faces to DXF or SVG**
3. In the dialog:
   - Verify the face is selected (shows in list)
   - Click "Select file..."
   - Choose a location and filename: `test_box.dxf`
   - Leave tolerance at default (0.01)
4. Click **OK**

**Expected Result:**
- ✓ File is created at chosen location
- ✓ File size > 0 bytes
- ✓ Can open in CAD software or text editor
- ✓ Contains DXF header and rectangle geometry

**Actual Result:** [ PASS / FAIL ]

**Notes:** _______________________________________________________________

---

## Test 2: Basic SVG Export

**Setup:**
Use the same box from Test 1

**Test Steps:**
1. Select the top face again
2. Go to **Utilities > Make > Export faces to DXF or SVG**
3. In the dialog:
   - Click "Select file..."
   - Choose filename: `test_box.svg`
4. Click **OK**

**Expected Result:**
- ✓ File is created
- ✓ Can open in web browser
- ✓ Shows rectangle outline
- ✓ Valid SVG XML format

**Actual Result:** [ PASS / FAIL ]

**Notes:** _______________________________________________________________

---

## Test 3: Multiple Faces

**Setup:**
1. Create a new sketch on XY plane
2. Draw a 50mm x 50mm square at origin
3. Draw a 30mm x 30mm square at (100, 0)
4. Extrude both 5mm (in one operation or separately)

**Test Steps:**
1. Select the first square's top face
2. Hold Ctrl (Windows) or Cmd (Mac)
3. Select the second square's top face (both should be selected)
4. Go to **Utilities > Make > Export faces to DXF or SVG**
5. Choose filename: `test_multiple.dxf`
6. Click **OK**

**Expected Result:**
- ✓ Both squares exported
- ✓ Arranged horizontally with spacing between them
- ✓ Second square to the right of the first

**Actual Result:** [ PASS / FAIL ]

**Notes:** _______________________________________________________________

---

## Test 4: Part with Hole

**Setup:**
1. Create a new sketch on XY plane
2. Draw a 100mm x 100mm square
3. Draw a 30mm diameter circle in the center
4. Extrude 10mm

**Test Steps:**
1. Select the top face (the one with the hole)
2. Export to DXF: `test_hole.dxf`
3. Open the file in CAD software or DXF viewer

**Expected Result:**
- ✓ Outer square in color 1 (cyan in DXF)
- ✓ Inner circle in color 2 (yellow in DXF)
- ✓ Hole is a complete closed circle
- ✓ Different colors visible in viewer

**Actual Result:** [ PASS / FAIL ]

**Notes:** _______________________________________________________________

---

## Test 5: Curved Edges (Arc Conversion)

**Setup:**
1. Create a sketch on XY plane
2. Draw a rectangle 80mm x 40mm
3. Use the Fillet tool to round all corners (10mm radius)
4. Extrude 5mm

**Test Steps:**
1. Select the top face
2. Export to DXF: `test_curves.dxf`
3. Set tolerance to 0.01mm (default)
4. Open the output file

**Expected Result:**
- ✓ Corners are rounded (not sharp)
- ✓ Curves converted to line segments
- ✓ Segments are small enough to appear smooth
- ✓ No visible flat spots on corners

**Actual Result:** [ PASS / FAIL ]

**Notes:** _______________________________________________________________

---

## Test 6: Tolerance Setting

**Setup:**
Use the same filleted rectangle from Test 5

**Test Steps:**
1. Select the top face
2. Export with tolerance = **0.1mm**: `test_coarse.dxf`
3. Export again with tolerance = **0.001mm**: `test_fine.dxf`
4. Compare file sizes

**Expected Result:**
- ✓ Coarse file (0.1mm) is smaller
- ✓ Fine file (0.001mm) is larger
- ✓ Fine file has smoother curves (more segments)
- ✓ Coarse file has visible facets on curves

**Actual Result:** [ PASS / FAIL ]

**Notes:** _______________________________________________________________

---

## Test 7: Persistent Settings

**Setup:**
Use any existing design

**Test Steps:**
1. Select a face
2. Open Flatpack dialog
3. Choose an output file: `test_persist.dxf`
4. Set tolerance to **0.05**
5. Click **OK**
6. Close and reopen the design document
7. Open Flatpack dialog again (don't select anything)

**Expected Result:**
- ✓ Previously selected face is still selected
- ✓ Output filename is remembered
- ✓ Tolerance shows 0.05 (not default 0.01)

**Actual Result:** [ PASS / FAIL ]

**Notes:** _______________________________________________________________

---

## Error Test 1: No Face Selected

**Test Steps:**
1. Deselect everything (click in empty space)
2. Go to **Utilities > Make > Export faces to DXF or SVG**
3. Choose an output file
4. Try to click **OK**

**Expected Result:**
- ✓ Dialog shows "Face selection is not valid" or similar error
- ✓ OK button is disabled, or error shown on click
- ✓ Export does not proceed

**Actual Result:** [ PASS / FAIL ]

**Notes:** _______________________________________________________________

---

## Error Test 2: Invalid Tolerance

**Test Steps:**
1. Select a face
2. Open Flatpack dialog
3. Change tolerance to **-0.01** (negative)
4. Click **OK**

**Expected Result:**
- ✓ Error message about invalid expression or value
- ✓ Dialog stays open
- ✓ No file created

**Actual Result:** [ PASS / FAIL ]

**Test with zero:**
5. Change tolerance to **0**
6. Click **OK**

**Expected Result:**
- ✓ Error or warning about invalid tolerance
- ✓ Export does not proceed

**Actual Result:** [ PASS / FAIL ]

**Notes:** _______________________________________________________________

---

## Error Test 3: No Filename

**Test Steps:**
1. Select a face
2. Open Flatpack dialog
3. Do NOT click "Select file..." (leave filename empty)
4. Try to click **OK**

**Expected Result:**
- ✓ OK button is disabled OR
- ✓ Error message shown when clicking OK
- ✓ Export does not proceed

**Actual Result:** [ PASS / FAIL ]

**Notes:** _______________________________________________________________

---

## Error Test 4: Non-Planar Surface

**Setup:**
1. Create a cylinder (50mm diameter, 50mm height)

**Test Steps:**
1. Try to select the curved side face of the cylinder
2. Open Flatpack dialog
3. Try to export

**Expected Result:**
- ✓ Curved face cannot be selected, OR
- ✓ Error shown: "Selection is not a face" or "Face is not planar"
- ✓ Export does not proceed

**Actual Result:** [ PASS / FAIL ]

**Notes:** _______________________________________________________________

---

## Edge Test 1: Very Small Face

**Setup:**
1. Create a 1mm x 1mm x 1mm cube

**Test Steps:**
1. Select the top face
2. Export to DXF

**Expected Result:**
- ✓ Export succeeds
- ✓ File contains correct dimensions (1mm square)
- ✓ No errors or crashes

**Actual Result:** [ PASS / FAIL ]

**Notes:** _______________________________________________________________

---

## Edge Test 2: Very Large Face

**Setup:**
1. Create a 1000mm x 1000mm x 10mm box

**Test Steps:**
1. Select the top face
2. Export to DXF

**Expected Result:**
- ✓ Export succeeds
- ✓ No memory errors or crashes
- ✓ File contains correct dimensions

**Actual Result:** [ PASS / FAIL ]

**Notes:** _______________________________________________________________

---

## Edge Test 3: Complex Geometry

**Setup:**
1. Create a part with many holes (10+ holes of various sizes)
2. Include some curved edges and fillets

**Test Steps:**
1. Select the complex face
2. Export to DXF

**Expected Result:**
- ✓ Export completes (may take a few seconds)
- ✓ All holes present in output
- ✓ All holes in different color from outer edge
- ✓ No missing geometry

**Actual Result:** [ PASS / FAIL ]

**Notes:** _______________________________________________________________

---

## Platform Test: Windows

**Test Steps:**
1. Run Tests 1-4 on Windows 10 or 11
2. Verify menu location: **Utilities > Make > Export faces to DXF or SVG**

**Expected Result:**
- ✓ Add-in appears in correct menu location
- ✓ All basic tests pass
- ✓ DXF/SVG files open correctly

**Actual Result:** [ PASS / FAIL ]

**Notes:** _______________________________________________________________

---

## Platform Test: macOS

**Test Steps:**
1. Run Tests 1-4 on macOS
2. Verify menu location: **Utilities > Make > Export faces to DXF or SVG**

**Expected Result:**
- ✓ Add-in appears in correct menu location
- ✓ All basic tests pass
- ✓ DXF/SVG files open correctly
- ✓ Keyboard shortcuts work (Cmd instead of Ctrl)

**Actual Result:** [ PASS / FAIL ]

**Notes:** _______________________________________________________________

---

## Test Summary

**Date:** ________________

**Version tested:** ________________

**Platform:** ________________

**Total tests:** 18

**Passed:** ______

**Failed:** ______

**Notes/Issues:**

_______________________________________________________________

_______________________________________________________________

_______________________________________________________________

**Overall Status:** [ PASS / FAIL / PARTIAL ]

**Tested by:** ________________

**Ready for release:** [ YES / NO ]
