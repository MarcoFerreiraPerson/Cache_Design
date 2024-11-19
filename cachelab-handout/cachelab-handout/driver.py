# -*- coding: utf-8 -*- 
"""
AUTHOR: Tina Burns
LOCATION: Rutgers University
CLASS: CS211 Computer Architecture
SESSION: Fall 2024

DESCRIPTION:
This script tests the `csim` and `csim_new` code for the final project. It compares the results
of the student's cache simulator against the reference implementation.

This driver tests the correctness of the student's cache simulators
using `test_csim.py` and `test_csim_new.py`.

NOTES:
- Parts of this code were adapted from course materials with the instructor's permission.
- Some sections were generated with the assistance of AI and/or Large Language Models.

SOURCES:
Randal E. Bryant and David R. O'Hallaron, "Computer Systems: A Programmer's Perspective," 3rd Edition (CS:APP3e).
"""

import subprocess
import os
import sys
import argparse

def compute_miss_score(miss, lower, upper, full_score):
    """
    Compute the score based on the number of cache misses.

    Args:
        miss (int): Number of cache misses.
        lower (int): Lower bound of acceptable misses.
        upper (int): Upper bound of acceptable misses.
        full_score (int): Maximum score for the test.

    Returns:
        float: Calculated score based on the number of misses.
    """
    if miss <= lower:
        return full_score
    if miss >= upper: 
        return 0

    score = (miss - lower) * 1.0
    range = (upper - lower) * 1.0
    return round((1 - score / range) * full_score, 1)

def main():
    """
    Main function to test `csim` and `csim_new` cache simulators.
    """

    # Configure maximum scores
    max_scores = {
        'csim': 10,
        'csim_new': 70
    }

    # Parse command-line arguments
    parser = argparse.ArgumentParser(description="Test cache simulators.")
    parser.add_argument(
        "-A", "--autograde", action="store_true",
        help="Emit autoresult string for Autolab."
    )
    args = parser.parse_args()
    autograde = args.autograde

    # Test the standard cache simulator
    print("Part A: Testing cache simulator")
    print("Running ./test-csim.py")
    os.system("python3 test-csim.py")

    # Test the new cache simulator
    print("Part B: Testing your unique cache simulator")
    print("Running ./test-csim_new.py")
    os.system("python3 test-csim_new.py")

# Execute main only if the script is run directly
if __name__ == "__main__":
    main()
