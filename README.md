### GENERAL INFO ####
This simulator was used for Morphable Counters paper in MICRO-2018 (https://ieeexplore.ieee.org/abstract/document/8574558).  
This is a fork of USIMM (https://github.com/pranith/usimm)  


### COMPILATION INFO ### 
To compile the simulator: 
    1. Specify the path to your "zlib" package in ./simscript/env_zlib.sh. Then "source simscript/env_zlib.sh". 
    2. Inside ./src_turbo, and type "make". 
    3. This should create the executable inside ./bin/ 

    
### RUN INFO ### 
To run the simulator:
    1. Specify the path to "zlib" package in ./simscript/env_zlib.sh. Then "source simscript/env_zlib.sh".
    2. Specify the path to the folder containing the "simscript"  in runall.pl "trace_dir" variable.
    3. Set symbolic links to GAP_traces and WL_traces in "input" 
    3. Run the simulator using runall.pl with the commands shown in run_MICRO_5bn.sh, using the appropriate flags for workload and configuration parameters.

### TRACES INFO ###
-- The simulator takes in memory access traces for workloads.
-- A couple of sample traces are available at : https://www.dropbox.com/sh/98veamv5l6ht21r/AACVlNY4C-mhnUWmiUYZ_9Yta?dl=0
-- For more traces, please drop me a email at gururaj.s@gatech.edu.

### OTHER INFO ###   
The folders in this repo:
- bin: contains the simulator compiled binary goes.
- input: contains the files for DRAM power/timing parameters (e.g. 4Gb_x8.vi) & Memory-System parameters with memory-size, num-banks/ranks etc. (e.g. SGX_Baseline_16Gmem.cfg). Also need to modify the sym-links to point to benchmarks.
- obj: contains the object files produced during compilation.
- src_turbo: has all the code. Key files - ctr_sim.h/c that contains all the code for the organization & manipulations of counters, compression.h contains the compression mechanisms, memsys_ctr_flow.h that issues extra access for counters on data-access.
- RESULTS: contains the main results used in the paper
- simscript: contains the scripts for launching the simulator in different configurations (run_MICRO_5Bn.sh has all the run scripts).

Counter-Designs Supported: (defined in memOrg.h and used in ctr_sim.c)
Format below: #define-value - #define-variable - explanation.
1 - MONO8_CTR       -  SGX 8 byte counters - 8 per CL
4 - SPLIT64_CTR  -  Split counters - 64 per CL (1 x 64-bit Major, 64 x 6-bit Minor)
5 - SPLIT32_CTR_v1  -  Split counters - 32 per CL (1 x 64-bit Major, 32 x 12-bit Minor)
6 - SPLIT16_CTR_v1  -  Split counters - 16 per CL (1 x 64-bit Major, 16 x 24-bit Minor)
8 - SPLIT128_CTR    -  Split counters - 128 per CL (1 x 64-bit Major, 128 x 3-bit Minor)

Compression-Modes supported: (check compression.c)
0 - None.
7 - ZCC-only.
8 - ZCC + Rebasing (rebase when ZCC is going to have a overflow)

More information on how to specify the counter-design and the compression-mode are defined in ./simscript/run_MICRO_5bn.sh
        
