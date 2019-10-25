#!/usr/bin/perl -w

## README ##
## Used to fire a set of jobs for a set of workloads in a benchmark suite. Called in run_MICRO_5Bn. 
## ##

require ( "./bench_common.pl");
sub rtrim { my $s = shift; $s =~ s/\s+$//;       return $s };

$trace_dir = "/home/gattaca4/gururaj/sc_exp/MORPHCTR_REPO_2019/morphctr_code/";

#####################################
######### USAGE OPTIONS      ########
#####################################

$wsuite   = "testA";
$isuite   = "testA_name";
$sim_exe  = "../bin/usimm";
$dest_dir  = ".";
$sim_opt   = "  ";
$debug     = 0;
$firewidth = 30; # num parallel jobs at a time
$sleep     = 0;

#####################################
######### USAGE OPTIONS      ########
#####################################

sub usage(){

$USAGE = "Usage:  '$0 <-option> '";
 
print(STDERR "$USAGE\n");
print(STDERR "\t--h                    : help -- print this menu. \n");
print(STDERR "\t--r                    : rerun -- re-execute killed sims. \n");
print(STDERR "\t--d <dest_dir>         : name of the destination dir. \n");
print(STDERR "\t--w <workload/suite>   : workload suite from bench_common \n");
print(STDERR "\t--i <input/suite>      : input suite from bench_common \n");
print(STDERR "\t--s <sim_exe>          : simulator executable \n");
print(STDERR "\t--o <sim_options>      : simulator options \n");
print(STDERR "\t--dbg                  : debug \n");
print(STDERR "\t--f <val>              : firewidth, num of parallel jobs \n");
print(STDERR "\t--z <val>              : sleep for z seconds \n");
print(STDERR "\n");

exit(1);
}

######################################
########## PARSE COMMAND LINE ########
######################################
 
while (@ARGV) {
    $option = shift;
 
    if ($option eq "--dbg") {
        $debug = 1;
    }elsif ($option eq "--w") {
        $wsuite = shift;
    }elsif ($option eq "--i") {
        $isuite = shift;
    }elsif ($option eq "--s") {
        $sim_exe = shift;
    }elsif ($option eq "--o") {
        $sim_opt = shift;
    }elsif ($option eq "--d") {
        $dest_dir = shift;
    }elsif ($option eq "--f") {
        $firewidth = shift;
    }elsif ($option eq "--z") {
        $sleep = shift;
    }elsif ($option eq "--e") {
        $sim_exe = shift;
    }else{
	usage();
        die "Incorrect option ... Quitting\n";
    }
}


##########################################################             
# create full path to mysim and dest dir

$mysim = "$dest_dir"."/"."simGS.bin";
$myopt = "$dest_dir"."/"."simGS.opt";

print "$mysim\n";

##########################################################
# get the suite names, num_w, and num_p from bench_common

die "No benchmark set '$wsuite' defined in bench_common.pl\n"
    unless $SUITES{$wsuite};
@w = split(/\n/, $SUITES{$wsuite});
$num_w = scalar @w;


die "No benchmark set '$isuite' defined in bench_common.pl\n"
    unless $SUITES{$isuite};
@i = split(/\n/, $SUITES{$isuite});

##########################################################

if($debug==0){
    mkdir "$dest_dir";
    mkdir "$dest_dir/logs";
    #system ("ls $dest_dir");
    system ("cp $sim_exe $mysim");
    system ("chmod +x $mysim");
    system ("echo $sim_opt > $myopt");
}

##########################################################

for($ii=0; $ii< $num_w; $ii++){
    $iname = rtrim($i[$ii]);
    $wname = $w[$ii];

 #   die "No NUM_TRACES entry found in benchcommon.pl \n"
#	unless $NUM_TRACES{$iname};
    
#    for($jj=0; $jj< $NUM_TRACES{$iname}; $jj++){

	#$jj=0;
	$infiles = " ";
	
	@bmks = split(/ /,$wname);
	
	
	foreach (@bmks) {
	    $bname   = $_;
	    #$infile  = $trace_dir.$bname."_".$jj;
	    $infile  = $trace_dir.$bname;
	    $infiles = $infiles." ".$infile;
	}
	
	#$outfile = "$dest_dir/".$iname."_".$jj;
    
    ##Needed to run on ARM cluster
	#$outfile = "$dest_dir/".$iname;
    #	$cluster_comm = "bsub -q RD -J final.$iname -M 53886080 -R\"rusage[mem=51920]\" -n 1 -W 14400 -o $outfile";
    #	$exe = "$cluster_comm $mysim $sim_opt $infiles";
    
    ##Needed to run on GT Cluster
    $outfile = " > $dest_dir/".$iname;
    $cluster_comm = "nohup";
    $logfile = "$dest_dir/logs/".$iname; #For log file
    #	$exe = "$cluster_comm $mysim $sim_opt $infiles $outfile";
    $exe = "$cluster_comm $mysim $sim_opt $logfile $infiles $outfile"; #For log file
    
	#delete previous output file
	$remove = "rm -rf  $outfile";
	system("$remove") unless($debug);
	
	#background all jobs except the last one (acts as barrier)
	$exe .= " &" unless($ii%$firewidth==($firewidth-1));
	print "$exe\n";
	system("$exe") unless($debug);
	
#    }

}
