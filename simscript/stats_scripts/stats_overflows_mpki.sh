## Used to generate overflows-per-million-inst in Fig.11 and Fig.14

cd ..

mkdir stats_scripts/data/overflows_mpki/

#### SC64 vs ZCC ###
perl getdata.pl  -nh -s CUMU_OVERFLOWS_PER_MN_DATMEMACCESS_CTR_OVERALL -w results_hac_descpot1  -d  ../RESULTS/sample_results/SC64_SC64  ../RESULTS/sample_results/CC128_CC128  | tail -n +2  >stats_scripts/data/overflows_mpki/overflows_mpki_zcc.stat

grep -v mix stats_scripts/data/overflows_mpki/overflows_mpki_zcc.stat | sed 's/cc_t/TEMP_BENCH/g' | sed 's/pr_w/cc_t/g' | sed  's/TEMP_BENCH/pr_w/g' > stats_scripts/data/overflows_mpki/temp.stat; mv stats_scripts/data/overflows_mpki/temp.stat stats_scripts/data/overflows_mpki/overflows_mpki_zcc.stat;

#### SC64 vs SC128 vs ZCC ###
perl getdata.pl  -nh -s CUMU_OVERFLOWS_PER_MN_DATMEMACCESS_CTR_OVERALL -w results_hac_descpot1  -d  ../RESULTS/sample_results/SC64_SC64   ../RESULTS/sample_results/SC128_SC128  ../RESULTS/sample_results/CC128_CC128  | tail -n +2  >stats_scripts/data/overflows_mpki/overflows_mpki_zcc_sc128.stat

grep -v mix stats_scripts/data/overflows_mpki/overflows_mpki_zcc_sc128.stat | sed 's/cc_t/TEMP_BENCH/g' | sed 's/pr_w/cc_t/g' | sed  's/TEMP_BENCH/pr_w/g' > stats_scripts/data/overflows_mpki/temp.stat; mv stats_scripts/data/overflows_mpki/temp.stat stats_scripts/data/overflows_mpki/overflows_mpki_zcc_sc128.stat;

echo ". 1 1 1"  >> stats_scripts/data/overflows_mpki/overflows_mpki_zcc_sc128.stat;

perl getdata.pl  -nh -ns -amean -s CUMU_OVERFLOWS_PER_MN_DATMEMACCESS_CTR_OVERALL -w results_hac_descpot1_nomix -d ../RESULTS/sample_results/SC64_SC64   ../RESULTS/sample_results/SC128_SC128  ../RESULTS/sample_results/CC128_CC128  | tail -n +2 | sed 's/Amean/Average/'  >>stats_scripts/data/overflows_mpki/overflows_mpki_zcc_sc128.stat

sed  -i -- 's/_t/-twit/g' stats_scripts/data/overflows_mpki/overflows_mpki_zcc_sc128.stat
sed -i -- 's/_w/-web/g'   stats_scripts/data/overflows_mpki/overflows_mpki_zcc_sc128.stat

sed -i 's/^[ \t]*//g' stats_scripts/data/overflows_mpki/overflows_mpki_zcc_sc128.stat


#### ZCC vs ZCC+Rebasing ###
perl getdata.pl  -nh -s CUMU_OVERFLOWS_PER_MN_DATMEMACCESS_CTR_OVERALL -w results_hac_descpot1  -d ../RESULTS/sample_results/CC128_CC128   ../RESULTS/sample_results/CC128DUNC_7bINT_CC128_Flex8   | tail -n +2  >stats_scripts/data/overflows_mpki/overflows_mpki_zcc_flex.stat

grep -v mix stats_scripts/data/overflows_mpki/overflows_mpki_zcc_flex.stat | sed 's/cc_t/TEMP_BENCH/g' | sed 's/pr_w/cc_t/g' | sed  's/TEMP_BENCH/pr_w/g' >stats_scripts/data/overflows_mpki/temp.stat; mv stats_scripts/data/overflows_mpki/temp.stat stats_scripts/data/overflows_mpki/overflows_mpki_zcc_flex.stat

#### SC64 vs ZCC vs ZCC+Rebasing ###
perl getdata.pl  -nh -s CUMU_OVERFLOWS_PER_MN_DATMEMACCESS_CTR_OVERALL -w results_hac_descpot1  -d ../RESULTS/sample_results/SC64_SC64 ../RESULTS/sample_results/CC128_CC128   ../RESULTS/sample_results/CC128DUNC_7bINT_CC128_Flex8   | tail -n +2  >stats_scripts/data/overflows_mpki/overflows_mpki_zcc_sc64_flex.stat

grep -v mix stats_scripts/data/overflows_mpki/overflows_mpki_zcc_sc64_flex.stat | sed 's/cc_t/TEMP_BENCH/g' | sed 's/pr_w/cc_t/g' | sed  's/TEMP_BENCH/pr_w/g' >stats_scripts/data/overflows_mpki/temp.stat; mv stats_scripts/data/overflows_mpki/temp.stat stats_scripts/data/overflows_mpki/overflows_mpki_zcc_sc64_flex.stat

echo ". 1 1 1"  >> stats_scripts/data/overflows_mpki/overflows_mpki_zcc_sc64_flex.stat;

perl getdata.pl  -nh -ns -amean -s CUMU_OVERFLOWS_PER_MN_DATMEMACCESS_CTR_OVERALL -w results_hac_descpot1_nomix -d ../RESULTS/sample_results/SC64_SC64 ../RESULTS/sample_results/CC128_CC128   ../RESULTS/sample_results/CC128DUNC_7bINT_CC128_Flex8 | tail -n +2 | sed 's/Amean/Average/'>>stats_scripts/data/overflows_mpki/overflows_mpki_zcc_sc64_flex.stat

sed  -i -- 's/_t/-twit/g' stats_scripts/data/overflows_mpki/overflows_mpki_zcc_sc64_flex.stat
sed -i -- 's/_w/-web/g'   stats_scripts/data/overflows_mpki/overflows_mpki_zcc_sc64_flex.stat
sed -i 's/^[ \t]*//g' stats_scripts/data/overflows_mpki/overflows_mpki_zcc_sc64_flex.stat
