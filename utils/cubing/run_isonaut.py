#!/usr/bin/python3

"""

"""


from datetime import datetime
import os
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


def run_a_proc(working_dir, input_file, output_file, keep_cg):
    cmd = f"../utils/cubing/isonaut.py -i {input_file} -o {output_file} -k {keep_cg} >> isonaut.log 2>&1"
    subprocess.run(f"cd {working_dir}; {cmd} ",
                       capture_output=False, text=True, check=False, shell=True)


def run_process(slot_id, thread_slots, working_dir, input_file, out_dir, keep_cg):
    """ Required: the model files are: <input_file> (e.g. models.out),
            non_iso_models.out, leve1_non_iso_models.out, level2_non_iso_models.out.
        One or all of the models files may be absent.
        The final non-iso models file is <out_dir>/<input_file> (e.g.
         semi_working_7_0_1_models/semi_7.out.
    """
    has_out = os.path.isfile(f"{working_dir}/{input_file}") and os.path.getsize(f"{working_dir}/{input_file}") > 0
    has_non_iso = os.path.isfile(f"{working_dir}/non_iso_models.out")
    has_level1 = os.path.isfile(f"{working_dir}/level1_non_iso_models.out")
    has_level2 = os.path.isfile(f"{working_dir}/level2_non_iso_models.out")

    if has_out:
        if has_non_iso:
            next_out = "non_iso_models.out"
        elif has_level1:
            next_out = "level1_non_iso_models.out"
        elif has_level2:
            next_out = "level2_non_iso_models.out"
        else:
            next_out = f"../{out_dir}/{input_file}"
        run_a_proc(working_dir, input_file, next_out, keep_cg)

    if has_non_iso:
        if has_level1:
            next_out = "level1_non_iso_models.out"
        elif has_level2:
            next_out = "level2_non_iso_models.out"
        else:
            next_out = f"../{out_dir}/{input_file}"
        run_a_proc(working_dir, "non_iso_models.out", next_out, keep_cg)

    if has_level1:
        if has_level2:
            next_out = "level2_non_iso_models.out"
        else:
            next_out = f"../{out_dir}/{input_file}"
        run_a_proc(working_dir, "level1_non_iso_models.out", next_out, keep_cg)

    if has_level2:
        run_a_proc(working_dir, "level2_non_iso_models.out", f"../{out_dir}/{input_file}", keep_cg)

    #if os.path.isfile(f"{out_dir}/{input_file}"):
    #    subprocess.run(f"cd {out_dir}; touch {input_file}.available",
    #                   capture_output=False, text=True, check=False, shell=True)

    thread_slots[slot_id] = 0


def run_isonaut( input_dir_prefix, num_dir, input_file_name, print_canonical, num_threads ):
    thread_slots = [0] * num_threads
    for x in range(0, num_dir):
        in_dir = f"{input_dir_prefix}_{x}"
        if not os.path.isdir(in_dir):
            break
        out_dir = f"{in_dir}_models"
        os.makedirs(out_dir, exist_ok=True)
        
        slot_id = thread_available(num_threads, thread_slots)
        while slot_id < 0:
            time.sleep(0.1)
            slot_id = thread_available(num_threads, thread_slots)

        # print(f"^^^^^^^^^^^^^^^^^^^^^^{input_dir_prefix} {input_file_name} {num_threads} {slot_id}")
        thread_slots[slot_id] = threading.Thread(target=run_process,
                                                 args=(slot_id, thread_slots, in_dir, input_file_name,
                                                       out_dir, print_canonical))
        thread_slots[slot_id].start()

    while(not all_done(thread_slots)):
        time.sleep(2)
        


__all__ = ["run_isonaut"]

if __name__ == "__main__":
    mace4_exec = "../bin/mace4"
    cubes_options = 0   # bit-0 for work stealing
    
    parser = argparse.ArgumentParser(
        description='parallel processing for isonaut.')
    parser.add_argument('-d', dest='input_dir_prefix', type=str, default=None, help='models input file dir prefix')
    parser.add_argument('-n', dest='num_dir', type=int, default=1, help='number of input directories')
    parser.add_argument('-i', dest='input_file_name', type=str, default="models.out", help='models output file path')
    parser.add_argument('-w', dest='print_canonical', type=int, default=1, help='output the canonical graph')
    parser.add_argument('-t', dest='num_threads', type=int, default=num_threads)
    args = parser.parse_args()

    wall0 = time.time()
    cpu0 = time.process_time()
    run_isonaut( args.input_dir_prefix, args.num_dir, args.input_file_name, args.print_canonical, args.num_threads)
    cpu1 = time.process_time()
    wall1 = time.time()


    print( f'{datetime.now().strftime("%d/%m/%Y %H:%M:%S")} run_isonaut.py\nTotal CPU: {cpu1 - cpu0} sec\n Wall time: {wall1 - wall0} sec.')



