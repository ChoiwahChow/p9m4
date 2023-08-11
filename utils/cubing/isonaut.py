#!/usr/bin/python3

import sys
from datetime import datetime
import time


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
        line = fp.readline()
    return False


def find_non_iso(fp, ofp, keep_cg):
    model_count = 0
    non_iso_store = set()
    line = fp.readline()
    while line:
        if "interpretation" in line:
            model_count += 1
            model = list(line)
            inim = is_non_iso(fp, non_iso_store, model, keep_cg)
            if inim:
                ofp.write("".join(model))
        line = fp.readline()
    ofp.write(f"% number of non-isomorphic models: {len(non_iso_store)}\n")

    return model_count, len(non_iso_store)


def process_file(input_file, output_file, keep_cg):
    with open (input_file) as fp, \
         open (output_file, "w") as ofp:
        (model_count, non_iso_count) = find_non_iso(fp, ofp, keep_cg)
    print(f"number of models processed: {model_count}\nnumber of non isomorphic models: {non_iso_count}")
        


__all__ = ["isonaut"]

if __name__ == "__main__":

    input_file = sys.argv[1]
    output_file = sys.argv[2]
    keep_cg = False
    if len(sys.argv) > 3:
        keep_cg = int(sys.argv[3]) == 1

    propagated_models_count = 0
    t0 = time.time()
    process_file (input_file, output_file, keep_cg)
    t1 = time.time()
    runtime = t1 - t0
    print(f'Done run time = {runtime} seconds\n{datetime.now().strftime("%d/%m/%Y %H:%M:%S")}')
    
