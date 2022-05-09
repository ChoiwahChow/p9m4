#!/usr/bin/python3

"""
Runs all cubes in parallel

Mace4 options:
-O3   strictly follows the cube ordering
-A1   print the models in the format expected by isofilter


grep "Exiting with " semi_working_[0-9]/mace.log | grep model | utils/mace4/counter.py

"""


import os
import time
import subprocess
import threading
import ast

from extend_cubes import request_work


request_work_file = "request_work.txt"
work_file = "release_work.out"


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


def run_process(id, slot_id, thread_slots, order, input_file, cube, print_models, cubes_options, mace4, working_dir_prefix):
    working_dir = f"{working_dir_prefix}_{slot_id}"
    os.makedirs(working_dir, exist_ok=True)
    with (open(f"{working_dir}/cube.config", "w")) as fp:
        for x in cube:
            fp.write(f"{x}\n")

    subprocess.run(f"cd {working_dir}; {mace4} -n{order} -N{order} -m-1 -{print_models} -d{cubes_options} -O3 -f {input_file} >> mace.log 2>&1", 
                    capture_output=False, text=True, check=False, shell=True)      # ; mv models.out {id}.out",
    #if cp.returncode != 0:
    #    with( open("mace.log", "a")) as fp:
    #        fp.write(f"return code: {cp.returncode}\n\n")
        #raise RuntimeError(f"Failed mace4 {cube}\n")
    thread_slots[slot_id] = 0


def run_mace_jobs(mace4_exec, input_file, order, cubes, print_models, cubes_options, working_dir_prefix, id_counter, max_threads, thread_slots):
    """ 
    Args:
        mace4_exec (str): mace4 executable
        input_file (str): algebra input file
        order (int):      order of algebra
        cubes (str):      file path containing cubes
        print_models (str):  A1 to print, P0 not to
        cubes_options (int): bit-0  use work stealing
        working_dir_prefix (str):   prefix of working_dir
        id_counter (int):   jobs finished or being finished so far
        max_threads (int):  maximum number of mace4 processes
        thread_slots(List[int]): array of slots for threads
    """

    with (open(cubes)) as fp:
        all_cubes = fp.read().splitlines()
    for cube in all_cubes:
        seq = [int(x) for x in cube.rstrip().split(" ")]

        slot_id = thread_available(max_threads, thread_slots)
        while slot_id < 0:
            time.sleep(0.5)
            slot_id = thread_available(max_threads, thread_slots)
        id_counter += 1
        if id_counter % 1000 == 0:
            print(f"Doing {id_counter}", flush=True)
        thread_slots[slot_id] = threading.Thread(target=run_process,
                                                 args=(id, slot_id, thread_slots, order, f"../{input_file}", seq, print_models, cubes_options, f"../{mace4_exec}", working_dir_prefix))
        thread_slots[slot_id].start()
    return id_counter


def run_mace(mace4_exec, input_file, order, cubes, print_models, cubes_options, working_dir_prefix, max_threads):
    done = False
    thread_slots = [0] * max_threads
    cube_file = cubes
    os.makedirs(f"{working_dir_prefix}_stealing", exist_ok=True)
    stealing_file = f"{working_dir_prefix}_stealing/new_work.out"
    id_counter = 0
    while not done:
        # Path(stealing_file).unlink(True)
        id_counter = run_mace_jobs(mace4_exec, input_file, order, cube_file, print_models, cubes_options, working_dir_prefix, id_counter, max_threads, thread_slots)
        work_list = request_work(working_dir_prefix, request_work_file, work_file, max_threads, thread_slots)
        print(f"debug run_mace, request work returns {len(work_list)} jobs")
        if work_list:
            with (open(stealing_file, "w")) as fp:
                fp.write('\n'.join(work_list))
                fp.write('\n')
            cube_file = stealing_file
        else:
            done = True

    print("run_mace: All cubes are dispatched. Waiting for the last ones to finish...", flush=True)
    while(not all_done(thread_slots)):
        time.sleep(2)
        

def single_run_mace(mace4_exec, input_file, order, cubes, print_models, cubes_options, working_dir, max_threads):
    run_mace_jobs(mace4_exec, input_file, order, cubes, print_models, cubes_options, working_dir, max_threads)
    print("All cubes are dispatched. Waiting for the last ones to finish...", flush=True)
    while(not all_done(thread_slots)):
        time.sleep(2)
    

__all__ = ["run_mace"]

if __name__ == "__main__":
    mace4_exec = "../bin/mace4"
    cubes_options = 0   # bit-0 for work stealing
    
    order = 10
    cube_length = 50
    print_models = "P0"  # P0 - don't output models, A1 - output models
    algebra = "quasi"
    algebra = "hilbert"
    algebra = "quasi_ordered"
    algebra = "loops"
    algebra = "tarski"
    algebra = "semizero"
    algebra = "semi"
    algebra = "inv_semi"
    algebra = "quandles"
    
    single_run_mace(mace4_exec, f"inputs/{algebra}.in", order, f"utils/mace4/working/{algebra}{order}/cubes_{order}_{cube_length}.out",
             print_models, cubes_options, f"{algebra}_working{cube_length}", 8)
    
    
    
    
