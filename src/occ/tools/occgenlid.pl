#!/usr/bin/perl

################################################################################
# change log:
# n/a               andrewg     09/07/06        Created
#		    eggler      2009/04/06      added option file parm
#		    thallet     2012/08/09      Modified to use with OCC
################################################################################

################################################################################
#
# The purpose of this script is to take as input a occ binary file and convert
# it to a lid which can then be checked into cmvc.
#
################################################################################

use strict;
use warnings;

# ---- MODULES ----   #
use Getopt::Long;
use File::Path;
use File::Basename;

# ---- CONSTANTS ---- #

# ---- GLOBALS ---- #
my $g_debug = 0;

################################################################################
#
# Main program
#
################################################################################

exit(main());

################################################################################
#
# Main
#
################################################################################

sub main
{
    my $l_help = '';
    my $l_input_bin;
    my $l_lid_name;
    my $l_update_bin;
    my $optionsfile;


    if (!GetOptions('h|help'          => \$l_help,
		    'd|debug'         => \$g_debug,
		    'i|input_binary=s'=> \$l_input_bin,
		    'l|lid_name=s'    => \$l_lid_name,
		    'o|optionsfile=s' => \$optionsfile))
    {
	return -1;
    }

    #-- Handle the usage statement immediately.
    if ($l_help)
    {
	usageInfo();
	return 0;
    }

    if((!defined($l_input_bin)) || (!defined($l_lid_name)))
    {
	print "Must provide both --input_binary and --lid_name!\n";
	return -1;
    }

    if(! -e $l_input_bin)
    {
	print "Input binary file, $l_input_bin, does not exist!\n";
	return -1;
	
    }


    # We should now have some file called update_bin_* - lets rename it
    system ("cp $l_input_bin occ_update.bin");

    # Make sure that the extucode dir exists
    if( !(-d $ENV{'sb'}."/extucode") )
    {
       print "Directory ".$ENV{'sb'}."/extucode not found....Creating\n";
       system("mkdir " .$ENV{'sb'}."/extucode");
    }

    # Now lets run the build scripts
    system ("apyfipshdr -r ". $ENV{'bb'} . "/src/lidhdr/$l_lid_name.lidhdr -l ./occ_update.bin -o  " . $ENV{'sb'} . "/extucode/$l_lid_name.fips");

    system("apyruhhdr -r " . $ENV{'bb'} . "/src/lidhdr/$l_lid_name.lidhdr -l " . $ENV{'sb'} . "/extucode/$l_lid_name.fips");

    return(0);
}



################################################################################
#
# usageInfo
#
################################################################################

sub usageInfo
{
print<<EOF;
Usage:
    tpmdgenlid.pl -i <absolute bin file> -l <lid name>

OPERATIONS:
  -h, -H, --help     : Print this summary of options and exit.
  -d, --debug        : Displays additional output
  -i  --input_binary : Input binary file
  -l  --lid_name     : Lid name to output to w/o lid suffix (i.e. -l 81e00430)


EOF
}
