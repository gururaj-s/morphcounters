#!/usr/bin/perl -w

## README ##
## This file lists the benchmark names & trace-file paths for the benchmark suties used in run_MICRO_5bn.sh.
## ##

#******************************************************************************
# Benchmark Sets
# ************************************************************

%SUITES = ();

#***************************************************************
# MIX BENCHMARKS 
#***************************************************************

$SUITES{'specMIX_vnew'}      = 
'input/WL_traces/wl_10.gz input/WL_traces/wl_6.gz input/WL_traces/wl_5.gz input/WL_traces/wl_8.gz
input/WL_traces/wl_1.gz input/WL_traces/wl_4.gz input/WL_traces/wl_14.gz input/WL_traces/wl_2.gz
input/WL_traces/wl_7.gz input/WL_traces/wl_11.gz input/WL_traces/wl_3.gz input/WL_traces/wl_9.gz
input/WL_traces/wl_15.gz input/WL_traces/wl_13.gz input/WL_traces/wl_12.gz input/WL_traces/wl_0.gz
input/WL_traces/wl_4.gz input/WL_traces/wl_7.gz input/WL_traces/wl_6.gz input/WL_traces/wl_10.gz
input/WL_traces/wl_11.gz input/WL_traces/wl_9.gz input/WL_traces/wl_5.gz input/WL_traces/wl_15.gz';


##
$SUITES{'specMIX_name'}      =
'mix1
mix2
mix3
mix4
mix5
mix6';


#***************************************************************
# GAP BENCHMARK SUITE
#***************************************************************

##GAP v5 - with structs - 4Bn traces
$SUITES{'gap_v5'}      = 
'input/GAP_traces/pr_1_0.raw.usim.gz input/GAP_traces/pr_1_0.raw.usim.gz input/GAP_traces/pr_1_0.raw.usim.gz input/GAP_traces/pr_1_0.raw.usim.gz
input/GAP_traces/pr_2_0.raw.usim.gz input/GAP_traces/pr_2_0.raw.usim.gz input/GAP_traces/pr_2_0.raw.usim.gz input/GAP_traces/pr_2_0.raw.usim.gz
input/GAP_traces/cc_1_0.raw.usim.gz input/GAP_traces/cc_1_0.raw.usim.gz input/GAP_traces/cc_1_0.raw.usim.gz input/GAP_traces/cc_1_0.raw.usim.gz
input/GAP_traces/cc_2_0.raw.usim.gz input/GAP_traces/cc_2_0.raw.usim.gz input/GAP_traces/cc_2_0.raw.usim.gz input/GAP_traces/cc_2_0.raw.usim.gz
    input/GAP_traces/bc_2_0.raw.usim.gz input/GAP_traces/bc_2_0.raw.usim.gz input/GAP_traces/bc_2_0.raw.usim.gz input/GAP_traces/bc_2_0.raw.usim.gz';


$SUITES{'gap_name'}      = 
    'pr_t
    pr_w
    cc_t
    cc_w
    bc_w';


#***************************************************************
# SPEC BENCHMARK SUITE
#***************************************************************


$SUITES{'spec_hmpki_name1'}      = 
    'wl_0
    wl_12
    wl_5
    wl_4
    wl_10
    wl_1
    wl_11
    wl_8
    wl_3
    wl_9
    wl_13
    wl_14
    wl_7
    wl_6
    wl_15
    wl_2';

#SPEC with structs
$SUITES{'spec_hmpki_vnew'}      = 
    'input/WL_traces/wl_0.gz input/WL_traces/wl_0.gz input/WL_traces/wl_0.gz input/WL_traces/wl_0.gz
    input/WL_traces/wl_12.gz input/WL_traces/wl_12.gz input/WL_traces/wl_12.gz input/WL_traces/wl_12.gz
    input/WL_traces/wl_5.gz input/WL_traces/wl_5.gz input/WL_traces/wl_5.gz input/WL_traces/wl_5.gz
    input/WL_traces/wl_4.gz input/WL_traces/wl_4.gz input/WL_traces/wl_4.gz input/WL_traces/wl_4.gz
    input/WL_traces/wl_10.gz input/WL_traces/wl_10.gz input/WL_traces/wl_10.gz input/WL_traces/wl_10.gz
    input/WL_traces/wl_1.gz input/WL_traces/wl_1.gz input/WL_traces/wl_1.gz input/WL_traces/wl_1.gz
    input/WL_traces/wl_11.gz input/WL_traces/wl_11.gz input/WL_traces/wl_11.gz input/WL_traces/wl_11.gz
    input/WL_traces/wl_8.gz input/WL_traces/wl_8.gz input/WL_traces/wl_8.gz input/WL_traces/wl_8.gz
    input/WL_traces/wl_3.gz input/WL_traces/wl_3.gz input/WL_traces/wl_3.gz input/WL_traces/wl_3.gz
    input/WL_traces/wl_9.gz input/WL_traces/wl_9.gz input/WL_traces/wl_9.gz input/WL_traces/wl_9.gz
    input/WL_traces/wl_13.gz input/WL_traces/wl_13.gz input/WL_traces/wl_13.gz input/WL_traces/wl_13.gz
    input/WL_traces/wl_14.gz input/WL_traces/wl_14.gz input/WL_traces/wl_14.gz input/WL_traces/wl_14.gz
    input/WL_traces/wl_7.gz input/WL_traces/wl_7.gz input/WL_traces/wl_7.gz input/WL_traces/wl_7.gz
    input/WL_traces/wl_6.gz input/WL_traces/wl_6.gz input/WL_traces/wl_6.gz input/WL_traces/wl_6.gz
    input/WL_traces/wl_15.gz input/WL_traces/wl_15.gz input/WL_traces/wl_15.gz input/WL_traces/wl_15.gz
    input/WL_traces/wl_2.gz input/WL_traces/wl_2.gz input/WL_traces/wl_2.gz input/WL_traces/wl_2.gz';


#***************************************************************
# RESULTS BENCHMARK LISTS
#***************************************************************

$SUITES{'results_hac_descpot1'}      = 
    'wl_0
    wl_1
    wl_2
    wl_3
    wl_4    
    wl_5
    wl_6
    wl_7
    wl_8
    wl_9
    wl_10
    wl_11
    wl_12
    wl_13
    wl_14
    wl_15
    mix2
    mix5
    mix3
    mix1
    mix4
    mix6
    pr_t
    pr_w
    bc_w
    cc_t
    cc_w';


$SUITES{'results_hac_descpot1_nomix'}      = 
    'wl_0
    wl_1
    wl_2
    wl_3
    wl_4    
    wl_5
    wl_6
    wl_7
    wl_8
    wl_9
    wl_10
    wl_11
    wl_12
    wl_13
    wl_14
    wl_15
    pr_t
    pr_w
    bc_w
    cc_t
    cc_w';


$SUITES{'results_hac'}      = 
    'wl_0
    wl_1
    wl_11
    wl_12
    wl_3
    wl_5
    wl_4
    wl_10
    wl_6
    wl_9
    wl_2
    wl_7
    wl_14
    wl_8
    wl_13
    wl_15
    mix1
    mix2
    mix3
    mix4
    mix5
    mix6
    pr_t
    cc_t
    pr_w
    cc_w
    bc_w';


