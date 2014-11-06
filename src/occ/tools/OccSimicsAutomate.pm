##############################################################################
#
# @file    OccSimicsAutomate.pm
#
# @author  Timothy Hallett
# 
# @package OccSimicsAutomate
#
# @brief   Perl class for automating simics for OCC.  
#
# @verbatim
#
# Updates: 11/28/2011 -- thallet -- File/Class created
#          11/29/2011 -- thallet -- Methods for automating tracing and simics
#                                   added and tested.  Integration into Code
#                                   Coverage Tool (HOCCT) works as well.
#  v0.2    01/17/2012 -- thallet -- Made change to handle post_model_hook.simics
#  v0.2.1  01/19/2012 -- thallet -- Clear accum on Expect calls.  Changed simics
#                                   prompt string.  
#
# @endverbatim
# 
#
##############################################################################

package OccSimicsAutomate;

use strict;
use Expect;



##############################################################################
#-----------------------------------------------------------------------------
#
# Package Config/Default Parms
#
#-----------------------------------------------------------------------------
##############################################################################

# @brief VERSION - contains VERSION of this class
my $VERSION                  = "0.2.1";

# @brief DEBUG_LEVEL - sets debug level of this class's print output
my $DEBUG_LEVEL              = 1;

# @brief CMD_PROMPT - sets what Expect should see for Linux bash prompt
my $CMD_PROMPT               = '^.*[$%>#]\s';  

# @brief CMD_PROMPT - sets what Expect should see for Simics prompt
my $SIMICS_PROMPT            = "simics> \x1b.*m";  

# @brief EXPECT_COMMAND - sets what Expect should run to get a bash prompt
my $EXPECT_COMMAND           = "bash -i";

# @brief DEFAULT_TRACE_FILE - sets default filename for simics execution trace
my $DEFAULT_TRACE_FILE       = "occ_exe_trace";

# @brief DEFAULT_INSTRUCTION_FILE - sets default filename for parsed out instruction trace
my $DEFAULT_INSTRUCTION_FILE = "/tmp/occ_instructions.txt";

# @brief SIMICS_CYCLES_PER_MS - sets timing for how many simics cycles are in 1ms.
my $SIMICS_CYCLES_PER_MS     = (150016 * 4);


##############################################################################
#-----------------------------------------------------------------------------
#
# Package Global Variables
#
#-----------------------------------------------------------------------------
##############################################################################

# @var    G_occ_start_cmds 
# 
# @brief  Holds commands that need to run to initialize simics for OCC execution
#
my @G_occ_start_cmds = 
(
  "enable-magic-breakpoint",
  "new-symtable st_auto ../src/occ/occ.out",
  "\@conf.OccComplex.OccSimpleSlot.cpu.current_context.symtable = conf.st_auto",
);

# @var    G_trace_start_cmds
#
# @brief  Holds commands that need to run to initialize simics for tracing instructions
#
my @G_trace_start_cmds =
(
  "load-module trace",
  "new-tracer",
  "trace0->trace_data = 0",
  "trace0->trace_exceptions = 0",
  "trace0->print_virtual_address = 0",
  "trace0->print_memory_type = 0",
  "trace0->print_linear_address = 0",
  "trace0->print_data = 0",
);


##############################################################################
#-----------------------------------------------------------------------------
#
# Constructor
#
#-----------------------------------------------------------------------------
##############################################################################

