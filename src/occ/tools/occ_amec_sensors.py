# @file - occ_amec_sensors.py
# @brief Create Simics Commands for Reading AMEC Sensors
# 
#   Flag    Def/Fea    Userid    Date        Description
#   ------- ---------- --------  ----------  ----------------------------------
#                      thallet   02/28/2012  Created
#   $fk002  905632     fmkassem  11/18/2013  Removed cpm support.


from time import sleep
from random import randint
import sys
import re

##############################################################################
###########################################################
#   Globals
###########################################################
##############################################################################

# To create this listing, run the following command:
#\grep g_amec_sys sensor_table.c | \grep " SENSOR_PTR" | sed 's/),.*$/",/g' | sed 's/^.*&/"/g'
amec_sensors = [
"g_amec_sys.fw.ameintdur",
"g_amec_sys.fw.amessdur[0]",
"g_amec_sys.fw.amessdur[1]",
"g_amec_sys.fw.amessdur[2]",
"g_amec_sys.fw.amessdur[3]",
"g_amec_sys.fw.amessdur[4]",
"g_amec_sys.fw.amessdur[5]",
"g_amec_sys.fw.amessdur[6]",
"g_amec_sys.fw.amessdur[7]",
"g_amec_sys.fw.probe250us[0]",
"g_amec_sys.fw.probe250us[1]",
"g_amec_sys.fw.probe250us[2]",
"g_amec_sys.fw.probe250us[3]",
"g_amec_sys.fw.probe250us[4]",
"g_amec_sys.fw.probe250us[5]",
"g_amec_sys.fw.probe250us[6]",
"g_amec_sys.fw.probe250us[7]",
"g_amec_sys.fw.gpetickdur[0]",
"g_amec_sys.fw.gpetickdur[1]",
"g_amec_sys.fw.prcdupdatedur",
"g_amec_sys.sys.tempambient",
"g_amec_sys.sys.altitude",
"g_amec_sys.sys.pwr250us",
"g_amec_sys.fan.pwr250usfan",
"g_amec_sys.io.pwr250usio",
"g_amec_sys.storage.pwr250usstore",
"g_amec_sys.fan.fanspeedavg",
"g_amec_sys.sys.todclock0 ",
"g_amec_sys.sys.todclock1 ",
"g_amec_sys.sys.todclock2 ",
"g_amec_sys.proc[0].freqa250us",
"g_amec_sys.proc[0].ips2ms",
"g_amec_sys.proc[0].memsp2ms",
"g_amec_sys.proc[0].pwr250us",
"g_amec_sys.proc[0].pwr250usvdd",
"g_amec_sys.proc[0].cur250usvdd",
"g_amec_sys.proc[0].pwr250usvcs",
"g_amec_sys.proc[0].pwr250usmem",
"g_amec_sys.proc[0].sleepcnt2ms",
"g_amec_sys.proc[0].winkcnt2ms",
"g_amec_sys.proc[0].sp250us",
"g_amec_sys.proc[0].temp2ms",
"g_amec_sys.proc[0].temp2mspeak",
"g_amec_sys.proc[0].util2ms",
"g_amec_sys.sys.vrfan250usproc",
"g_amec_sys.sys.vrhot250usproc",
"g_amec_sys.sys.vrfan250usmem",
"g_amec_sys.sys.vrhot250usmem",
"g_amec_sys.proc[0].temp2mscent",
"g_amec_sys.proc[0].temp2msdimm",
"g_amec_sys.proc[0].memsp2ms_tls",
"g_amec_sys.proc[0].vrm[0].uvolt250us",
"g_amec_sys.proc[0].vrm[1].uvolt250us",
"g_amec_sys.proc[0].vrm[0].volt250us",
"g_amec_sys.proc[0].vrm[1].volt250us",
]

