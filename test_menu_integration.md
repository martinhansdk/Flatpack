# Flatpack Integration Testing

This directory contains integration tests for the Flatpack add-in.

## test_menu_integration.py

Tests that the add-in's expected menu locations (panel IDs) exist in Fusion 360.

### Purpose

Verifies that:
- The preferred panel location (`MakePanel`) exists
- The fallback panel location (`AddInsPanel`) exists
- Commands can be successfully registered and added to panels
- Lists all available panels for debugging

### Running the Test

**Option 1: Run from Fusion 360 Scripts & Add-Ins Dialog**

1. Open Fusion 360
2. Go to **Utilities > Add-Ins > Scripts and Add-Ins**
3. Click **Add** under "My Scripts"
4. Browse to `test_menu_integration.py`
5. Select the script and click **Run**
6. View results in the Text Commands window

**Option 2: Run manually via Python API**

```python
# In Fusion 360's Python console or script
import sys
sys.path.append('/path/to/Flatpack')
import test_menu_integration
test_menu_integration.run()
```

### Test Output

The test will report:
- ✓ Which expected panels were found
- ⚠ Warnings if preferred location is missing
- ✗ Errors if fallback location is missing
- List of all available panel IDs
- Overall PASS/FAIL status

### Example Output

```
======================================================================
FLATPACK MENU INTEGRATION TEST RESULTS
======================================================================

1. Panel ID Verification:
   Preferred panel (MakePanel): ✓ FOUND
   Fallback panel (AddInsPanel): ✓ FOUND

   Available panels (15):
      - AddInsPanel
      - AssemblePanel
      - ConstructPanel
      - CreatePanel
      - InspectPanel
      - InsertPanel
      - MakePanel
      - ModifyPanel
      - SelectPanel
      - SketchPanel
      ...

2. Command Registration Test:
   Command created: ✓ YES
   Command added to panel: ✓ YES

======================================================================
STATUS: ✓ PASS - Add-in should work correctly
======================================================================
```

### When to Run This Test

- Before releasing a new version
- After Fusion 360 updates
- When users report menu location issues
- During development to verify panel IDs

### CI/CD Integration

This test requires a running Fusion 360 instance with UI access.

**Current CI Status:**  
✅ Test script exists and is validated in CI  
⚠️ **Full test execution requires Fusion 360 headless mode** (not currently automated)

**Options for CI automation:**

1. **Manual pre-release testing** (Current approach)
   - Run test manually before each release
   - Document results in release notes
   - Simple and reliable

2. **Fusion 360 command-line scripting** (If supported)
   ```bash
   # Windows
   "C:\Program Files\Autodesk\Autodesk Fusion 360\Fusion360.exe" --script test_menu_integration.py
   
   # macOS
   /Applications/Autodesk\ Fusion\ 360.app/Contents/MacOS/Fusion360 --script test_menu_integration.py
   ```
   Check Fusion 360 documentation for headless/scripted execution support.

3. **Dedicated test environment**
   - Set up Windows/macOS VM with Fusion 360
   - Run test via remote desktop or automation tools
   - Complex but provides full automation

**To enable in CI:**  
If Fusion 360 supports headless script execution, update `.github/workflows/build.yml`:
- Replace placeholder test with actual Fusion 360 script execution
- Ensure Fusion 360 license allows CI usage

**Current CI placeholder:**  
The workflow includes a placeholder step that verifies the test script exists.
This catches if the test file is accidentally deleted, but doesn't run the actual test.

### Troubleshooting

**"This script must be run from within Fusion 360's Python API context"**
- The script needs access to Fusion 360's API
- Must be run from within Fusion 360, not standalone Python

**Test fails with panel not found**
- Fusion 360's menu structure has changed
- Update `PREFERRED_PANEL` or `FALLBACK_PANEL` in `Flatpack.cpp`
- Report issue on GitHub

**No output visible**
- Check the Text Commands window (bottom of Fusion 360)
- Or check the Output window in the Scripts and Add-Ins dialog