#------------------------------------------------
# @fn     new
# 
# @brief  Constructor, spawns bash shell with expect.
#   
# @param  None  
#
# @return self - Pointer to $self as class pointer
#------------------------------------------------
sub new
{
    my $class = shift;
    my $self =  
    {
      debug_lvl  => $DEBUG_LEVEL,
      prompt     => $CMD_PROMPT,
      shell_cmd  => $EXPECT_COMMAND,
      cyc_per_ms => $SIMICS_CYCLES_PER_MS,
      trace_file => $DEFAULT_TRACE_FILE,
      instr_file => $DEFAULT_INSTRUCTION_FILE,
      expect_obj => undef,
      last_rc    => undef,
      temp_file1 => undef,
      temp_file2 => undef,
    };

    # Spawn a bash shell to run in
    my @params;
    $self->{expect_obj} = Expect->spawn($self->{shell_cmd}, @params) or die "Cannot spawn $self->{shell_cmd}: $!\n";
    #$self->{expect_obj}->log_file("filename", "w");  #Later, add option to log all expect activity?
    #$self->{expect_obj}->log_file("filename");  #Later, add option to log all expect activity?
    $self->{last_rc} = $self->{expect_obj}->expect(10,'-re',$self->{prompt});
    
    #If call fails, print out error
    if(0 == $self->{last_rc})
    {
      print "Could not get bash prompt!"; die;
    }

    # For now, lets die if we aren't in a sandbox
    if( !defined($ENV{"sb"}) ){die "This script needs to run in an ODE sandbox!\n";}

    
    bless ($self, $class);
    return $self;
}

##############################################################################
#-----------------------------------------------------------------------------
#
# Destructor
#
#-----------------------------------------------------------------------------
##############################################################################

#------------------------------------------------
# @fn     DESTROY
# 
# @brief  Cleans up class, closes simics & expect.
#   
# @param  None  
#
# @return None
#------------------------------------------------
sub DESTROY
{
  my ( $self ) = @_;
  
  simics_cmd($self,"exit");
  
  $self->{expect_obj}->soft_close();
}


##############################################################################
#-----------------------------------------------------------------------------
#
# Base Methods
#
#-----------------------------------------------------------------------------
##############################################################################

#------------------------------------------------
# @fn     prereq_simics
# 
# @brief  Makes sure that prereqs are in place to starts the simics session
#   
# @param  None  
#
# @return rc - Return code of expect operation
#------------------------------------------------
sub prereq_simics
{
  my ( $self ) = @_;

  # Make sure we have the simics command script setup to run
  #if( !(-e $ENV{"sb"}."/../post_model_hook.simics") )
  {
    if( -e $ENV{"sb"}."/occ/tools/occ_post_model_hook.simics" )
    {
      print "Setting up Symlinks...\n";
      system("rm ".$ENV{"sb"}."/../simics/post_model_hook.simics");
      system("ln -s ".$ENV{"sb"}."/occ/tools/occ_post_model_hook.simics ".$ENV{"sb"}."/../simics/post_model_hook.simics");
    } 
  }
}

#------------------------------------------------
# @fn     start_simics
# 
# @brief  Starts the simics session via expect
#   
# @param  None  
#
# @return rc - Return code of expect operation
#------------------------------------------------
sub start_simics
{
  my ( $self ) = @_;

  prereq_simics($self);
  
  $self->{expect_obj}->clear_accum();
  $self->{expect_obj}->send("start_simics -m occ_alone -st -batch_mode\n");
  $self->{last_rc} = $self->{expect_obj}->expect(300,'-re',$SIMICS_PROMPT);
  
  #If call fails, print out error
  if(0 == $self->{last_rc})
  {
    print "Couldn't get simics prompt ".$self->{last_rc}." \n"; die;
  }

  return $self->{last_rc};
}

#------------------------------------------------
# @fn     close
# 
# @brief  Exits & Closes the simics session via expect
#   
# @param  None  
#
# @return None
#------------------------------------------------
sub close
{
  my ( $self ) = @_;

  simics_cmd_w_timeout($self,1,"exit");
  
  $self->{expect_obj}->soft_close();
}

#------------------------------------------------
# @fn     simics_cmd
# 
# @brief  Runs a simics command, watis for "simics>"
#   
# @param  cmd - String with command that will be sent  
#
# @return rc - Return code of expect operation
#------------------------------------------------
sub simics_cmd
{
  my ( $self, $cmd ) = @_;
  
  $self->{expect_obj}->clear_accum();
  $self->{expect_obj}->send("$cmd\n");
  $self->{last_rc} = $self->{expect_obj}->expect(10,'-re',$SIMICS_PROMPT) or return;
  
  #If a person wanted to parse out the command, you would use this
  #expect call here:
  # @lines = $self->{expect_obj}->before();
  # foreach(@lines){do something to $_};
  
  return $self->{last_rc};
}


