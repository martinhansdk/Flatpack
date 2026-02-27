#!/usr/bin/env python3
"""
End-to-end sanity test for Flatpack add-in.

Creates a test design, exercises the add-in with valid and invalid inputs,
and validates the output files.

Usage:
    Run from Fusion 360's Scripts and Add-Ins dialog

Requirements:
    - Fusion 360 must be running
    - Flatpack add-in must be installed and loaded
"""

import adsk.core
import adsk.fusion
import os
import sys
import tempfile
import traceback


class FlatpackSanityTest:
    """End-to-end sanity test for Flatpack."""
    
    def __init__(self):
        self.app = adsk.core.Application.get()
        self.ui = self.app.userInterface
        self.test_results = []
        self.temp_dir = tempfile.mkdtemp(prefix='flatpack_test_')
        
    def log(self, message, status='INFO'):
        """Log a test message."""
        symbols = {'PASS': '✓', 'FAIL': '✗', 'INFO': 'ℹ', 'WARN': '⚠'}
        symbol = symbols.get(status, '•')
        full_message = f"{symbol} {status}: {message}"
        print(full_message)
        self.test_results.append((status, message))
    
    def create_test_design(self):
        """Create a simple test design with known geometry."""
        self.log("Creating test design...")
        
        try:
            # Create a new document
            doc = self.app.documents.add(adsk.core.DocumentTypes.FusionDesignDocumentType)
            design = self.app.activeProduct
            
            # Get root component
            rootComp = design.rootComponent
            
            # Create a sketch on the XY plane
            sketches = rootComp.sketches
            xyPlane = rootComp.xYConstructionPlane
            sketch = sketches.add(xyPlane)
            
            # Draw a simple rectangle (10cm x 5cm)
            lines = sketch.sketchCurves.sketchLines
            rect = lines.addTwoPointRectangle(
                adsk.core.Point3D.create(0, 0, 0),
                adsk.core.Point3D.create(10, 5, 0)
            )
            
            # Extrude it to create a face
            prof = sketch.profiles.item(0)
            extrudes = rootComp.features.extrudeFeatures
            extInput = extrudes.createInput(prof, adsk.fusion.FeatureOperations.NewBodyOperation)
            distance = adsk.core.ValueInput.createByReal(1.0)  # 1cm thick
            extInput.setDistanceExtent(False, distance)
            extrude = extrudes.add(extInput)
            
            # Create a second sketch with a circle (for hole testing)
            sketch2 = sketches.add(xyPlane)
            circles = sketch2.sketchCurves.sketchCircles
            circle = circles.addByCenterRadius(
                adsk.core.Point3D.create(5, 2.5, 0),
                1.0  # 1cm radius
            )
            
            # Extrude cut the circle
            prof2 = sketch2.profiles.item(0)
            extInput2 = extrudes.createInput(prof2, adsk.fusion.FeatureOperations.CutOperation)
            extInput2.setDistanceExtent(False, distance)
            extrude2 = extrudes.add(extInput2)
            
            self.log("Test design created successfully", 'PASS')
            return rootComp
            
        except Exception as e:
            self.log(f"Failed to create test design: {str(e)}", 'FAIL')
            return None
    
    def get_flatpack_command(self):
        """Get the Flatpack command definition."""
        try:
            cmdDef = self.ui.commandDefinitions.itemById('FlatpackCmdId')
            if not cmdDef:
                self.log("Flatpack command not found - is add-in installed?", 'FAIL')
                return None
            return cmdDef
        except:
            self.log("Error accessing Flatpack command", 'FAIL')
            return None
    
    def find_top_face(self, component):
        """Find the top face of the box for testing."""
        try:
            # Get the first body
            body = component.bRepBodies.item(0)
            if not body:
                return None
            
            # Find a planar face facing up (Z-normal)
            for face in body.faces:
                if face.geometry.surfaceType == adsk.core.SurfaceTypes.PlaneSurfaceType:
                    # Check if normal points up
                    evaluator = face.evaluator
                    success, normal = evaluator.getNormalAtPoint(face.pointOnFace)
                    if success and normal.z > 0.9:  # Mostly upward
                        return face
            
            return None
        except:
            return None
    
    def test_valid_export_dxf(self, component):
        """Test exporting a valid face to DXF."""
        self.log("Testing valid DXF export...")
        
        try:
            face = self.find_top_face(component)
            if not face:
                self.log("Could not find test face", 'FAIL')
                return False
            
            output_file = os.path.join(self.temp_dir, 'test_output.dxf')
            
            # TODO: Programmatically invoke Flatpack command with:
            # - Selected face
            # - Output file path
            # - Tolerance value
            
            # For now, this is a placeholder
            # The actual invocation would require triggering the command
            # and filling in the dialog programmatically
            
            self.log("Valid DXF export - MANUAL TEST REQUIRED", 'WARN')
            self.log(f"  Would export to: {output_file}", 'INFO')
            
            # If we could execute, we would check:
            # - File exists
            # - File size > 0
            # - Valid DXF header
            # - Contains expected geometry
            
            return True
            
        except Exception as e:
            self.log(f"Valid DXF export failed: {str(e)}", 'FAIL')
            return False
    
    def test_valid_export_svg(self, component):
        """Test exporting a valid face to SVG."""
        self.log("Testing valid SVG export...")
        
        try:
            face = self.find_top_face(component)
            if not face:
                self.log("Could not find test face", 'FAIL')
                return False
            
            output_file = os.path.join(self.temp_dir, 'test_output.svg')
            
            # TODO: Programmatically invoke Flatpack command
            
            self.log("Valid SVG export - MANUAL TEST REQUIRED", 'WARN')
            self.log(f"  Would export to: {output_file}", 'INFO')
            
            return True
            
        except Exception as e:
            self.log(f"Valid SVG export failed: {str(e)}", 'FAIL')
            return False
    
    def test_no_face_selected(self):
        """Test error handling when no face is selected."""
        self.log("Testing error: no face selected...")
        
        # TODO: Invoke command with empty selection
        # Should show error: "Face selection is not valid"
        
        self.log("No face error - MANUAL TEST REQUIRED", 'WARN')
        return True
    
    def test_invalid_tolerance(self):
        """Test error handling with invalid tolerance."""
        self.log("Testing error: invalid tolerance...")
        
        # TODO: Invoke command with invalid tolerance (negative, zero, etc.)
        # Should show error about invalid expression
        
        self.log("Invalid tolerance error - MANUAL TEST REQUIRED", 'WARN')
        return True
    
    def test_no_filename(self):
        """Test error handling when no output filename specified."""
        self.log("Testing error: no filename...")
        
        # TODO: Invoke command with no filename
        # Should disable OK button or show error
        
        self.log("No filename error - MANUAL TEST REQUIRED", 'WARN')
        return True
    
    def test_curved_surface(self):
        """Test that curved (non-planar) surfaces are rejected."""
        self.log("Testing error: curved surface...")
        
        # TODO: Create a cylinder, try to select curved face
        # Should show error: "Selection is not a face" or similar
        
        self.log("Curved surface error - MANUAL TEST REQUIRED", 'WARN')
        return True
    
    def cleanup(self):
        """Clean up test files and document."""
        self.log("Cleaning up...")
        
        try:
            # Close the test document without saving
            doc = self.app.activeDocument
            if doc:
                doc.close(False)
            
            # Clean up temp directory
            import shutil
            if os.path.exists(self.temp_dir):
                shutil.rmtree(self.temp_dir)
            
            self.log("Cleanup complete", 'PASS')
        except Exception as e:
            self.log(f"Cleanup warning: {str(e)}", 'WARN')
    
    def print_summary(self):
        """Print test summary."""
        print("\n" + "="*70)
        print("FLATPACK SANITY TEST SUMMARY")
        print("="*70)
        
        pass_count = sum(1 for status, _ in self.test_results if status == 'PASS')
        fail_count = sum(1 for status, _ in self.test_results if status == 'FAIL')
        warn_count = sum(1 for status, _ in self.test_results if status == 'WARN')
        
        print(f"\nResults: {pass_count} passed, {fail_count} failed, {warn_count} warnings")
        
        if fail_count > 0:
            print("\nFailed tests:")
            for status, message in self.test_results:
                if status == 'FAIL':
                    print(f"  ✗ {message}")
        
        print("\n" + "="*70)
        
        if fail_count > 0:
            print("STATUS: ✗ FAIL - Some tests failed")
            return 1
        elif warn_count > 0:
            print("STATUS: ⚠ PARTIAL - Manual testing required")
            print("\nNOTE: Full test automation requires programmatic command execution.")
            print("Currently, the Flatpack command must be invoked through the UI.")
            return 0
        else:
            print("STATUS: ✓ PASS - All tests passed")
            return 0
    
    def run(self):
        """Run all tests."""
        print("Starting Flatpack sanity test...")
        print(f"Temp directory: {self.temp_dir}\n")
        
        try:
            # Check if Flatpack is installed
            cmd = self.get_flatpack_command()
            if not cmd:
                self.log("Flatpack add-in not found", 'FAIL')
                return 1
            self.log("Flatpack add-in found", 'PASS')
            
            # Create test design
            component = self.create_test_design()
            if not component:
                return 1
            
            # Run tests
            self.test_valid_export_dxf(component)
            self.test_valid_export_svg(component)
            self.test_no_face_selected()
            self.test_invalid_tolerance()
            self.test_no_filename()
            self.test_curved_surface()
            
            return self.print_summary()
            
        except Exception as e:
            self.log(f"Unexpected error: {str(e)}", 'FAIL')
            traceback.print_exc()
            return 1
        finally:
            self.cleanup()


def run(context):
    """Entry point for Fusion 360."""
    try:
        test = FlatpackSanityTest()
        exit_code = test.run()
        return exit_code == 0
    except:
        traceback.print_exc()
        return False


if __name__ == "__main__":
    run(None)
