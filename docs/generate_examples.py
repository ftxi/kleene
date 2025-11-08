#!/usr/bin/env python3
# generate_examples.py
# Generates examples.json listing only .kl files in the ex/ folder relative to this script

import os
import json

# Get the directory of this script
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
EX_DIR = os.path.join(SCRIPT_DIR, "ex")
JSON_FILE = os.path.join(EX_DIR, "examples.json")

if not os.path.isdir(EX_DIR):
    print(f"Directory {EX_DIR} does not exist.")
    exit(1)

# List only .kl files (ignore other files and directories)
files = [f for f in os.listdir(EX_DIR)
         if os.path.isfile(os.path.join(EX_DIR, f)) and f.endswith(".kl")]

# Write JSON file
with open(JSON_FILE, "w") as f:
    json.dump(files, f, indent=2)

print(f"Generated {JSON_FILE} with {len(files)} .kl files.")
