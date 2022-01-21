#!/usr/bin/python3

"""
Runs all cubes in parallel

Mace4 options:
-O3   strictly follows the cube ordering
-A1   print the models in the format expected by isofilter


grep "Exiting with " semi_working_[0-9]/mace.log | utils/mace4/counter.py

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


def run_process(id, slot_id, thread_slots, order, input_file, cube, print_models, working_dir):
    working_dir = f"{working_dir}_{slot_id}"
    os.makedirs(working_dir, exist_ok=True)
    with (open(f"{working_dir}/cube.config", "w")) as fp:
        for x in cube:
            fp.write(f"{x}\n")

    subprocess.run(f"cd {working_dir}; {mace4} -n{order} -N{order} -m-1 -{print_models} -O3 -f {input_file} >> mace.log 2>&1", 
                    capture_output=False, text=True, check=False, shell=True)      # ; mv models.out {id}.out",
    #if cp.returncode != 0:
    #    with( open("mace.log", "a")) as fp:
    #        fp.write(f"return code: {cp.returncode}\n\n")
        #raise RuntimeError(f"Failed mace4 {cube}\n")
    thread_slots[slot_id] = 0


def run_mace(input_file, order, cubes, print_models, working_dir, max_threads):
    """ 
    
    """
    thread_slots = [0] * max_threads
    id = 0

    with (open(cubes)) as fp:
        all_cubes = fp.read().splitlines()
    for cube in all_cubes:
        seq = [int(x) for x in cube.rstrip().split(" ")]

        slot_id = thread_available(max_threads, thread_slots)
        while slot_id < 0:
            time.sleep(0.5)
            slot_id = thread_available(max_threads, thread_slots)
        id += 1
        if id % 1000 == 0:
            print(f"Doing {id}", flush=True)
        thread_slots[slot_id] = threading.Thread(target=run_process, args=(id, slot_id, thread_slots, order, f"../{input_file}", seq, print_models, working_dir))
        thread_slots[slot_id].start()

    print("All cubes are dispatched. Waiting for the last ones to finish...", flush=True)
    while(not all_done(thread_slots)):
        time.sleep(2)
    

__all__ =["run_mace"]

if __name__ == "__main__":
    order = 9
    cube_length = 2
    print_models = "P0"  # P0 - don't output models, A1 - output models
    algebra = "semi"
    algebra = "quasi"
    algebra = "tarski"
    algebra = "semizero"
    algebra = "hilbert"
    algebra = "quasi_ordered"
    algebra = "loops"
    algebra = "inv_semi"
    
    run_mace(f"inputs/{algebra}.in", order, f"utils/mace4/{algebra}{order}/cubes_2_{order}_{cube_length}.out",
             print_models, f"{algebra}_working3", 8)
    
    
    
    
