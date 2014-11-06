# @file - occ_gdb.py
# @brief Start GDB on OCC
# 
#   Flag    Def/Fea    Userid    Date        Description
#   ------- ---------- --------  ----------  ----------------------------------
#                      thallet   08/14/2012  Created


from time import sleep
from random import randint
import sys
import re

G_gdb_path             = "/gsa/ausgsa/home/t/h/thallet/web/shared/gdb_ppc_eabi/usr/local/bin/powerpc-\*-eabi-gdb";
G_ddd_path             = "ddd --debugger " + G_gdb_path;
G_gdb_port = 0
G_gdb_hostname = ""

##############################################################################
###########################################################
#   Functions
###########################################################
##############################################################################

def start_gdb_server( occ_idx ):
  global G_gdb_port
  global G_gdb_hostname

  # TODO:  Add a check here for not restarting gdb if already started
  tick = cli.quiet_run_command("object-exists occgdb" + str(occ_idx))
  exists = tick[0]
  if exists:
      print "Object occgdb" + str(occ_idx) + "already exists"
  else: 
      tick = cli.run_command("new-gdb-remote cpu = " + G_occ_list[int(occ_idx)] + ".cpu " + "port=0 name=occgdb" + str(occ_idx))
  
  tick = cli.run_command("occgdb" + str(occ_idx) + "->listen")
  port = tick
  tick = cli.quiet_run_command("!hostname")
  hostname = tick[0]

  print "Port:     %d" % port
  print "Hostname: %s" % hostname

  gdb_start_commands = [
    "echo \"Automating GDB Connection to OCC in Simics: " + hostname +": " + str(port) + "\n",
    "set architecture powerpc:common\n",
    "symbol-file " + os.getenv("sb") + "/../obj/ppc/occc/405/occ/occ.out\n",
    "target remote " + hostname + ":" + str(port) + "\n",
    "handle SIGTRAP nostop noprint\n",
  ]
  fout = open("occgdb_" +  str(occ_idx) + "_gdb_cmds.txt","w")
  for cmd in gdb_start_commands:
      fout.write(cmd)
  fout.close()

  G_gdb_port = port
  G_gdb_hostname = hostname

  return


def ddd_connect( occ_idx ):
  os.system(G_ddd_path + " --command=occgdb_" +  str(occ_idx) + "_gdb_cmds.txt &")
  return

def gdb_connect( occ_idx ):
  if occ_idx == 0:
     os.system("xterm -bg darkolivegreen -fg wheat -sb -sl 10000 -geometry 120x40 -hold -title \"GDB -> OCC["+ str(occ_idx) +"] [" + os.getenv("SANDBOX") + "] @ " + G_gdb_hostname + ":" + str(G_gdb_port) + "\" -e " + G_gdb_path + " --command=occgdb_" +  str(occ_idx) + "_gdb_cmds.txt &")
  else:
     os.system("xterm -bg honeydew4 -fg wheat -sb -sl 10000 -geometry 120x40 -hold -title \"GDB -> OCC["+ str(occ_idx) +"] [" + os.getenv("SANDBOX") + "] @ " + G_gdb_hostname + ":" + str(G_gdb_port) + "\" -e " + G_gdb_path + " --command=occgdb_" +  str(occ_idx) + "_gdb_cmds.txt &")

  return

def occ_gdb_start ( occ_idx ):
  start_gdb_server(int(occ_idx))
  gdb_connect(int(occ_idx))
  return 

def occ_ddd_start ( occ_idx ):
  start_gdb_server(int(occ_idx))
  ddd_connect(int(occ_idx))
  return 


##############################################################################
###########################################################
# Register some New Simics Commands
###########################################################
##############################################################################

#----------------------------------------------------------
# Simics Command:  occ-gdb-start
#
new_command("occ-gdb-start", occ_gdb_start,
  args = [arg(int_t, "occidx")],
  alias = "",
  type = "occ-fw-module-commands",
  short = "Connect GDB to OCC",
  doc_items = [("NOTE", "")],
  see_also = ["occ-ddd-start"],
  doc = """
""")

#----------------------------------------------------------
# Simics Command:  occ-ddd-start
#
new_command("occ-ddd-start", occ_ddd_start,
  args = [arg(int_t, "occidx","?")],
  alias = "",
  type = "occ-fw-module-commands",
  short = "Connect DDD to OCC",
  doc_items = [("NOTE", "")],
  see_also = ["occ-ddd-start"],
  doc = """
""")
