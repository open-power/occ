#!/usr/bin/perl
#
# Homemade OCC Code Coverage Tool (HOCCT)
#
# Written by: Timothy Hallett (tghallett@us.ibm.com)
# Date:       11/17/2011
# Purpose:    To determine code coverage by parsing OCC Simics Execution Trace 
#             and comparing it to OCC objdump source code output.
# 
# Updates:    11/28/2011 - Automatic dump of source/assembler within sandbox
#                        - Handle CLI args better (flags instead of static)   
#                        - Added ability to interleave source code into ccov
#                          output files, with variable parameters for number 
#                          of lines of source context. 
#             11/29/2011 - Added ability to use simics (via OccSimincsAutomate
#                          package) to generate the execution/instruction trace             
#             11/30/2011 - Modified Help to give useful information.
#                          Added timing of simics & viewer for source-interleave dump
#                          Added autobuild of occ.out & image.bin if they are not 
#                          present in the sandbox.
#             01/17/2012 - Check for zero instructions run (and helpful error)      
#             01/20/2012 - Updated tool for multi-user/system use (unique instructions.txt filename)
#                          Updated tool to start code coverage from __ssx_boot() instead of main()       
#            
# Usage:      hocct.pl [args]
# 
#             Output File will be dumped to: /tmp/occ_source_dump.ccov.txt
#
#             It will contain ":::::" at the start of lines that have been 
#             executed in the simics trace that was grabbed.
#
#             It is receommeded that if a user wants a graphical view of the 
#             code coverage, they use the following command:
#               gvimdiff /tmp/occ_source_dump.txt /tmp/occ_source_dump.ccov.txt
#                           
# Ideas:      - When opening in Gvim, could eaily convert to HTML (:TOhtml), save 
#                 and then have a visual representation of the changes.
#             - If it is in HTML, might as well make the hyperlinks actually 
#                 link to the file...somehow...                           
#             - Of course, 1 & 1a are moot if you are well versed at GVIM and use
#                 that as the diff tool.  If that is the case, then you can just use 
#                 the gF command to go directly to the source & line number you have
#                 under the cursor.  
#
# Todo:       11/30/2011 - Have arg that allows folders to be skipped for counting 
#                          code coverage, or for source-interleave tool.              
#             11/30/2011 - Create a sandbox and enter it, check out files, compile, 
#                          start simics, etc.  Easy to do, hard to do correctly...
#                          ...i.e. ask user for backing build/svn revs? etc...
#             12/01/2011 - Could probably easily allow user to pass in command file to
#                          run while getting trace... 
#             01/20/2012 - Code coverage on applet space?  Complex, Would need to break on applet
#                          start & end, and keep track of which applet ran.

use strict;
use File::Basename;

# First email in the list is the primary contact
my @G_contact_emails = ("tghallett\@us.ibm.com","someone_else\@us.ibm.com");
my $G_primary_contact = @G_contact_emails[0];
my $VERSION = "0.1.0";

# Will be using following env vars
# $ENV{"sb"}
# $ENV{"MCP_PATH"}
# $ENV{"MCP_VER"}
# $ENV{"SANDBOX"}
# $ENV{"USER"}
# 

# Global variables modified by CLI args
my $G_instructions_file     = "";
my $G_source_dump_file      = "";
my $G_diff_prog             = "";
my $G_viewer                = "";
my $G_objdump_args          = "-d -l";
my $G_debug_lvl             = 0;
my $G_interleave_source     = 0;
my $G_num_src_lines_context = 5;
my $G_num_src_lines_before  = 0;
my $G_date                  = "occ";
my $G_ccov_time_in_ms       = 25;

#Global variables that are internal only
my $G_inst_counter      = 0;
my $G_source_counter    = 0;
my @G_func_list_from_source;
my @G_func_executed;

# For debug statistics 
my $G_time_started            = 0;
my $G_time_compare_started    = 0;
my $G_time_compare_ended      = 0;
my $G_time_simics_started     = 0;
my $G_time_simics_ended       = 0;
my $G_time_interleave_started = 0;
my $G_time_interleave_ended   = 0;

#------------------------------------------------
# Parse Arguments
#------------------------------------------------
@ARGV = ('-') unless @ARGV;
#Available -: __c__fg____lmnopqr__u_wxyz1234567890
#Available +: abcdefghijklmnopqrstuvwxyz1234567890

