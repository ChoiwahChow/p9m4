#!/usr/bin/python3

"""
Extend cubes in parallel

Mace4 options:
-O3   strictly follows the cube ordering
-A1   print the models in the format expected by isofilter


cat quandles_working10_*/4.out | grep "^cube" | sort --parallel=8 -u | sed 's/[^ ]* //' > utils/cubing/working/quandles10/cubes4.out


grep "Exiting with " semi_working_[0-9]/mace.log | utils/mace4/counter.py

"""


import os
from pathlib import Path
import time
import subprocess
import threading
import ast


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


def run_process(id, slot_id, thread_slots, order, cube_length, input_file, interp_out_file, cubes, 
                print_models, mace4, cubes_options, working_dir):
    """
    Spawn a new Python process to do cube generation, and wait for the process to complete.  After the 
    completion of the Mace4 process, update <thread_slots> with <slot_id> to zero to say the thread is done.
    Mace4 error outputs are directed/appended to mace.out in the <working_dir>
    Mace4 outputs are directed/appended to <cube_length>.out in the <working_dir>
    The models are written out to "models.out" if <print_models> is "A1".  It will have cononical graph
    written out after each model.

    TODO: allow setting of max model cache size.

    Args:
        cubes_options (int): bit vector lsb 1 - do work stealing, 2 - cube has num cells filled at beginning
        cubes (str):  cubes separated by new line character
        mace4 (str):  the complete path name of Mace4
        working_dir (str):  the effective working directory is <working_dir>_<slot_id>.
    """
    working_dir = f"{working_dir}_{slot_id}"
    os.makedirs(working_dir, exist_ok=True)
    
    if cubes is not None:
        with (open(f"{working_dir}/cube.config", "w")) as fp:
            for cube in cubes:
                fp.write(f"{cube}\n")
    # print(f"debug ******run_process in extend_cube, mace4 path name****************************** {mace4}")

    out_file = ""
    if interp_out_file:
        out_file = f"-a {interp_out_file}"

    opt = f"-n{order} -N{order} -m-1 -W-1 -w1 -{print_models} -d{cubes_options} -C{cube_length} -O3 {out_file} -f {input_file}"
    cmd = f"cd {working_dir}; {mace4} {opt} >> {cube_length}.out 2>>mace.out"
    # print(f"debug mace4 cmd**************************{cmd}")

    subprocess.run(cmd, 
                   capture_output=False, text=True, check=False, shell=True)

    #if cp.returncode != 0:
    #    with( open("mace.log", "a")) as fp:
    #        fp.write(f"return code: {cp.returncode}\n\n")
        #raise RuntimeError(f"Failed mace4 {cube}\n")
    thread_slots[slot_id] = 0


def extend_cube_jobs(input_file, interp_out_file, order, new_cube_length, cubes, print_models, mace4, 
                     cubes_options, working_dir, max_threads, thread_slots):
    """
    Args:
        cubes_options (int): bit vector lsb 1 - do work stealing, 2 - cube has num cells filled at beginning
    """
    id = 0
    if cubes is None:
        thread_slots[0] = threading.Thread(target=run_process, 
                                           args=(id, id, thread_slots, order, new_cube_length, f"../{input_file}",
                                                 interp_out_file, None, print_models, f"../{mace4}",
                                                 cubes_options, working_dir))
        thread_slots[0].start()
    else:
        with (open(cubes)) as fp:
            all_cubes = fp.read().splitlines()
        last_id = id
        while all_cubes:
            num = len(all_cubes) // max_threads + 1
            seqs = all_cubes[:num]
            all_cubes = all_cubes[num:]
    
            slot_id = thread_available(max_threads, thread_slots)
            while slot_id < 0:
                time.sleep(0.5)
                slot_id = thread_available(max_threads, thread_slots)
            id += num
            if last_id - id > 1000:
                print(f"Debug, extend_cubes, Doing {id}", flush=True)
                last_id = id
            thread_slots[slot_id] = threading.Thread(target=run_process, 
                                                     args=(id, slot_id, thread_slots, order, new_cube_length,
                                                           f"../{input_file}", interp_out_file, seqs, print_models,
                                                           f"../{mace4}", cubes_options, working_dir))
            thread_slots[slot_id].start()
    