#------------------------------------------------
# @fn     simics_cmd_w_response
# 
# @brief  Runs a simics command, watis for "simics>"
#   
# @param  cmd - String with command that will be sent  
#
# @return rc - Return code of expect operation
#------------------------------------------------
sub simics_cmd_w_response
{
  my ( $self, $cmd ) = @_;
  
  $self->{expect_obj}->clear_accum();
  $self->{expect_obj}->send("$cmd\n");
  $self->{last_rc} = $self->{expect_obj}->expect(10,'-re',$SIMICS_PROMPT) or return;
  
  #If a person wanted to parse out the command, you would use this
  #expect call here:
  my @lines = $self->{expect_obj}->before();
  # foreach(@lines){do something to $_};
  return @lines;
}

#------------------------------------------------
# @fn     get_last_simics_cmd_response
# 
# @brief  Returns what expect read before last prompt
#   
# @return All response from last command before prompt
#------------------------------------------------
sub get_last_simics_cmd_response
{
  my ( $self ) = @_;

  return $self->{expect_obj}->before();
}

#------------------------------------------------
# @fn     simics_cmd_w_timeout
# 
# @brief  Runs a simics command, watis for "simics>"
#   
# @param  t   - Timeout in seconds of expect operation
# @param  cmd - String with command that will be sent  
#
# @return rc - Return code of expect operation
#------------------------------------------------
sub simics_cmd_w_timeout
{
  my ($self, $t, $cmd) = @_;
  
  $self->{expect_obj}->clear_accum();
  $self->{expect_obj}->send("$cmd\n");
  $self->{last_rc} = $self->{expect_obj}->expect($t,'-re',$SIMICS_PROMPT) or return;
  
  return $self->{last_rc};
}

#------------------------------------------------
# @fn     simics_cmd_w_timeout_w_running
# 
# @brief  Runs a simics command, watis for "running>"
#   
# @param  t   - Timeout in seconds of expect operation
# @param  cmd - String with command that will be sent  
#
# @return rc - Return code of expect operation
#------------------------------------------------
sub simics_cmd_w_timeout_w_running
{
  my ($self, $t, $cmd) = @_;
  
  $self->{expect_obj}->clear_accum();
  $self->{expect_obj}->send("$cmd\n");
  $self->{last_rc} = $self->{expect_obj}->expect($t,'-re','running> ') or return;

  return $self->{last_rc};
}

#------------------------------------------------
# @fn     simics_cmd_w_running
# 
# @brief  Runs a simics command, watis for "running>"
#   
# @param  cmd - String with command that will be sent  
#
# @return rc - Return code of expect operation
#------------------------------------------------
sub simics_cmd_w_running
{
  my ( $self, $cmd ) = @_;
  
  $self->{expect_obj}->clear_accum();
  $self->{expect_obj}->send("$cmd\n");
  $self->{last_rc} = $self->{expect_obj}->expect(10,'-re','running> ') or return;
  
  return $self->{last_rc};
}

#------------------------------------------------
# @fn     DEBUG
# 
# @brief  Prints out debug data, based on debug_lvl
#   
# @param  lvl - Min Level of debug at which string will print
# @param  str - String that will be printed  
#
# @return None
#------------------------------------------------
sub DEBUG
{
  my ($self,$lvl,$str) = @_;
  if($lvl <= $self->{debug_lvl})
  {
    print $str;
  }
}

#------------------------------------------------
# @fn     DEBUG
# 
# @brief  Prints out debug data, based on debug_lvl
#   
# @param  lvl - Min Level of debug at which string will print
# @param  str - String that will be printed  
#
# @return None
#------------------------------------------------
sub SET_DEBUG_LVL
{
  my ($self,$lvl) = @_;
  
  $self->{debug_lvl} = $lvl;
  
  if($lvl <= 5)
  {
    print "Turning on Expect debugging\n\n";
    $self->{expect_obj}->debug(3);
    $self->{expect_obj}->exp_internal(1);
  }
}



#-----------------------------------------------------------------------------
# Base Utilities (generic, based off base methods)
#-----------------------------------------------------------------------------

