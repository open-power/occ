#!/usr/bin/perl
##############################################################################
#
# @file    occ_gdb.pl
#
# @author  Timothy Hallett
# 
# @brief   Perl script for starting OCC Simics & GDB
#
# @verbatim
#
# Updates: 
#  v0.1.0  01/10/2012 -- thallet -- created.
#  v0.1.1  01/27/2012 -- thallet -- Allow starting @ __ssx_boot or in OCC
#                                   bootloader.  
#
# @endverbatim
# 
#
##############################################################################



use strict;

use lib "/afs/rch/usr4/thallet/public/bin/";
use OccSimicsAutomate;

my $VERSION = "0.1.1";

#----------------------------------------------------------
# GDB/DDD Related Globals
#----------------------------------------------------------
my $G_gdb_path             = "/gsa/ausgsa/home/t/h/thallet/web/shared/gdb_ppc_eabi/usr/local/bin/powerpc-\*-eabi-gdb";
my $G_ddd_path             = "ddd --debugger $G_gdb_path";
my $G_gdbscript_filename   = "/tmp/occgdb.".$ENV{"USER"}.".txt";
my $G_gdb_cmd_file         = "";
my $G_gdb                  = 1;
my $G_ddd                  = 0;
my $G_xterm                = 1;
my $G_custom_term          = 0;
my $G_nexus                = 0;
my $G_ssx_boot_start       = 0;
my $G_occ_bootloader_start = 0;


#----------------------------------------------------------
# Simics Related Globals
#----------------------------------------------------------
my $port     = "0";
my $name     = "occgdb";
my $hostname = "";



#----------------------------------------------------------
# Handle Args
#----------------------------------------------------------
while ($ARGV = shift) 
{
  if ($ARGV =~ m%^-d%i)
  {
    # Use DDD instead of GDB
    $G_gdb = 0;
    $G_ddd = 1;
  }
  elsif ($ARGV =~ m%^-f%i)
  {
    # Allow user to specify file with gdb commands
    # that will be run after OCC GDB setup commands 
    $G_gdb_cmd_file = shift;
  }
  elsif ($ARGV =~ m%^-n%i)
  {
    # Allow user to specify file with gdb commands
    # that will be run after OCC GDB setup commands 
    $G_nexus = 1;
  }
  elsif ($ARGV =~ m%^-t%i)
  {
    $G_xterm = 0;
  }
  elsif ($ARGV =~ m%^-p%i)
  {
    # Allow user to specify path to gdb
    $G_gdb_path = shift;
    $G_ddd_path = "ddd --debugger $G_gdb_path"
  }
  elsif ($ARGV =~ m%^-s%i)
  {
    $G_ssx_boot_start = 1;
  }
  elsif ($ARGV =~ m%^-x%i)
  {
    $G_occ_bootloader_start = 1;
  }
  else
  {
    # Use defaults
    dump_help();
    exit;
  }
}

my @months = qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec);
my @weekDays = qw(Sun Mon Tue Wed Thu Fri Sat Sun);
my ($second, $minute, $hour, $dayOfMonth, $month, $yearOffset, $dayOfWeek, $dayOfYear, $daylightSavings) = localtime();
my $year = 1900 + $yearOffset;
my $hhmmss = sprintf("%02d:%02d:%02d",$hour,$minute,$second);
my $G_time = @weekDays[$dayOfWeek]." ".$months[$month]." ".$dayOfMonth.", $year ".$hhmmss;

