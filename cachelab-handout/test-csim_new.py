# -*- coding: utf-8 -*- 
"""
AUTHOR: Tina Burns
LOCATION: Rutgers University
CLASS: CS211 Computer Architecture
SESSION: Fall 2024

DESCRIPTION:
This script tests the `csim` code for the final project. It compares the results
of the student's cache simulator against the reference implementation.

NOTES:
- Parts of this code were adapted from course materials with the instructor's permission.
- Some sections were generated with the assistance of AI and/or Large Language Models.

SOURCES:
Randal E. Bryant and David R. O'Hallaron, "Computer Systems: A Programmer's Perspective," 3rd Edition (CS:APP3e).
"""

import os
import subprocess
import signal
import sys
from typing import List, Tuple

# Constants
MAX_STR = 1024
N = 8  # Number of tests


def usage(argv):
    print(f"Usage: {argv[0]} [-h]")
    print("Options:")
    print("  -h    Print this help message.")


def sigalrm_handler(signum, frame):
    print("Error: Program timed out.")
    print("TEST_CSIM_RESULTS=0,0,0,0,0,0")
    sys.exit(1)


def runtrace(s, E, b, trace) -> Tuple[bool, List[int], List[int]]:
    """Runs the reference and test simulators and collects results."""
    ref_results = [-1, -1, -1]
    test_results = [-1, -1, -1]

    # Run the reference simulator
    cmd = f"./csim-ref -s {s} -E {E} -b {b} -t {trace} > /dev/null"
    os.system("rm -rf .csim_results")
    status = os.system(cmd)
    if status != 0:
        print(f"Error running reference simulator: Status {status}")
        return False, ref_results, test_results

    # Get the results from the reference simulator
    try:
        with open(".csim_results", "r") as f:
            ref_results = list(map(int, f.readline().split()))
    except FileNotFoundError:
        print("Error: Results for reference simulator not found.")
        return False, ref_results, test_results

    # Run the test simulator
    cmd = f"./csim_new -s {s} -E {E} -b {b} -t {trace} > /dev/null"
    os.system("rm -rf .csim_results")
    status = os.system(cmd)
    if status != 0:
        print(f"Error running test simulator: Status {status}")
        return False, ref_results, test_results

    # Get the results from the test simulator
    try:
        with open(".csim_results", "r") as f:
            test_results = list(map(int, f.readline().split()))
    except FileNotFoundError:
        print("Error: Results for test simulator not found.")
        return False, ref_results, test_results

    return True, ref_results, test_results


def test_csim():
    """Tests the cache simulator for correctness."""
    s = [1, 4, 2, 1, 2, 3, 5, 5]
    E = [1, 2, 1, 2, 2, 2, 1, 1]
    b = [1, 4, 4, 2, 3, 4, 5, 5]
    weight = [1, 1, 1, 1, 1, 1, 1, 1]  # Adjust weights to total 8 points
    trace = [
        "traces/yi2.trace", "traces/yi.trace", "traces/dave.trace",
        "traces/trans.trace", "traces/trans.trace",  "traces/trans.trace",
        "traces/trans.trace", "traces/long.trace"
    ]

    total_credits = 0
    total_points = 0
    points = [0] * N
    results = []

    # Run each test
    for i in range(N):
        status, ref_results, test_results = runtrace(s[i], E[i], b[i], trace[i])
        if not status:
            ref_results = [-1, -1, -1]
            test_results = [-1, -1, -1]
        results.append((ref_results, test_results))

    # Compute points
    for i in range(N):
        ref, test = results[i]
        if all(v != -1 for v in ref + test):
            points[i] += (ref[0] == test[0]) * weight[i]
            points[i] += (ref[1] == test[1]) * weight[i]
            points[i] += (ref[2] == test[2]) * weight[i]
        total_credits += points[i]

    # Adjust to ensure total_points equals 8
    total_points = total_credits/3;

    # Display results
    print(f"\n{'':>17}{'Your simulator':>17}{'Reference simulator':>35}")
    print(f"{'Test':<4}{'(s,E,b)':<4}{'Hits':>10}{'Misses':>8}{'Hit Rate':>10}"
          f"{'Hits':>8}{'Misses':>8}{'Hit Rate':>10}")
          
    ref_total_access = 0; 
    ref_total_hits = 0;
    ref_total_miss = 0;
    ref_total_hit_rate = 0;
    test_total_access = 0; 
    test_total_hits = 0;
    test_total_miss = 0;
    test_total_hit_rate = 0; 
    
    for i in range(N):
        ref, test = results[i]
      
        ref_total_access = ref_total_access+ ref[0]+ref[1]; 
        ref_total_hits = ref_total_hits + ref[0];
        ref_total_miss = ref_total_hits + ref[1];
        test_total_access = test_total_access + test[0] + test[1]; 
        test_total_hits = test_total_hits + test[0];
        test_total_miss = test_total_miss + test[1];

        #Print Data
        print(f"{i:<6}({s[i]},{E[i]},{b[i]}): {test[0]:>8}{test[1]:>8}{calculate_hit_rate(test[0], test[0] + test[1]):>8.1f}% "
              f"{ref[0]:>8}{ref[1]:>8}{calculate_hit_rate(ref[0], ref[0] + ref[1]):>8.1f}% {trace[i]}")

    # Display hit rate comparison
    print(f"\n{'Reference Total Hit Rate:':<30}{(ref_total_hits / ref_total_access) * 100:.3f}%")
    print(f"{'Simulator Total Hit Rate:':<30}{(test_total_hits / test_total_access) * 100:.3f}%\n")

    # Result summary
    PASS = False
    print("\nRESULT SUMMARY:")
    total = 0;
    if ref_total_hits == test_total_hits:
        print("Number of hits does not differ.")
        PASS = False
        total = 0;
    elif ref_total_miss == test_total_miss:
        print("Number of misses does not differ.")
        PASS = False
        total = 0;
    else:
        if test_total_hit_rate >= ref_total_hit_rate - 25:
            print("Simulator Hit Ratio Meets Criteria.")
            PASS = True
            total = 60;
        else:
            print("Simulator Hit Ratio Does Not Meet Criteria")
            PASS = False
            total = 20;
 
    # Adds extra credit to score
    if test_total_hit_rate > ref_total_hit_rate + 1: 
        EC = test_total_hit_rate - ref_total_hit_rate; 
        print(f"Extra Credit Awarded: {EC}:.0f points. \n");
        total = total + EC;
 
    print(f"SCORE for Part B: {total}/60\n");  
    print("The program will be evaluted for additional test and criteria not included in these test cases. ");

    return total;
    
def calculate_hit_rate(hits, accesses):
    """Calculates the hit rate as a percentage."""
    return (hits / accesses * 100) if accesses > 0 else 0.0

def main():
    import argparse

    parser = argparse.ArgumentParser(description="Test the cache simulator.")
    #parser.add_argument("-h", action="help", help="Print this help message.")
    args = parser.parse_args()

    # Install SIGALRM handler
    signal.signal(signal.SIGALRM, sigalrm_handler)
    signal.alarm(20)  # Timeout after 20 seconds

    test_csim()

if __name__ == "__main__":
    main()