#------------------------------------------------
# @fn     waitForSimicsPrompt
# 
# @brief  Helper function, returns when "simics>" is seen
#   
# @return None
#------------------------------------------------
sub waitForSimicsPrompt
{
  my ( $self ) = @_;
  
  simics_cmd($self,"echo");
}

#------------------------------------------------
# @fn     waitForRunningPrompt
# 
# @brief  Helper function, returns when "running>" is seen
#   
# @return None
#------------------------------------------------
sub waitForRunningPrompt
{
  my ( $self ) = @_;
  
  simics_cmd_w_timeout_w_running($self,10,"echo");
}

#------------------------------------------------
# @fn     getSimicsTime
# 
# @brief  Helper function
#   
# @return None
#------------------------------------------------
sub getSimicsTime
{
  my ( $self ) = @_;
  
  simics_cmd($self,"print-time");
  simics_cmd($self,"print-time -t");
}

#------------------------------------------------
# @fn     dumpOccStackTrace
# 
# @brief  Helper function
#   
# @return None
#------------------------------------------------
sub dumpOccStackTrace
{
  my ( $self ) = @_;
  
  simics_cmd($self,"stack-trace");
}

#------------------------------------------------
# @fn     interact
# 
# @brief  Swithes to interactive mode so that user can
#         interact directly with simics, when script wants
#         that to happen
#   
# @return rc - Return code of expect operation
#------------------------------------------------
sub interact
{
  my ( $self ) = @_;
  
  waitForSimicsPrompt($self);
  waitForSimicsPrompt($self);
  waitForSimicsPrompt($self);
  waitForSimicsPrompt($self);
  waitForSimicsPrompt($self);
  waitForSimicsPrompt($self);
  waitForSimicsPrompt($self);
  waitForSimicsPrompt($self);
  waitForSimicsPrompt($self);

  print "Switching to Interactive Mode:  Type \"END\" and hit \"<Return>\" twice to return to script\n\n";
  
  $self->{expect_obj}->interact(\*STDIN, "END");
  
  return $self->{last_rc};
}

#------------------------------------------------
# @fn     setOccVariable
# 
# @brief  Sets variable abstracting weird simics syntax
#   
# @param  var   - String with variable name
# @param  value - Value to write to variable
#
# @return None
#------------------------------------------------
sub setOccVariable
{
  my ( $self, $var, $value ) = @_;

  simics_cmd($self,"set (sym \"&$var\") $value (sym \"sizeof($var)\")");
}

#------------------------------------------------
# @fn     getOccVariable
# 
# @brief  Gets variable abstracting weird simics syntax
#   
# @param  var   - String with variable name
#
# @return None
#------------------------------------------------
sub getOccVariable
{
  my ( $self, $var) = @_;

  simics_cmd($self,"psym \"$var\"");
}



##############################################################################
#-----------------------------------------------------------------------------
#
# OCC Simics Execution
#
#-----------------------------------------------------------------------------
##############################################################################

#------------------------------------------------
# @fn     startAndRunOccUntilMain
# 
# @brief  Automates all steps needed to reach OCC
#         application code main(), then stops on
#         breakpoint there.
#   
# @return None
#------------------------------------------------
sub startAndRunOccUntilMain
{
  my ( $self ) = @_;
  
  start_simics($self);
  runOccSimicsInitCmds($self);
  runOccSimicsTraceInitCmds($self);
  setOccBreakpointOnFunction($self,"main");
  runOccToBreakpoint($self);
  deleteOccBreakpoints($self);
  getSimicsTime($self);
}

#------------------------------------------------
# @fn     startAndRunOccUntilSsxBoot
# 
# @brief  Automates all steps needed to reach SSX
#         entry point __ssx_boot(), then stops on
#         breakpoint there.
#   
# @return None
#------------------------------------------------
sub startAndRunOccUntilSsxBoot
{
  my ( $self ) = @_;
  
  start_simics($self);
  runOccSimicsInitCmds($self);
  runOccSimicsTraceInitCmds($self);
  setOccBreakpointOnFunction($self,"__ssx_boot");
  runOccToBreakpoint($self);
  deleteOccBreakpoints($self);
  getSimicsTime($self);
}


