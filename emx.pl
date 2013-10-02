#!/usr/bin/perl -w

@stack = ();
$ind = "\t";
$start = "";
$end = "";
$linelength=0;

while(<>)
{
    my ($line,$outline);
    
    $outline = "";    
    chomp;
    $start = $1 if(/^#\$START=(.*)$/);
    $end   = $1 if(/^#\$END=(.*)$/);
    $linelength = $1 if(/^#\$LENGTH=(.*)$/);
    next if(/^#/); #commented line
    next if(/^\s*$/); #blank line

    $line = $_;
    if(/^@(.*)/) #inserted line
    {
        print $1."\n";
        next;
    
    }
    
#print STDERR "ORIGINAL LINE>>".$line."<<\n";    
    $outline .= $start;
    $outline .= (($ind=substr($ind, 0, -1)) ."</".pop(@stack).">") if(/^\s*;\s*$/); #close the element (in the top of stack)
    $outline .= "$ind<$1>$2</$1>" if(/^\s*(\w+)=([^=;]*);\s*$/); #simple node with data    
    if(/^\s*(\w+)(;?)(\s(.*))?\s*$/) #start e.
    {
	    my $closed=0;
	    $closed = 1 if($2 eq ";");
	    my $node = $1;
	    my $par = $4;
#print STDERR "PAR>>".$par."<<\n" if(defined $par);    
	    if(defined $par) 
	    {
    		$outline .= "$ind<$node";
	        while($par ne "" && $par =~ /\s*(\w+)=([^=;]*);\s*/)
	        {
				$k=$1; $ov=$v=$2;
				$v=~s/<l>/\(/;
				$v=~s/<r>/\)/;		    
	            $outline .= " $k=\\\"$v\\\"";
	            $par =~ s/$k=$ov;//;
#print STDERR "PARemiminate>>".$par."<<\n";    
	        }
	        $outline .= "".($closed==1 ?" /" :"").">";   
	    }
	    else
	    { $outline .= "$ind<$node".($closed==1 ?"/" :"").">"; }
	
	    if(!$closed)
	    {
	        $ind = $ind . "\t" ; 
    	    push @stack,$node;
	    }
    }
    $outline .= "$end\n";

    print $outline;    
}