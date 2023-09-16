#!/usr/bin/python3

"""
Runs all cubes in parallel

Mace4 options:
-O3   strictly follows the cube ordering
-A1   print the models in the format expected by isofilter


grep "Exiting with " semi_working_[0-9]/mace.log | grep model | utils/cubing/counter.py

"""


from datetime import datetime
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


def run_process(id, slot_id, thread_slots, order, input_file, interp_out_file, cubes,
                print_models, cubes_options, mace4, working_dir_prefix, hook_cmd):
    working_dir = f"{working_dir_prefix}_{slot_id}"
    os.makedirs(working_dir, exist_ok=True)
    with (open(f"{working_dir}/cube.config", "w")) as fp:
        for cube in cubes:
            fp.write(f"{cube}\n")
        #for x in cube:
        #    fp.write(f"{x}\n")

    out_file = ""
    if interp_out_file:
        out_file = f"-a {interp_out_file}"

    if print_models.startswith("A"):
        iso_filter_opt = print_models.replace("A", "W")
    else:
        iso_filter_opt = "W-1"
    opt = f"-n{order} -N{order} -{print_models} -{iso_filter_opt} -w1 -m-1 -b100000 -d{cubes_options} "
    opt += f'-x {hook_cmd} -O3 {out_file} -f {input_file}'
    subprocess.run(f"cd {working_dir}; {mace4} {opt} >> mace.log 2>&1", 
                   capture_output=False, text=True, check=False, shell=True)
    #if cp.returncode != 0:
    #    with( open("mace.log", "a")) as fp:
    #        fp.write(f"return code: {cp.returncode}\n\n")
        #raise RuntimeError(f"Failed mace4 {cube}\n")
    thread_slots[slot_id] = 0


def run_mace_jobs(mace4_exec, input_file, interp_out_file, order, cubes, print_models, cubes_options,
                  working_dir_prefix, hook_cmd, id_counter, max_threads, thread_slots):
    """ 
    Args:
        mace4_exec (str): mace4 executable
        input_file (str): algebra input file
        order (int):      order of algebra
        cubes (str):      file path containing cubes, one cube per line
        print_models (str):  A1 to print, P0 not to
        cubes_options (int): bit-0  use work stealing
        working_dir_prefix (str):   prefix of working_dir
        id_counter (int):   jobs finished or being finished so far
        max_threads (int):  maximum number of mace4 processes
        thread_slots(List[int]): array of slots for threads
    """

    with (open(cubes)) as fp:
        all_cubes = fp.read().splitlines()
    # all_cubes.sort(key=lambda x: len(x))
    all_cubes = [x.rstrip() for x in all_cubes]

    while all_cubes:
        num = len(all_cubes) // max_threads + 1
        seqs = all_cubes[:num]
        all_cubes = all_cubes[num:]
        
        slot_id = thread_available(max_threads, thread_slots)
        while slot_id < 0:
            time.sleep(0.1)
            slot_id = thread_available(max_threads, thread_slots)
        id_counter += num
        if id_counter % 1000 == 0:
            print(f"Doing {id_counter}", flush=True)
        thread_slots[slot_id] = threading.Thread(target=run_process,
                                                 args=(id, slot_id, thread_slots, order, f"../{input_file}",
                                                       interp_out_file, seqs, print_models, cubes_options,
                                                       f"../{mace4_exec}", working_dir_prefix, hook_cmd))
        thread_slots[slot_id].start()
    return id_counter


def run_mace(mace4_args, cubes, working_dir_prefix, max_threads):
    mace4_exec = mace4_args['mace4_exe']
    order = mace4_args['order']
    print_model = mace4_args['print_model']
    cubes_options = mace4_args['cubes_options']
    interp_out_file = mace4_args['output_file']
    input_file = mace4_args['input_file']
    hook_cmd = mace4_args['hook']

    done = False
    thread_slots = [0] * max_threads
    cube_file = cubes
    os.makedirs(f"{working_dir_prefix}_stealing", exist_ok=True)
    stealing_file = f"{working_dir_prefix}_stealing/new_work.out"
    steal_work = cubes_options % 2 == 1
    id_counter = 0
    while not done:
        # Path(stealing_file).unlink(True)
        id_counter = run_mace_jobs(mace4_exec, input_file, interp_out_file, order, cube_file, print_model,
                                   cubes_options, working_dir_prefix, hook_cmd, id_counter, max_threads, thread_slots)
        work_list = list()
        if steal_work and max_threads > 1:
            work_list = request_work(working_dir_prefix, request_work_file, work_file, max_threads, thread_slots)
        print(f"debug run_mace, request work returns {len(work_list)} jobs", flush=True)
        if work_list:
            with (open(stealing_file, "w")) as fp:
                fp.write('\n'.join(work_list))
                fp.write('\n')
            cube_file = stealing_file
        else:
            done = True

    print(f'{datetime.now().strftime("%d/%m/%Y %H:%M:%S")}run_mace: All cubes are dispatched. Waiting for the last ones to finish...', flush=True)
    while(not all_done(thread_slots)):
        time.sleep(2)
        


__all__ = ["run_mace"]

if __name__ == "__main__":
    mace4_exec = "../bin/mace4"
    cubes_options = 0   # bit-0 for work stealing
    
