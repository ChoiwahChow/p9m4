#!/usr/bin/python3

import sys
import re
import json


def extract_row(func, line):
    if func == 'bin':
        arr = [x for x in line.split(',') if x]
        arr[-1] = re.sub("[^0-9]", "", arr[-1])
        arr = [int(x)+1 for x in arr]
    return arr


def mace_to_gap(fn):
    models = list()
    with (open(fn)) as fp:
        alg = list()
        tbl = list()
        line = fp.readline()
        in_model = False
        while line:
            line = line.strip()
            if 'interpretation' in line or '])]).' in line:
                in_model = False
                if '])]).' in line:
                    row = extract_row(func, line)
                    tbl.append(row)
                if tbl:
                    alg.append(tbl)
                if alg:
                    models.append(alg)
                alg = list()
                tbl = list()
            elif '])' in line:
                if tbl:
                    alg.append(tbl)
                    tbl = list()
                    in_model = False
            elif 'function' in line :
                if '(_,_),' in line:
                    in_model = True
                    tbl = list()
                    func = 'bin'
            elif in_model:
                row = extract_row(func, line)
                tbl.append(row)
            line = fp.readline()
    return models 


def write_alg(fp, alg):
    fp.write('[ ')
    all_models = list()
    for op in alg:
        tbl = list()
        if isinstance(op[0], list):
            for row in op:
                row = [str(x) for x in row]
                tbl.append(f"[ {','.join(row)} ]")
            all_models.append(f"[ {','.join(tbl)} ]")
            # fp.write(f"[ {','.join(tbl)} ]")
    fp.write(",".join(all_models))
    fp.write(' ]')

def isofilter(fn, ofn):
    models = mace_to_gap(fn)
    with (open(ofn, "w")) as fp:
        fp.write('m := [\n')
        if len(models) == 1:
            write_alg(fp, models[0])
        else:
            for alg in models:
                write_alg(fp, alg)
                fp.write(",\n")
        fp.write("];\n")
    return len(models)


__all__ = ["isofilter"]


if __name__ == "__main__":
    models = isofilter("models.out", "models.g")
    print(models)
