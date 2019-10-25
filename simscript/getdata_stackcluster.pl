#!/usr/bin/perl -w
## README ##
## Used to collate data, but with an array of statistics. Calls getdata.pl multiple times for each stat, to generate stack_cluster graphs like Fig.16
## ##
require ( "./bench_common.pl");
use Scalar::Util qw(looks_like_number);
use Data::Dumper;

$base_ipc_dir = "../RESULTS/A.BASE.01MBL3/" ;
$wsuite    = "spec2k6";
$norm_dir  = -1;


#####################################
######### USAGE OPTIONS      ########
#####################################

$ipcsum    = 0;
$ws        = 0;
$fairness  = 0;
$print_max = 0;
$max_start = 0;
$max_end   = 0;
$min_start = 0;
$min_end   = 0;
$print_no_stats   = 0;
$print_no_header  = 0;
$scurve    = 0;
$wlimit    = 0;
$print_amean     = 0;
$print_gmean     = 0;
$hmean     = 0;
$invert    = 0;
$printmask = ""; 
$nowarn    = 0;
$precint   = 0;
$dstat     = "";
$nstat     = "";
$numstat1  = "";
$numstat2  = "";
$mstat     = 0;
$slist     = "";
$cstat     = "";
$debug     = 0;
$noxxxx    = 0;
$excel = 1;
$bargraphpl = 0;
$perc       = 0;
$mult       = 1;
$add        = 0;

#####################################
######### GLOBAL VARIABLES   ########
#####################################

#####################################
######### USAGE OPTIONS      ########
#####################################

sub usage_1(){

    $USAGE = "Usage:  '$0 <-option> '";
    
    print(STDERR "$USAGE OF GETDATA_STACKCLUSTER\n");    
    print(STDERR "\t-s <num_stats> <statnames>  : number of stats, followed by name of the stat  \n");
    print(STDERR "\t-w <workload/suite>    : name of the workload suite from bench_common \n");
    print(STDERR "\t-d <statdirs>          : directory for stats (more than one ok) \n");
    print(STDERR "\t-n <stat1>,<stat2>,<stat3> <config1>,<config2>,<config3>   : print stats normalized to the sum of theses stat and dir <nums> \n");
    print(STDERR "\t-gmean                 : print geometric mean. \n");
    print(STDERR "\t-amean                 : print arithmetic mean. \n");
    print(STDERR "\t-bargraphpl            : output for bargraphpl. \n");
    print(STDERR "\t-perc                  : get stackcluster of percentages. \n");
    print(STDERR "\t-mult                  : default applied in end. first mult then add . \n");
    print(STDERR "\t-add                   : default applied in end. first mult then add . \n");
    
    print(STDERR "$USAGE OF GETDATA\n");        
    print(STDERR "\t-h                     : help -- print this menu. \n");
    print(STDERR "\t-t                     : throughput. \n");
    print(STDERR "\t-ws                    : weighted speedup. \n");
    print(STDERR "\t-hf                    : fairness. \n");
    print(STDERR "\t-b <basedir>           : base directory for ws and fairness \n");
    print(STDERR "\t-n <pos>               : print stats normalized to this dir <num> in dir list \n");
    print(STDERR "\t-d <statdirs>          : directory for stats (more than one ok) \n");
    print(STDERR "\t-s <statname>          : name of the stat (wild card ok) \n");
    print(STDERR "\t-nstat <statname>      : name of the numerator stat (X in X/Y ) \n");
    print(STDERR "\t-dstat <statname>      : name of the denominator stat (Y in X/Y ) \n");
    print(STDERR "\t-numstat1 <statname>   : name of the num1 stat (X in X-Y ) \n");
    print(STDERR "\t-numstat2 <statname>   : name of the num2 stat (Y in X-Y ) \n");
    print(STDERR "\t-mstat <val>           : value for multiplying stat \n");
    print(STDERR "\t-cstat <str>           : complex stat: A separate function computes stats for each string \n");
    print(STDERR "\t-ds <statname>         : name of the denominator stat (Y in X/Y ) \n");
    print(STDERR "\t-w <workload/suite>    : name of the workload suite from bench_common \n");
    print(STDERR "\t-wlimit <num>          : limit the number of workloads in suite to <num> \n");
    print(STDERR "\t-ns                    : print no individual stats. \n");
    print(STDERR "\t-nh                    : print no header. \n");
    print(STDERR "\t-max <start><end>      : print max vals of row from col start to col end. \n");
    print(STDERR "\t-min <start><end>      : print min vals of row from col start to col end. \n");
    print(STDERR "\t-scurve                : first directory must be base, second data. \n");
    print(STDERR "\t-amean                 : print arithmetic mean. \n");
    print(STDERR "\t-gmean                 : print geometric mean. \n");
    print(STDERR "\t-hmean                 : print harmonic mean. \n");

    print(STDERR "\t-invert                : print inverted values (useful for CPI) \n");
    print(STDERR "\t-printmask <str>       : print mask for columns ( 1-0-1-0 prints first and third col) \n");
    print(STDERR "\t-nowarn                : supress any warnings (be careful with this option) \n");
    print(STDERR "\t-precint               : precesion of integer for print values (default 3 digits after decimal) \n");
    print(STDERR "\t-slist <list>          : stats list ( separated by : \"stats1:stats2:stats3\") \n");
    print(STDERR "\t-debug                 : verbose mode for debugging. \n");
    print(STDERR "\t-noxxxx                : Print 0 for no data instead of xxxx. \n");

    print(STDERR "\n");

    exit(1);
}