#------------------------------------------------
# @fn     runOccSimicsInitCmds
# 
# @brief  Runs batch of simics commands to init OCC 
#         Simics Environment
#    
# @return None
#------------------------------------------------
sub runOccSimicsInitCmds
{
  my ( $self ) = @_;

  foreach(@G_occ_start_cmds)
  {
    simics_cmd($self,$_);
  }
}

#------------------------------------------------
# @fn     setOccBreakpointOnFunction
# 
# @brief  Sets breakpoint
#   
# @param  func - String with name of function to break on
#
# @return None
#------------------------------------------------
sub setOccBreakpointOnFunction
{
  my ( $self, $func ) = @_;

  simics_cmd($self,"break (sym $func)");
}

#------------------------------------------------
# @fn     setOccBreakpointOnWriteToVariable
# 
# @brief  Sets breakpoint
#   
# @param  var - String with name of variable to break on
#               when it is written to
#
# @return None
#------------------------------------------------
sub setOccBreakpointOnWriteToVariable
{
  my ( $self, $var ) = @_;

  setOccBreakpointOnAddressWrite($self,"(sym \"&$var\")"); 
}

#------------------------------------------------
# @fn     setOccBreakpointOnCycles
# 
# @brief  Sets breakpoint
#   
# @param  num_cycles - Number of cycles until now that
#                      simics will run until it breaks.
#
# @return None
#------------------------------------------------
sub setOccBreakpointOnCycles
{
  my ( $self, $num_cycles ) = @_;

  simics_cmd($self,"OccComplex.OccSimpleSlot.cpu.cycle-break $num_cycles");
}

#------------------------------------------------
# @fn     setOccBreakpointOnSource
# 
# @brief  Sets breakpoint
#   
# @param  file - Filename that breakpoint is in
# @param  line - Line in filename that breakpoint is on
#
# @return None
#------------------------------------------------
sub setOccBreakpointOnSource
{
  my ( $self, $file, $line ) = @_;

  simics_cmd($self,"break -x (pos $file:$line)");
}

#------------------------------------------------
# @fn     setOccBreakpointOnAddress
# 
# @brief  Sets breakpoint
#   
# @param  hex_addr - "0x"-prefixed string with address
#                    to break on when accessed as read, 
#                    write, or executed (see $flags)
# @param  flags    - String; can be -r, -w, -x.  -x is default
#
# @return None
#------------------------------------------------
sub setOccBreakpointOnAddress
{
  # flags can be -r, -w, -x.  -x is default
  my ( $self, $hex_addr, $flags ) = @_;

  simics_cmd($self,"break $hex_addr $flags");
}

#------------------------------------------------
# @fn     setOccBreakpointOnAddressExecute
# 
# @brief  Sets breakpoint
#   
# @param  hex_addr - "0x"-prefixed string with address
#                    to break on when accessed as execute, 
#
# @return None
#------------------------------------------------
sub setOccBreakpointOnAddressExecute
{
  my ( $self, $hex_addr) = @_;

  setOccBreakpointOnAddress($self, $hex_addr, "-x");
}

#------------------------------------------------
# @fn     setOccBreakpointOnAddressWrite
# 
# @brief  Sets breakpoint
#   
# @param  hex_addr - "0x"-prefixed string with address
#                    to break on when accessed as write, 
#
# @return None
#------------------------------------------------
sub setOccBreakpointOnAddressWrite
{
  my ( $self, $hex_addr) = @_;

  setOccBreakpointOnAddress($self, $hex_addr, "-w");
}

#------------------------------------------------
# @fn     runOccToBreakpoint
# 
# @brief  Runs simics until breakpoint is set, or 
#         timeout (60 seconds) is reached.
#   
# @return None
#------------------------------------------------
sub runOccToBreakpoint
{
  my ( $self ) = @_;

  simics_cmd_w_timeout_w_running($self,60,"r");
  waitForSimicsPrompt($self);
}

#------------------------------------------------
# @fn     deleteOccBreakpoints
# 
# @brief  Deletes all breakpoints 
#   
# @return None
#------------------------------------------------
sub deleteOccBreakpoints
{
  my ( $self ) = @_;

  simics_cmd($self,"delete -all");
}