#\grep g_amec_sys sensor_table.c | \grep " CORE_SENSOR_PTR" | sed 's/),.*$/",/g' | sed 's/^.*&/"/g' | sed 's/core, /core[0]./g'
amec_core_sensors = [
"g_amec_sys.proc[0].core[0].freq250us",
"g_amec_sys.proc[0].core[0].freqa250us",
"g_amec_sys.proc[0].core[0].ips2ms",
"g_amec_sys.proc[0].core[0].mcpifd2ms",
"g_amec_sys.proc[0].core[0].mcpifi2ms",
"g_amec_sys.proc[0].core[0].spurr2ms",
"g_amec_sys.proc[0].core[0].temp2ms",
"g_amec_sys.proc[0].core[0].util2ms",
"g_amec_sys.proc[0].core[0].mstl2ms",
"g_amec_sys.proc[0].core[0].cmt2ms",
"g_amec_sys.proc[0].core[0].cmrd2ms",
"g_amec_sys.proc[0].core[0].cmwr2ms",
"g_amec_sys.proc[0].core[0].ppic",
"g_amec_sys.proc[0].core[0].pwrpx250us",
]

#\grep g_amec_sys sensor_table.c | \grep " PORTPAIR_SENSOR_PTR" | sed 's/),.*$/",/g' | sed 's/^.*&/"/g' | sed 's/memctl, /memctl[0]./g' | sed 's/portpair, /portpair[0]./g'
amec_portpair_sensors = [
"g_amec_sys.proc[0].memctl[0].centaur.portpair[0].mac2ms",
"g_amec_sys.proc[0].memctl[0].centaur.portpair[0].mpu2ms",
"g_amec_sys.proc[0].memctl[0].centaur.portpair[0].mirb2ms",
"g_amec_sys.proc[0].memctl[0].centaur.portpair[0].mirl2ms",
"g_amec_sys.proc[0].memctl[0].centaur.portpair[0].mirm2ms",
"g_amec_sys.proc[0].memctl[0].centaur.portpair[0].mirh2ms",
"g_amec_sys.proc[0].memctl[0].centaur.portpair[0].mts2ms",
"g_amec_sys.proc[0].memctl[0].centaur.portpair[0].memsp2ms", 
]

##############################################################################
###########################################################
#   Functions
###########################################################
##############################################################################
def print_symbol( str ):
  tick = cli.quiet_run_command(G_occ_list[0] + ".cpu.psym " + '\"' + str + '\"')
  print str + ": ", tick[1].rstrip('\n')
  return

def print_all_as_hex( str ):
  tick = cli.quiet_run_command(G_occ_list[0] + ".cpu.psym " + '\"' + str + '\"')
  splitter = re.compile(r'(\d+)')
  p_sym = tick[1].rstrip('\n')
  r_sym = splitter.split(p_sym)
  out_str = ''
  for item in r_sym:
     if item.isdigit():
         if (int(item) > 0xffffffff):
            out_str += "0x%016x" % int(item)
         else:
            out_str += "0x%08x" % int(item)
     else:
         out_str += `item`
  
  print out_str
  return

def print_symbol_value_hex( str ):
  tick = cli.quiet_run_command(G_occ_list[0] + ".cpu.psym " + '\"' + str + '\"')
  print str + ": \t", "0x%04x" % int(tick[1].rstrip('\n'))
  return

def print_symbol_value_hex_apss( str ):
  tick = cli.quiet_run_command(G_occ_list[0] + ".cpu.psym " + '\"' + str + '\"')
  value = int(tick[1].rstrip('\n'))
  print str + ": \t", "0x%04x (raw=0x%04x), %d" % ((value&0x0fff), (value),(value&0x0fff))
  return

