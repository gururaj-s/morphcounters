#NONSECURE

##NONSECURE WITH NON_ECC MEMORY
./runall.pl  --w spec_hmpki_vnew --i spec_hmpki_name1 --f 30 --d "../RESULTS/sample_results/nonsecure" --o "2 8192 8 0 5000 32 0 7 7 25000 0 7 7 7 7"
./runall.pl  --w gap_v5 --i gap_name --f 30 --d "../RESULTS/sample_results/nonsecure" --o "2 8192 8 0 5000 32 0 7 7 25000 0 7 7 7 7"
./runall.pl  --w specMIX_vnew --i specMIX_name --f 30 --d "../RESULTS/sample_results/nonsecure" --o "2 8192 8 0 5000 32 0 7 7 25000 0 7 7 7 7"