######################################
########## PARSE COMMAND LINE ########
######################################

while (@ARGV) {
    $option = shift;
    
    if ($option eq "-t") {
        $ipcsum = 1;
    }elsif ($option eq "-ws") {
        $ws = 1;
    }elsif ($option eq "-hf") {
        $fairness = 1;
    }elsif ($option eq "-b") {
        $base_ipc_dir = shift;
    }elsif ($option eq "-n") {
        $norm_stat = shift;
        $norm_dir = shift;        
        @norm_stat_list = split ",", $norm_stat;
        @norm_dir_list = split ",", $norm_dir;
        die "Num of -n params for stats and dirs not matching" if ($#norm_stat_list != $#norm_dir_list)
    }elsif ($option eq "-perc") {
        $perc = 1;
    }elsif ($option eq "-mult") {
        $mult = shift;
        die "Multiplicant is nan" if (!looks_like_number($mult))                                      
    }elsif ($option eq "-add") {
        $add = shift;
        die "Addition is nan" if (!looks_like_number($add))                                      
    }
    elsif ($option eq "-h"){
        usage_1();
    }elsif ($option eq "-d") {
        while(@ARGV){
            $mydir = shift;
            push (@dirs, $mydir);
        }
    }elsif ($option eq "-s") {
        
        $num_stat = shift;
        if (looks_like_number($num_stat) ) {
            for (my $i = 0; $i < $num_stat ; $i++) {
                $mystat = shift;
                push (@stats,$mystat);
            }
        }
        else {
            die "Expected number of stats after -s"
        }
        
    }elsif ($option eq "-w") {
        $wsuite = shift;
    }elsif ($option eq "-wlimit") {
        $wlimit = shift;
    }elsif ($option eq "-ns") {
        $print_no_stats = 1;
    }elsif ($option eq "-nh") {
        $print_no_header = 1;
    }elsif ($option eq "-max") {
        $print_max = 1;
        $max_start = shift;    
        $max_end   = shift;    
    }elsif ($option eq "-min") {
        {   no warnings 'once';
            $print_min = 1; 
        }        
        $min_start = shift;    
        $min_end   = shift;    
    }elsif ($option eq "-scurve") {
        $scurve = 1;
    }elsif ($option eq "-amean") {
        $print_amean = 1;
    }elsif ($option eq "-gmean") {
        $print_gmean = 1;
    }elsif ($option eq "-hmean") {
        $hmean = 1;
    }elsif ($option eq "-invert") {
        $invert = 1;
    }elsif ($option eq "-printmask") {
        $printmask = shift;
    }elsif ($option eq "-precint") {
        $precint = 1;
    }elsif ($option eq "-nowarn") {
        $nowarn = 1;
    }elsif ($option eq "-nstat") {
        $nstat = shift;
    }elsif ($option eq "-dstat") {
        $dstat = shift;
    }elsif ($option eq "-numstat1") {
        $numstat1 = shift;
    }elsif ($option eq "-numstat2") {
        $numstat2 = shift;
    }elsif ($option eq "-mstat") {
        $mstat = shift;
    }elsif ($option eq "-slist") {
        $slist = shift;
    }elsif ($option eq "-cstat") {
        $cstat = shift;
    }elsif ($option eq "-debug") {
        $debug = 1;
    }elsif ($option eq "-noxxxx") {
        $noxxxx = 1;        
    }elsif ($option eq "-bargraphpl") {
        $bargraphpl = 1;
        $excel      = 0;
        $table      = 0;
    }elsif ($option eq "-table") {
        $bargraphpl = 0;
        $excel      = 0;
        $table      = 1;
    }
    else{
        usage_1();
        die "Incorrect option ... Quitting\n";
    }
}



##########################################################
# get the suite names, num_w, and num_p from bench_common

die "No benchmark set '$wsuite' defined in bench_common.pl\n"
        unless $SUITES{$wsuite};
    
my (@w) = split(/\s+/, $SUITES{$wsuite});
$num_w = scalar @w;
@bmks = split/-/,$w[0];
$num_p = scalar @bmks;
$num_p = 16;

$num_w = $wlimit if($wlimit && $wlimit < $num_w);

read_stats();
normalize_stats();
if($perc) {
    get_perc_stats();
}
mult_and_add();
if($excel) {
    output_excel_stats();
}
if($bargraphpl){
    output_bargraphpl_stats();
}
if($table){
    output_table();
}

############################# READ STATS ######################

sub read_stats{
    
    $command_1 = "perl getdata.pl -nh -noxxxx -s "; #Followed by stat
    $command_2 = " -w ".$wsuite." -d "; # Followed by dir

    for ($i=0;$i<= $#dirs; $i++){
        #print $i."\n";
        my @output_dir ;
        for ($j=0; $j <= $#stats; $j++) {                
            my $command_full = $command_1.$stats[$j].$command_2.$dirs[$i];
            print $command_full."\n" if($debug);
            my $output_dir_stats = `$command_full`;
            #print $output_dir_stats,"\n";                
            my @output_dir_stats_arr = split "\n", $output_dir_stats;
            foreach my $item(@output_dir_stats_arr) {
                $item =~ s/^\s*[a-z_A-Z0-9]*\s*//;
            }
            splice @output_dir_stats_arr, 0, 1;
            push(@output_dir,\@output_dir_stats_arr);
        }
        push(@output,\@output_dir);
    }
    
    
}

sub mult_and_add{
    for ($i=0;$i<= $#dirs; $i++){          
        for ($j=0; $j <= $#stats; $j++) {
            for($k = 0;$k < $num_w;$k++){                
                $output[$i][$j][$k] = $mult * $output[$i][$j][$k] + $add;
            }
        }
    }

}

sub normalize_stats{

    #    for ($i=0;$i<= $#dirs; $i++){
    #        
    #        for ($j=0; $j <= $#stats; $j++) {                
    #            print $dirs[$i],"-",$stats[$j],"\n";
    #
    #            for($k = 0;$k < $num_w;$k++){
    #                print $output[$i][$j][$k],"\n";
    #            }
    #        }             
    #    }  

    
    for($k = 0;$k < $num_w;$k++){
        my $norm_val = 0;
        for ($i =0; $i <= $#norm_stat_list; $i++){ 
            $norm_val +=  $output[$norm_dir_list[$i]][$norm_stat_list[$i]][$k];  
        }
        if($norm_val == 0){
            $norm_val = 1;
        }
        push(@norm_vals_arr,$norm_val);
    }

    #    for($k = 0;$k < $num_w;$k++){
    #    print $norm_vals_arr[$k],"\n";
    #}
    
    ## NORMALIZE USING CALCULATED VALUES
    for ($i=0;$i<= $#dirs; $i++){          
        for ($j=0; $j <= $#stats; $j++) {
            for($k = 0;$k < $num_w;$k++){
                if($norm_vals_arr[$k]){
                    $output[$i][$j][$k] =  $output[$i][$j][$k] /  $norm_vals_arr[$k];
                }
                else {
                    $output[$i][$j][$k] = "xxx"
                }
                
            }
        }
    }
    
}


sub get_perc_stats{

    
    for($k = 0;$k < $num_w;$k++){ #for each workload
        for($i = 0;$i <= $#dirs;$i++){ #for each directory
            #Calculate sum
            my $sum = 0;
            for ($j =0; $j <= $#stats; $j++){ 
                if($output[$i][$j][$k]){
                    $sum +=  $output[$i][$j][$k];
                }
            }
            if($sum == 0){
                $sum = 1;
            }
     
            #Normalize by sum
            for ($j =0; $j <= $#stats; $j++){ 
                
                $output[$i][$j][$k] = $output[$i][$j][$k]/$sum;
            }
            
        }
    }
    
}



sub output_excel_stats {
    ### PRINT HEADER ##
    if($print_no_header != 1){
        $header =  "WL"."\t";
        for ($i=0;$i<= $#dirs; $i++){          
            for ($j=0; $j <= $#stats; $j++) {
                $header = $header.$dirs[$i]."_".$stats[$j]."\t";
            }
        }
        print $header."\n";
    }
    
    #### PRINTING DATA #######
    if($print_no_stats != 1){
        for($k = 0;$k < $num_w;$k++){
            print $w[$k]."\t";
            my $tabs = "\t";
            for ($i=0;$i<= $#dirs; $i++){          
                for ($j=0; $j <= $#stats; $j++) {
                    if(looks_like_number($output[$i][$j][$k])){
                        printf("%.3f\t",$output[$i][$j][$k]);
                    }
                    else {
                        printf("%s\t",$output[$i][$j][$k]);
                        
                    }
                    $tabs=$tabs."\t";
                }
                print "\n";
                print $tabs;
            }
            print "\n"
                
        }
    }

    ### PRINT GEOMEAN ###
    if($print_gmean){
        print_geomean_excel();
    }    

    ### PRINT GEOMEAN ###
    if($print_amean){
        print_arithmean_excel();
    }    
       
}

sub print_geomean_excel {
    for ($i=0;$i<= $#dirs; $i++){          
        for ($j=0; $j <= $#stats; $j++) {
            $statprod[$i][$j] = 1;
            
            for($k = 0;$k < $num_w;$k++){

                if(looks_like_number($output[$i][$j][$k])){
                    $statprod[$i][$j] *= $output[$i][$j][$k];
                }
                else {
                    $statprod[$i][$j] *= 0;                    
                }
            }
            $geomean[$i][$j] =  $statprod[$i][$j] ** (1/$num_w);
        }
        
    }

    print "Gmean"."\t";
    my $tabs = "\t";
    for ($i=0;$i<= $#dirs; $i++){          
        for ($j=0; $j <= $#stats; $j++) {
            if(looks_like_number($geomean[$i][$j])){
                printf("%.3f\t",$geomean[$i][$j]);
            }
            else {
                printf("%s\t",$geomean[$i][$j]);                
            }
            $tabs=$tabs."\t";
        }
        print "\n";
        print $tabs;
    }
    print "\n"
        
}


sub print_arithmean_excel {
    for ($i=0;$i<= $#dirs; $i++){          
        for ($j=0; $j <= $#stats; $j++) {
            $statsum[$i][$j] = 0;
            
            for($k = 0;$k < $num_w;$k++){

                if(looks_like_number($output[$i][$j][$k])){
                    $statsum[$i][$j] += $output[$i][$j][$k];
                }
                else {
                    $statsum[$i][$j] += 0;                    
                }
            }
            $arithmean[$i][$j] =  $statsum[$i][$j] / $num_w ;
        }
        
    }

    print "Amean"."\t";
    my $tabs = "\t";
    for ($i=0;$i<= $#dirs; $i++){          
        for ($j=0; $j <= $#stats; $j++) {
            if(looks_like_number($arithmean[$i][$j])){
                printf("%.3f\t",$arithmean[$i][$j]);
            }
            else {
                printf("%s\t",$arithmean[$i][$j]);                
            }
            $tabs=$tabs."\t";
        }
        print "\n";
        print $tabs;
    }
    print "\n"
        
}


sub output_bargraphpl_stats {
    ### PRINT HEADER ##
    if($print_no_header != 1){
        $header =  "WL"."\t";
        for ($i=0;$i<= $#dirs; $i++){          
            for ($j=0; $j <= $#stats; $j++) {
                $header = $header.$dirs[$i]."_".$stats[$j]."\t";
            }
        }
        print $header."\n";
    }
    
    #### PRINTING DATA #######
    if($print_no_stats != 1){
        for($k = 0;$k < $num_w;$k++){
            #print $w[$k]."\t";
            print "multimulti=".$w[$k]."\n";

            for ($i=0;$i<= $#dirs; $i++){
                print n2a($i+1)." ";
                for ($j=0; $j <= $#stats; $j++) {
                    if(looks_like_number($output[$i][$j][$k])){
                        printf("%.3f ",$output[$i][$j][$k]);
                    }
                    else {
                        printf("%s ",$output[$i][$j][$k]);                        
                    }

                }
                print "\n";
            }
            print "\n"                
        }
    }

    ### PRINT GEOMEAN ###
    if($print_gmean){
        print_geomean_bargraphpl();
    }    

    ### PRINT ARITHMEAN ###
    if($print_amean){
        print_arithmean_bargraphpl();
    }    

}

sub print_geomean_bargraphpl {
    for ($i=0;$i<= $#dirs; $i++){          
        for ($j=0; $j <= $#stats; $j++) {
            $statprod[$i][$j] = 1;
            
            for($k = 0;$k < $num_w;$k++){

                if(looks_like_number($output[$i][$j][$k])){
                    $statprod[$i][$j] *= $output[$i][$j][$k];
                }
                else {
                    $statprod[$i][$j] *= 0;                    
                }
            }
            $geomean[$i][$j] =  $statprod[$i][$j] ** (1/$num_w);
        }
        
    }

    print "multimulti=Gmean"."\n";
    my $tabs = "\t";
    for ($i=0;$i<= $#dirs; $i++){          
        print n2a($i+1)." ";
        for ($j=0; $j <= $#stats; $j++) {
            if(looks_like_number($geomean[$i][$j])){
                printf("%.3f ",$geomean[$i][$j]);
            }
            else {
                printf("%s ",$geomean[$i][$j]);                
            }
        }
        print "\n";
    }
    print "\n"
        
}


sub print_arithmean_bargraphpl {
    for ($i=0;$i<= $#dirs; $i++){          
        for ($j=0; $j <= $#stats; $j++) {
            $statsum[$i][$j] = 0;
            
            for($k = 0;$k < $num_w;$k++){

                if(looks_like_number($output[$i][$j][$k])){
                    $statsum[$i][$j] += $output[$i][$j][$k];
                }
                else {
                    $statsum[$i][$j] += 0;                    
                }
            }
            $arithmean[$i][$j] =  $statsum[$i][$j] / $num_w ;
        }
        
    }

    print "multimulti=Amean"."\n";
    my $tabs = "\t";
    for ($i=0;$i<= $#dirs; $i++){          
        print n2a($i+1)." ";
        for ($j=0; $j <= $#stats; $j++) {
            if(looks_like_number($arithmean[$i][$j])){
                printf("%.3f ",$arithmean[$i][$j]);
            }
            else {
                printf("%s ",$arithmean[$i][$j]);                
            }
        }
        print "\n";
    }
    print "\n"
        
}


sub n2a {
    my ($n) = @_;
    my @a;
    while ($n > 0) {
        unshift @a, ($n-1) % 26;
        $n = int(($n-1)/26); #Edited for failing corner case
    }
    join '', map chr(ord('A') + $_), @a;
}

sub output_table {
    ### PRINT HEADER ##
    if($print_no_header != 1){
        $header =  "WL"."\t";
        for ($i=0;$i<= $#dirs; $i++){          
            for ($j=0; $j <= $#stats; $j++) {
                $header = $header.$dirs[$i]."_".$stats[$j]."\t";
            }
        }
        print $header."\n";
    }
    
    #### PRINTING DATA #######
    if($print_no_stats != 1){
        for($k = 0;$k < $num_w;$k++){
            print $w[$k]."\t";
            my $tabs = "\t";
            for ($i=0;$i<= $#dirs; $i++){          
                for ($j=0; $j <= $#stats; $j++) {
                    if(looks_like_number($output[$i][$j][$k])){
                        printf("%.3f\t",$output[$i][$j][$k]);
                    }
                    else {
                        printf("%s\t",$output[$i][$j][$k]);
                        
                    }
                    #$tabs=$tabs."\t";
                }
                print "\t";
                #print $tabs;
            }
            print "\n"
                
        }
    }

    ### PRINT GEOMEAN ###
    if($print_gmean){
        print_geomean_table();
    }    

    ### PRINT ARITHMEAN ###
    if($print_amean){
        print_arithmean_table();
    }    
    
}


sub print_geomean_table {
    for ($i=0;$i<= $#dirs; $i++){          
        for ($j=0; $j <= $#stats; $j++) {
            $statprod[$i][$j] = 1;
            
            for($k = 0;$k < $num_w;$k++){

                if(looks_like_number($output[$i][$j][$k])){
                    $statprod[$i][$j] *= $output[$i][$j][$k];
                }
                else {
                    $statprod[$i][$j] *= 0;                    
                }
            }
            $geomean[$i][$j] =  $statprod[$i][$j] ** (1/$num_w);
        }
        
    }

    print "Gmean"."\t";
    my $tabs = "\t";
    for ($i=0;$i<= $#dirs; $i++){          
        for ($j=0; $j <= $#stats; $j++) {
            if(looks_like_number($geomean[$i][$j])){
                printf("%.3f\t",$geomean[$i][$j]);
            }
            else {
                printf("%s\t",$geomean[$i][$j]);                
            }
            #$tabs=$tabs."\t";
        }
        print "\t";
        #print $tabs;
    }
    print "\n"
        
}


sub print_arithmean_table {
    for ($i=0;$i<= $#dirs; $i++){          
        for ($j=0; $j <= $#stats; $j++) {
            $statsum[$i][$j] = 1;
            
            for($k = 0;$k < $num_w;$k++){

                if(looks_like_number($output[$i][$j][$k])){
                    $statsum[$i][$j] += $output[$i][$j][$k];
                }
                else {
                    $statsum[$i][$j] += 0;                    
                }
            }
            $arithmean[$i][$j] =  $statsum[$i][$j] / $num_w;
        }
        
    }

    print "Amean"."\t";
    my $tabs = "\t";
    for ($i=0;$i<= $#dirs; $i++){          
        for ($j=0; $j <= $#stats; $j++) {
            if(looks_like_number($arithmean[$i][$j])){
                printf("%.3f\t",$arithmean[$i][$j]);
            }
            else {
                printf("%s\t",$arithmean[$i][$j]);                
            }
            #$tabs=$tabs."\t";
        }
        print "\t";
        #print $tabs;
    }
    print "\n"
        
}
