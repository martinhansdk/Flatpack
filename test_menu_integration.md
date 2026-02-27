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

This test requires a running Fusion 360 instance, so it **cannot run in standard CI**. However, it can be:

1. **Run manually** before releases
2. **Automated** if you have a Fusion 360 installation in CI (Windows/macOS build agent with Fusion installed)
3. **Documented** for users to run and report results

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
