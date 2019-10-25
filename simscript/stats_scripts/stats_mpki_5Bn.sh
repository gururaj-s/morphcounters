## Used to generate MPKI results in Fig. 16

cd ..

perl getdata_stackcluster.pl -nh -bargraphpl -n 0 0 -s 9 USIMM_NEW_DATA_MPKI USIMM_NEW_CTR_MPKI USIMM_UPDATED_MTREE_MPKI_0 USIMM_UPDATED_MTREE_MPKI_1 USIMM_UPDATED_MTREE_MPKI_2 USIMM_UPDATED_MTREE_MPKI_3 USIMM_UPDATED_MTREE_MPKI_4 USIMM_UPDATED_MTREE_MPKI_5 USIMM_NEW_TWIN_MPKI  -w results_hac_descpot1 -d ../RESULTS/sample_results/vault_nocomp  ../RESULTS/sample_results/SC64_SC64 ../RESULTS/sample_results/CC128DUNC_7bINT_CC128_Flex8 | tail -n +1  >stats_scripts/data/mpki.stat 

echo "multimulti=." >> stats_scripts/data/mpki.stat
echo "A 0 0 0 0 0 0 0 0 0"  >> stats_scripts/data/mpki.stat
echo "B 0 0 0 0 0 0 0 0 0"  >> stats_scripts/data/mpki.stat
echo "C 0 0 0 0 0 0 0 0 0"  >> stats_scripts/data/mpki.stat
echo ""  >> stats_scripts/data/mpki.stat

perl getdata_stackcluster.pl -nh -ns -amean -bargraphpl -n 0 0 -s 9 USIMM_NEW_DATA_MPKI USIMM_NEW_CTR_MPKI USIMM_UPDATED_MTREE_MPKI_0 USIMM_UPDATED_MTREE_MPKI_1 USIMM_UPDATED_MTREE_MPKI_2 USIMM_UPDATED_MTREE_MPKI_3 USIMM_UPDATED_MTREE_MPKI_4 USIMM_UPDATED_MTREE_MPKI_5 USIMM_NEW_TWIN_MPKI  -w spec_hmpki_name1 -d ../RESULTS/sample_results/vault_nocomp  ../RESULTS/sample_results/SC64_SC64 ../RESULTS/sample_results/CC128DUNC_7bINT_CC128_Flex8  | tail -n +1 | sed 's/Amean/SPEC/' >>stats_scripts/data/mpki.stat

perl getdata_stackcluster.pl -nh -ns -amean -bargraphpl -n 0 0 -s 9 USIMM_NEW_DATA_MPKI USIMM_NEW_CTR_MPKI USIMM_UPDATED_MTREE_MPKI_0 USIMM_UPDATED_MTREE_MPKI_1 USIMM_UPDATED_MTREE_MPKI_2 USIMM_UPDATED_MTREE_MPKI_3 USIMM_UPDATED_MTREE_MPKI_4 USIMM_UPDATED_MTREE_MPKI_5 USIMM_NEW_TWIN_MPKI  -w specMIX_name -d ../RESULTS/sample_results/vault_nocomp  ../RESULTS/sample_results/SC64_SC64 ../RESULTS/sample_results/CC128DUNC_7bINT_CC128_Flex8 | tail -n +1 | sed 's/Amean/MIX/' >>stats_scripts/data/mpki.stat

perl getdata_stackcluster.pl -nh -ns -amean -bargraphpl -n 0 0 -s 9 USIMM_NEW_DATA_MPKI USIMM_NEW_CTR_MPKI USIMM_UPDATED_MTREE_MPKI_0 USIMM_UPDATED_MTREE_MPKI_1 USIMM_UPDATED_MTREE_MPKI_2 USIMM_UPDATED_MTREE_MPKI_3 USIMM_UPDATED_MTREE_MPKI_4 USIMM_UPDATED_MTREE_MPKI_5 USIMM_NEW_TWIN_MPKI   -w  gap_name -d ../RESULTS/sample_results/vault_nocomp  ../RESULTS/sample_results/SC64_SC64 ../RESULTS/sample_results/CC128DUNC_7bINT_CC128_Flex8 |  tail -n +1 | sed 's/Amean/GAP/' >>stats_scripts/data/mpki.stat


perl getdata_stackcluster.pl -nh -ns -amean -bargraphpl -n 0 0 -s 9 USIMM_NEW_DATA_MPKI USIMM_NEW_CTR_MPKI USIMM_UPDATED_MTREE_MPKI_0 USIMM_UPDATED_MTREE_MPKI_1 USIMM_UPDATED_MTREE_MPKI_2 USIMM_UPDATED_MTREE_MPKI_3 USIMM_UPDATED_MTREE_MPKI_4 USIMM_UPDATED_MTREE_MPKI_5 USIMM_NEW_TWIN_MPKI   -w   results_hac_descpot1 -d ../RESULTS/sample_results/vault_nocomp  ../RESULTS/sample_results/SC64_SC64 ../RESULTS/sample_results/CC128DUNC_7bINT_CC128_Flex8 |  tail -n +1 | sed 's/Amean/ALL28/' >>stats_scripts/data/mpki.stat

sed  -i -- 's/_t/-twit/g' stats_scripts/data/mpki.stat
sed -i -- 's/_w/-web/g'   stats_scripts/data/mpki.stat

sed -i 's/^[ \t]*//g' stats_scripts/data/mpki.stat


sed -e 's/mix2/TEMP1_BENCH/g' -e 's/mix5/TEMP2_BENCH/g' -e 's/mix1/TEMP4_BENCH/g'  -e 's/mix4/TEMP5_BENCH/g'  stats_scripts/data/mpki.stat | sed -e 's/TEMP1_BENCH/mix1/g' -e 's/TEMP2_BENCH/mix2/g' -e 's/TEMP4_BENCH/mix4/g'  -e 's/TEMP5_BENCH/mix5/g' > stats_scripts/data/temp.stat; mv stats_scripts/data/temp.stat stats_scripts/data/mpki.stat;

awk '{ print $1,$2,$3,$4,$5,$6+$7+$8+$9,$10}' stats_scripts/data/mpki.stat | sed "/^multimulti/s/0[ ]*$//" | sed "/^[ ]*0/s/0[ ]*$//" >stats_scripts/data/temp.stat | mv stats_scripts/data/temp.stat stats_scripts/data/mpki.stat
