# @file - occ_simics_debug.py
# @brief Create Simics Generic Commands
# 
#   Flag    Def/Fea    Userid    Date        Description
#   ------- ---------- --------  ----------  ----------------------------------
#                      thallet   03/28/2012  Created


from time import sleep
from random import randint
import sys
import re

G_occ_list = []

##############################################################################
###########################################################
#   Functions
###########################################################
##############################################################################
def print_sym_as_hex( str ):
  tick = cli.quiet_run_command(G_occ_list[0] + ".cpu.psym " + '\"' + str + '\"')
  splitter = re.compile(r' (\d+)')
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

def print_sym_as_hex64( str ):
  tick = cli.quiet_run_command(G_occ_list[0] + ".cpu.psym " + '\"' + str + '\"')
  splitter = re.compile(r' (\d+)')
  p_sym = tick[1].rstrip('\n')
  r_sym = splitter.split(p_sym)
  out_str = ''
  for item in r_sym:
     if item.isdigit():
            out_str += "0x%016x" % int(item)
     else:
         out_str += `item`
  
  print out_str
  return


def print_simics_attr_as_hex64( str ):
  tick = cli.quiet_run_command(str)
  splitter = re.compile(r'(\d+)')
  p_sym = tick[0]
  print "0x%016x" % p_sym
  return


def getscom( int ):
  tick = cli.quiet_run_command(G_occ_list[0] + ".pib_master->address = " + str(int))
  tick = cli.quiet_run_command(G_occ_list[0] + ".pib_master->data")
  p_sym = tick[0]
  print "0x%016x" % p_sym
  return

def putscom( addr, data ):
  tick = cli.quiet_run_command(G_occ_list[0] + ".pib_master->address = " + str(addr))
  tick = cli.quiet_run_command(G_occ_list[0] + ".pib_master->data = " + str(data))
  getscom(addr)
  return

def generate_occlist():
    tick = cli.quiet_run_command("$occc[0]=0; $b=0; foreach $c in (get-object-list -all occ_simple) {$occc[$b] = $c; $b = $b + 1}")
    num = cli.quiet_run_command("print -d $b")
    for idx in range(int(num[1].rstrip('\n'))):
        tick = cli.quiet_run_command("echo $occc[" + str(idx) + "]")
        G_occ_list.insert(idx,tick[1].rstrip('\n'))
    G_occ_list.sort()
    for idx in range(int(num[1].rstrip('\n'))):
        print str(idx) + ": " + G_occ_list[idx]

    return


generate_occlist()

##############################################################################
###########################################################
# Register some New Simics Commands
###########################################################
##############################################################################


#----------------------------------------------------------
#
#
new_command("hsym", print_sym_as_hex,
  args = [arg(str_t, "symbol-name")],
  alias = "hsym",
  type = "occ-fw-module-commands",
  short = "Print a symbol as hex (uint64 or uint32) for all structure members",
  doc_items = [("NOTE", "This command is best")],
  see_also = ["h64sym"],
  doc = """
  <b>hsym</b> is best.
  This is its documentation. <i>arg</i>
  is the first argument...""")

#----------------------------------------------------------
#
#
new_command("h64sym", print_sym_as_hex64,
  args = [arg(str_t, "symbol-name")],
  alias = "hsym64",
  type = "occ-fw-module-commands",
  short = "Print a symbol as hex (uint64) for all structure members",
  doc_items = [("NOTE", "This command is best")],
  see_also = ["hsym"],
  doc = """
  <b>h64sym</b> is best.
  This is its documentation. <i>arg</i>
  is the first argument...""")

 
#----------------------------------------------------------
#
#
new_command("h64attr", print_simics_attr_as_hex64,
  args = [arg(str_t, "simics-attribute")],
  alias = "hattr",
  type = "occ-fw-module-commands",
  short = "Print a simics attribute as hex (uint64) for all structure members",
  doc_items = [("NOTE", "This command is best")],
  see_also = ["hsym","h64sym"],
  doc = """
  <b>h64attr</b> is best.
  This is its documentation. <i>arg</i>
  is the first argument...""")

#----------------------------------------------------------
#
#
new_command("getscom", getscom,
  args = [arg(int_t, "scomAddress")],
  alias = "gs",
  type = "occ-fw-module-commands",
  short = "Do a getscom",
  doc_items = [("NOTE", "This command is best")],
  see_also = ["hsym","h64sym"],
  doc = """
  <b>h64attr</b> is best.
  This is its documentation. <i>arg</i>
  is the first argument...""")


#----------------------------------------------------------
#
#
new_command("putscom", putscom,
  args = [arg(int_t, "scomAddress"),arg(int_t, "data")],
  alias = "ps",
  type = "occ-fw-module-commands",
  short = "Do a getscom",
  doc_items = [("NOTE", "This command is best")],
  see_also = ["hsym","h64sym"],
  doc = """
  <b>h64attr</b> is best.
  This is its documentation. <i>arg</i>
  is the first argument...""")

