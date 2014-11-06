#!/usr/bin/perl -w

#----------------------------------------------------------------------------
use strict;
use Cwd;

#use Getopt::Long;
#use File::Basename;
#use Time::Local;
#use POSIX qw(strftime);
#use Carp;
#use FindBin;		# std
#use Cwd 'abs_path';

#use Time::Local;

use lib "$ENV{'sb'}/simu/fsp/perl/CPAN";
use lib "$ENV{'bb'}/src/simu/fsp/perl/CPAN";

# Setup/declare some of our initial variables
my $configuration_file = "$ENV{'HOME'}/.occ_simics_config";
my $cur_dir = getcwd();
my $use_gdb = 0;	# Default to not allowing GDB in simics
my $verbose = 0;	# Default to non-verbose output
my $commands = "";

my $target = "occ-simple-common.simics";
my $binary_file = "validation/apps/ocb_irq/ocb_irq.out";
my $simics_workspace = "";
my @search_paths;

# Read in the configuration file
read_configuration_file();

#start by handling the command line arguments
process_command_line();

# Add $simics_workspace and $cur_dir to @search_paths
push @search_paths, $simics_workspace;
push @search_paths, $cur_dir;

# Now put all our read in options together...
$target = find_full_path($target);
$binary_file = find_full_path($binary_file);


# And setup our command line arguments
my $binary_cmd = "oci_space.load-binary $binary_file";
my $symtable_cmd = "new-symtable st $binary_file";
my $gdb_cmd = "new-gdb-remote";

my $full_command = "\"" . $binary_cmd . ";" . $symtable_cmd . ";" . $commands;
if($use_gdb)
{
    $full_command .= $gdb_cmd . ";";
}
$full_command .= "\"";

# Create the simics command
if($verbose)
{
    print "Target: $target\n";
    print "Binary: $binary_file\n";
}
my $simics_command = "ulimit -c unlimited; $simics_workspace/simics -verbose $target -e $full_command"; 


# Run the simics command
if($verbose)
{
    print "\nRunning $simics_command\n";
}

system("$simics_command");

exit 0;




###############################################################################
#			Helper Functions    				      #
###############################################################################


################################################################################
#
# Processes any command line arguments.  
#
################################################################################
sub process_command_line
{
    my $n = 0;

    while ( $n < @ARGV )
    {
	$_ = $ARGV[$n];
	chomp($_);

	if (/^-h$/i || /^--h$/i || /^-help$/i || /^--help$/i)
	{
	    print "start_occ_simics.pl [options] [binary .out file]\n\n";
	    print "Based on your \$HOME/.occ_simics_config and passed in arguments, it will start\n";
	    print "a simics session.  The parameters in the EXAMPLE_.occ_simics_config file:\n";
	    print "\tSIMICS_WORKSPACE: Path to simics workspace\n";
	    print "\tDEFAULT_TARGET: Target you wish you to use in the target directory under simics-workspace\n";
	    print "\tDEFAULT_BINARY: Binary value to use like occ.out\n";
	    print "\tPATH: Paths to check when looking for target/binary files.  Checked recursively\n";
	    print "\tCOMMAND: Comands to run immediately when starting simics.  Can add directories, set output-radix, etc\n";
	    print "\n";
	    print "Arguments to start_occ_simics.pl\n";
	    print "-t  Modifies the target\n";
	    print "-g  Automatically enables GDB\n";
	    print "-v  Verbose option\n";
	    print "\n";   	   
	    exit 0;
	}
	elsif(/^-t$/i || /^--target$/i)
	{
	    $target = $ARGV[$n + 1] or die "Bad command line variable: expecting a target\n";
	    $n++;
	}
	elsif(/\.out/)
	{
	    $binary_file = $ARGV[$n];	    
	}
	elsif(/^-g$/i)
	{
	    $use_gdb = 1;
	}
	elsif(/^-v/)
	{
	    $verbose = 1;
	}
	else
	{
	    print ("Unrecognized command: $_ \n");
	    print ("To view all the options and help text run \'start_simics -h\' \n");
	    exit 1;
	}

	$n++;
    }
}

################################################################################
#
# Parses the configuration file located at $configuration_file
#
################################################################################
sub read_configuration_file
{
    
    open(CONFIG_FILE, "$configuration_file") || warn "Could not read $configuration_file\n";

    
    while(<CONFIG_FILE>)
    {
	#print $_;
	if(/^SIMICS_WORKSPACE\s*=\s*(\/.*)/)
	{
	    $simics_workspace = $1;
	    chomp $simics_workspace;

	    my $last_char = substr $simics_workspace,-1,1;
	    if($last_char ne '/')
	    {
		$simics_workspace .= "/";
	    }
	}
	elsif(/^PATH\s*=\s*(\/.*)/)
	{
	    my $path = $1;
	    chomp $path;

	    my $last_char = substr $path,-1,1;
	    if($last_char ne '/')
	    {
		$path .= "/";
	    }
	   
	    push @search_paths, $path;
	}
	elsif(/^DEFAULT_TARGET\s*=\s*(\/.*)/)
	{
	    $target = $1;
	    chomp $target;
	}
	elsif(/^DEFAULT_BINARY\s*=\s*(\/.*)/)
	{
	    $binary_file = $1;
	    chomp $binary_file;
	}
	elsif(/^COMMAND\s*=\s*(.*)/)
	{
	    my $tmp_cmd = $1;
	    chomp $tmp_cmd;
	    #print "Saw $tmp_cmd\n";
	    $commands .= $1 . ";";
	}
    }

    close(CONFIG_FILE);
}



################################################################################
#
# Finds the full path for a passed in file using @search_paths
#
################################################################################
sub find_full_path
{
    my $arg = shift;

    # If $arg starts with "/" assume we were given the full path
    if($arg =~ /^\//)
    {
	# Do nothing
    }
    else
    {
	my $entry_found = 0;

	foreach my $path (@search_paths)
	{
	    # Need to find the file based off either our $cur_dir or $simics_workspace
	    my @output = `find $path 2>&1 | grep $arg `;
	    foreach my $entry (@output)
	    {
		chomp $entry;
		if($entry =~ /$arg$/)
		{
		    $arg = $entry;		# $arg now has the full path
		    $entry_found = 1;
		    last;
		}
	    }

	    if($entry_found)
	    {
		last;
	    }
	}

	$entry_found or die "Could not find $arg";    
    }

    return $arg;
}
