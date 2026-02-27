"""Locate the Fusion 360 C++ API dir and copy it to a folder in the current directory called fusion_36_api/.

When running in CI, it can be hard to find the place where the API files have been installed, because part of
the path is unpredictable. This script searches the usual directories for a marker file and then copies
the API to a local directory. This simplifies both CMakeLists.txt and makes it easier to cache the API
in the Github workflow.
"""


import argparse
from pathlib import Path
import shutil
from typing import Optional
import os
import sys

POSSIBLE_LOCATIONS = [
    "{HOME}/Library/Application Support/Autodesk",
    "{APPDATA}/Autodesk/Autodesk Fusion 360/API",
    "C:\Program Files\Autodesk",
    "C:\Program Files (x86)\Autodesk",
]

DESTINATION_DIR = Path('fusion_360_api')

def find_api_dir():
    """Search the POSSIBLE_LOCATIONS to find the first one that contains the Fusion 360 C++ API files.
    Return the root dir of the API. If multiple match then the first is returned.
    """
    for rootdir in POSSIBLE_LOCATIONS:
        try:
            rootdir_expanded = Path(rootdir.format(**os.environ))
        except KeyError:
            print(f"NOTE: Skipping {rootdir}", file=sys.stderr)
            continue

        for dir in rootdir_expanded.rglob('FusionAll.h'):
            return dir.parent.parent.parent
        
    return None


def main():
    api_dir = find_api_dir()

    if not api_dir:
        print("ERROR: Fusion 360 not found!", file=sys.stderr)
        return 1
    
    print(f"Copying {api_dir} to {DESTINATION_DIR}")
    shutil.copytree(api_dir, DESTINATION_DIR)
        

if __name__ == '__main__':
    sys.exit(main())