while ($ARGV = shift) 
{
  if ($ARGV =~ m%^-i%i)
  {
    $G_instructions_file = shift;
  }
  elsif ($ARGV =~ m%^-s%i)
  {
    $G_source_dump_file = shift;
  }
  elsif ($ARGV =~ m%^-d%i)
  {
    # External Diff program
    $G_diff_prog = shift;
  } 
  elsif ($ARGV =~ m%^-w%i)
  {
    # External EDITOR program
    $G_viewer = shift;
  } 
  elsif ($ARGV =~ m%^-v%i)
  {
    # Makes use of DEBUG subroutine to spit out debug into
    $G_debug_lvl = shift;
  } 
  elsif ($ARGV =~ m%^-j%i)
  {
    # Don't play with this unless you know what you are doing
    $G_objdump_args = shift;
  } 
  elsif ($ARGV =~ m%^-t%i)
  {
    # Don't play with this unless you know what you are doing
    $G_ccov_time_in_ms = shift;
  } 
  elsif ($ARGV =~ m%^-e%i)
  {
    # Create ccov src interleaved output file
    if($G_interleave_source == 0){$G_interleave_source = 1;}
  } 
  elsif ($ARGV =~ m%^-k%i)
  {
    # Hide executed assembly from ccov src interleaved output file
    $G_interleave_source = 2;
  } 
  elsif ($ARGV =~ m%^-a%i)
  {
    # Number of src lines after current in ccov src interleaved output file
    $G_num_src_lines_context = shift;
  } 
  elsif ($ARGV =~ m%^-b%i)
  {
    # Number of src lines before current in ccov src interleaved output file
    $G_num_src_lines_before = shift;
  } 
  elsif ($ARGV =~ m%^-u%i)
  {
    # Create unique filenames each run (appends date/time)
    $G_date = "";
  } 
  elsif ($ARGV =~ m%^-h%i)
  {
    dump_help();
    exit 0;
  } 
  else
  {
    dump_help();
    exit 0;
  }
}

#------------------------------------------------
# Check for all required arguments
#------------------------------------------------
if(0)
{
  # No arguments required at this time.
}


#------------------------------------------------
# For now, lets die if we aren't in a sandbox
#------------------------------------------------
if( !defined($ENV{"sb"}) ){die "This script needs to run in an ODE sandbox!\n";}

#------------------------------------------------
# Get all Date/Time info for use later in script
#------------------------------------------------
my ($second, $minute, $hour, $dayOfMonth, $monthOffset, $yearOffset, $dayOfWeek, $dayOfYear, $daylightSavings) = localtime();
my $year = 1900 + $yearOffset;
my $month = 1 + $monthOffset;
my $day =  $dayOfMonth;
my $yyyymmdd = sprintf("%04d%02d%02d",$year,$month,$day);
my $hhmmss = sprintf("%02d%02d%02d",$hour,$minute,$second);

if(!($G_date))
{
  $G_date = "".$yyyymmdd."_".$hhmmss."";
}
$G_time_started = time();

#------------------------------------------------
# Run commands to generate binaries if needed
#------------------------------------------------
if( !(-e "".$ENV{"sb"}."/occ/occ.out") )
{
  system("pushd \$sb; make clean && make; popd");

  if( !(-e "".$ENV{"sb"}."/occ/occ.out") ){die "No \$sb/occ/occ.out to run objdump against!";}
}

if( !(-e "".$ENV{"sb"}."/image.bin") )
{
  system("pushd \$sb; make combineImage; popd");

  if( !(-e "".$ENV{"sb"}."/image.bin") ){die "No \$sb/image.bin to run simics with!";}
}

#------------------------------------------------
# Run commands to generate objdump file if needed
#------------------------------------------------
if(!($G_source_dump_file))
{
  my $objdump_args = $G_objdump_args;
  my $elf_file = "".$ENV{"sb"}."/occ/occ.out";
  my $output_file = "/tmp/occ_source_".$ENV{"USER"}."_".$ENV{"SANDBOX"}."_".$G_date.""; 
  $G_source_dump_file = $output_file;

  my $temp_elf = "/tmp/occ.out.".$ENV{"USER"}."";

  system("cp $elf_file $temp_elf");
  
  print "Generating Source/Assembler Dump File: $G_source_dump_file\n";
  system("".$ENV{"MCP_PATH"}."/opt/mcp/bin/ppcnf-".$ENV{"MCP_VER"}."-objdump ".$objdump_args." ".$temp_elf." > ".$output_file."");

  system("ls -lrth $output_file");
  
  system("rm $temp_elf");
}


