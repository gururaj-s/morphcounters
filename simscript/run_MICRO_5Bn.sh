## IMPORTANT ##
## Make sure the trace_dir variable is correctly set in runall.pl.
## Make sure that sym-link for traces is set in input (input/{WL_traces,GAP_traces})

## FORMAT ##
# ./runall.pl --w <BENCHMARK-SUITE-PATHS> --i <BENCHMARK-SUITE-NAMES>
# --f <NUMBER OF WORKLOADS TO FIRE IN PARALLEL> --d <RESULTS-DIRECTORY-PATH> --o <CONFIG-PARAMETERS>

# Config-Parameters: Found in main.c (line-448)
#1 - SIM_MODE (2 - means that first counters warmed up without any timing, then timing measured)
#2 - L3_SIZE (in KB)
#3 - L3_ASSOC
#4 - COMPRESSION_MODE (0 - none, 7-ZCC-only , 8-ZCC+Rebasing , see ctr_sim.c and compreession.c) 
#5 - INSTRUCTIONS_TO_SIMULATE in Millions (default 5000M)
#6 - METADATA-CACHE-SIZE in KB (128K)
#7 - OS_PAGE_MAPPING_POLICY (Default-0 i.e. Random Mapping for 4KB pages)
#8 - ENCRYPTION-CTR-DESIGN (1-SGX-Mono8, 4-Split64, 5-Split32, 6-Split16, 8-Split128, more in memOrg.h)
#9 - INTEGRITY-TREE-COUNTER-DESIGN (Beyond beyond Great Grandparent. Refer #.8 for possible vals)
#10 - INSTRUCTIONS-TO-WARMUP (Default 25000M)
#11 - SECURE-MEMORY-MODE (Default - 4 where MAC is in ECC-Chip)
#12-15 INTEGRITY-TREE COUNTER-DESIGN [Leaf,Parent,GrandParent, Great-GrandParent] (Refer #.8 for possible vals)

## KEY CONFIG PARAMS THAT CHANGE ##
## COMPRESSION_MODE (4), ENCRYPTION-CTR-DESIGN (8), INTEGRITY-TREE COUNTER-DESIGN (12-15).

##--------------
##Main Performance Results

##VAULT - SC-64 for Encryption, SC-32 for MT-1, SC-16 for MT-2 
./runall.pl   --w spec_hmpki_vnew --i spec_hmpki_name1 --f 30 --d "../RESULTS/sample_results/vault_nocomp" --o "2 8192 8 0 5000 128 0 7 6 25000 4 5 6 6 6"
./runall.pl                   --w gap_v5 --i gap_name --f 30 --d "../RESULTS/sample_results/vault_nocomp" --o "2 8192 8 0 5000 128 0 7 6 25000 4 5 6 6 6"
./runall.pl         --w specMIX_vnew --i specMIX_name --f 30 --d "../RESULTS/sample_results/vault_nocomp" --o "2 8192 8 0 5000 128 0 7 6 25000 4 5 6 6 6"


##SplitTree64 - SC-64 counters for Encryption & Integrity 
./runall.pl   --w spec_hmpki_vnew --i spec_hmpki_name1 --f 30 --d "../RESULTS/sample_results/SC64_SC64" --o "2 8192 8 0 5000 128 0 4 4 25000 4 4 4 4 4"
./runall.pl                   --w gap_v5 --i gap_name --f 30 --d "../RESULTS/sample_results/SC64_SC64" --o "2 8192 8 0 5000 128 0 4 4 25000 4 4 4 4 4"
./runall.pl         --w specMIX_vnew --i specMIX_name --f 30 --d "../RESULTS/sample_results/SC64_SC64" --o "2 8192 8 0 5000 128 0 4 4 25000 4 4 4 4 4"

##Morphable Counters (with ZCC and Rebasing)
./runall.pl  --w spec_hmpki_vnew --i spec_hmpki_name1 --f 30 --d "../RESULTS/sample_results/CC128DUNC_7bINT_CC128_Flex8" --o "2 8192 8 8 5000 128 0 14 8 25000 4 8 8 8 8"
./runall.pl   --w gap_v5 --i gap_name --f 30 --d "../RESULTS/sample_results/CC128DUNC_7bINT_CC128_Flex8" --o "2 8192 8 8 5000 128 0 14 8 25000 4 8 8 8 8"
./runall.pl          --w specMIX_vnew --i specMIX_name --f 30 --d "../RESULTS/sample_results/CC128DUNC_7bINT_CC128_Flex8" --o "2 8192 8 8 5000 128 0 14 8 25000 4 8 8 8 8"

##----------------
##For Fig.11 and Fig.14: Overflows without rebasing.

##Morphable Counters with ZCC-only
#./runall.pl   --w spec_hmpki_vnew --i spec_hmpki_name1 --f 30 --d "../RESULTS/sample_results/CC128_CC128" --o "2 8192 8 7 5000 128 0 8 8 25000 4 8 8 8 8"
#./runall.pl                    --w gap_v5 --i gap_name --f 30 --d "../RESULTS/sample_results/CC128_CC128" --o "2 8192 8 7 5000 128 0 8 8 25000 4 8 8 8 8"
#./runall.pl          --w specMIX_vnew --i specMIX_name --f 30 --d "../RESULTS/sample_results/CC128_CC128" --o "2 8192 8 7 5000 128 0 8 8 25000 4 8 8 8 8"

##----------------
##For Fig.5: Motivation data with SC-128.

##SplitTree128 - SC-128 counters for Encryption & Integrity (assumed MAC in ECC-Chip like Synergy)
#./runall.pl  --w spec_hmpki_vnew --i spec_hmpki_name1 --f 30 --d "../RESULTS/sample_results/SC128_SC128" --o "2 8192 8 0 5000 128 0 8 8 25000 4 8 8 8 8"
#./runall.pl                   --w gap_v5 --i gap_name --f 30 --d "../RESULTS/sample_results/SC128_SC128" --o "2 8192 8 0 5000 128 0 8 8 25000 4 8 8 8 8"
#./runall.pl         --w specMIX_vnew --i specMIX_name --f 30 --d "../RESULTS/sample_results/SC128_SC128" --o "2 8192 8 0 5000 128 0 8 8 25000 4 8 8 8 8"

#------------------