#------------------------------------------------
# @fn     runOccForTimeInterval
# 
# @brief  Runs OCC for time interval of x ms
#   
# @param  time_in_ms - Time in ms that simics should run
#                      before stopping.
#
# @return None
#------------------------------------------------
sub runOccForTimeInterval
{
  my ( $self, $time_in_ms ) = @_;

  # x = 150016 cycles ~= 250us; 
  # x * 40 = 10ms
   
  my $ms_per_loop  = 10;
  my $cyc_per_ms   = $self->{cyc_per_ms};
  my $cyc_per_loop = $cyc_per_ms * 10;  

  my $loops = int($time_in_ms / $ms_per_loop);
  my $cyc_in_last_loop = ($time_in_ms % $ms_per_loop) * $cyc_per_ms;
  
  for (my $f=0; $f<$loops; $f++)
  {
    setOccBreakpointOnCycles($self,$cyc_per_loop);
    runOccToBreakpoint($self);
  }

  setOccBreakpointOnCycles($self,$cyc_in_last_loop);
  runOccToBreakpoint($self);
}


##############################################################################
#-----------------------------------------------------------------------------
#
# OCC Simics Instruction Trace
#
#-----------------------------------------------------------------------------
##############################################################################

#------------------------------------------------
# @fn     runOccSimicsTraceInitCmds
# 
# @brief  Initializes Simics and filesyste to grab 
#         OCC instruction trace
#   
# @return None
#------------------------------------------------
sub runOccSimicsTraceInitCmds
{
  my ( $self ) = @_;

  $self->{temp_file1} = "/tmp/temp_a".time().".txt";
  $self->{temp_file2} = "/tmp/temp_b".time().".txt";
  
  if ( -e $self->{instr_file} )
  {
    system("rm ".$self->{instr_file}."");
  }
  system("touch ".$self->{instr_file}."");
    
  system("touch ".$self->{trace_file}."");
  system("cat /dev/null >| ".$self->{trace_file}."");

  
  foreach(@G_trace_start_cmds)
  {
    simics_cmd($self,$_);
  }
}

#------------------------------------------------
# @fn     startOccSimicsTrace
# 
# @brief  Starts the simics instruction trace 
#   
# @return None
#------------------------------------------------
sub startOccSimicsTrace
{
  my ( $self ) = @_;

  simics_cmd($self,"trace0.start ".$self->{trace_file}."");
  simics_cmd($self,"!touch ".$self->{trace_file}."");
}

#------------------------------------------------
# @fn     stopOccSimicsTrace
# 
# @brief  Stops the simics instruction trace and
#         verifies file exists before returning.
#   
# @return None
#------------------------------------------------
sub stopOccSimicsTrace
{
  my ( $self ) = @_;

  # Stop the trace
  simics_cmd_w_timeout($self,200,"trace0.stop");
  
  # Flush the filesystem (not sure this actually does anything useful)
  simics_cmd($self,"!sync");

  # Used to see an issue where the file wouldn't get updated in GSA between
  # the simics pool system and the sandbox pool system, so this is a check
  # making sure the file update occurs
  if( (-z $ENV{"sb"}."/../simics/".$self->{trace_file}."") ){print "..";sleep(5);}
  if( (-z $ENV{"sb"}."/../simics/".$self->{trace_file}."") ){print "..";sleep(5);}
  if( (-z $ENV{"sb"}."/../simics/".$self->{trace_file}."") ){print "..";sleep(5);}
  if( (-z $ENV{"sb"}."/../simics/".$self->{trace_file}."") ){print "..";sleep(5);}
  if( (-z $ENV{"sb"}."/../simics/".$self->{trace_file}."") ){print "..";sleep(5);}
  if( (-z $ENV{"sb"}."/../simics/".$self->{trace_file}."") ){print "..";sleep(5);}
  if( (-z $ENV{"sb"}."/../simics/".$self->{trace_file}."") ){system("sync"); DEBUG($self,1,"ERROR: Don't see trace file ".$ENV{"sb"}."/../simics/".$self->{trace_file}."");}
}


