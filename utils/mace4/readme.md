
## Running Mace4 with the Isomorphic Cubes Algorithms

### Software Requirement

The Mace4 executable is compiled in C++ version 11, and scripts are written in Python 3.8.  They are tested in Linux computers:

Linux 4.19.0-6-amd64 #1 SMP Debian 4.19.67-2+deb10u1 (2019-09-20) x86_64 GNU/Linux


### File Organization

top directory

    |-- bin           # directory for executables
        |-- mace4     # Mace 4 executable
    |-- p9m4          # top working directory
        |-- inputs    # Mace4 inputs files
        |-- utils
            |-- mace                  # scripts to generate models
                |-- bootstrap.py      # top script to kick off model enumeration process
                |-- extend_cubes.py   # functions to extend the length of cubes using Mace4
                |-- multi_cube_analyzer.py   # helper functions to remove isomorphic cubes
                |-- iso_cubes.py             # functions to check for isomorphism between cubes
                |-- run_cubes.py             # functions to execute Mace4 to enumerate models
                |-- some other scripts for future use and for testing               
                

### Model Enumeration
The algebra supported are listed in bootstrap.py, and the input files in Mace4 format are in .../p9m4/inputs.
Edit bootstrap.py to specify the algebra, order, and the desired target cube length. E.g. to enumerate all models of semigroups of order 7, using cubes of length 25:

    algebra = "semi"
    target_cube_length = 25
    order = 7

The supported cube lengths for each type of algebra are listed at the top of the file bootsrap.py.  For example, semigroups contains only 1 binary and the supported cube lengths are:

    cube_sequence_2 = [2, 4, 9, 16, 25, 36, 49, 64]

All parameters for running semigroups is in the following entry in the run_data dictionary:

            'semi':   {'seq': cube_sequence_2, 'relations': [False], 
                         'input': 'semi', 'arities': [2], 'radius': r_2, 'remove': -1},

To run the script, issue the command in the .../p9m4 directory  

    utils/mace4/bootsctrap.py
    
To clean up the working directory after the run, issue the command in the .../p9m4 directory:

    rm -rf *_working_*
    rm -rf utils/mace4/working
    
    

                       