def print_sensor_quick( str ):
  sample = ".sample"
  max    = ".sample_max"
  min    = ".sample_min"
  mnsnsr = ".mini_sensor"
  update = ".update_tag"

  s_sample = cli.quiet_run_command(G_occ_list[0] + ".cpu.psym " + '\"' + str + sample + '\"')
  s_max = cli.quiet_run_command(G_occ_list[0] + ".cpu.psym " + '\"' + str + max + '\"')
  s_min = cli.quiet_run_command(G_occ_list[0] + ".cpu.psym " + '\"' + str + min + '\"')
  s_upd = cli.quiet_run_command(G_occ_list[0] + ".cpu.psym " + '\"' + str + update + '\"')
  s_mns = cli.quiet_run_command(G_occ_list[0] + ".cpu.psym " + '\"*' + str + mnsnsr + '\"')
  
  p_str = str.replace("g_amec_sys.","")
  p_sample = s_sample[1].rstrip('\n')
  p_max = s_max[1].rstrip('\n')
  p_min = s_min[1].rstrip('\n')
  p_upd = s_upd[1].rstrip('\n')
  p_mns = s_mns[1].rstrip('\n')

  print p_str.ljust(45) + ":\t", p_sample.rjust(6) + ", " \
      + p_max.rjust(6) + ", " \
      + p_min.rjust(6) + ", " \
      + p_mns.rjust(6) + ", " \
      + p_upd.rjust(10) 
  return

def print_core_sensor_quick( ):
  for sensor in amec_core_sensors:
    print ""
    for x in range(0,12):
      name = sensor.replace("core[0]","core["+str(x)+"]")
      print_sensor_quick(name)

def print_sensor_quick_string( strin ):
  print "Name".ljust(45) + " \t", "Sample".rjust(6) + ", " \
      + "Max".rjust(6) + ", " \
      + "Min".rjust(6) + ", " \
      + "MnSnsr".rjust(6) + ", " \
      + "UpdateTag".rjust(10) 

  for sensor in amec_core_sensors:
    for x in range(0,12):
      name = sensor.replace("core[0]","core["+str(x)+"]")
      if re.search( strin, name ):
        print_sensor_quick(name)
  for sensor in amec_sensors:
    if re.search( strin, sensor ):
      print_sensor_quick(sensor)
  for sensor in amec_portpair_sensors:
    for x in range(0,8):
      t_name = sensor.replace("memctl[0]","memctl["+str(x)+"]")
      for x in range(0,2):
         name = t_name.replace("portpair[0]","portpair["+str(x)+"]")
         if re.search( strin, name ):
           print_sensor_quick(name)


def print_array_quick( strin, idx ):
  for x in range(0,idx):
      name = strin.replace("[0]","["+str(x)+"]")
      s_sample = cli.quiet_run_command(G_occ_list[0] + ".cpu.psym " + '\"' + name + '\"')
      p_sample = s_sample[1].rstrip('\n')
      print name + ":\t" + p_sample

def set_value( strin, value ):
      s_sample = cli.quiet_run_command(G_occ_list[0] + ".cpu.psym " + '\"' + strin + '\"')
      p_sample = s_sample[1].rstrip('\n')
      print "Old: " + strin + ":\t" + p_sample

      s_sample = cli.quiet_run_command( G_occ_list[0] + ".oci_space.set (" + G_occ_list[0] + ".cpu.sym " + '\"&' + strin + '\") ' + str(value) + " (sym \"sizeof(" + strin + ")\")")

      s_sample = cli.quiet_run_command(G_occ_list[0] + ".cpu.psym " + '\"' + strin + '\"')
      p_sample = s_sample[1].rstrip('\n')
      print "New: " + strin + ":\t" + p_sample


def print_sensors_all( ):
  print "Name".ljust(45) + " \t", "Sample".rjust(6) + ", " \
      + "Max".rjust(6) + ", " \
      + "Min".rjust(6) + ", " \
      + "MnSnsr".rjust(6) + ", " \
      + "UpdateTag".rjust(10) 
  for sensor in amec_sensors:
    print_sensor_quick(sensor)

  print_core_sensor_quick()

  for sensor in amec_portpair_sensors:
    for x in range(0,8):
      t_name = sensor.replace("memctl[0]","memctl["+str(x)+"]")
      for x in range(0,2):
         name = t_name.replace("portpair[0]","portpair["+str(x)+"]")
         print_sensor_quick(name)
    print ""

  print __file__



