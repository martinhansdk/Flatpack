# Flatpack Sanity Test

End-to-end integration test for the Flatpack add-in.

## Overview

`test_sanity.py` is a comprehensive test script that:
- Creates a test design with known geometry
- Attempts to exercise the Flatpack add-in
- Validates outputs
- Tests error cases

## Current Limitations

**⚠️ IMPORTANT:** This test is currently **semi-automated** due to Fusion 360 API limitations.

### What Works (Automated)
- ✅ Detecting if Flatpack add-in is installed
- ✅ Creating test geometry (box with hole)
- ✅ Finding faces programmatically
- ✅ Test framework and reporting

### What Doesn't Work (Manual)
- ❌ **Programmatically invoking the Flatpack command**
- ❌ Filling in the dialog with test parameters
- ❌ Clicking OK to execute the command
- ❌ Validating the output files

### Why?

Fusion 360's API does not provide a way to:
1. Programmatically execute UI commands with parameters
2. Fill in command dialog fields from code
3. Trigger command execution without user interaction

The add-in is implemented as a `CommandCreatedEvent` handler, which requires user interaction through the UI.

## Running the Test

### Interactive Mode (Current)

1. Open Fusion 360
2. Install Flatpack add-in (if not already installed)
3. Go to **Utilities > Add-Ins > Scripts and Add-Ins**
4. Click **Add** under "My Scripts"
5. Browse to `test_sanity.py`
6. Click **Run**

The script will:
- Create test geometry
- Verify Flatpack is installed
- Print what it would test
- Wait for you to manually test the scenarios

### Manual Test Procedure

After running the script, manually test:

**Test 1: Valid DXF Export**
1. Select the top face of the box
2. Run Flatpack (Utilities > Make > Export faces to DXF or SVG)
3. Choose a DXF filename
4. Click OK
5. Verify file exists and can be opened

**Test 2: Valid SVG Export**
1. Select the same face
2. Run Flatpack
3. Choose an SVG filename
4. Click OK
5. Verify file exists and can be opened in browser

**Test 3: No Face Selected**
1. Deselect everything
2. Run Flatpack
3. Try to click OK without selecting a face
4. Should show error: "Face selection is not valid"

**Test 4: Invalid Tolerance**
1. Select a face
2. Run Flatpack
3. Enter negative tolerance (e.g., "-0.01")
4. Click OK
5. Should show error about invalid expression

**Test 5: No Filename**
1. Select a face
2. Run Flatpack
3. Don't select an output file
4. OK button should be disabled or show error

**Test 6: Curved Surface**
1. Create a cylinder
2. Try to select the curved side face
3. Run Flatpack
4. Should reject non-planar face

## Future Improvements

### Option 1: Record/Playback
Use Fusion 360's script recording to capture command execution, then replay it in tests.
- Brittle - breaks if UI changes
- Not truly automated
- Better than nothing

### Option 2: Direct API Testing
Test the underlying C++ functions directly instead of through the UI.
- Requires exposing internal functions
- More work but more robust
- True unit testing

### Option 3: UI Automation
Use external UI automation tools (e.g., PyAutoGUI, Sikuli).
- Very brittle
- Platform-specific
- Requires visual recognition
- Not recommended

### Option 4: Hybrid Approach (Recommended)
- Keep current test for setup validation
- Add manual test checklist (like above)
- Document expected results
- Run before each release

## Test Output

Example output:
```
Starting Flatpack sanity test...
Temp directory: /tmp/flatpack_test_abc123

ℹ INFO: Creating test design...
✓ PASS: Test design created successfully
✓ PASS: Flatpack add-in found
⚠ WARN: Valid DXF export - MANUAL TEST REQUIRED
ℹ INFO:   Would export to: /tmp/flatpack_test_abc123/test_output.dxf
⚠ WARN: Valid SVG export - MANUAL TEST REQUIRED
⚠ WARN: No face error - MANUAL TEST REQUIRED
⚠ WARN: Invalid tolerance error - MANUAL TEST REQUIRED
⚠ WARN: No filename error - MANUAL TEST REQUIRED
⚠ WARN: Curved surface error - MANUAL TEST REQUIRED

======================================================================
FLATPACK SANITY TEST SUMMARY
======================================================================

Results: 2 passed, 0 failed, 6 warnings

======================================================================
STATUS: ⚠ PARTIAL - Manual testing required

NOTE: Full test automation requires programmatic command execution.
Currently, the Flatpack command must be invoked through the UI.
```

## Adding to TODO List

Add this to your pre-release checklist:
- [ ] Run `test_sanity.py` to create test design
- [ ] Manually execute the 6 test scenarios above
- [ ] Verify all expected behaviors
- [ ] Document any failures

## CI Integration

This test **cannot run in automated CI** because:
1. Requires Fusion 360 UI interaction
2. No headless command execution
3. Cannot programmatically control the add-in

However, you can:
- Run it manually before releases
- Document test results in release notes
- Use it as a development sanity check

## Files Created

The test creates temporary files in `/tmp/flatpack_test_*`:
- Test design (auto-deleted)
- Would create DXF/SVG outputs (if execution was automated)

All files are automatically cleaned up after the test completes.
