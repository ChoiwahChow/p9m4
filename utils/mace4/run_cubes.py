#!/usr/bin/python3

"""
Runs all cubes in parallel
"""


import os
import time
import subprocess
import threading
import ast


mace4 = "../build/mace4"


def thread_available(thread_count, thread_slots):
    for x in range(thread_count):
        if thread_slots[x] == 0:
            return x
    return -1


def all_done(thread_slots):
    for x in range(len(thread_slots)):
        if thread_slots[x] != 0:
            return False
    return True


def run_process(id, slot_id, thread_slots, order, input_file, cube):
    working_dir = f"working_{slot_id}"
    os.makedirs(working_dir, exist_ok=True)
    with (open(f"{working_dir}/cube.config", "w")) as fp:
        for x in cube:
            fp.write(f"{x}\n")

    subprocess.run(f"cd {working_dir}; {mace4} -n{order} -N{order} -m-1 -A1 -f {input_file} >> mace.log 2>&1", 
                    capture_output=False, text=True, check=False, shell=True)      # ; mv models.out {id}.out",
    #if cp.returncode != 0:
    #    with( open("mace.log", "a")) as fp:
    #        fp.write(f"return code: {cp.returncode}\n\n")
        #raise RuntimeError(f"Failed mace4 {cube}\n")
    thread_slots[slot_id] = 0


def run_mace(input_file, order, cubes, working_dir, max_threads):
    """ 
    """
    thread_slots = [0] * max_threads
    id = 0    

    with (open(cubes)) as fp:
        all_cubes = fp.readlines()
    for cube in all_cubes:
        cube = ast.literal_eval(cube)
        seq = [x[1] for x in cube]

        slot_id = thread_available(max_threads, thread_slots)
        while slot_id < 0:
            time.sleep(2)
            slot_id = thread_available(max_threads, thread_slots)
        id += 1
        if id % 1000 == 0:
            print(f"Doing {id}", flush=True)
        thread_slots[slot_id] = threading.Thread(target=run_process, args=(id, slot_id, thread_slots, order, f"../{input_file}", seq))
        thread_slots[slot_id].start()

    print("All cubes are dispatched. Waiting for the last ones to finish...", flush=True)
    while(not all_done(thread_slots)):
        time.sleep(2)
    

__all__ =["run_mace"]

if __name__ == "__main__":
    run_mace("inputs/semi.in", 7, "utils/mace4/order7/cubes_2_7_5.json", "working_dir", 8)
