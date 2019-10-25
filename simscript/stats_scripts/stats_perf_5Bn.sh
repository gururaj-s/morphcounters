## Used to generate results in Fig. 15

cd ..

perl getdata.pl  -nh -s TOTAL_IPC -w results_hac_descpot1 -n 1 -d ../RESULTS/sample_results/vault_nocomp  ../RESULTS/sample_results/SC64_SC64 ../RESULTS/sample_results/CC128DUNC_7bINT_CC128_Flex8 | tail -n +2  >stats_scripts/data/performance.stat

echo ". 0 0 0"  >> stats_scripts/data/performance.stat

perl getdata.pl  -nh -ns -gmean -s TOTAL_IPC -w spec_hmpki_name1 -n 1 -d ../RESULTS/sample_results/vault_nocomp  ../RESULTS/sample_results/SC64_SC64 ../RESULTS/sample_results/CC128DUNC_7bINT_CC128_Flex8  | tail -n +2 | sed 's/Gmean/SPEC/' >>stats_scripts/data/performance.stat

perl getdata.pl  -nh -ns -gmean -s TOTAL_IPC -w specMIX_name -n 1 -d ../RESULTS/sample_results/vault_nocomp  ../RESULTS/sample_results/SC64_SC64 ../RESULTS/sample_results/CC128DUNC_7bINT_CC128_Flex8 | tail -n +2 | sed 's/Gmean/MIX/' >>stats_scripts/data/performance.stat

perl getdata.pl  -nh -ns -gmean -s TOTAL_IPC -w gap_name -n 1 -d ../RESULTS/sample_results/vault_nocomp  ../RESULTS/sample_results/SC64_SC64 ../RESULTS/sample_results/CC128DUNC_7bINT_CC128_Flex8  |  tail -n +2 | sed 's/Gmean/GAP/' >>stats_scripts/data/performance.stat


perl getdata.pl  -nh -ns -gmean -s TOTAL_IPC -w results_hac -n 1 -d ../RESULTS/sample_results/vault_nocomp  ../RESULTS/sample_results/SC64_SC64 ../RESULTS/sample_results/CC128DUNC_7bINT_CC128_Flex8 |  tail -n +2 | sed 's/Gmean/ALL28/' >>stats_scripts/data/performance.stat

sed  -i -- 's/_t/-twit/g' stats_scripts/data/performance.stat
sed -i -- 's/_w/-web/g'   stats_scripts/data/performance.stat

sed -i 's/^[ \t]*//g' stats_scripts/data/performance.stat

sed -e 's/mix2/TEMP1_BENCH/g' -e 's/mix5/TEMP2_BENCH/g' -e 's/mix1/TEMP4_BENCH/g'  -e 's/mix4/TEMP5_BENCH/g'  stats_scripts/data/performance.stat | sed -e 's/TEMP1_BENCH/mix1/g' -e 's/TEMP2_BENCH/mix2/g' -e 's/TEMP4_BENCH/mix4/g'  -e 's/TEMP5_BENCH/mix5/g' > stats_scripts/data/temp.stat; mv stats_scripts/data/temp.stat stats_scripts/data/performance.stat ;