#------------------------------------------------
# Run simics to generate instruction trace file if needed
#------------------------------------------------
$G_time_simics_started = time();
if(!($G_instructions_file))
{
  print "Generating OCC Instruction Trace via Simics:\n";
  sleep(1);

  use lib "/afs/rch/usr4/thallet/public/bin/";
  use OccSimicsAutomate;

  $G_instructions_file = "/tmp/instructions_".$ENV{"USER"}."_".$ENV{"SANDBOX"}."_".$G_date.".txt";

  my $simics = new OccSimicsAutomate;
  $OccSimicsAutomate::DEBUG_LEVEL = 1;

  $simics->{instr_file} = $G_instructions_file;
  $simics->startAndRunOccUntilSsxBoot();

  $simics->runOccWithInstructionTrace($G_ccov_time_in_ms);
 
  $simics->close();
}
$G_time_simics_ended = time();


#------------------------------------------------
# Open Files
#------------------------------------------------
open(INST,"<$G_instructions_file");
open(SOURCE,"<$G_source_dump_file");
open(CCOV,">$G_source_dump_file.ccov.txt");


#-------------------------------------------------------------------
# Compare Instructions to Source; Create new file with Coverage Info
#-------------------------------------------------------------------
$G_time_compare_started = time();

while (<INST>)
{
  my $str = $_;
  chomp($str);
  
  DEBUG(2,"Searching for $str\n");
  if( !($_ =~ /^fff/)){DEBUG(1,"Invalid instruction address: $str\n"); next;}  #should check to make sure address makes sense
  
  while(<SOURCE>)
  {    
    # Only increment source line count if address starts the line.
    if($_ =~ /^fff/){$G_source_counter++}  
    
    # If this line contains a function call, add it to list.
    if($_ =~ /(^fff..... <)(.*)(>:)/){push(@G_func_list_from_source,$2);}
    
    # See if this source line matches the instruction 
    if($_ =~ /^$str/)
    {
      # Print to the screen when we see we covered a function call
      # but don't leave while look, because there will be another 
      # instruction with this same address
      if($_ =~ /(^fff..... <)(.*)(>:)/){push(@G_func_executed,$2); DEBUG(0,"Executed Function: $2\n");print CCOV ":::::$_";}
      else
      {
        $G_inst_counter++;     # Found instruction that matches
        print CCOV ":::::$_";

        last;
      }
    }
    else
    {
      print CCOV $_;
    }
    if(eof(SOURCE)){print "Couldn't find instruction @ $str!!!\n";}
  }
}

#------------------------------------------------
# Finish dumping rest of source file 
#------------------------------------------------
while(<SOURCE>)
{
  print CCOV $_;
}


#------------------------------------------------
# Compare list of functions (executed v. source)
#------------------------------------------------
foreach (@G_func_executed)
{
  my $func = $_;

  # This must be set to zero before the following foreach 
  # loop runs.  It is used as the index to the array.
  my $idx = 0;
  
  # Loops through function list in source, looking for
  # match to an executed function.
  foreach(@G_func_list_from_source)
  {
    $idx++;
    
    # If funtion matches...
    if($_ =~ /$func/)
    {
      #print "Removing $func Idx: ".($idx-1)."\n";
      
      # Remove element from this array
      splice(@G_func_list_from_source,$idx-1,1);

      # Leave for loop
      last;
    }
  }
}

foreach(@G_func_list_from_source)
{
  DEBUG(0,"Didn't Execute Function: $_\n");
}


#------------------------------------------------
# Print Results to screen
#------------------------------------------------
if($G_source_counter)
{
  printf("Code Coverage:  %2.1f %   (%d of %d instructions executed)\n",($G_inst_counter/$G_source_counter)*100,$G_inst_counter,$G_source_counter);
  print "\nDumped executed code coverage file into ccov output file: $G_source_dump_file.ccov.txt\n\n";
}
else
{
  die "No instructions were executed, exiting script without continuing processing\n";
}


