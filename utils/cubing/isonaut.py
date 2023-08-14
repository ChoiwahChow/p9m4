#!/usr/bin/python3

import sys
from datetime import datetime
import time

import argparse


def is_non_iso(fp, non_iso_store, model, keep_cg):
    line = fp.readline()
    while line:
        if not line.startswith("%c%"):
            model.append(line)
            line = fp.readline()
            continue
        if keep_cg:
            model.append(line)
        cg = line[3:].rstrip()
        if cg not in non_iso_store:
            non_iso_store.add(cg) 
            return True
        else:
            return False
        #line = fp.readline()
    return False


def find_non_iso(non_iso_store, fp, ofp, keep_cg):
    model_count = 0
    line = fp.readline()
    while line:
        if "interpretation" in line:
            model_count += 1
            model = list(line)
            inim = is_non_iso(fp, non_iso_store, model, keep_cg)
            if inim:
                ofp.write("".join(model))
        line = fp.readline()

    return model_count


def process_files(input_files, output_file, keep_cg):
    non_iso_store = set()
    total_model_count = 0
    with open (output_file, "a") as ofp:
        for input_file in input_files:
            with open (input_file) as fp:
                model_count = find_non_iso(non_iso_store, fp, ofp, keep_cg)
                total_model_count += model_count
        ofp.write(f"%Processed {total_model_count} models. Number of non-isomorphic models: {len(non_iso_store)}\n")
    print(f"total number of models processed: {total_model_count}\ntotal number of non isomorphic models: {len(non_iso_store)}")


def process_file(input_file, output_file, keep_cg):
    non_iso_store = set()
    with open (input_file) as fp, \
         open (output_file, "a") as ofp:
        model_count = find_non_iso(non_iso_store, fp, ofp, keep_cg)
        ofp.write(f"%Processed {model_count} models. Number of non-isomorphic models: {len(non_iso_store)}\n")
    print(f"number of models processed: {model_count}\nnumber of non isomorphic models: {len(non_iso_store)}")
     


__all__ = ["isonaut"]

if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        description='hashing to filter out models having the same canonical form')
    parser.add_argument('input_files', nargs='*', type=str, default=None,
                        help='mace4 input files.')
    parser.add_argument('-i', dest='input_file', type=str, default="models.out", help='models input file path')
    parser.add_argument('-o', dest='output_file', type=str, default="non_iso_models.out",
                        help='models output file path')
    parser.add_argument('-k', dest='keep_cg', type=int, default=0, help='output the canonical form')
    args = parser.parse_args()

    t0 = time.time()
    cpu0 = time.process_time()
    if args.input_files:
        process_files (args.input_files, args.output_file, args.keep_cg)
    else:
        process_file (args.input_file, args.output_file, args.keep_cg)
    cpu1 = time.process_time()
    t1 = time.time()
    cpu_time = cpu1 - cpu0
    runtime = t1 - t0
    print(f'Done CPU time = {cpu_time} s.  Wall-clock time = {runtime} seconds\n{datetime.now().strftime("%d/%m/%Y %H:%M:%S")}')
    