#----------------------------------------------------------
# Start Simics, start OCC, start GDB, then hand control to user
#----------------------------------------------------------
if(1)
{
  #----------------------------------------------------------
  # Start Simics & run OCC in Simics until main(), __ssx_boot or
  # first instruction.
  #----------------------------------------------------------
  my $simics = new OccSimicsAutomate;
  if($G_ssx_boot_start)
  {
    $simics->startAndRunOccUntilSsxBoot();
  }
  elsif($G_occ_bootloader_start)
  {
    $simics->start_simics();
    $simics->getSimicsTime();
  }
  else
  {
    $simics->startAndRunOccUntilMain();
  }
  
  #-------------------------------------------------------- 
  # Start GDB Session
  #-------------------------------------------------------- 
  $simics->simics_cmd("new-gdb-remote port=$port name=$name");

  
  #-------------------------------------------------------- 
  # Get Hostname Simics is running on
  #-------------------------------------------------------- 
  $simics->simics_cmd("!hostname > occ_simics_hostname");
  sleep(5);
  open(HOSTNAME,"<".$ENV{"sb"}."/../simics/occ_simics_hostname");
  while(<HOSTNAME>)
  {
    if ($_ =~ /ibm/){$hostname = $_; chomp($hostname);}
  }
  close(HOSTNAME);
  $simics->simics_cmd("!echo \"\" > occ_simics_hostname");

  
  #-------------------------------------------------------- 
  # Get Port GDB is listening on
  #-------------------------------------------------------- 
  $simics->simics_cmd("pipe \"print -d $name->listen\" \"cat > occ_simics_port\"");
  sleep(5);
  open(PORT,"<".$ENV{"sb"}."/../simics/occ_simics_port");
  while(<PORT>)
  {
    if ($_ =~ /[0-9]{4,}/){$port = $_; chomp($port);}
  }
  close(PORT);
  $simics->simics_cmd("!echo \"\" > occ_simics_port");
  
  
  #-------------------------------------------------------- 
  # Create a file with GDB Initialization commands
  #-------------------------------------------------------- 
  my @gdb_start_commands = 
  (
    "echo \"Automating GDB Connection to OCC in Simics: $hostname:$port\n",
    "set architecture powerpc:common\n",
    "symbol-file ".$ENV{"sb"}."/occ/occ.out\n",
    "target remote $hostname:$port"
  );
  open(GDB_SCRIPT,">$G_gdbscript_filename");
  foreach(@gdb_start_commands)
  {
    print GDB_SCRIPT $_;
  }
  close(GDB_SCRIPT);
  
  #-------------------------------------------------------- 
  # If user passed in a GDB command file, append it to GDBscript
  # we just created
  #--------------------------------------------------------
  if($G_gdb_cmd_file)
  {
    system("cat $G_gdb_cmd_file >> $G_gdbscript_filename");
  }

  #--------------------------------------------------------
  # Make simics command prompt interactive
  # or tell user that it is not interactive
  #--------------------------------------------------------
  if($G_xterm == 0)
  {
    system("echo \"Simics Command Line is Unavailable while running GDB\"");
  }

  #-------------------------------------------------------- 
  # Now start GDB or DDD, passing in the command file
  #--------------------------------------------------------   
  if($G_ddd)
  {
    system("$G_ddd_path --command=$G_gdbscript_filename");
  }
  elsif($G_nexus)
  {
     # Don't start GDB
  }
  elsif($G_xterm)
  {
     system("xterm -bg darkolivegreen -fg wheat -sb -sl 10000 -geometry 120x40 -hold -title \"GDB -> OCC [".$ENV{"SANDBOX"}."] @ $hostname:$port, started $G_time\" -e $G_gdb_path --command=$G_gdbscript_filename &");
  }
  elsif($G_custom_term)
  {
     system("$G_custom_term \"$G_gdb_path --command=$G_gdbscript_filename\"");
  }
  else
  {
    system("$G_gdb_path --command=$G_gdbscript_filename");
  }
  
  #--------------------------------------------------------
  # Make simics command prompt interactive
  #--------------------------------------------------------
  $simics->interact();

  #-------------------------------------------------------- 
  # When we are done, close simics & clean up files
  #--------------------------------------------------------  
  $simics->close();
  system("if [ -e $G_gdbscript_filename ]; then rm $G_gdbscript_filename; fi");
}


sub dump_help
{
  my $name = "occgdb.pl";
  
  print "\n";
  print "Usage:  $name [options]\n";
  print "\n";
  print "Version: $VERSION\n\n";
  
  print "Usage Notes:\n";
  print "  This script will start simics and make a gdb or ddd connection to it automatically.\n";
  print "  - Must be run from sandbox\n";
  print "  - DDD must be in PATH\n"; 

  print "\n";

  print "Options:\n";
  print "\t[-d]                  Use DDD instead of GDB\n";
  print "\t[-n]                  Use Nexus Debugger instead of GDB (must manually start & attach Nexus)\n";
  print "\t[-t]                  Don't start a seperate xterm to do debugging (no simics interaction allowed) \n";
  print "\t[-f <gdb cmd file>]   Use <gdb command file> to run gdb commands after OCC has started\n";
  print "\t[-p <path/to/gdb>]    Use <path/to/gdb> as the gdb executable\n";
  print "\t[-s]                  Don't start @ OCC main(), start @ __ssx_boot() instead.\n";
  print "\t[-x]                  Don't start @ OCC main(), start @ first instruction (bootloader)) \n";

  print "\n";
}


##############################################################################
# Notes:
# ----------------------------------------------------------------------------
#
# GDB Command File Example Commands:
#   graph display g_amec_sys.fw.ameintdur
#   break rtl_do_tick
#   c
#   print g_amec_sys.fw.ameintdur
# 