#------------------------------------------------
# Cleanup
#------------------------------------------------
close(SOURCE);
close(CCOV);
close(INST);

$G_time_compare_ended = time();


#------------------------------------------------
# Do a diff if requested
#------------------------------------------------
if($G_diff_prog)
{
  system("$G_diff_prog $G_source_dump_file $G_source_dump_file.ccov.txt");
}


#------------------------------------------------
# Do a source dump info ccov file if requested
#------------------------------------------------
$G_time_interleave_started = time();
if($G_interleave_source)
{
  print "Dumping source code into ccov output file: $G_source_dump_file.ccov_src.txt\n";
 
  open(CCOV,"<$G_source_dump_file.ccov.txt");
  open(SRCCOV,">$G_source_dump_file.ccov_src.txt");

  my $last_file = "";
  my $last_dir = "";
  my $last_line_num = 0;
  my $l_new_file = 0;
  
  while(<CCOV>)
  {    
    # Grab file/line num every time
    if($_ =~ /(^\/gsa\/.*)(:)([0-9]*$)/)
    {
      if($l_new_file == 0){print SRCCOV "\n\n"};
      $last_file = $1;
      $last_line_num = $3;
      $l_new_file = 1;
      DEBUG(1,"Source File: $last_file, Line: $last_line_num\n");
      my ($f, $d) = fileparse($last_file);
      $last_dir = $d;
      DEBUG(4,"Basename: $last_dir\n");
      if(2 == $G_interleave_source){next;}
    }  
    elsif ($_ =~ /(^\..*\/.*\.[chS])(:)([0-9]*$)/)
    {
      if($l_new_file == 0){print SRCCOV "\n\n"};
      $last_file = $last_dir.$1;
      if ( !(-e $last_file)){$last_file = $ENV{"sb"}."/occ/".$1;}
      if ( !(-e $last_file)){$last_file = $ENV{"sb"}."/ssx/".$1;}
      if ( !(-e $last_file)){$last_file = $ENV{"sb"}."/ssx/ppc405/".$1;}
      $last_line_num = $3;
      $l_new_file = 1;
      DEBUG(1,"Source File: $last_file, Line: $last_line_num\n");
      if(2 == $G_interleave_source){next;}
    }  
    elsif ($_ =~ /(^[a-zA-Z]*\/.*\.[chS])(:)([0-9]*$)/)
    {
      if($l_new_file == 0){print SRCCOV "\n\n"};
      $last_file = $ENV{"sb"}."/occ/".$1;
      $last_line_num = $3;
      $l_new_file = 1;
      DEBUG(1,"Source File: $last_file, Line: $last_line_num\n");
      if(2 == $G_interleave_source){next;}
    } 

    # If line starts with fff, it is instruction that didn't get executed
    if($_ =~ /^fff/)
    {
      if($l_new_file)
      {
        $l_new_file = 0;
        open(READ,"<$last_file");
        my @src_lines = <READ>;

        print SRCCOV "-----------------------------------------------------------------------------\n";
        print SRCCOV "$last_file:$last_line_num\n";
        print SRCCOV "-----------------------------------------------------------------------------\n";
        
        print SRCCOV "\n";
        print SRCCOV "Source:\n";
        print SRCCOV "------------------------------\n";
        for(my $f=($G_num_src_lines_before*-1); $f<$G_num_src_lines_context; $f++)
        {
          if($last_line_num+$f > 0)
          {
            print SRCCOV "".($last_line_num+$f).":\t".@src_lines[($last_line_num+$f-1)];
          }
        }
        print SRCCOV "\n";
        print SRCCOV "Assembly:\n";
        print SRCCOV "------------------------------\n";
        
        close(READ);
      }
      print SRCCOV $_;
    }
    else
    {
      if(2 == $G_interleave_source)
      {
        if( !($_ =~ /^:::::/) )
        {
          print SRCCOV $_;
        }
      }
      else
      {
        print SRCCOV $_;
      }
    }

  
  }
  
  close(CCOV);
  close(SRCCOV);

  #------------------------------------------------
  # Open file for viewing if requested
  #------------------------------------------------
  if($G_viewer)
  {
    system("$G_viewer $G_source_dump_file.ccov_src.txt");
  }
}
$G_time_interleave_ended = time();





