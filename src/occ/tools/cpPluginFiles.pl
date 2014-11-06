#!/usr/bin/perl

# This is a utility to generate tmgtSrcParse.C automatically.
# It will searching backing build if file is not present in the sandbox.

use strict;
use File::Find ();
use Time::localtime;


#use re 'debug';
my $VERSION = 1.0;


sub usage() {
    print "Usage: $0 -o  output directory  \n";
    print "\n";
    print "-o:     Used as the output directory where plugins files are dropped\n";
    print "\n\n";
    exit 1;
}


# Locals
my $base = $ENV{PWD};
my $baseBB = $ENV{PWD};
my $output;
my $arg;
my %Comps;
my $ignore = 1;
my $fips_comp_id = "$ENV{bs}/fstp/fsp/fips_comp_id.H";
#occ/incl/comp_ids.h
my $wordn = "userdata";
my $wordnd = 0;
my $Errno = 0;
my $NA = 1;

while ($ARGV = shift) 
{
    if ( $ARGV =~ m/-o/i ) 
    {
	$output = shift;
    }
}

if ( $output eq "" )
{
    print "Please specify an output directory\n\n";
    usage();
}
# Hardcode just the values that occc needs
$base="$ENV{SANDBOXBASE}/src/occc/405/";
#$baseBB="$ENV{bb}/src/occc/405/";
#$baseBB="/afs/austin.ibm.com/projects/esw/occ810/Builds/occ810_130926a/src/occc/405/";
$baseBB="$ENV{BACKED_SANDBOXDIR}";
my @values = split(':', $baseBB);
foreach my $val (@values) 
{
    if ( $val =~ m/occ810/)
    {
        $baseBB=$val;
    }
 }
#we should not input -d option, let $ignore = 1;  #so compile won't fail if there are dups
$wordnd = 4;  #look at userdata4
$wordn .= $wordnd;
#$NA = 0;
my $IgnoreDir = "ssx";

# grab all the files we want to copy
# *_service_codes.h, occSrcParse.C, comp_ids.h
my $IncludeFiles=`find $base -type f -name \"comp_ids.h\" -or -name \"*_service_codes.h\" -or -name \"*SrcParse.C"`;
my @IncludeFilesArray= split("\n", $IncludeFiles);
foreach my $val (@IncludeFilesArray) 
{
    print "cp $val $output\n";
    system "cp $val $output\n";
}