#------------------------------------------------
# @fn     parseOccSimicsTraceFile
# 
# @brief  Parses the simics instruction trace and
#         prepares files for another pass.
#   
# @return None
#------------------------------------------------
sub parseOccSimicsTraceFile
{
  my ( $self ) = @_;
  
  my $trace_file = $self->{trace_file};
  DEBUG($self,1,"Lines in Trace file: ");
  system("wc -l \$sb/../simics/$trace_file");

  DEBUG($self,1,"Get instructions\n");
  system("strings \$sb/../simics/$trace_file | head -n 2");

  # Simics sometimes dumps some garbage into the trace file, so need to pass it through
  # strings first.
  system("\\strings \$sb/../simics/$trace_file | \\grep inst | \\grep  -ho \"p:0x........\" | \\grep -ho \"........\$\" | sort | uniq > ".$self->{temp_file1}."");
  
  system("cat ".$self->{instr_file}." ".$self->{temp_file1}." | sort | uniq > ".$self->{temp_file2}."");
  system("mv ".$self->{temp_file2}." ".$self->{instr_file}."");

  DEBUG($self,1,"Emptying Trace files to save space\n");
  DEBUG($self,1,"Lines in Inst file: ");
  system("wc -l ".$self->{instr_file}."");
  system("LINES=`wc -l \$sb/../simics/$trace_file | cut -d\" \" -f1`; if [ \"\$LINES\" -lt 3 ]; then mv \$sb/../simics/$trace_file /tmp/failure__t_ccov_".int(rand(1000)).".txt; fi");
  system("LINES=`wc -l ".$self->{instr_file}." | cut -d\" \" -f1`; if [ \"\$LINES\" -eq 0 ]; then mv \$sb/../simics/$trace_file /tmp/failure_ccov_".int(rand(1000)).".txt; fi");
  #if( (-z $ENV{"sb"}."/../simics/".$self->{instr_file}."") ){system("mv $self->{instr_file} /tmp/failure_ccov.txt"); die "Instruction File Empty!!!\n";}
  
  system("echo \"\" > \$sb/../simics/$trace_file");
  system("echo \"\" > ".$self->{temp_file1}."");
}


#------------------------------------------------
# @fn     runOccWithInstructionTrace
# 
# @brief  Runs simics w/ instruction trace, parses instructions
#         out of trace file, and appends to overall instruction
#         trace.
#
# @param  time_in_ms - Time in ms that simics should
#                      execute OCC and grab the instruction
#                      trace. 
#   
# @return None
#------------------------------------------------
sub runOccWithInstructionTrace
{
  my ( $self, $time_in_ms ) = @_;

  # x = 150016 cycles ~= 250us; 
  # x * 40 = 10ms
   
  my $ms_per_loop  = 10;
  my $cyc_per_ms   = $self->{cyc_per_ms};
  my $cyc_per_loop = $cyc_per_ms * 10;  

  my $loops = int($time_in_ms / $ms_per_loop);
  my $cyc_in_last_loop = ($time_in_ms % $ms_per_loop) * $cyc_per_ms;
  
  #WORKAROUND:  First Loop (10 cycles for workaround)
  startOccSimicsTrace($self);
  setOccBreakpointOnCycles($self,10);
  runOccToBreakpoint($self);
  stopOccSimicsTrace($self);
  parseOccSimicsTraceFile($self);

  for (my $f=0; $f<$loops; $f++)
  {
    startOccSimicsTrace($self);
    setOccBreakpointOnCycles($self,$cyc_per_loop);
    runOccToBreakpoint($self);
    stopOccSimicsTrace($self);
    parseOccSimicsTraceFile($self);
  }

  #Last Loop
  startOccSimicsTrace($self);
  setOccBreakpointOnCycles($self,$cyc_in_last_loop);
  runOccToBreakpoint($self);
  stopOccSimicsTrace($self);
  parseOccSimicsTraceFile($self);
}


##############################################################################
#-----------------------------------------------------------------------------
#
# End of Package 
#
#-----------------------------------------------------------------------------
##############################################################################

1;

##############################################################################
#-----------------------------------------------------------------------------
#
# End of File 
#
#-----------------------------------------------------------------------------
##############################################################################