#------------------------------------------------
# Dump some Stats if in Debug
#------------------------------------------------
if($G_debug_lvl > 0)
{
  my $fs1 = -s "$G_instructions_file";
  my $fs2 = -s "$G_source_dump_file";
  my $fs3 = -s "$G_source_dump_file.ccov.txt";
  my $fs4 = -s "$G_source_dump_file.ccov_src.txt";
  
  my $script_time = time() - $G_time_started;
  my $mDiff = int($script_time / 60);
  my $sDiff = sprintf("%02d", $script_time - 60 * $mDiff);

  $script_time = $G_time_compare_ended - $G_time_compare_started;
  my $cmDiff = int($script_time / 60);
  my $csDiff = sprintf("%02d", $script_time - 60 * $cmDiff);

  $script_time = $G_time_simics_ended - $G_time_simics_started;
  my $smDiff = int($script_time / 60);
  my $ssDiff = sprintf("%02d", $script_time - 60 * $smDiff);

  $script_time = $G_time_interleave_ended - $G_time_interleave_started;
  my $imDiff = int($script_time / 60);
  my $isDiff = sprintf("%02d", $script_time - 60 * $imDiff);

  printf("Total Time     (mm:ss) = %02d\:%02d\n",$mDiff,$sDiff);
  printf("  - Simics     (mm:ss) = %02d\:%02d\n",$smDiff,$ssDiff);
  printf("  - Compare    (mm:ss) = %02d\:%02d\n",$cmDiff,$csDiff);
  printf("  - Interleave (mm:ss) = %02d\:%02d\n",$imDiff,$isDiff);
  print  "\n";
  print  "File Sizes:\n";
  printf("  - $G_instructions_file: %.2f kB\n",($fs1/1024));
  printf("  - $G_source_dump_file:              %.2f  kB\n",($fs2/1024));
  printf("  - $G_source_dump_file.ccov.txt:     %.2f  kB\n",($fs3/1024)); 
  printf("  - $G_source_dump_file.ccov_src.txt: %.2f  kB\n",($fs4/1024));
  print  "\n";
}





exit 0;
#------------------------------------------------
# End of Program
#------------------------------------------------









##############################################################################
##############################################################################
#
# Subroutines
#
##############################################################################
##############################################################################

#Simics command reference
# 

#
sub DEBUG
{
  my ($lvl,$str) = @_;
  if($lvl <= $G_debug_lvl)
  {
    print $str;
  }
}


