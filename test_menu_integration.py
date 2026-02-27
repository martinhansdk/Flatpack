#!/usr/bin/env python3
"""
Integration test for Flatpack add-in menu placement.

This script verifies that the add-in can find its expected menu locations
by checking panel IDs in a running Fusion 360 instance.

Usage:
    python test_menu_integration.py

Requirements:
    - Fusion 360 must be installed
    - Run from within Fusion 360's Python API context or use the API to start Fusion
"""

import sys
import traceback

try:
    import adsk.core
    import adsk.fusion
except ImportError:
    print("ERROR: This script must be run from within Fusion 360's Python API context")
    print("To run this test:")
    print("1. Open Fusion 360")
    print("2. Go to Utilities > Add-Ins > Scripts and Add-Ins")
    print("3. Select this script and click 'Run'")
    sys.exit(1)


def test_panel_ids():
    """Test that expected panel IDs exist in Fusion 360."""
    
    app = adsk.core.Application.get()
    ui = app.userInterface
    
    # Panel IDs that Flatpack expects
    PREFERRED_PANEL = "MakePanel"
    FALLBACK_PANEL = "AddInsPanel"
    
    results = {
        'preferred_found': False,
        'fallback_found': False,
        'available_panels': [],
        'warnings': [],
        'errors': []
    }
    
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
            panel = ui.allToolbarPanels().itemById("MakePanel")
            if not panel:
                panel = ui.allToolbarPanels().itemById("AddInsPanel")
            
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
    
    # Panel ID Tests
    print("\n1. Panel ID Verification:")
    print(f"   Preferred panel (MakePanel): {'✓ FOUND' if results['panel_test']['preferred_found'] else '✗ NOT FOUND'}")
    print(f"   Fallback panel (AddInsPanel): {'✓ FOUND' if results['panel_test']['fallback_found'] else '✗ NOT FOUND'}")
    
    if results['panel_test']['warnings']:
        print("\n   Warnings:")
        for warning in results['panel_test']['warnings']:
            print(f"   ⚠ {warning}")
    
    print(f"\n   Available panels ({len(results['panel_test']['available_panels'])}):")
    for panel_id in sorted(results['panel_test']['available_panels']):
        print(f"      - {panel_id}")
    
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
    
    print("="*70 + "\n")
    
    return 0 if success else 1


def run():
    """Main test runner."""
    
    print("Starting Flatpack menu integration test...")
    print(f"Fusion 360 API Version: {adsk.core.Application.get().version}")
    
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