def print_apss_pwr( ):
    strin = "G_apss_pwr_meas->adc[0]"
    for x in range(0,16):
      name = strin.replace("[0]","["+str(x)+"]")
      print_symbol_value_hex_apss(name)
    print ""
    print_symbol_value_hex("G_apss_pwr_meas->gpio[0]")
    print_symbol_value_hex("G_apss_pwr_meas->gpio[1]")
    print ""
    print_symbol_value_hex("G_apss_pwr_meas->tod")


##############################################################################
###########################################################
# Register some New Simics Commands
###########################################################
##############################################################################

#----------------------------------------------------------
#
#
new_command("occ-print-sensor", print_sensor_quick_string,
  args = [arg(str_t, "name")],
  alias = "ps",
  type = "occ-fw-module-commands",
  short = "Print a sensor",
  doc_items = [("NOTE", "This command is best")],
  see_also = ["my_other_command"],
  doc = """
  <b>my-command</b> is best.
  This is its documentation. <i>arg</i>
  is the first argument...""")

#----------------------------------------------------------
#
#
new_command("occ-print-sensors-all", print_sensors_all,
  args = [],
  alias = "psa",
  type = "occ-fw-module-commands",
  short = "Print all sensors",
  doc_items = [("NOTE", "This command is best")],
  see_also = ["my_other_command"],
  doc = """
  <b>my-command</b> is best.
  This is its documentation. <i>arg</i>
  is the first argument...""")

#----------------------------------------------------------
#
#
new_command("occ-print-array", print_array_quick,
  args = [arg(str_t, "name"),arg(int_t, "arg", "?", 10),],
  alias = "pa",
  type = "occ-fw-module-commands",
  short = "Print an array",
  doc_items = [("NOTE", "This command is best")],
  see_also = ["my_other_command"],
  doc = """
  <b>my-command</b> is best.
  This is its documentation. <i>arg</i>
  is the first argument...""")

#----------------------------------------------------------
#
#
new_command("occ-set-value", set_value,
  args = [arg(str_t, "name"),arg(int_t, "arg", "?", 10),],
  alias = "sv",
  type = "occ-fw-module-commands",
  short = "Set a variable to a value",
  doc_items = [("NOTE", "This command is best")],
  see_also = ["my_other_command"],
  doc = """
  <b>my-command</b> is best.
  This is its documentation. <i>arg</i>
  is the first argument...""")

#----------------------------------------------------------
#
#
new_command("occ-dump-raw-power", print_apss_pwr,
  args = [],
  alias = "drp",
  type = "occ-fw-module-commands",
  short = "OCC Dump Raw Power from APSS",
  doc_items = [("NOTE", "This command is best")],
  see_also = ["my_other_command"],
  doc = """
  <b>my-command</b> is best.
  This is its documentation. <i>arg</i>
  is the first argument...""")

# this is my Python function that will be called when the
# command is invoked from the Simics prompt.
#def my_command_fun(int_arg, str_arg, flag_arg):
#  print "My integer: %d" % int_arg
#  print "My string: %s" % str_arg
#  print "Flag is",
#  if flag_arg:
#    print "given"
#  else:
#    print "not given"
#  return int_arg
## register our new command
#new_command("my-command", my_command_fun,
#  args = [arg(int_t, "arg", "?", 10), arg(str_t, "name"),
#  arg(flag_t, "-f")],
#  alias = "mc",
#  type = "my-module-commands",
#  short = "my command does it",
#  doc_items = [("NOTE", "This command is best")],
#  see_also = ["my_other_command"],
#  doc = """
#  <b>my-command</b> is best.
#  This is its documentation. <i>arg</i>
#  is the first argument...""")