##############################################################################
##############################################################################
#
# Help Dump
#
##############################################################################
##############################################################################
sub dump_help()
{
  my $name = "hocct.pl";
  
  print "\n";
  print "Usage:  $name [options]\n";
  print "\n";
  print "Version: $VERSION\n\n";
  
  print "Usage Notes:\n";
  print "  - Must be run from sandbox unless both [-s] and [-i] args are passed in\n";
  print "  - Output File(s) will be dumped to *.ccov.txt and or *.ccov_src.txt\n";
  print "  - Ccov file will contain \":::::\" at the start of lines that have been\n"; 
  print "    executed in the simics instruction trace that was used.\n";

  print "\n";

  print "Options:\n";
  print "\t[-i <file>]      Use <file> as instruction trace instead of generating one.\n";
  print "\t[-s <file>]      Use <file> as source dump file instead of generating one.\n";
  print "\t[-d <program>]   Use <program> as diff viewer, and view diff of source dump\n";
  print "\t                 and Code Coverage modified source dump.\n";
  print "\t[-w <program>]   Use <program as viewer, and view source-interleaved-code-coverage\n";
  print "\t                 output file.\n";
  print "\t[-v <lvl>]       Set verbose to debug level <lvl>\n";
  print "\t[-t <time(ms)>]  Run simics for <time(ms)> of OCC execution time to gather\n"; 
  print "\t                 instruction trace (can't be used with [-i]\n";
  print "\t[-e]             Create source-interleaved-code-coverage file *.ccov_src.txt\n";
  print "\t                 leaving all executed instructions from file\n";
  print "\t[-k]             Create source-interleaved-code-coverage file *.ccov_src.txt\n";
  print "\t                 but remove all executed instructions from file\n";
  print "\t[-a <num>]       Used with [-e] or [-k].  When doing source-interleave, print\n";
  print "\t                 <num> lines after 'unexecuted' source line as context.\n";
  print "\t[-b <num>]       Used with [-e] or [-k].  When doing source-interleave, print\n";
  print "\t                 <num> lines after 'unexecuted' source line as context.\n";
  print "\t[-u]             Create unique filenames for each run (appends date/time to filename\n";
  print "\t[-h]             Dump Help\n";
  print "\n";

  DEBUG(1,"Secret Options:\n");
  DEBUG(1,"\t[-j <string>]    Modifiy objdump arguments\n");
  DEBUG(1,"\n");
  
  print "Examples:\n";
  print "\t- Get code coverage automatically for 35ms of OCC time\n";
  print "\t     $name -t 35\n";
  print "\n";
  print "\t- Get code coverage w/ source-interleave automatically for 35ms of OCC time\n";
  print "\t  and print 3 lines of source before un-executed instruction and 5 lines after\n";
  print "\t  then view source-interleaved file in gvim when complete\n";
  print "\t     $name -t 35 -k -a 5 -b 3 -w /usr/bin/gvim\n";
  print "\n";
  print "\t- Get code coverage based on an already dumped instruction trace\n";
  print "\t     $name -i /tmp/instructions.txt\n";
  print "\n";
  print "\t- Get code coverage based on an already dumped instruction trace, and stop most\n";
  print "\t  STDOUT printing\n";
  print "\t     $name -i /tmp/instructions.txt -v \"-1\"\n";
  print "\n";
  print "\t- Get code coverage based on an already dumped instruction trace and source dump\n";
  print "\t     $name -i /tmp/instructions.txt -s /tmp/source_dump.txt\n";
  print "\n";
    
  print "Contact:\n";
  print "\tFor bugs, feature requests, etc., please contact:\n";
  print "\tPrimary Contact: $G_primary_contact\n";
  print "\tAll Contacts:    ";
  foreach(@G_contact_emails){print "$_, ";}print "\n";
  print "\n";

  
  #----------------------------------------
  # Only print this if we are in debug mode 
  #----------------------------------------
  if(1 <= $G_debug_lvl)
  {
print <<END;
------------------------------------------------------------------------------
Instructions for Use
------------------------------------------------------------------------------
To get the instruction trace needed for <instruction trace file>:
 - Start Simics
 - Load OCC
 - Set a breakpoint (if you don't do this, the file will be way too huge 
   before you can Ctrl+C)
 - Run in Simics:
     simics> load-module trace
     simics> new-tracer
     simics> trace0->trace_data = 0
     simics> trace0->trace_exceptions = 0
     simics> trace0.start occ_exe_trace
     simics> r
     running>...breakpoint hits
     simics> trace0.stop

 - Run in Sandbox (Bash Prompt):
     \\grep inst \$sb/../simics/occ_exe_trace | \\grep  -ho "p:0x........" | \\grep -ho "........\$" | sort | uniq > /tmp/occ_instructions.txt
     
     - This grabs all instructions run & physical mem addresses, sorts them, 
      and removes duplicate addresses)
     - This takes ~1.3seconds/250us worth of OCC data 
     

To get the source file needed for <source dump file>:
 - After compiling:
     ppcnf-mcp5-objdump -d -l \$sb/occ/occ.out > /tmp/occ_source_dump.txt
     -- or --
     The script will just take care of this for you.

Output File will be dumped to: /tmp/occ_source_dump.ccov.txt
 - It will contain ":::::" at the start of lines that have been 
   executed in the simics trace that was grabbed.

 - It is recommeded that if a user wants a graphical view of the 
   code coverage, they use the following command:
     gvimdiff /tmp/occ_source_dump.txt /tmp/occ_source_dump.ccov.txt

Notes:
 - To trace 8 real-world seconds of OCC Simics Execution, simics 
   creates a 250MB file.
   - This takes ~45 seconds to parse w/ grep/sort/uniq
 - To trace 250us in OCC time, Simics creates a 10MB file.
   - This takes ~1.5 seconds to parse w/ grep/sort/uniq  
 - It is very important that both the instruction trace and source file
   have all their addresses in numerical order.
     
END
}
}

