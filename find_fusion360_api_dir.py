import argparse
from pathlib import Path
import os
import sys

POSSIBLE_LOCATIONS = [
    "{HOME}/Library/Application Support/Autodesk",
    "{APPDATA}/Autodesk/Autodesk Fusion 360/API",
    "{HOME}/Library/Application Support/Autodesk/Autodesk Fusion 360/API",
    "C:\Program Files\Autodesk",
    "C:\Program Files (x86)\Autodesk",
]

def main():

    for rootdir in POSSIBLE_LOCATIONS:
        try:
            rootdir_expanded = Path(rootdir.format(**os.environ))
        except KeyError:
            print(f"NOTE: Skipping {rootdir}", file=sys.stderr)
            continue

        for dir in rootdir_expanded.rglob('FusionAll.h'):
            print(dir.parent.parent.parent.parent)
            return 0

    print("ERROR: Fusion 360 not found!", file=sys.stderr)
    return 1
        

if __name__ == '__main__':
    sys.exit(main())