def request_work(working_dir_prefix, request_work_file, work_file, max_threads, thread_slots):
    """
    does work stealing.  Requests work from running processes by putting out a 
    file in the working directory of the process
    """
    print("debug request_work*****************\n")
    requested = False
    done = False
    first_round = True
    last_round = 0
    work_list = list()
    while not done:
        all_threads_completed = True
        for index, thread in enumerate(thread_slots):
            r_file_path = f"{working_dir_prefix}_{index}/{request_work_file}"
            c_file_path = f"{working_dir_prefix}_{index}/{work_file}"
            if os.path.exists(c_file_path):
                with (open(c_file_path)) as fp:
                    x = fp.read().splitlines()
                if x[-1].startswith("End"):
                    work_list.extend(x[:-1])
                    if os.path.exists(c_file_path):
                        os.remove(c_file_path)
                        # Path(c_file_path).unlink(False)
                    if os.path.exists(r_file_path):
                        os.remove(r_file_path)
                        # Path(r_file_path).unlink(True)
            if thread == 0:
                if os.path.exists(r_file_path):
                    os.remove(r_file_path)
                    # Path(r_file_path).unlink(True)
            elif os.path.exists(f"{working_dir_prefix}_{index}"):
                all_threads_completed = False
                if last_round > 0:                    
                    if os.path.exists(r_file_path):
                        os.remove(r_file_path)
                    # Path(r_file_path).unlink(True)
                elif first_round:
                    open(r_file_path, 'w').close()
                    requested = True
        if all_threads_completed:
            return work_list
        first_round = False
        if not requested:
            return work_list     # no active threads, so return
        time.sleep(2)
        if work_list:   # harvested something, ready to return
            if last_round > 1:
                done = True
            last_round += 1
    return work_list
    

#def extend_cubes(input_file, order, new_cube_length, cubes, print_models, mace4, working_dir_prefix, 
#                 max_threads, cubes_options, request_work_file, work_file):
def extend_cubes(mace4_args, new_cube_length, cubes, working_dir_prefix, 
                 max_threads, request_work_file, work_file):
    """
    spawns out Mac4 to extend cubes
    A file <working_dir_prefix>_stealing/new_work.out is used to hold the cubes given by busy workers
    Args:
        cubes_options (int): bit vector lsb 1 - do work stealing, 2 - cube has num cells filled at beginning
    """
    input_file = mace4_args['input_file']
    order = mace4_args['order']
    print_model = mace4_args['print_model']
    mace4 = mace4_args['mace4_exe']
    cubes_options = mace4_args['cubes_options']
    interp_out_file = mace4_args['output_file']
    
    done = False
    thread_slots = [0] * max_threads
    cube_file = cubes
    os.makedirs(f"{working_dir_prefix}_stealing", exist_ok=True)
    stealing_file = f"{working_dir_prefix}_stealing/new_work.out"

    while not done:
        # Path(stealing_file).unlink(True)
        extend_cube_jobs(input_file, interp_out_file, order, new_cube_length, cube_file, print_model,
                         mace4, cubes_options, working_dir_prefix, max_threads, thread_slots)
        work_list = list()
        if cubes_options % 2 == 1:
            work_list = request_work(working_dir_prefix, request_work_file, work_file, max_threads, thread_slots)
        print(f"debug extend_cubes, back from requested work, got {len(work_list)} jobs")
        if work_list:
            with (open(stealing_file, "w")) as fp:
                fp.write('\n'.join(work_list))
                fp.write('\n')
            cube_file = stealing_file
        else:
            done = True

    print("extend_cubes: All cubes are dispatched. Waiting for the last ones to finish...", flush=True)
    while(not all_done(thread_slots)):
        time.sleep(1)
    

    
__all__ =["extend_cubes"]

if __name__ == "__main__":
    default_mace4 = "../bin/mace4"
    max_threads = 8

