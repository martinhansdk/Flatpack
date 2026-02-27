#!/usr/bin/env python3
"""
Integration test for Flatpack add-in menu placement.

This script verifies that the add-in can find its expected menu locations
by checking panel IDs in a running Fusion 360 instance.

It extracts the expected panel IDs from Flatpack.cpp automatically.

Usage:
    # From within Fusion 360:
    Run via Scripts and Add-Ins dialog
    
    # From command line (CI):
    fusion360 --script test_menu_integration.py

Requirements:
    - Fusion 360 must be installed and running
"""

import sys
import os
import re
import traceback

# Try to determine if we're running in CI or interactive mode
IS_CI = os.environ.get('CI') or os.environ.get('GITHUB_ACTIONS')

try:
    import adsk.core
    import adsk.fusion
except ImportError:
    print("ERROR: This script must be run from within Fusion 360's Python API context")
    if not IS_CI:
        print("To run this test:")
        print("1. Open Fusion 360")
        print("2. Go to Utilities > Add-Ins > Scripts and Add-Ins")
        print("3. Select this script and click 'Run'")
    sys.exit(1)


def extract_panel_ids_from_cpp():
    """Extract PREFERRED_PANEL and FALLBACK_PANEL from Flatpack.cpp."""
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    cpp_file = os.path.join(script_dir, 'Flatpack.cpp')
    
    if not os.path.exists(cpp_file):
        return None, None, f"Flatpack.cpp not found at {cpp_file}"
    
    try:
        with open(cpp_file, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Look for: const char* PREFERRED_PANEL = "MakePanel";
        preferred_match = re.search(r'const\s+char\*\s+PREFERRED_PANEL\s*=\s*"([^"]+)"', content)
        fallback_match = re.search(r'const\s+char\*\s+FALLBACK_PANEL\s*=\s*"([^"]+)"', content)
        
        preferred = preferred_match.group(1) if preferred_match else None
        fallback = fallback_match.group(1) if fallback_match else None
        
        return preferred, fallback, None
        
    except Exception as e:
        return None, None, f"Error reading Flatpack.cpp: {str(e)}"


def test_panel_ids():
    """Test that expected panel IDs exist in Fusion 360."""
    
    app = adsk.core.Application.get()
    ui = app.userInterface
    
    # Extract panel IDs from source code
    PREFERRED_PANEL, FALLBACK_PANEL, error = extract_panel_ids_from_cpp()
    
    results = {
        'preferred_panel_id': PREFERRED_PANEL,
        'fallback_panel_id': FALLBACK_PANEL,
        'preferred_found': False,
        'fallback_found': False,
        'available_panels': [],
        'warnings': [],
        'errors': []
    }
    
    if error:
        results['errors'].append(error)
        return results
    
    if not PREFERRED_PANEL or not FALLBACK_PANEL:
        results['errors'].append("Could not extract panel IDs from Flatpack.cpp")
        return results
    
    # Get all available panels
    all_panels = ui.allToolbarPanels()
    if not all_panels:
        results['errors'].append("Unable to access toolbar panels")
        return results
    
    # List all available panel IDs for debugging
    for i in range(all_panels.count):
        panel = all_panels.item(i)
        if panel:
            results['available_panels'].append(panel.id)
    
    # Check for preferred panel
    preferred_panel = all_panels.itemById(PREFERRED_PANEL)
    if preferred_panel:
        results['preferred_found'] = True
    else:
        results['warnings'].append(f"Preferred panel '{PREFERRED_PANEL}' not found")
    
    # Check for fallback panel
    fallback_panel = all_panels.itemById(FALLBACK_PANEL)
    if fallback_panel:
        results['fallback_found'] = True
    else:
        results['errors'].append(f"Fallback panel '{FALLBACK_PANEL}' not found")
    
    return results


def test_command_registration():
    """Test that the Flatpack command can be registered."""
    
    app = adsk.core.Application.get()
    ui = app.userInterface
    
    COMMAND_ID = "FlatpackCmdIdTest"
    
    # Get panel IDs from source
    PREFERRED_PANEL, FALLBACK_PANEL, _ = extract_panel_ids_from_cpp()
    
    results = {
        'command_created': False,
        'command_added_to_panel': False,
        'errors': []
    }
    
    try:
        # Try to create a test command definition
        cmd_defs = ui.commandDefinitions
        test_cmd = cmd_defs.itemById(COMMAND_ID)
        
        # Clean up if it already exists
        if test_cmd:
            test_cmd.deleteMe()
        
        # Create new command definition
        test_cmd = cmd_defs.addButtonDefinition(
            COMMAND_ID,
            "Flatpack Test Command",
            "Test command for integration testing"
        )
        
        if test_cmd:
            results['command_created'] = True
        
            # Try to add it to the preferred panel
            panel = ui.allToolbarPanels().itemById(PREFERRED_PANEL) if PREFERRED_PANEL else None
            if not panel and FALLBACK_PANEL:
                panel = ui.allToolbarPanels().itemById(FALLBACK_PANEL)
            
            if panel:
                control = panel.controls.addCommand(test_cmd)
                if control:
                    results['command_added_to_panel'] = True
                    # Clean up
                    control.deleteMe()
            
            # Clean up command definition
            test_cmd.deleteMe()
        
    except Exception as e:
        results['errors'].append(f"Exception during command test: {str(e)}")
    
    return results


def print_results(results):
    """Print test results in a readable format."""
    
    print("\n" + "="*70)
    print("FLATPACK MENU INTEGRATION TEST RESULTS")
    print("="*70)
    
    # Configuration
    print(f"\nConfiguration (from Flatpack.cpp):")
    print(f"   Preferred panel: {results['panel_test']['preferred_panel_id']}")
    print(f"   Fallback panel:  {results['panel_test']['fallback_panel_id']}")
    
    # Panel ID Tests
    print("\n1. Panel ID Verification:")
    print(f"   Preferred panel: {'✓ FOUND' if results['panel_test']['preferred_found'] else '✗ NOT FOUND'}")
    print(f"   Fallback panel:  {'✓ FOUND' if results['panel_test']['fallback_found'] else '✗ NOT FOUND'}")
    
    if results['panel_test']['warnings']:
        print("\n   Warnings:")
        for warning in results['panel_test']['warnings']:
            print(f"   ⚠ {warning}")
    
    print(f"\n   Available panels ({len(results['panel_test']['available_panels'])}):")
    for panel_id in sorted(results['panel_test']['available_panels']):
        marker = ""
        if panel_id == results['panel_test']['preferred_panel_id']:
            marker = " ← PREFERRED"
        elif panel_id == results['panel_test']['fallback_panel_id']:
            marker = " ← FALLBACK"
        print(f"      - {panel_id}{marker}")
    
    # Command Registration Tests
    print("\n2. Command Registration Test:")
    print(f"   Command created: {'✓ YES' if results['command_test']['command_created'] else '✗ NO'}")
    print(f"   Command added to panel: {'✓ YES' if results['command_test']['command_added_to_panel'] else '✗ NO'}")
    
    # Errors
    all_errors = results['panel_test']['errors'] + results['command_test']['errors']
    if all_errors:
        print("\n3. ERRORS:")
        for error in all_errors:
            print(f"   ✗ {error}")
    
    # Overall Status
    print("\n" + "="*70)
    success = (
        results['panel_test']['fallback_found'] and
        results['command_test']['command_created'] and
        not all_errors
    )
    
    if success:
        print("STATUS: ✓ PASS - Add-in should work correctly")
        if not results['panel_test']['preferred_found']:
            print("NOTE: Preferred location not found - will use fallback")
    else:
        print("STATUS: ✗ FAIL - Add-in may not work correctly")
        print("ACTION REQUIRED: Update panel IDs in Flatpack.cpp")
        if IS_CI:
            print("\nSuggested panel IDs from available panels:")
            for panel_id in sorted(results['panel_test']['available_panels'])[:5]:
                print(f"   - {panel_id}")
    
    print("="*70 + "\n")
    
    return 0 if success else 1


def run():
    """Main test runner."""
    
    print("Starting Flatpack menu integration test...")
    
    app = adsk.core.Application.get()
    print(f"Fusion 360 API Version: {app.version}")
    
    if IS_CI:
        print("Running in CI mode")
    
    results = {
        'panel_test': test_panel_ids(),
        'command_test': test_command_registration()
    }
    
    return print_results(results)


if __name__ == "__main__":
    try:
        exit_code = run()
        sys.exit(exit_code)
    except Exception as e:
        print(f"\n✗ FATAL ERROR: {str(e)}")
        print("\nStack trace:")
        traceback.print_exc()
        sys.exit(1)
